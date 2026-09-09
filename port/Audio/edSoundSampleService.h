#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace Audio
{
struct SampleDescription
{
	std::uint32_t handle = 0;
	std::uint32_t sampleRate = 0;
	std::uint32_t dataSize = 0;
	std::uint32_t loopStartOffset = 0;
	float loopEndOffset = 0;
	bool looping = false;
};

struct DecodedSample
{
	std::vector<std::int16_t> pcm;
	std::uint32_t sampleRate = 0;
	std::uint32_t loopBegin = 0;
	std::uint32_t loopLength = 0;
};

struct SampleControls
{
	float left = 1;
	float right = 1;
	float pitch = 1;
};

enum class SampleCommandType { Create, Start, Stop, Pause, Resume, Update, Destroy };
struct SampleCommand
{
	SampleCommandType type;
	std::uint32_t instanceId;
	SampleDescription sample{};
	SampleControls controls{};
};

// The service owns PCM until the voice is destroyed. All calls, including
// polling, run on the game thread; no Eden objects cross into XAudio2 callbacks.
class SampleVoice
{
public:
	virtual ~SampleVoice() = default;
	virtual bool SetControls(const SampleControls& controls) = 0;
	virtual bool SetPlaying(bool playing) = 0;
	virtual bool IsFinished() = 0;
	virtual std::uint64_t GetFramesPlayed() = 0;
};
using SampleVoiceFactory = std::unique_ptr<SampleVoice> (*)(const DecodedSample& sample);

bool DecodeSample(const std::uint8_t* data, std::size_t size, const SampleDescription& description, DecodedSample& out);
void QueueSampleCommand(const SampleCommand& command);
void FlushSampleCommands();
std::vector<std::uint32_t> PollFinishedSamples();
bool GetSamplePosition(std::uint32_t instanceId, std::uint32_t& adpcmOffset);
void DestroySample(std::uint32_t instanceId);
void InvalidateSample(std::uint32_t handle);
void ResetSamples();
// Replaces the device adapter for deterministic tests; resets voices and commands.
void SetSampleVoiceFactory(SampleVoiceFactory factory);
}
