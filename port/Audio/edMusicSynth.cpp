#include "edMusicSynth.h"
#include <algorithm>
#include <cmath>

namespace Audio
{
namespace
{
struct Envelope
{
	int level = 0, phase = 0, counter = 0;
	void Release() { if (phase != 3) { phase = 3; counter = 0; } }
	float Step(unsigned first, unsigned second)
	{
		int shift, step;
		bool decrease, exponential;
		switch (phase) {
		case 0: shift = (first >> 10) & 31; step = (first >> 8) & 3; decrease = false; exponential = (first & 0x8000) != 0; break;
		case 1: shift = (first >> 4) & 15; step = 0; decrease = true; exponential = true; break;
		case 2: shift = (second >> 8) & 31; step = (second >> 6) & 3; decrease = (second & 0x4000) != 0; exponential = (second & 0x8000) != 0; break;
		default: shift = second & 31; step = 0; decrease = true; exponential = (second & 32) != 0; break;
		}
		int delta = (decrease ? -8 + step : 7 - step) * (1 << std::max(0, 11 - shift));
		int increment = 0x8000 >> std::max(0, shift - 11);
		if (exponential && !decrease && level > 0x6000) {
			if (shift < 10) delta /= 4;
			else if (shift >= 11) increment /= 4;
			else { delta /= 2; increment /= 2; }
		} else if (exponential && decrease) delta = static_cast<int>(std::floor(delta * (level / 32768.0)));
		const bool hold = phase == 3 ? shift == 31 : (phase != 1 && shift == 31 && step == 3);
		if (!hold) increment = std::max(1, increment);
		counter += increment;
		if (counter >= 0x8000) {
			counter &= 0x7fff;
			level = std::clamp(level + delta, 0, 32767);
			if ((phase == 0 && level == 32767) || (phase == 1 && level <= std::min(32767u, ((first & 15) + 1) * 2048))) {
				++phase;
				counter = 0;
			}
		}
		return level / 32768.0f;
	}
};
struct Channel
{
	unsigned program = 0, volume = 127;
	float expression = 1, pan = 0, bend = 0, modulation = 0;
	bool sustain = false;
};
struct Note
{
	const MusicRegion* region;
	const MusicProgram* program;
	unsigned channel, key;
	float velocity;
	double position = 0;
	Envelope envelope;
	bool released = false;
	std::uint64_t age = 0;
};
}

struct MusicSynth::State
{
	std::shared_ptr<const MusicSong> song;
	std::shared_ptr<const MusicBank> bank;
	MusicControls controls;
	std::array<Channel, 16> channels;
	std::vector<Note> notes;
	std::size_t event = 0;
	double tick = 0;
	unsigned tempo = 500000, voiceLimit;
	std::uint64_t frames = 0;
	bool ended = false;
	void ResetSequence()
	{
		event = 0; tick = 0; tempo = 500000; channels = {}; notes.clear(); ended = false;
	}
	void Event(const MusicEvent& message)
	{
		if (message.tempo) { tempo = message.tempo; return; }
		const unsigned index = message.status & 15;
		auto& channel = channels[index];
		const auto kind = message.status >> 4;
		if (kind == 8 || (kind == 9 && message.second == 0)) {
			for (auto& note : notes) if (note.channel == index && note.key == message.first) {
				note.released = true;
				if (!channel.sustain) note.envelope.Release();
			}
		} else if (kind == 9) {
			for (const auto& region : bank->programs[channel.program].regions) {
				if (message.first < region.keyLow || message.first > region.keyHigh || message.second < region.velocityLow || message.second > region.velocityHigh) continue;
				if (notes.size() == voiceLimit) notes.erase(notes.begin());
				notes.push_back({&region, &bank->programs[channel.program], index, message.first, message.second / 127.0f});
			}
		} else if (kind == 12) channel.program = message.first;
		else if (kind == 14) channel.bend = (static_cast<int>(message.first | (message.second << 7)) - 8192) / 8192.0f;
		else if (kind == 11) {
			switch (message.first) {
			case 1: channel.modulation = message.second / 127.0f; break;
			case 7: channel.volume = message.second; break;
			case 10: channel.pan = (static_cast<int>(message.second) - 64) / 64.0f; break;
			case 11: channel.expression = message.second / 127.0f; break;
			case 64:
				channel.sustain = message.second >= 64;
				if (!channel.sustain) for (auto& note : notes) if (note.channel == index && note.released) note.envelope.Release();
				break;
			case 120: std::erase_if(notes, [index](const auto& note) { return note.channel == index; }); break;
			case 123: for (auto& note : notes) if (note.channel == index) note.envelope.Release(); break;
			}
		}
	}
};

MusicSynth::MusicSynth(std::shared_ptr<const MusicSong> song, std::shared_ptr<const MusicBank> bank, unsigned voiceLimit)
	: state(std::make_unique<State>())
{
	state->song = std::move(song); state->bank = std::move(bank);
	state->voiceLimit = std::clamp(voiceLimit, 1u, 128u);
}
MusicSynth::~MusicSynth() = default;
void MusicSynth::Restart() { state->ResetSequence(); state->frames = 0; }
void MusicSynth::SetControls(const MusicControls& controls) { state->controls = controls; }
bool MusicSynth::Finished() const { return state->ended && state->notes.empty(); }
std::uint64_t MusicSynth::Frames() const { return state->frames; }

void MusicSynth::Render(std::span<float> stereo)
{
	auto& s = *state;
	std::fill(stereo.begin(), stereo.end(), 0);
	for (std::size_t frame = 0; frame + 1 < stereo.size(); frame += 2) {
		while (s.event < s.song->events.size() && s.song->events[s.event].tick <= s.tick) s.Event(s.song->events[s.event++]);
		if (!s.ended && s.tick >= s.song->endTick) {
			if (s.controls.looping) {
				const double carry = s.tick - s.song->endTick;
				s.ResetSequence(); s.tick = carry;
				while (s.event < s.song->events.size() && s.song->events[s.event].tick <= s.tick) s.Event(s.song->events[s.event++]);
			} else { s.ended = true; for (auto& note : s.notes) note.envelope.Release(); }
		}
		float left = 0, right = 0;
		for (auto& note : s.notes) {
			const auto& region = *note.region;
			const auto& sample = s.bank->samples[region.sample];
			const auto& channel = s.channels[note.channel];
			if (note.position >= sample.pcm.size()) continue;
			const auto index = static_cast<std::size_t>(note.position);
			const auto next = index + 1 == sample.loopEnd ? sample.loopBegin : std::min(index + 1, sample.pcm.size() - 1);
			const float value = (sample.pcm[index] + (sample.pcm[next] - sample.pcm[index]) * static_cast<float>(note.position - index)) / 32768.0f;
			const auto triangle = [](double phase) { return static_cast<float>(1 - 4 * std::abs(std::fmod(phase + 0.25, 1.0) - 0.5)); };
			const double age = note.age / static_cast<double>(MusicSampleRate);
			const auto& program = *note.program;
			const auto fade = [age](float delay, float duration) {
				return age < delay ? 0.0f : duration > 0 ? static_cast<float>(std::min(1.0, (age - delay) / duration)) : 1.0f;
			};
			const unsigned lfoMask = note.released ? 6 : 5;
			const float pitchLfo = program.pitchWave == 3 && (region.lfoFlags & lfoMask)
				? triangle(age / program.pitchCycle) * (program.pitchDepth + program.modulationDepth * channel.modulation) * fade(region.pitchDelay, region.pitchFade) : 0;
			const float ampLfo = program.ampWave == 3 && (region.lfoFlags & (lfoMask << 4))
				? 1 + triangle(age / program.ampCycle) * program.ampDepth * fade(region.ampDelay, region.ampFade) : 1;
			const float midiGain = MusicMidiVolume(channel.volume, s.controls.streamVolume, s.controls.channels[note.channel]) / 127.0f;
			const float gain = note.envelope.Step(region.adsr1, region.adsr2) * region.gain * note.velocity * midiGain * channel.expression * ampLfo;
			const float pan = std::clamp(region.pan + channel.pan, -1.0f, 1.0f);
			left += value * gain * std::min(1.0f, 1 - pan);
			right += value * gain * std::min(1.0f, 1 + pan);
			const float bend = channel.bend * (channel.bend < 0 ? region.bendDown : region.bendUp);
			note.position += (sample.rate / static_cast<double>(MusicSampleRate)) * std::exp2((note.key - region.root + bend + pitchLfo) / 12.0);
			++note.age;
			if (sample.loopEnd > sample.loopBegin && note.position >= sample.loopEnd)
				note.position = sample.loopBegin + std::fmod(note.position - sample.loopBegin, sample.loopEnd - sample.loopBegin);
		}
		std::erase_if(s.notes, [&](const auto& note) {
			return (note.envelope.phase == 3 && note.envelope.level == 0) || note.position >= s.bank->samples[note.region->sample].pcm.size();
		});
		if (s.controls.mono) left = right = (left + right) * 0.5f;
		stereo[frame] = std::clamp(left * s.controls.volume, -1.0f, 1.0f);
		stereo[frame + 1] = std::clamp(right * s.controls.volume, -1.0f, 1.0f);
		if (!s.ended) s.tick += s.song->ppqn * (1000000.0 / s.tempo) * std::clamp(s.controls.tempo, 0.001f, 16.0f) / MusicSampleRate;
		++s.frames;
	}
}
}
