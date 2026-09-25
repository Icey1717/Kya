#include "edSoundStreamService.h"
#include "edSoundDevice.h"

#include "log.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <XAudio2.h>
#include <objbase.h>
#endif

namespace Audio
{
namespace
{

using Clock = std::chrono::steady_clock;

// SOUND_GetStreamInfo reports the PS2 VAG cursor in compressed stream bytes.
// XAudio2 reports the cursor in decoded PCM sample frames instead.
constexpr std::uint64_t VagBytesPerBlock = 16;
constexpr std::uint64_t VagSamplesPerBlock = 28;

struct Stream
{
	StreamInfo info;
	Clock::time_point startedAt{};
	std::uint64_t positionAtStart = 0;
	std::vector<std::int16_t> samples;
	std::uint32_t sampleRate = 0;

#ifdef _WIN32
	IXAudio2SourceVoice* sourceVoice = nullptr;
#endif
};

std::unordered_map<std::uint32_t, Stream> streams;

#ifdef _WIN32
IXAudio2* audioEngine = nullptr;
IXAudio2MasteringVoice* masteringVoice = nullptr;
bool audioDeviceFailed = false;
bool audioOwnsCom = false;

void LogAudioError(const char* message, HRESULT result)
{
	Log::GetInstance().AddLog(LogLevel::Error, "Audio", "{} (HRESULT 0x{:08x})", message,
		static_cast<unsigned long>(result));
}

bool EnsureAudioDevice()
{
	if (audioEngine != nullptr)
		return true;
	if (audioDeviceFailed)
		return false;

	HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(result) && result != RPC_E_CHANGED_MODE) {
		LogAudioError("CoInitializeEx failed", result);
		audioDeviceFailed = true;
		return false;
	}
	// Balance both S_OK and S_FALSE; an existing STA belongs to its caller.
	audioOwnsCom = SUCCEEDED(result);
	result = XAudio2Create(&audioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result)) {
		LogAudioError("XAudio2Create failed", result);
		audioDeviceFailed = true;
		return false;
	}

	result = audioEngine->CreateMasteringVoice(&masteringVoice);
	if (FAILED(result)) {
		LogAudioError("CreateMasteringVoice failed", result);
		audioEngine->Release();
		audioEngine = nullptr;
		audioDeviceFailed = true;
		return false;
	}

	return true;
}

void DestroySourceVoice(Stream& stream)
{
	if (stream.sourceVoice == nullptr)
		return;

	stream.sourceVoice->Stop(0);
	stream.sourceVoice->FlushSourceBuffers();
	stream.sourceVoice->DestroyVoice();
	stream.sourceVoice = nullptr;
}

bool CreateSourceVoice(Stream& stream)
{
	if (stream.sourceVoice != nullptr)
		return true;
	if (!EnsureAudioDevice() || stream.samples.empty() || stream.sampleRate == 0)
		return false;

	WAVEFORMATEX format{};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = static_cast<WORD>(stream.info.channels);
	format.nSamplesPerSec = stream.sampleRate;
	format.wBitsPerSample = 16;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	const HRESULT result = audioEngine->CreateSourceVoice(&stream.sourceVoice, &format);
	if (FAILED(result)) {
		LogAudioError("CreateSourceVoice failed", result);
		stream.sourceVoice = nullptr;
		return false;
	}

	const HRESULT volumeResult = stream.sourceVoice->SetVolume(stream.info.volume);
	if (FAILED(volumeResult)) {
		LogAudioError("SourceVoice SetVolume failed", volumeResult);
		DestroySourceVoice(stream);
		return false;
	}
	return true;
}
#endif

struct PsxAdpcmState
{
	int previous = 0;
	int previousPrevious = 0;
};

bool DecodePsxAdpcm(const std::uint8_t* data, std::size_t size, PsxAdpcmState& state,
	std::vector<std::int16_t>& samples)
{
	constexpr std::size_t BlockSize = 16;
	constexpr std::size_t SamplesPerBlock = 28;
	constexpr int Filter0[] = {0, 60, 115, 98, 122};
	constexpr int Filter1[] = {0, 0, -52, -55, -60};

	if (data == nullptr || size < BlockSize || (size % BlockSize) != 0)
		return false;

	for (std::size_t blockOffset = 0; blockOffset < size; blockOffset += BlockSize) {
		const std::uint8_t* block = data + blockOffset;
		const int shift = block[0] & 0xf;
		const int filter = std::min<int>(block[0] >> 4, 4);
		if (shift > 12)
			return false;

		for (std::size_t sampleIndex = 0; sampleIndex < SamplesPerBlock; sampleIndex++) {
			const std::uint8_t packed = block[2 + sampleIndex / 2];
			int nibble = (sampleIndex & 1) == 0 ? packed & 0xf : packed >> 4;
			if (nibble >= 8)
				nibble -= 16;

			int sample = (nibble * 4096) >> shift;
			sample += (state.previous * Filter0[filter] + state.previousPrevious * Filter1[filter] + 32) >> 6;
			sample = std::clamp(sample, -32768, 32767);
			samples.push_back(static_cast<std::int16_t>(sample));
			state.previousPrevious = state.previous;
			state.previous = sample;
		}
	}

	return true;
}

