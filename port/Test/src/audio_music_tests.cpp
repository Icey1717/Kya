#include <gtest/gtest.h>
#include "edMusicData.h"
#include "edMusicSynth.h"
#include "edMusicService.h"
#include "edSysTransferService.h"
#include "edMusic/edMusic.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <chrono>
#include <thread>

namespace
{
void Word(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value)
{
	for (unsigned i = 0; i < 4; ++i) bytes.at(offset + i) = static_cast<std::uint8_t>(value >> (8 * i));
}
std::uint32_t ReadWord(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
	std::uint32_t value = 0;
	for (unsigned i = 0; i < 4; ++i) value |= std::uint32_t(bytes.at(offset + i)) << (8 * i);
	return value;
}
std::vector<std::uint8_t> SongBytes()
{
	std::vector<std::uint8_t> bytes(92);
	std::copy_n("IECSsreV", 8, bytes.begin()); Word(bytes, 8, 16);
	std::copy_n("IECSuqeS", 8, bytes.begin() + 16); Word(bytes, 24, 32); Word(bytes, 36, 48);
	std::copy_n("IECSidiM", 8, bytes.begin() + 48); Word(bytes, 56, 44); Word(bytes, 64, 20);
	Word(bytes, 68, 6); bytes[72] = 96;
	const std::uint8_t events[] = {0, 0xc0, 0x80, 0x90, 60, 100, 96, 0x80, 60, 0, 0xff, 0x2f};
	std::copy(std::begin(events), std::end(events), bytes.begin() + 74);
	return bytes;
}
std::shared_ptr<Audio::MusicBank> TestBank()
{
	auto bank = std::make_shared<Audio::MusicBank>();
	Audio::MusicSample sample;
	sample.rate = Audio::MusicSampleRate;
	for (int i = 0; i < 64; ++i) sample.pcm.push_back(static_cast<std::int16_t>(std::sin(i * 6.283185307 / 64) * 8000));
	sample.loopEnd = 64;
	bank->samples.push_back(std::move(sample));
	Audio::MusicRegion region;
	region.adsr1 = 0x000f; region.adsr2 = 0x1fc0;
	bank->programs[0].regions.push_back(region);
	return bank;
}

std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>> BankBytes()
{
	std::vector<std::uint8_t> header(272), body(32, 0x77);
	body[0] = body[16] = 8; body[1] = 4; body[17] = 3;
	std::copy_n("IECSsreV", 8, header.begin()); Word(header, 8, 16);
	std::copy_n("IECSdaeH", 8, header.begin() + 16); Word(header, 24, 64);
	Word(header, 28, 272); Word(header, 32, 32);
	Word(header, 36, 196); Word(header, 40, 170); Word(header, 44, 108); Word(header, 48, 80);
	auto chunk = [&](unsigned offset, const char* tag, unsigned size) {
		std::copy_n(tag, 8, header.begin() + offset); Word(header, offset + 8, size); Word(header, offset + 16, 20);
	};
	chunk(80, "IECSigaV", 28); header[104] = 0x80; header[105] = 0xbb; header[106] = 1;
	chunk(108, "IECSlpmS", 62);
	header[132] = 127; header[139] = 60; header[141] = 64; header[144] = 127;
	header[146] = 15; header[148] = 0xc0; header[149] = 0x1f;
	chunk(170, "IECStesS", 26); header[192] = 127; header[193] = 1;
	chunk(196, "IECSgorP", 76); Word(header, 216, 36);
	header[220] = 1; header[221] = 20; header[222] = 127; header[223] = 64;
	header[256] = 127; header[259] = 1; header[261] = 1; header[268] = 127; header[269] = 64;
	return {header, body};
}
std::uint32_t Transfer(const std::vector<std::uint8_t>& bytes)
{
	std::uint32_t handle = 0;
	Audio::Submit(bytes.data(), static_cast<unsigned>(bytes.size()), 4, Audio::TransferFlags::KeepInIop, 4, 5, 6, &handle, 4, 0, nullptr);
	Audio::PumpAll(); return handle;
}
struct OutputState
{
	unsigned queued = 0, submitted = 0;
	double energy = 0;
	bool playing = false, destroyed = false;
};
std::vector<std::shared_ptr<OutputState>> outputs;
class FakeOutput final : public Audio::MusicOutput
{
	std::shared_ptr<OutputState> state;
public:
	explicit FakeOutput(std::shared_ptr<OutputState> data) : state(std::move(data)) {}
	~FakeOutput() override { state->destroyed = true; }
	bool Submit(const float* pcm, unsigned count) override {
		if (count != 960) return false;
		for (unsigned i = 0; i < count; ++i) state->energy += pcm[i] * pcm[i];
		++state->queued; ++state->submitted; return true;
	}
	unsigned Queued() override { return state->queued; }
	bool Play(bool playing) override { state->playing = playing; return true; }
};
std::unique_ptr<Audio::MusicOutput> CreateOutput()
{
	auto state = std::make_shared<OutputState>(); outputs.push_back(state);
	return std::make_unique<FakeOutput>(state);
}
class MusicServiceTest : public testing::Test
{
protected:
	void SetUp() override
	{
		Audio::Reset(); outputs.clear(); Audio::SetMusicOutputFactory(CreateOutput); Audio::InitializeMusic(2, 24);
		const auto [header, body] = BankBytes();
		ASSERT_TRUE(Audio::InstallMusicBank(5, Transfer(header), Transfer(body)));
		ASSERT_TRUE(Audio::InstallMusicSong(7, Transfer(SongBytes())));
	}
	void TearDown() override { Audio::SetMusicOutputFactory(nullptr); Audio::Reset(); outputs.clear(); }
	void Start()
	{
		Audio::QueueMusicCommand({Audio::MusicCommandType::Song, 0, 7});
		Audio::QueueMusicCommand({Audio::MusicCommandType::Bank, 0, 5});
		Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
		Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2));
		Audio::ServiceMusic();
	}
};
}

