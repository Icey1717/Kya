#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace Audio
{
struct MusicEvent
{
	std::uint64_t tick = 0;
	std::uint8_t status = 0, first = 0, second = 0;
	std::uint32_t tempo = 0;
};
struct MusicSong
{
	std::uint16_t ppqn = 0;
	std::uint64_t endTick = 0;
	std::vector<MusicEvent> events;
};
struct MusicSample
{
	std::vector<std::int16_t> pcm;
	std::uint32_t rate = 0, loopBegin = 0, loopEnd = 0;
};
struct MusicRegion
{
	std::uint16_t sample = 0, adsr1 = 0, adsr2 = 0;
	int keyLow = 0, keyHigh = 127, velocityLow = 0, velocityHigh = 127;
	float root = 60, gain = 1, pan = 0;
	float bendDown = 2, bendUp = 2;
	unsigned lfoFlags = 0;
	float pitchDelay = 0, pitchFade = 0, ampDelay = 0, ampFade = 0;
};
struct MusicProgram
{
	std::vector<MusicRegion> regions;
	unsigned pitchWave = 0, ampWave = 0;
	float pitchCycle = 0.1f, ampCycle = 0.1f;
	float pitchDepth = 0, modulationDepth = 0, ampDepth = 0;
};
struct MusicBank
{
	std::vector<MusicSample> samples;
	std::array<MusicProgram, 128> programs;
};

// Parsers own all returned data and leave output empty on failure.
bool ParseMusicSong(std::span<const std::uint8_t> bytes, MusicSong& out, std::string& error);
bool ParseMusicBank(std::span<const std::uint8_t> header, std::span<const std::uint8_t> body,
	MusicBank& out, std::string& error);
}