std::uint32_t ReadBigEndian32(const std::uint8_t* data)
{
	return (static_cast<std::uint32_t>(data[0]) << 24) |
		(static_cast<std::uint32_t>(data[1]) << 16) |
		(static_cast<std::uint32_t>(data[2]) << 8) |
		static_cast<std::uint32_t>(data[3]);
}

bool StartsWithInsensitive(const std::string& value, const char* prefix)
{
	std::size_t index = 0;
	for (; prefix[index] != '\0'; index++) {
		if (index >= value.size())
			return false;
		const char left = value[index] >= 'a' && value[index] <= 'z' ? value[index] - ('a' - 'A') : value[index];
		const char right = prefix[index] >= 'a' && prefix[index] <= 'z' ? prefix[index] - ('a' - 'A') : prefix[index];
		if (left != right)
			return false;
	}
	return true;
}

std::string GetHostStreamPath(const std::string& path)
{
	std::string hostPath = path;
	if (StartsWithInsensitive(hostPath, "<CDVD>")) {
		hostPath.erase(0, 6);
		if (hostPath.size() >= 2 && hostPath[0] == '0' && hostPath[1] == ':')
			hostPath.erase(0, 2);
	}
	else {
		const std::size_t colon = hostPath.find(':');
		if (colon != std::string::npos && StartsWithInsensitive(hostPath.substr(0, colon), "cdrom0"))
			hostPath.erase(0, colon + 1);
	}

	while (!hostPath.empty() && (hostPath.front() == '\\' || hostPath.front() == '/'))
		hostPath.erase(hostPath.begin());
	if (hostPath.size() >= 2 && hostPath[hostPath.size() - 2] == ';' && hostPath.back() == '1')
		hostPath.erase(hostPath.size() - 2);
	return hostPath;
}

std::vector<std::string> GetStreamPathCandidates(const char* path)
{
	const std::string originalPath(path);
	const std::string hostPath = GetHostStreamPath(originalPath);
	std::vector<std::string> candidates;
	candidates.push_back(originalPath);
	if (hostPath != originalPath)
		candidates.push_back(hostPath);
	if (!hostPath.empty()) {
		candidates.push_back("bin/WIN/" + hostPath);
		candidates.push_back("assets/" + hostPath);
		if (hostPath.find('.') == std::string::npos) {
			candidates.push_back(hostPath + ".VAG");
			candidates.push_back("bin/WIN/" + hostPath + ".VAG");
			candidates.push_back("assets/" + hostPath + ".VAG");
		}
	}
	return candidates;
}

std::uint64_t CurrentPosition(Stream& stream)
{
#ifdef _WIN32
	if (stream.sourceVoice != nullptr && !stream.samples.empty()) {
		XAUDIO2_VOICE_STATE state{};
		stream.sourceVoice->GetState(&state, 0);
		if (state.BuffersQueued == 0 && state.SamplesPlayed >= stream.samples.size())
			stream.info.playing = false;
		return state.SamplesPlayed * VagBytesPerBlock / VagSamplesPerBlock;
	}
#endif

	if (!stream.info.playing || stream.info.sampleRate == 0)
		return stream.info.position;

	const auto elapsed = std::chrono::duration<float>(Clock::now() - stream.startedAt).count();
	const float unitsPerSecond = static_cast<float>(stream.info.sampleRate) * (2.0f / 3.5f);
	return stream.positionAtStart + static_cast<std::uint64_t>((std::max)(0.0f, elapsed) * unitsPerSecond);
}

} // namespace

#ifdef _WIN32
IXAudio2* GetAudioEngine() { return EnsureAudioDevice() ? audioEngine : nullptr; }
IXAudio2MasteringVoice* GetAudioMasteringVoice() { return masteringVoice; }
void ShutdownAudioDevice()
{
	if (masteringVoice) masteringVoice->DestroyVoice();
	if (audioEngine) audioEngine->Release();
	masteringVoice = nullptr;
	audioEngine = nullptr;
	audioDeviceFailed = false;
	if (audioOwnsCom) CoUninitialize();
	audioOwnsCom = false;
}
#endif

bool DecodeRawAdpcm(const std::uint8_t* data, std::size_t size, std::vector<std::int16_t>& samples)
{
	samples.clear();
	if (!data || !size || size % 16) return false;
	for (std::size_t offset = 0; offset < size; offset += 16)
		if ((data[offset] >> 4) > 4 || (data[offset] & 15) > 12) return false;
	PsxAdpcmState state;
	return DecodePsxAdpcm(data, size, state, samples);
}

