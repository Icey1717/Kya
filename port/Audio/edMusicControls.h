#pragma once
#include <array>
#include <algorithm>

namespace Audio
{
constexpr unsigned MusicSampleRate = 48000;
// Eden converts 16-bit API volumes to bytes. MIDI uses 128 as unity;
// HSyn master volume instead uses 256 as unity.
constexpr unsigned MusicVolumeByte(unsigned volume) { return std::min(volume, 65535u) * 255u / 65535u; }
constexpr unsigned MusicMidiVolume(unsigned midi, unsigned stream, unsigned channel)
{
	return std::min(127u, (std::min(midi, 127u) * std::min(stream, 255u) * std::min(channel, 255u)) >> 14);
}
struct MusicControls
{
	float volume = 1, tempo = 1;
	unsigned streamVolume = 128;
	bool looping = false, mono = false;
	std::array<unsigned, 16> channels;
	MusicControls() { channels.fill(128); }
};
}
