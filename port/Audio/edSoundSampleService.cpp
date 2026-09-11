#include "edSoundSampleService.h"
#include "edSoundStreamService.h"
#include "edSoundDevice.h"
#include "edSysTransferService.h"
#include "log.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <XAudio2.h>
#endif

namespace Audio
{
namespace
{
struct SampleEntry
{
	SampleDescription description;
	std::shared_ptr<DecodedSample> decoded;
};
struct Instance
{
	// Declaration order ensures the voice releases its buffer before PCM dies.
	std::shared_ptr<DecodedSample> decoded;
	std::unique_ptr<SampleVoice> voice;
	bool started = false;
};
std::unordered_map<std::uint32_t, SampleEntry> cache;
std::unordered_map<std::uint32_t, Instance> instances;
std::vector<SampleCommand> commands;
std::vector<std::uint32_t> failedInstances;
SampleVoiceFactory voiceFactory = nullptr;

float FiniteClamp(float value, float low, float high, float fallback)
{
	return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
}

#ifdef _WIN32
class XAudioSampleVoice final : public SampleVoice
{
public:
	IXAudio2SourceVoice* voice = nullptr;
	IXAudio2MasteringVoice* destination = nullptr;
	std::uint32_t channels = 0;

	~XAudioSampleVoice() override
	{
		if (voice) voice->DestroyVoice();
	}
	bool SetControls(const SampleControls& controls) override
	{
		std::vector<float> matrix(channels, 0.0f);
		const float left = FiniteClamp(controls.left, -1, 1, 0);
		const float right = FiniteClamp(controls.right, -1, 1, 0);
		if (channels == 1) matrix[0] = (left + right) * 0.5f;
		else { matrix[0] = left; matrix[1] = right; }
		return SUCCEEDED(voice->SetOutputMatrix(destination, 1, channels, matrix.data())) &&
			SUCCEEDED(voice->SetFrequencyRatio(FiniteClamp(controls.pitch, XAUDIO2_MIN_FREQ_RATIO, 4.0f, 1.0f)));
	}
	bool SetPlaying(bool playing) override { return SUCCEEDED(playing ? voice->Start() : voice->Stop()); }
	bool IsFinished() override
	{
		XAUDIO2_VOICE_STATE state{};
		voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		return state.BuffersQueued == 0;
	}
	std::uint64_t GetFramesPlayed() override
	{
		XAUDIO2_VOICE_STATE state{};
		voice->GetState(&state);
		return state.SamplesPlayed;
	}
};
#endif

std::unique_ptr<SampleVoice> CreateVoice(const DecodedSample& sample)
{
	if (voiceFactory) return voiceFactory(sample);
#ifdef _WIN32
	IXAudio2* engine = GetAudioEngine();
	if (!engine) return nullptr;
	auto result = std::make_unique<XAudioSampleVoice>();
	result->destination = GetAudioMasteringVoice();
	XAUDIO2_VOICE_DETAILS details{};
	result->destination->GetVoiceDetails(&details);
	result->channels = details.InputChannels;
	WAVEFORMATEX format{};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = sample.sampleRate;
	format.wBitsPerSample = 16;
	format.nBlockAlign = 2;
	format.nAvgBytesPerSec = sample.sampleRate * 2;
	if (FAILED(engine->CreateSourceVoice(&result->voice, &format, 0, 4.0f))) return nullptr;
	XAUDIO2_BUFFER buffer{};
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = static_cast<UINT32>(sample.pcm.size() * sizeof(std::int16_t));
	buffer.pAudioData = reinterpret_cast<const BYTE*>(sample.pcm.data());
	buffer.LoopBegin = sample.loopBegin;
	buffer.LoopLength = sample.loopLength;
	buffer.LoopCount = sample.loopLength ? XAUDIO2_LOOP_INFINITE : 0;
	if (FAILED(result->voice->SubmitSourceBuffer(&buffer))) return nullptr;
	return result;
#else
	return nullptr;
#endif
}

std::shared_ptr<DecodedSample> LoadSample(const SampleDescription& description)
{
	LoadedDataInfo data;
	if (!LookupLoadedData(description.handle, data)) return nullptr;
	auto found = cache.find(description.handle);
	if (found != cache.end()) {
		const auto& old = found->second.description;
		if (old.sampleRate == description.sampleRate && old.dataSize == description.dataSize &&
			old.loopStartOffset == description.loopStartOffset && old.loopEndOffset == description.loopEndOffset && old.looping == description.looping)
			return found->second.decoded;
	}
	auto decoded = std::make_shared<DecodedSample>();
	if (!DecodeSample(data.data, data.size, description, *decoded)) return nullptr;
	cache[description.handle] = {description, decoded};
	return decoded;
}
}

bool DecodeSample(const std::uint8_t* data, std::size_t size, const SampleDescription& description, DecodedSample& out)
{
	out = {};
	if (!data || !description.dataSize || description.dataSize > size || description.dataSize % 16 ||
		description.sampleRate < 1000 || description.sampleRate > 200000 ||
		description.dataSize / 16 > std::numeric_limits<std::uint32_t>::max() / 56)
		return false;
	// The loader rewrites terminal flags for SPU playback. Use its preserved
	// metadata, not those flags, to decide whether the sample repeats.
	std::uint32_t playBytes = description.dataSize;
	if (!std::isfinite(description.loopEndOffset) || description.loopEndOffset < 0 ||
		description.loopEndOffset >= static_cast<double>(description.dataSize) ||
		std::fmod(description.loopEndOffset, 16.0f) != 0)
		return false;
	// An end offset names the terminal block, which contributes 28 PCM frames.
	// Zero can also mean no marker was found; inspect block zero to disambiguate.
	if (description.loopEndOffset != 0 || (data[1] & 1))
		playBytes = static_cast<std::uint32_t>(description.loopEndOffset) + 16;
	DecodedSample decoded;
	if (!DecodeRawAdpcm(data, playBytes, decoded.pcm)) return false;
	decoded.sampleRate = description.sampleRate;
	if (description.looping) {
		if (description.loopStartOffset % 16 || description.loopStartOffset >= playBytes) return false;
		decoded.loopBegin = description.loopStartOffset / 16 * 28;
		decoded.loopLength = static_cast<std::uint32_t>(decoded.pcm.size()) - decoded.loopBegin;
	}
	out = std::move(decoded);
	return true;
}

void QueueSampleCommand(const SampleCommand& command) { commands.push_back(command); }

void DestroySample(std::uint32_t instanceId)
{
	instances.erase(instanceId);
	commands.erase(std::remove_if(commands.begin(), commands.end(), [instanceId](const auto& c) { return c.instanceId == instanceId; }), commands.end());
	failedInstances.erase(std::remove(failedInstances.begin(), failedInstances.end(), instanceId), failedInstances.end());
}

void FlushSampleCommands()
{
	// Move the list so failure/destruction can safely discard pending state.
	auto batch = std::move(commands);
	commands.clear();
	for (const auto& command : batch) {
		const auto id = command.instanceId;
		if (!id) continue;
		if (command.type == SampleCommandType::Destroy || command.type == SampleCommandType::Stop) {
			DestroySample(id);
			continue;
		}
		bool success = true;
		if (command.type == SampleCommandType::Create) {
			DestroySample(id);
			Instance instance;
			instance.decoded = LoadSample(command.sample);
			if (instance.decoded) instance.voice = CreateVoice(*instance.decoded);
			success = instance.voice && instance.voice->SetControls(command.controls);
			if (success) instances.emplace(id, std::move(instance));
			Log::GetInstance().AddLog(LogLevel::Info, "AudioInstances",
				"host-create id=0x{:08x} handle={} success={} left={} right={} pitch={}",
				id, command.sample.handle, success, command.controls.left, command.controls.right, command.controls.pitch);
		}
		else {
			auto found = instances.find(id);
			if (found == instances.end()) continue;
			auto& instance = found->second;
			switch (command.type) {
			case SampleCommandType::Start:
			case SampleCommandType::Resume:
				success = instance.voice->SetPlaying(true);
				instance.started = success;
				Log::GetInstance().AddLog(LogLevel::Info, "AudioInstances", "host-start id=0x{:08x} success={}", id, success);
				break;
			case SampleCommandType::Pause: success = instance.voice->SetPlaying(false); break;
			case SampleCommandType::Update: success = instance.voice->SetControls(command.controls); break;
			default: break;
			}
		}
		if (!success) {
			DestroySample(id);
			failedInstances.push_back(id);
			Log::GetInstance().AddLog(LogLevel::Error, "Audio", "Sample command {} failed (instance {}, handle {})",
				static_cast<unsigned>(command.type), id, command.sample.handle);
		}
	}
}

std::vector<std::uint32_t> PollFinishedSamples()
{
	auto finished = std::move(failedInstances);
	failedInstances.clear();
	for (auto it = instances.begin(); it != instances.end();) {
		if (it->second.started && it->second.voice->IsFinished()) {
			Log::GetInstance().AddLog(LogLevel::Info, "AudioInstances", "host-finished id=0x{:08x}", it->first);
			finished.push_back(it->first);
			it = instances.erase(it);
		}
		else ++it;
	}
	return finished;
}

bool GetSamplePosition(std::uint32_t instanceId, std::uint32_t& adpcmOffset)
{
	auto found = instances.find(instanceId);
	if (found == instances.end()) return false;
	const auto& sample = *found->second.decoded;
	auto frames = found->second.voice->GetFramesPlayed();
	if (sample.loopLength && frames >= sample.loopBegin)
		frames = sample.loopBegin + (frames - sample.loopBegin) % sample.loopLength;
	else frames = std::min<std::uint64_t>(frames, sample.pcm.size());
	adpcmOffset = static_cast<std::uint32_t>(frames / 28 * 16);
	return true;
}

void InvalidateSample(std::uint32_t handle) { cache.erase(handle); }
void ResetSamples()
{
	commands.clear();
	failedInstances.clear();
	instances.clear();
	cache.clear();
}
void SetSampleVoiceFactory(SampleVoiceFactory factory)
{
	ResetSamples();
	voiceFactory = factory;
}
}