bool DecodeVag(const std::uint8_t* data, std::size_t size, std::vector<std::int16_t>& samples,
	std::uint32_t& sampleRate)
{
	constexpr std::size_t HeaderSize = 0x30;

	samples.clear();
	sampleRate = 0;
	if (data == nullptr || size < HeaderSize || data[0] != 'V' || data[1] != 'A' || data[2] != 'G' || data[3] != 'p')
		return false;

	const std::uint32_t dataSize = ReadBigEndian32(data + 0xc);
	sampleRate = ReadBigEndian32(data + 0x10);
	if (sampleRate == 0 || dataSize == 0 || size - HeaderSize < dataSize)
		return false;

	const std::size_t blockCount = dataSize / 16;
	samples.reserve(blockCount * 28);
	PsxAdpcmState state;
	if (!DecodePsxAdpcm(data + HeaderSize, blockCount * 16, state, samples))
		return false;

	return !samples.empty();
}

bool DecodeMib(const std::uint8_t* data, std::size_t size, std::uint32_t channels,
	std::uint32_t interleaveBlockSize, std::uint32_t sampleRate, std::vector<std::int16_t>& samples)
{
	samples.clear();
	if (data == nullptr || size == 0 || sampleRate == 0 || channels == 0 || channels > 2 ||
		interleaveBlockSize < 16 || (interleaveBlockSize % 16) != 0)
		return false;

	const std::size_t interleaveFrameSize = static_cast<std::size_t>(channels) * interleaveBlockSize;
	const std::size_t frameCount = (size + interleaveFrameSize - 1) / interleaveFrameSize;
	std::vector<PsxAdpcmState> states(channels);
	std::vector<std::vector<std::int16_t>> channelSamples(channels);

	for (std::size_t frameIndex = 0; frameIndex < frameCount; frameIndex++) {
		std::size_t decodedSamplesPerChannel = SIZE_MAX;
		for (std::uint32_t channel = 0; channel < channels; channel++) {
			const std::size_t channelOffset = frameIndex * interleaveFrameSize + static_cast<std::size_t>(channel) * interleaveBlockSize;
			if (channelOffset >= size)
				return false;

			const std::size_t channelSize = (std::min)(static_cast<std::size_t>(interleaveBlockSize), size - channelOffset);
			channelSamples[channel].clear();
			if (!DecodePsxAdpcm(data + channelOffset, channelSize, states[channel], channelSamples[channel]))
				return false;
			decodedSamplesPerChannel = (std::min)(decodedSamplesPerChannel, channelSamples[channel].size());
		}

		if (decodedSamplesPerChannel == 0 || decodedSamplesPerChannel == SIZE_MAX)
			return false;
		samples.reserve(samples.size() + decodedSamplesPerChannel * channels);
		for (std::size_t sampleIndex = 0; sampleIndex < decodedSamplesPerChannel; sampleIndex++) {
			for (std::uint32_t channel = 0; channel < channels; channel++)
				samples.push_back(channelSamples[channel][sampleIndex]);
		}
	}

	return !samples.empty();
}

void RegisterStream(std::uint32_t streamId, std::uint32_t blockSize, float sampleRate, std::uint32_t channels)
{
	Stream& stream = streams[streamId];
#ifdef _WIN32
	DestroySourceVoice(stream);
#endif
	stream.info.ready = true;
	stream.info.playing = false;
	stream.info.blockSize = blockSize;
	stream.info.volume = 1.0f;
	stream.info.channels = channels;
	stream.info.sampleRate = static_cast<std::uint32_t>(std::fabs(sampleRate));
	stream.info.position = 0;
	stream.positionAtStart = 0;
	stream.samples.clear();
	stream.sampleRate = 0;
}

