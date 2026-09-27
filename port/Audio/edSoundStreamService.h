#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace Audio
{

struct StreamInfo
{
	bool ready = false;
	bool playing = false;
	bool finished = false;
	float volume = 1.0f;
	std::uint32_t blockSize = 0;
	std::uint32_t channels = 1;
	std::uint32_t sampleRate = 0;
	std::uint64_t position = 0;
	float duration = 0.0f;
	std::string path;
};

void RegisterStream(std::uint32_t streamId, std::uint32_t blockSize, float sampleRate, std::uint32_t channels = 1);
void PrepareStream(std::uint32_t streamId);
bool StartStream(std::uint32_t streamId);
bool SetStreamVolume(std::uint32_t streamId, float volume);
bool StopStream(std::uint32_t streamId);
bool SeekStream(std::uint32_t streamId, float seconds);
bool GetStreamInfo(std::uint32_t streamId, StreamInfo& out);
bool IsStreamFinished(std::uint32_t streamId);
std::vector<std::pair<std::uint32_t, StreamInfo>> GetStreams();
bool UnregisterStream(std::uint32_t streamId);
void ResetStreams();

bool LoadStream(std::uint32_t streamId, const char* path);
bool DecodeVag(const std::uint8_t* data, std::size_t size, std::vector<std::int16_t>& samples, std::uint32_t& sampleRate);
bool DecodeRawAdpcm(const std::uint8_t* data, std::size_t size, std::vector<std::int16_t>& samples);
bool DecodeMib(const std::uint8_t* data, std::size_t size, std::uint32_t channels,
	std::uint32_t interleaveBlockSize, std::uint32_t sampleRate, std::vector<std::int16_t>& samples);

} // namespace Audio