TEST(AudioMusic, ParsesCompressedEventTimingAndRejectsTruncation)
{
	auto bytes = SongBytes(); Audio::MusicSong song; std::string error;
	ASSERT_TRUE(Audio::ParseMusicSong(bytes, song, error)) << error;
	ASSERT_EQ(song.events.size(), 3u);
	EXPECT_EQ(song.events[1].tick, 0u); EXPECT_EQ(song.events[2].tick, 96u);
	EXPECT_EQ(song.endTick, 96u);
	for (std::size_t size = 0; size < 86; ++size) {
		EXPECT_FALSE(Audio::ParseMusicSong({bytes.data(), size}, song, error));
		EXPECT_TRUE(song.events.empty());
	}
	bytes[79] = 0xff;
	EXPECT_FALSE(Audio::ParseMusicSong(bytes, song, error));
}

TEST(AudioMusic, RenderIsIndependentOfChunkSizeAndHonorsGainAndRestart)
{
	auto song = std::make_shared<Audio::MusicSong>(); std::string error;
	ASSERT_TRUE(Audio::ParseMusicSong(SongBytes(), *song, error));
	auto bank = TestBank(); Audio::MusicSynth first(song, bank, 32), second(song, bank, 32);
	std::vector<float> whole(60000), pieces(60000);
	first.Render(whole);
	for (std::size_t offset = 0; offset < pieces.size(); offset += 200) second.Render({pieces.data() + offset, 200});
	EXPECT_EQ(whole, pieces);
	EXPECT_TRUE(first.Finished());
	EXPECT_GT(std::accumulate(whole.begin(), whole.end(), 0.0, [](double sum, float x) { return sum + std::abs(x); }), 1);
	first.Restart(); Audio::MusicControls muted; muted.channels[0] = 0; first.SetControls(muted);
	first.Render(whole); EXPECT_TRUE(std::all_of(whole.begin(), whole.end(), [](float x) { return x == 0; }));
	first.Restart(); Audio::MusicControls looping; looping.looping = true; first.SetControls(looping);
	first.Render(whole); EXPECT_FALSE(first.Finished()); EXPECT_EQ(first.Frames(), 30000u);
}