bool LoadStream(std::uint32_t streamId, const char* path)
{
	auto it = streams.find(streamId);
	if (it == streams.end() || path == nullptr)
		return false;

	std::ifstream file;
	std::string resolvedPath;
	for (const std::string& candidate : GetStreamPathCandidates(path)) {
		file.open(candidate, std::ios::binary | std::ios::ate);
		if (file) {
			resolvedPath = candidate;
			break;
		}
		file.clear();
	}
	if (!file) {
		Log::GetInstance().AddLog(LogLevel::Error, "Audio", "Could not open stream file '{}'", path);
		return false;
	}
	Log::GetInstance().AddLog(LogLevel::Info, "Audio", "Loaded stream '{}' as '{}'", path, resolvedPath);

	const std::streamoff fileSize = file.tellg();
	if (fileSize <= 0 || static_cast<std::uintmax_t>(fileSize) > static_cast<std::uintmax_t>(SIZE_MAX))
		return false;
	std::vector<std::uint8_t> data(static_cast<std::size_t>(fileSize));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
	if (!file)
		return false;

	Stream& stream = it->second;
	std::vector<std::int16_t> samples;
	std::uint32_t sampleRate = stream.info.sampleRate;
	const bool isVag = data.size() >= 4 && data[0] == 'V' && data[1] == 'A' && data[2] == 'G' && data[3] == 'p';
	const bool decoded = isVag
		? DecodeVag(data.data(), data.size(), samples, sampleRate)
		: DecodeMib(data.data(), data.size(), stream.info.channels, stream.info.blockSize, sampleRate, samples);
	if (!decoded) {
		Log::GetInstance().AddLog(LogLevel::Error, "Audio", "Could not decode stream '{}'", path);
		return false;
	}

#ifdef _WIN32
	DestroySourceVoice(stream);
#endif
	stream.samples = std::move(samples);
	stream.sampleRate = sampleRate;
	stream.info.sampleRate = sampleRate;
	stream.info.position = 0;
	stream.positionAtStart = 0;
	stream.info.playing = false;
	return true;
}

void PrepareStream(std::uint32_t streamId)
{
	auto it = streams.find(streamId);
	if (it == streams.end())
		return;

	Stream& stream = it->second;
#ifdef _WIN32
	if (stream.sourceVoice != nullptr) {
		stream.sourceVoice->Stop(0);
		stream.sourceVoice->FlushSourceBuffers();
	}
#endif
	stream.info.position = 0;
	stream.info.playing = false;
	stream.positionAtStart = 0;
}

bool StartStream(std::uint32_t streamId)
{
	auto it = streams.find(streamId);
	if (it == streams.end() || !it->second.info.ready)
		return false;

	Stream& stream = it->second;
#ifdef _WIN32
	if (!stream.samples.empty()) {
		if (!CreateSourceVoice(stream))
			return false;

		XAUDIO2_VOICE_STATE state{};
		stream.sourceVoice->GetState(&state, 0);
		if (state.BuffersQueued == 0) {
			XAUDIO2_BUFFER buffer{};
			buffer.AudioBytes = static_cast<UINT32>(stream.samples.size() * sizeof(std::int16_t));
			buffer.pAudioData = reinterpret_cast<const BYTE*>(stream.samples.data());
			const HRESULT result = stream.sourceVoice->SubmitSourceBuffer(&buffer);
			if (FAILED(result)) {
				LogAudioError("SubmitSourceBuffer failed", result);
				return false;
			}
		}

		const HRESULT result = stream.sourceVoice->Start(0);
		if (FAILED(result)) {
			LogAudioError("SourceVoice Start failed", result);
			return false;
		}
		stream.info.playing = true;
		return true;
	}
#endif

	stream.info.position = CurrentPosition(stream);
	stream.positionAtStart = stream.info.position;
	stream.startedAt = Clock::now();
	stream.info.playing = true;
	return true;
}

bool SetStreamVolume(std::uint32_t streamId, float volume)
{
	auto it = streams.find(streamId);
	if (it == streams.end()) return false;
	volume = std::isfinite(volume) ? std::clamp(volume, 0.0f, 1.0f) : 0.0f;
	auto& stream = it->second;
#ifdef _WIN32
	if (stream.sourceVoice) {
		const HRESULT result = stream.sourceVoice->SetVolume(volume);
		if (FAILED(result)) {
			LogAudioError("SourceVoice SetVolume failed", result);
			return false;
		}
	}
#endif
	stream.info.volume = volume;
	return true;
}

bool StopStream(std::uint32_t streamId)
{
	auto it = streams.find(streamId);
	if (it == streams.end())
		return false;

	Stream& stream = it->second;
	stream.info.position = CurrentPosition(stream);
#ifdef _WIN32
	if (stream.sourceVoice != nullptr)
		stream.sourceVoice->Stop(0);
#endif
	stream.positionAtStart = stream.info.position;
	stream.info.playing = false;
	return true;
}

bool GetStreamInfo(std::uint32_t streamId, StreamInfo& out)
{
	auto it = streams.find(streamId);
	if (it == streams.end())
		return false;

	Stream& stream = it->second;
	stream.info.position = CurrentPosition(stream);
	out = stream.info;
	return true;
}

bool UnregisterStream(std::uint32_t streamId)
{
	auto it = streams.find(streamId);
	if (it == streams.end())
		return false;
#ifdef _WIN32
	DestroySourceVoice(it->second);
#endif
	streams.erase(it);
	return true;
}

void ResetStreams()
{
#ifdef _WIN32
	for (auto& [streamId, stream] : streams)
		DestroySourceVoice(stream);
#endif
	streams.clear();
}

} // namespace Audio
