#include "edMusicData.h"
#include "edSoundStreamService.h"

#include <algorithm>
#include <stdexcept>

namespace Audio
{
namespace
{
class Reader
{
public:
	std::span<const std::uint8_t> bytes;
	explicit Reader(std::span<const std::uint8_t> data) : bytes(data) {}
	void Check(std::size_t offset, std::size_t count) const
	{
		if (offset > bytes.size() || count > bytes.size() - offset)
			throw std::runtime_error("out of bounds at byte " + std::to_string(offset));
	}
	unsigned U8(std::size_t offset) const { Check(offset, 1); return bytes[offset]; }
	int S8(std::size_t offset) const { return static_cast<std::int8_t>(U8(offset)); }
	unsigned U16(std::size_t offset) const { return U8(offset) | (U8(offset + 1) << 8); }
	std::uint32_t U32(std::size_t offset) const { return U16(offset) | (U16(offset + 2) << 16); }
	void Tag(std::size_t offset, const char* tag) const
	{
		Check(offset, 8);
		if (!std::equal(bytes.begin() + offset, bytes.begin() + offset + 8, tag))
			throw std::runtime_error("unexpected chunk at byte " + std::to_string(offset));
	}
	Reader Chunk(std::size_t offset, const char* tag) const
	{
		Tag(offset, tag);
		const auto size = U32(offset + 8);
		if (size < 16) throw std::runtime_error("short chunk");
		Check(offset, size);
		return Reader(bytes.subspan(offset, size));
	}
	std::vector<std::size_t> Table() const
	{
		const auto maximum = U32(12);
		if (maximum > 4095) throw std::runtime_error("excessive table size");
		Check(16, (maximum + 1) * 4);
		std::vector<std::size_t> result;
		for (unsigned i = 0; i <= maximum; ++i) {
			const auto offset = U32(16 + i * 4);
			if (offset != UINT32_MAX) Check(offset, 1);
			result.push_back(offset);
		}
		return result;
	}
};

float Pan(unsigned value)
{
	return (value <= 127 ? static_cast<int>(value) - 64 : 191 - static_cast<int>(value)) / 64.0f;
}
}

bool ParseMusicSong(std::span<const std::uint8_t> bytes, MusicSong& out, std::string& error)
{
	out = {};
	error.clear();
	try {
		Reader file(bytes);
		file.Tag(0, "IECSsreV");
		file.Tag(16, "IECSuqeS");
		Reader midi = file.Chunk(file.U32(36), "IECSidiM");
		if (midi.U32(12) != 0) throw std::runtime_error("multiple MIDI blocks are unsupported");
		const auto block = midi.U32(16);
		const auto relative = midi.U32(block);
		out.ppqn = midi.U16(block + 4);
		if (!out.ppqn || relative != 6) throw std::runtime_error("unsupported MIDI timing/compression header");
		std::size_t cursor = static_cast<std::size_t>(block) + relative;
		auto byte = [&]() { return midi.U8(cursor++); };
		auto variable = [&]() {
			std::uint32_t value = 0;
			for (int i = 0; i < 4; ++i) {
				const auto next = byte();
				value = (value << 7) | (next & 127);
				if (!(next & 128)) return value;
			}
			throw std::runtime_error("overlong event delta");
		};
		bool skipDelta = false;
		unsigned running = 0;
		std::uint64_t tick = 0;
		while (cursor < midi.bytes.size()) {
			if (!skipDelta) tick += variable();
			skipDelta = false;
			unsigned status = byte();
			if (status < 128) { --cursor; status = running; }
			else if (status != 255) running = status;
			MusicEvent event{};
			event.tick = tick;
			event.status = static_cast<std::uint8_t>(status);
			const auto kind = status >> 4;
			if (kind == 8 || kind == 9 || kind == 11 || kind == 12 || kind == 14) {
				event.first = static_cast<std::uint8_t>(byte());
				unsigned last = event.first;
				if (kind == 9 || kind == 11 || kind == 14) last = byte();
				skipDelta = (last & 128) != 0;
				if (kind == 8 || kind == 12) event.first = last & 127;
				else event.second = last & 127;
				if (event.first > 127) throw std::runtime_error("invalid MIDI data byte");
				if (kind == 11 && event.first != 1 && event.first != 7 && event.first != 10 && event.first != 11 && event.first != 64 && event.first != 120 && event.first != 123)
					throw std::runtime_error("unsupported controller " + std::to_string(event.first));
			} else if (status == 255) {
				const unsigned meta = byte();
				if (meta == 47) {
					out.endTick = tick;
					if (!tick) throw std::runtime_error("zero-length song");
					return true;
				}
				if (meta != 81 || byte() != 3) throw std::runtime_error("unsupported meta event");
				const auto high = byte(), middle = byte(), low = byte();
				event.tempo = (high << 16) | (middle << 8) | low;
				if (!event.tempo) throw std::runtime_error("zero tempo");
			} else throw std::runtime_error("unsupported event " + std::to_string(status));
			out.events.push_back(event);
		}
		throw std::runtime_error("missing end-of-song event");
	} catch (const std::exception& exception) {
		error = exception.what();
		out = {};
		return false;
	}
}

bool ParseMusicBank(std::span<const std::uint8_t> header, std::span<const std::uint8_t> body,
	MusicBank& out, std::string& error)
{
	out = {};
	error.clear();
	try {
		Reader file(header);
		file.Tag(0, "IECSsreV");
		file.Tag(16, "IECSdaeH");
		if (file.U32(28) > header.size() || file.U32(32) > body.size()) throw std::runtime_error("short bank payload");
		Reader programs = file.Chunk(file.U32(36), "IECSgorP");
		Reader sets = file.Chunk(file.U32(40), "IECStesS");
		Reader samples = file.Chunk(file.U32(44), "IECSlpmS");
		Reader waves = file.Chunk(file.U32(48), "IECSigaV");
		const auto programTable = programs.Table(), setTable = sets.Table(), sampleTable = samples.Table(), waveTable = waves.Table();
		if (programTable.size() > out.programs.size()) throw std::runtime_error("unsupported program bank");
		out.samples.resize(waveTable.size());
		for (std::size_t i = 0; i < waveTable.size(); ++i) {
			const auto entry = waveTable[i];
			if (entry == UINT32_MAX) continue;
			std::size_t start = waves.U32(entry), end = file.U32(32);
			for (auto other : waveTable) {
				if (other == UINT32_MAX) continue;
				const auto candidate = waves.U32(other);
				if (candidate > start) end = std::min(end, static_cast<std::size_t>(candidate));
			}
			if (start >= end || end > body.size() || (end - start) % 16) throw std::runtime_error("invalid ADPCM range");
			auto& sample = out.samples[i];
			sample.rate = waves.U16(entry + 4);
			if (!sample.rate) throw std::runtime_error("zero sample rate");
			std::size_t loop = start;
			for (auto block = start; block < end; block += 16) {
				if (body[block + 1] & 4) loop = block;
				if (body[block + 1] & 1) { end = block + 16; break; }
			}
			if (!DecodeRawAdpcm(body.data() + start, end - start, sample.pcm)) throw std::runtime_error("invalid ADPCM data");
			if (waves.U8(entry + 6) == 1) {
				sample.loopBegin = static_cast<std::uint32_t>((loop - start) / 16 * 28);
				sample.loopEnd = static_cast<std::uint32_t>(sample.pcm.size());
			}
		}
		for (std::size_t i = 0; i < programTable.size(); ++i) {
			const auto program = programTable[i];
			if (program == UINT32_MAX) continue;
			programs.Check(program, 36);
			auto& instrument = out.programs[i];
			instrument.pitchWave = programs.U8(program + 14);
			instrument.ampWave = programs.U8(program + 15);
			if ((instrument.pitchWave != 0 && instrument.pitchWave != 3) || (instrument.ampWave != 0 && instrument.ampWave != 3))
				throw std::runtime_error("unsupported instrument LFO waveform");
			instrument.pitchCycle = std::max(1u, programs.U16(program + 20)) / 1000.0f;
			instrument.ampCycle = std::max(1u, programs.U16(program + 22)) / 1000.0f;
			instrument.pitchDepth = programs.U16(program + 24) / 128.0f;
			instrument.modulationDepth = programs.U16(program + 28) / 128.0f;
			instrument.ampDepth = programs.U8(program + 32) / 127.0f;
			const auto splitCount = programs.U8(program + 4), stride = programs.U8(program + 5);
			if (stride != 20) throw std::runtime_error("unsupported split stride");
			const std::size_t splitStart = program + programs.U32(program);
			programs.Check(splitStart, splitCount * stride);
			for (unsigned splitIndex = 0; splitIndex < splitCount; ++splitIndex) {
				const auto split = splitStart + splitIndex * stride;
				const auto setIndex = programs.U16(split);
				if (setIndex >= setTable.size() || setTable[setIndex] == UINT32_MAX) throw std::runtime_error("missing sample set");
				const auto set = setTable[setIndex];
				const auto count = sets.U8(set + 3);
				sets.Check(set + 4, count * 2);
				for (unsigned j = 0; j < count; ++j) {
					const auto sampleIndex = sets.U16(set + 4 + j * 2);
					if (sampleIndex >= sampleTable.size() || sampleTable[sampleIndex] == UINT32_MAX) throw std::runtime_error("missing sample parameters");
					const auto source = sampleTable[sampleIndex];
					samples.Check(source, 42);
					MusicRegion region;
					region.sample = samples.U16(source);
					if (region.sample >= out.samples.size() || out.samples[region.sample].pcm.empty()) throw std::runtime_error("missing instrument waveform");
					region.keyLow = programs.U8(split + 2);
					region.keyHigh = programs.U8(split + 4);
					if (region.keyHigh < region.keyLow) region.keyHigh = 127;
					region.velocityLow = std::max(sets.U8(set + 1), samples.U8(source + 2));
					region.velocityHigh = std::min(sets.U8(set + 2), samples.U8(source + 4));
					region.root = samples.U8(source + 11) - programs.S8(program + 8) - programs.S8(split + 18)
						- (programs.S8(program + 9) + programs.S8(split + 19) + samples.S8(source + 12)) / 100.0f;
					region.gain = (programs.U8(program + 6) / 127.0f) * (programs.U8(split + 16) / 127.0f) * (samples.U8(source + 16) / 127.0f);
					region.pan = Pan(programs.U8(program + 7)) + Pan(programs.U8(split + 17)) + Pan(samples.U8(source + 13));
					region.bendDown = programs.U16(split + 6) / 128.0f;
					region.bendUp = programs.U16(split + 8) / 128.0f;
					region.adsr1 = samples.U16(source + 18);
					region.adsr2 = samples.U16(source + 20);
					region.pitchDelay = samples.U16(source + 32) / 1000.0f;
					region.pitchFade = samples.U16(source + 34) / 1000.0f;
					region.ampDelay = samples.U16(source + 36) / 1000.0f;
					region.ampFade = samples.U16(source + 38) / 1000.0f;
					region.lfoFlags = samples.U8(source + 40);
					out.programs[i].regions.push_back(region);
				}
			}
		}
		return true;
	} catch (const std::exception& exception) {
		error = exception.what();
		out = {};
		return false;
	}
}
}