TEST(AudioMusic, OriginalIopVolumeMapping)
{
	EXPECT_EQ(Audio::MusicVolumeByte(65535), 255u);
	EXPECT_EQ(Audio::MusicVolumeByte(32768), 127u);
	EXPECT_EQ(Audio::MusicVolumeByte(256), 0u);
	EXPECT_EQ(Audio::MusicVolumeByte(257), 1u);
	EXPECT_EQ(Audio::MusicMidiVolume(64, 128, 128), 64u);
	EXPECT_EQ(Audio::MusicMidiVolume(64, 255, 128), 127u);
	EXPECT_EQ(Audio::MusicMidiVolume(100, 127, 128), 99u);
	EXPECT_EQ(Audio::MusicMidiVolume(100, 64, 64), 25u);
	EXPECT_EQ(Audio::MusicMidiVolume(127, 255, 255), 127u);
	EXPECT_EQ(Audio::MusicMidiVolume(127, 0, 128), 0u);
	EXPECT_EQ(Audio::MusicMidiVolume(127, 255, 0), 0u);

	auto song = std::make_shared<Audio::MusicSong>(); std::string error;
	ASSERT_TRUE(Audio::ParseMusicSong(SongBytes(), *song, error));
	song->events.insert(song->events.begin(), {0, 0xb0, 7, 64});
	Audio::MusicSynth synth(song, TestBank(), 32);
	auto render = [&](unsigned stream, float master) {
		Audio::MusicControls controls; controls.streamVolume = stream; controls.volume = master;
		synth.Restart(); synth.SetControls(controls);
		std::vector<float> pcm(2000); synth.Render(pcm); return pcm;
	};
	const auto unity = render(128, 1), full = render(255, 1), halfMaster = render(255, 0.5f), muted = render(0, 1);
	for (unsigned i = 0; i < unity.size(); ++i) {
		EXPECT_NEAR(full[i], unity[i] * (127.0f / 64), 0.000001f);
		EXPECT_FLOAT_EQ(halfMaster[i], full[i] * 0.5f);
		EXPECT_EQ(muted[i], 0);
	}
}

TEST(AudioMusic, MusicTransfersRetainOwnedBytes)
{
	Audio::Reset(); std::uint32_t handle = 0;
	auto bytes = SongBytes();
	const auto transfer = Audio::Submit(bytes.data(), static_cast<unsigned>(bytes.size()), 4, Audio::TransferFlags::KeepInIop, 4, 5, 6, &handle, 4, 0, nullptr);
	bytes.clear(); Audio::PumpThrough(transfer);
	Audio::LoadedDataInfo data; ASSERT_TRUE(Audio::LookupLoadedData(handle, data));
	EXPECT_EQ(data.size, 92u);
	ASSERT_TRUE(Audio::InstallMusicSong(7, handle));
	EXPECT_TRUE(Audio::ReleaseLoadedData(handle));
	Audio::RemoveMusicSong(7); Audio::Reset();
}

TEST(AudioMusic, BankRejectsTruncationAndBadReferences)
{
	auto [header, body] = BankBytes(); Audio::MusicBank bank; std::string error;
	ASSERT_TRUE(Audio::ParseMusicBank(header, body, bank, error)) << error;
	ASSERT_EQ(bank.samples.size(), 1u); EXPECT_EQ(bank.samples[0].loopEnd, 56u);
	for (std::size_t size = 0; size < header.size(); ++size) {
		EXPECT_FALSE(Audio::ParseMusicBank({header.data(), size}, body, bank, error)); EXPECT_TRUE(bank.samples.empty());
	}
	header[194] = 127;
	EXPECT_FALSE(Audio::ParseMusicBank(header, body, bank, error));
}

