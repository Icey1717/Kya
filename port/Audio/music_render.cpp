#include "edMusicData.h"
#include "edMusicSynth.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace
{
unsigned Word(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
	unsigned result = 0;
	for (unsigned i = 0; i < 4; ++i) result |= unsigned(bytes.at(offset + i)) << (i * 8);
	return result;
}
void Write(std::ostream& output, unsigned value, unsigned count)
{
	for (unsigned i = 0; i < count; ++i) output.put(static_cast<char>(value >> (8 * i)));
}
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		std::cerr << "Usage: KyaMusicRender LEVELIOP.BNK song-index seconds output.wav\n";
		return 1;
	}
	try {
		const unsigned selected = std::stoul(argv[2]), seconds = std::stoul(argv[3]);
		if (!seconds || seconds > 600) throw std::runtime_error("duration must be 1..600 seconds");
		std::ifstream input(argv[1], std::ios::binary);
		if (!input) throw std::runtime_error("cannot open archive");
		std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
		if (Word(bytes, 8) != 0x42414e4b || (Word(bytes, 12) & 1)) throw std::runtime_error("expected an uncompressed Eden archive");
		const unsigned count = Word(bytes, 40), table = Word(bytes, 48), types = Word(bytes, 52);
		if (count > bytes.size() / 16) throw std::runtime_error("invalid entry count");
		std::vector<std::uint8_t> body;
		auto bank = std::make_shared<Audio::MusicBank>();
		auto song = std::make_shared<Audio::MusicSong>();
		unsigned songIndex = 0, bankCount = 0; bool found = false;
		for (unsigned i = 0; i < count; ++i) {
			const auto type = Word(bytes, 8ull + types + i * 4ull);
			if (type != 0x30005 && type != 0x30006 && type != 0x30007) continue;
			const auto offset = Word(bytes, 8ull + table + i * 16ull), size = Word(bytes, 12ull + table + i * 16ull);
			if (offset > bytes.size() || size > bytes.size() - offset) throw std::runtime_error("entry is out of bounds");
			const std::span<const std::uint8_t> payload(bytes.data() + offset, size);
			std::string error;
			if (type == 0x30006) body.assign(payload.begin(), payload.end());
			if (type == 0x30007) {
				if (++bankCount != 1) throw std::runtime_error("tool requires a single-bank level");
				if (!Audio::ParseMusicBank(payload, body, *bank, error)) throw std::runtime_error(error);
			}
			if (type == 0x30005 && songIndex++ == selected) {
				if (!Audio::ParseMusicSong(payload, *song, error)) throw std::runtime_error(error);
				found = true;
			}
		}
		if (!found || bankCount != 1) throw std::runtime_error("song or bank is missing");
		Audio::MusicSynth synth(song, bank, 24);
		std::ofstream output(argv[4], std::ios::binary);
		if (!output) throw std::runtime_error("cannot create WAV");
		const unsigned dataSize = seconds * Audio::MusicSampleRate * 4;
		output.write("RIFF", 4); Write(output, 36 + dataSize, 4); output.write("WAVEfmt ", 8);
		Write(output, 16, 4); Write(output, 1, 2); Write(output, 2, 2); Write(output, Audio::MusicSampleRate, 4);
		Write(output, Audio::MusicSampleRate * 4, 4); Write(output, 4, 2); Write(output, 16, 2);
		output.write("data", 4); Write(output, dataSize, 4);
		std::vector<float> pcm(Audio::MusicSampleRate * 2);
		double energy = 0;
		for (unsigned second = 0; second < seconds; ++second) {
			synth.Render(pcm);
			for (float sample : pcm) {
				energy += sample * sample;
				Write(output, static_cast<std::uint16_t>(static_cast<std::int16_t>(sample * 32767)), 2);
			}
		}
		if (!output) throw std::runtime_error("failed to write WAV");
		std::cout << "Rendered " << song->events.size() << " events, " << song->ppqn << " PPQN, "
			<< seconds << " seconds; mean square amplitude " << energy / (seconds * Audio::MusicSampleRate * 2) << '\n';
		return 0;
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n'; return 1;
	}
}
