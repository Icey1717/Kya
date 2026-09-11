#include "edMusicService.h"
#include "edMusicSynth.h"
#include "edSysTransferService.h"
#include "edSoundDevice.h"
#include "log.h"
#include <chrono>
#include <deque>
#include <mutex>
#include <thread>
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
#ifdef _WIN32
class XAudioMusicOutput final : public MusicOutput
{
	IXAudio2SourceVoice* voice = nullptr;
	std::deque<std::vector<float>> buffers;
public:
	~XAudioMusicOutput() override { if (voice) voice->DestroyVoice(); }
	bool Initialize()
	{
		auto* engine = GetAudioEngine();
		if (!engine) return false;
		WAVEFORMATEX format{};
		format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		format.nChannels = 2; format.nSamplesPerSec = MusicSampleRate;
		format.wBitsPerSample = 32; format.nBlockAlign = 8;
		format.nAvgBytesPerSec = MusicSampleRate * 8;
		return SUCCEEDED(engine->CreateSourceVoice(&voice, &format));
	}
	unsigned Queued() override
	{
		XAUDIO2_VOICE_STATE status{};
		voice->GetState(&status, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		while (buffers.size() > status.BuffersQueued) buffers.pop_front();
		return status.BuffersQueued;
	}
	bool Submit(const float* samples, unsigned count) override
	{
		buffers.emplace_back(samples, samples + count);
		XAUDIO2_BUFFER buffer{};
		buffer.AudioBytes = count * sizeof(float);
		buffer.pAudioData = reinterpret_cast<const BYTE*>(buffers.back().data());
		if (FAILED(voice->SubmitSourceBuffer(&buffer))) { buffers.pop_back(); return false; }
		return true;
	}
	bool Play(bool playing) override { return SUCCEEDED(playing ? voice->Start() : voice->Stop()); }
};
#endif
struct Stream
{
	unsigned song = UINT32_MAX, bank = UINT32_MAX;
	MusicControls controls;
	unsigned channelVolumeMask = 0;
	std::unique_ptr<MusicSynth> synth;
	std::unique_ptr<MusicOutput> output;
	bool playing = false, completed = false;
	void Clear() { output.reset(); synth.reset(); playing = false; completed = false; }
};
struct Service
{
	std::mutex mutex;
	std::vector<Stream> streams;
	std::vector<MusicCommand> commands;
	std::unordered_map<unsigned, std::shared_ptr<MusicSong>> songs;
	std::unordered_map<unsigned, std::shared_ptr<MusicBank>> banks;
	unsigned voices = 32;
	MusicOutputFactory factory = nullptr;
	// Destroy the worker before any state it uses, including the mutex.
	std::jthread worker;
};
Service service;

void Error(const std::string& error, unsigned index)
{
	Log::GetInstance().AddLog(LogLevel::Error, "Audio", "Music {}: {}", index, error);
}
void Fail(Stream& stream, unsigned index)
{
	stream.Clear(); stream.completed = true;
	Error("playback failed", index);
}
bool Prepare(Stream& stream)
{
	const auto song = service.songs.find(stream.song);
	const auto bank = service.banks.find(stream.bank);
	if (song == service.songs.end() || bank == service.banks.end()) return false;
	stream.synth = std::make_unique<MusicSynth>(song->second, bank->second, service.voices);
	stream.synth->SetControls(stream.controls);
	if (service.factory) stream.output = service.factory();
#ifdef _WIN32
	else {
		auto output = std::make_unique<XAudioMusicOutput>();
		if (output->Initialize()) stream.output = std::move(output);
	}
#endif
	return stream.output != nullptr;
}
}