TEST_F(MusicServiceTest, PauseResumeReplacementAndUnload)
{
	Start(); ASSERT_EQ(outputs.size(), 1u); EXPECT_EQ(outputs[0]->queued, 3u);
	Audio::QueueMusicCommand({Audio::MusicCommandType::Pause, 0});
	Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2));
	Audio::ServiceMusic(); EXPECT_FALSE(outputs[0]->playing); EXPECT_EQ(outputs[0]->submitted, 3u);
	Audio::QueueMusicCommand({Audio::MusicCommandType::Resume, 0});
	Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2)); EXPECT_TRUE(outputs[0]->playing); EXPECT_EQ(outputs.size(), 1u);
	Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
	Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2));
	EXPECT_TRUE(outputs[0]->destroyed); ASSERT_EQ(outputs.size(), 2u);
	Audio::RemoveMusicBank(5); EXPECT_TRUE(outputs[1]->destroyed);
	EXPECT_TRUE(Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2)).empty());
}

TEST_F(MusicServiceTest, CompletionIsOnceAndReplacementCancelsStaleCompletion)
{
	Start();
	for (unsigned i = 0; i < 30 && !outputs.back()->destroyed; ++i) {
		outputs.back()->queued = 0; Audio::ServiceMusic();
	}
	EXPECT_TRUE(outputs.back()->destroyed);
	EXPECT_EQ(Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2)), std::vector<unsigned>{0});
	EXPECT_TRUE(Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2)).empty());
	Start();
	for (unsigned i = 0; i < 30 && !outputs.back()->destroyed; ++i) { outputs.back()->queued = 0; Audio::ServiceMusic(); }
	Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
	EXPECT_TRUE(Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2)).empty());
}

TEST_F(MusicServiceTest, UnloadCancelsQueuedReplacementAndSlotReuse)
{
	Start();
	auto [header, body] = BankBytes();
	ASSERT_TRUE(Audio::InstallMusicBank(6, Transfer(header), Transfer(body)));
	Audio::QueueMusicCommand({Audio::MusicCommandType::Bank, 0, 6});
	Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
	Audio::RemoveMusicBank(6);
	ASSERT_TRUE(Audio::InstallMusicBank(6, Transfer(header), Transfer(body)));
	EXPECT_TRUE(Audio::FlushMusicCommands(std::vector<Audio::MusicControls>(2)).empty());
	EXPECT_EQ(outputs.size(), 1u); EXPECT_TRUE(outputs[0]->destroyed);
}

TEST_F(MusicServiceTest, ChannelVolumePersistsUntilSongReinitialization)
{
	Audio::QueueMusicCommand({Audio::MusicCommandType::Song, 0, 7});
	Audio::QueueMusicCommand({Audio::MusicCommandType::Bank, 0, 5});
	Audio::QueueMusicCommand({Audio::MusicCommandType::ChannelVolume, 0, 0, 0});
	Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
	const std::vector<Audio::MusicControls> controls(2);
	Audio::FlushMusicCommands(controls); Audio::ServiceMusic();
	ASSERT_EQ(outputs.size(), 1u); EXPECT_EQ(outputs[0]->energy, 0);
	outputs[0]->queued = 0;
	Audio::FlushMusicCommands(controls); Audio::ServiceMusic();
	EXPECT_EQ(outputs[0]->energy, 0);
	Audio::QueueMusicCommand({Audio::MusicCommandType::Song, 0, 7});
	Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
	Audio::FlushMusicCommands(controls); Audio::ServiceMusic();
	ASSERT_EQ(outputs.size(), 2u); EXPECT_GT(outputs[1]->energy, 0);
}

TEST_F(MusicServiceTest, EdenInstallPlaybackFadeAndRemoval)
{
	const auto oldStatus = edMusicStatus;
	auto* oldStreams = pedMusicStreams; auto* oldSongs = _pedMusicSongs; auto* oldBanks = _pedMusicBanks;
	ed_music_stream streams[2]{}; ed_music_song songs[2]{}; _ed_music_bank banks[2]{};
	pedMusicStreams = streams; _pedMusicSongs = songs; _pedMusicBanks = banks;
	edMusicStatus = {}; edMusicStatus.nbStreams = edMusicStatus.nbSongs = edMusicStatus.nbBanks = 2;
	edMusicStatus.masterVolume = 65535; edMusicStatus.masterTempo = 256;
	for (unsigned i = 0; i < 2; ++i) {
		streams[i].index = i; streams[i].field_0xc = 1; streams[i].volume = 65535; streams[i].tempo = 256;
		std::fill_n(streams[i].aChannelVolumes, 16, 65535);
		songs[i].flags = banks[i].flags = 1;
	}
	// Restore global Eden state even if an assertion aborts this test body.
	auto restore = std::shared_ptr<void>(nullptr, [&](void*) {
		Audio::PumpAll(); Audio::ResetMusic();
		edMusicStatus = oldStatus; pedMusicStreams = oldStreams; _pedMusicSongs = oldSongs; _pedMusicBanks = oldBanks;
	});
	auto [header, body] = BankBytes(); auto song = SongBytes();
	EXPECT_EQ(edMusicBankInstallNoWait(body.data(), header.data(), static_cast<uint>(body.size()), static_cast<uint>(header.size())), 0u);
	EXPECT_EQ(edMusicSongInstallNoWait(song.data(), static_cast<uint>(song.size())), 0u);
	EXPECT_TRUE(edMusicAreAllMusicDataLoaded());
	EXPECT_EQ(banks[0].flags & 6, 4u); EXPECT_EQ(songs[0].flags & 6, 4u);
	const auto songHandle = static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(songs[0].field_0x0));
	edMusicWinStreamSetSong(0, 0); edMusicWinStreamSetBank(0, 0); edMusicWinStreamPlay(0, 0);
	edMusicFlush(); Audio::ServiceMusic(); ASSERT_EQ(outputs.size(), 1u);
	EXPECT_TRUE(outputs[0]->playing);
	streams[0].fadeFlags |= 1; streams[0].volumeTempoFadeTime = 0;
	streams[0].targetVolume = 0; streams[0].targetTempo = 128;
	edMusicFlush(); EXPECT_EQ(streams[0].volume, 0u); EXPECT_EQ(streams[0].tempo, 128u); EXPECT_FALSE(outputs[0]->playing);
	edMusicStopAllSongs(); edMusicFlush(); EXPECT_TRUE(outputs[0]->destroyed);
	edMusicSongRemove(0); edMusicBankRemove(0);
	Audio::LoadedDataInfo data; EXPECT_FALSE(Audio::LookupLoadedData(songHandle, data));
}