void ServiceMusic()
{
	std::lock_guard lock(service.mutex);
	std::array<float, 960> block{}; // 10 ms stereo, at most 30 ms queued per stream.
	for (unsigned index = 0; index < service.streams.size(); ++index) {
		auto& stream = service.streams[index];
		if (!stream.playing || !stream.output || !stream.synth) continue;
		unsigned queued = stream.output->Queued();
		while (queued < 3 && !stream.synth->Finished()) {
			stream.synth->Render(block);
			if (!stream.output->Submit(block.data(), static_cast<unsigned>(block.size()))) { Fail(stream, index); break; }
			++queued;
		}
		if (stream.playing && queued == 0 && stream.synth->Finished()) {
			stream.Clear(); stream.completed = true;
		}
	}
}
void ResetMusic()
{
	if (service.worker.joinable()) { service.worker.request_stop(); service.worker.join(); }
	std::lock_guard lock(service.mutex);
	service.streams.clear(); service.commands.clear(); service.songs.clear(); service.banks.clear();
}
void InitializeMusic(unsigned streams, unsigned voices)
{
	ResetMusic();
	std::lock_guard lock(service.mutex);
	service.streams.resize(std::min(streams, 128u)); service.voices = voices;
}
void SetMusicOutputFactory(MusicOutputFactory factory)
{
	ResetMusic(); service.factory = factory;
}
bool InstallMusicSong(unsigned index, std::uint32_t handle)
{
	RemoveMusicSong(index);
	LoadedDataInfo data;
	if (!LookupLoadedData(handle, data)) { Error("missing song transfer", index); return false; }
	auto song = std::make_shared<MusicSong>(); std::string error;
	if (!ParseMusicSong({data.data, data.size}, *song, error)) { Error(error, index); return false; }
	std::lock_guard lock(service.mutex); service.songs[index] = std::move(song);
	return true;
}
bool InstallMusicBank(unsigned index, std::uint32_t headerHandle, std::uint32_t bodyHandle)
{
	RemoveMusicBank(index);
	LoadedDataInfo header, body;
	if (!LookupLoadedData(headerHandle, header) || !LookupLoadedData(bodyHandle, body)) { Error("missing bank transfer", index); return false; }
	auto bank = std::make_shared<MusicBank>(); std::string error;
	if (!ParseMusicBank({header.data, header.size}, {body.data, body.size}, *bank, error)) { Error(error, index); return false; }
	std::lock_guard lock(service.mutex); service.banks[index] = std::move(bank);
	return true;
}
void RemoveMusicSong(unsigned index)
{
	std::lock_guard lock(service.mutex);
	std::vector<bool> affected(service.streams.size());
	for (unsigned i = 0; i < service.streams.size(); ++i) affected[i] = service.streams[i].song == index;
	for (const auto& command : service.commands) if (command.type == MusicCommandType::Song && command.value == index) affected[command.stream] = true;
	for (unsigned i = 0; i < affected.size(); ++i) if (affected[i]) { service.streams[i].Clear(); service.streams[i].song = UINT32_MAX; }
	std::erase_if(service.commands, [&](const auto& command) { return affected[command.stream]; });
	service.songs.erase(index);
}
void RemoveMusicBank(unsigned index)
{
	std::lock_guard lock(service.mutex);
	std::vector<bool> affected(service.streams.size());
	for (unsigned i = 0; i < service.streams.size(); ++i) affected[i] = service.streams[i].bank == index;
	for (const auto& command : service.commands) if (command.type == MusicCommandType::Bank && command.value == index) affected[command.stream] = true;
	for (unsigned i = 0; i < affected.size(); ++i) if (affected[i]) { service.streams[i].Clear(); service.streams[i].bank = UINT32_MAX; }
	std::erase_if(service.commands, [&](const auto& command) { return affected[command.stream]; });
	service.banks.erase(index);
}
void QueueMusicCommand(MusicCommand command)
{
	std::lock_guard lock(service.mutex);
	if (command.stream < service.streams.size()) service.commands.push_back(command);
}
std::vector<unsigned> FlushMusicCommands(const std::vector<MusicControls>& controls)
{
	std::vector<unsigned> finished;
	{
		std::lock_guard lock(service.mutex);
		for (unsigned i = 0; i < service.streams.size() && i < controls.size(); ++i) {
			const auto channels = service.streams[i].controls.channels;
			service.streams[i].controls = controls[i];
			for (unsigned channel = 0; channel < 16; ++channel)
				if (service.streams[i].channelVolumeMask & (1u << channel)) service.streams[i].controls.channels[channel] = channels[channel];
			if (service.streams[i].synth) service.streams[i].synth->SetControls(service.streams[i].controls);
		}
		for (const auto& command : service.commands) {
			auto& stream = service.streams[command.stream];
			switch (command.type) {
			case MusicCommandType::ChannelVolume:
				if (command.channel < 16) {
					stream.channelVolumeMask |= 1u << command.channel;
					stream.controls.channels[command.channel] = MusicVolumeByte(command.value);
					if (stream.synth) stream.synth->SetControls(stream.controls);
				}
				break;
			case MusicCommandType::Song:
				stream.Clear(); stream.song = command.value;
				stream.channelVolumeMask = 0; stream.controls.channels.fill(128);
				break;
			case MusicCommandType::Bank: stream.Clear(); stream.bank = command.value; break;
			case MusicCommandType::Mute: stream.Clear(); break;
			case MusicCommandType::Pause:
				stream.playing = false;
				if (stream.output && !stream.output->Play(false)) Fail(stream, command.stream);
				break;
			case MusicCommandType::Restart: stream.Clear(); [[fallthrough]];
			case MusicCommandType::Resume:
				stream.completed = false;
				if (!stream.synth && !Prepare(stream)) { Fail(stream, command.stream); break; }
				stream.playing = true;
				if (!stream.output->Play(true)) Fail(stream, command.stream);
				break;
			}
		}
		service.commands.clear();
		for (unsigned i = 0; i < service.streams.size(); ++i) if (service.streams[i].completed) {
			service.streams[i].completed = false; finished.push_back(i);
		}
		if (!service.factory && !service.worker.joinable() && !service.streams.empty())
			service.worker = std::jthread([](std::stop_token stop) {
				while (!stop.stop_requested()) {
					ServiceMusic();
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
				}
			});
	}
	return finished;
}
}