TEST(AudioMusic, DISABLED_XAudio2CompletionAndShutdown)
{
	Audio::Reset(); Audio::SetMusicOutputFactory(nullptr); Audio::InitializeMusic(1, 24);
	auto cleanup = std::shared_ptr<void>(nullptr, [](void*) { Audio::Shutdown(); });
	auto [header, body] = BankBytes();
	ASSERT_TRUE(Audio::InstallMusicBank(0, Transfer(header), Transfer(body)));
	ASSERT_TRUE(Audio::InstallMusicSong(0, Transfer(SongBytes())));
	Audio::QueueMusicCommand({Audio::MusicCommandType::Song, 0, 0});
	Audio::QueueMusicCommand({Audio::MusicCommandType::Bank, 0, 0});
	Audio::QueueMusicCommand({Audio::MusicCommandType::Restart, 0});
	std::vector<Audio::MusicControls> controls(1); controls[0].volume = 0;
	const auto begin = std::chrono::steady_clock::now();
	bool finished = false;
	while (std::chrono::steady_clock::now() - begin < std::chrono::seconds(3)) {
		if (!Audio::FlushMusicCommands(controls).empty()) { finished = true; break; }
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	EXPECT_TRUE(finished);
	// An immediate failure notification must not pass as real playback completion.
	EXPECT_GE(std::chrono::steady_clock::now() - begin, std::chrono::milliseconds(450));
}

// Local game assets stay out of the test fixtures and repository history.
TEST(AudioMusic, DISABLED_ParseAndRenderInstalledLevelAssets)
{
	const auto root = std::filesystem::path(TEST_DATA_DIRECTORY).parent_path().parent_path().parent_path().parent_path() / "assets/CDEURO/LEVEL";
	unsigned songs = 0, banks = 0, mappedNotes = 0, unmappedNotes = 0;
	for (const auto& directory : std::filesystem::directory_iterator(root)) {
		const auto path = directory.path() / "LEVELIOP.BNK";
		if (!std::filesystem::exists(path)) continue;
		std::ifstream input(path, std::ios::binary);
		std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
		const auto count = ReadWord(bytes, 40), table = ReadWord(bytes, 48), types = ReadWord(bytes, 52);
		std::vector<std::uint8_t> body; std::shared_ptr<Audio::MusicBank> bank;
		for (unsigned i = 0; i < count; ++i) {
			const auto type = ReadWord(bytes, 8 + types + i * 4);
			if (type != 0x30005 && type != 0x30006 && type != 0x30007) continue;
			const auto offset = ReadWord(bytes, 8 + table + i * 16), size = ReadWord(bytes, 12 + table + i * 16);
			ASSERT_LE(static_cast<std::size_t>(offset) + size, bytes.size());
			const std::span<const std::uint8_t> payload(bytes.data() + offset, size);
			std::string error;
			if (type == 0x30006) body.assign(payload.begin(), payload.end());
			if (type == 0x30007) {
				bank = std::make_shared<Audio::MusicBank>();
				ASSERT_TRUE(Audio::ParseMusicBank(payload, body, *bank, error)) << path << ": " << error;
				++banks;
			}
			if (type == 0x30005) {
				auto song = std::make_shared<Audio::MusicSong>();
				ASSERT_TRUE(Audio::ParseMusicSong(payload, *song, error)) << path << ": " << error;
				ASSERT_NE(bank, nullptr);
				std::array<unsigned, 16> programs{};
				unsigned songMappedNotes = 0;
				for (const auto& event : song->events) {
					const unsigned channel = event.status & 15;
					if ((event.status >> 4) == 12) programs[channel] = event.first;
					if ((event.status >> 4) == 9 && event.second != 0) {
						const auto& regions = bank->programs[programs[channel]].regions;
						const bool mapped = std::any_of(regions.begin(), regions.end(), [&](const auto& region) {
							return event.first >= region.keyLow && event.first <= region.keyHigh && event.second >= region.velocityLow && event.second <= region.velocityHigh;
						});
						// Sparse banks can leave keys/programs unassigned. Count
						// these for reference comparison; never substitute another instrument.
						if (mapped) { ++mappedNotes; ++songMappedNotes; } else ++unmappedNotes;
					}
				}
				EXPECT_GT(songMappedNotes, 0u) << path << " song " << i;
				Audio::MusicSynth synth(song, bank, 32);
				std::vector<float> pcm(Audio::MusicSampleRate * 2);
				synth.Render(pcm);
				EXPECT_TRUE(std::all_of(pcm.begin(), pcm.end(), [](float x) { return std::isfinite(x); }));
				++songs;
			}
		}
	}
	EXPECT_GT(songs, 0u); EXPECT_GT(banks, 0u);
	std::cout << "Parsed " << songs << " songs and " << banks << " banks; " << mappedNotes
		<< " mapped notes, " << unmappedNotes << " notes outside defined regions.\n";
}
