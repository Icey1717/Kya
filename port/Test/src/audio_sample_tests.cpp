#include <gtest/gtest.h>
#include "edSoundSampleService.h"
#include "edSoundStreamService.h"
#include "edSysTransferService.h"
#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInstance.h"
#include "Actor.h"
#include "Audio.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <chrono>
#include <thread>

extern uint edSoundCurrentInstancesNumber;
extern uint edSoundNextFreeInstanceIndex;
extern ed_sound_instance* pedSoundInstanceListTail;
extern void CAudioManager_SoundFinishedInstancesCallback(ed_sound_instance_finished*, uint);
extern int edSoundLoadToSoundRamNoWaitDirect(void*, uint, long, ed_sound_sample*, void (*)(void*));
extern void SetFlag_00288910(void*);

namespace
{
struct VoiceState
{
	const Audio::DecodedSample* sample = nullptr;
	Audio::SampleControls controls{};
	bool playing = false;
	bool finished = false;
	bool destroyed = false;
	std::uint64_t frames = 0;
	std::vector<std::string> events;
};
std::vector<std::shared_ptr<VoiceState>> voices;
bool failVoiceCreation = false;

class TestVoice : public Audio::SampleVoice
{
	std::shared_ptr<VoiceState> state;
public:
	explicit TestVoice(std::shared_ptr<VoiceState> value) : state(std::move(value)) {}
	~TestVoice() override
	{
		// Also checks PCM lifetime on unload/reset (useful with ASan).
		EXPECT_FALSE(state->sample->pcm.empty());
		EXPECT_EQ(state->sample->pcm[0], 1);
		state->destroyed = true;
	}
	bool SetControls(const Audio::SampleControls& controls) override
	{
		state->controls = controls;
		state->events.push_back("controls");
		return true;
	}
	bool SetPlaying(bool playing) override
	{
		state->playing = playing;
		state->events.push_back(playing ? "start" : "pause");
		return true;
	}
	bool IsFinished() override { return state->finished; }
	std::uint64_t GetFramesPlayed() override { return state->frames; }
};

std::unique_ptr<Audio::SampleVoice> CreateTestVoice(const Audio::DecodedSample& sample)
{
	if (failVoiceCreation) return nullptr;
	auto state = std::make_shared<VoiceState>();
	state->sample = &sample;
	voices.push_back(state);
	return std::make_unique<TestVoice>(state);
}

class AudioSamples : public testing::Test
{
protected:
	std::array<std::uint8_t, 48> adpcm{};
	Audio::SampleDescription description{};
	void SetUp() override
	{
		Audio::Reset();
		voices.clear();
		failVoiceCreation = false;
		Audio::SetSampleVoiceFactory(CreateTestVoice);
		for (unsigned offset = 0; offset < adpcm.size(); offset += 16) {
			adpcm[offset] = 12;
			adpcm[offset + 2] = 1;
		}
		adpcm[33] = 1;
		std::uint32_t handle = 0;
		Audio::Submit(adpcm.data(), static_cast<std::uint32_t>(adpcm.size()), 64, Audio::TransferFlags::None,
			1, 2, 3, &handle, 4, 0, nullptr);
		Audio::PumpAll();
		description = {handle, 22050, 48, 16, 32, false};
	}
	void TearDown() override
	{
		Audio::SetSampleVoiceFactory(nullptr);
		Audio::Reset();
		voices.clear();
	}
	void Play(std::uint32_t id)
	{
		Audio::QueueSampleCommand({Audio::SampleCommandType::Create, id, description, {0.25f, 0.75f, 1.5f}});
		Audio::QueueSampleCommand({Audio::SampleCommandType::Start, id});
		Audio::FlushSampleCommands();
	}
};

TEST_F(AudioSamples, DecodesSignedNibblesAndInclusiveLoopEnd)
{
	adpcm[2] = 0xf1;
	description.looping = true;
	Audio::DecodedSample decoded;
	ASSERT_TRUE(Audio::DecodeSample(adpcm.data(), adpcm.size(), description, decoded));
	EXPECT_EQ(decoded.pcm.size(), 84u);
	EXPECT_EQ(decoded.pcm[0], 1);
	EXPECT_EQ(decoded.pcm[1], -1);
	EXPECT_EQ(decoded.loopBegin, 28u);
	EXPECT_EQ(decoded.loopLength, 56u);
	description.looping = false;
	ASSERT_TRUE(Audio::DecodeSample(adpcm.data(), adpcm.size(), description, decoded));
	EXPECT_EQ(decoded.loopLength, 0u);
}

TEST_F(AudioSamples, RejectsMalformedDataAndInvalidLoops)
{
	Audio::DecodedSample decoded;
	EXPECT_FALSE(Audio::DecodeSample(adpcm.data(), 47, description, decoded));
	adpcm[16] = 0x5c;
	EXPECT_FALSE(Audio::DecodeSample(adpcm.data(), adpcm.size(), description, decoded));
	adpcm[16] = 12;
	description.loopEndOffset = std::numeric_limits<float>::quiet_NaN();
	EXPECT_FALSE(Audio::DecodeSample(adpcm.data(), adpcm.size(), description, decoded));
	description.loopEndOffset = 32;
	description.loopStartOffset = 48;
	description.looping = true;
	EXPECT_FALSE(Audio::DecodeSample(adpcm.data(), adpcm.size(), description, decoded));
}

TEST_F(AudioSamples, SharesPcmAndRetainsItUntilVoicesAreDestroyed)
{
	Play(0x10000);
	Play(0x10001);
	ASSERT_EQ(voices.size(), 2u);
	EXPECT_EQ(voices[0]->sample, voices[1]->sample);
	EXPECT_EQ(voices[0]->events, (std::vector<std::string>{"controls", "start"}));
	EXPECT_TRUE(Audio::ReleaseLoadedData(description.handle));
	EXPECT_FALSE(voices[0]->destroyed);
	Audio::DestroySample(0x10000);
	EXPECT_TRUE(voices[0]->destroyed);
	EXPECT_FALSE(voices[1]->destroyed);
	voices[1]->finished = true;
	EXPECT_EQ(Audio::PollFinishedSamples(), (std::vector<std::uint32_t>{0x10001}));
	EXPECT_TRUE(voices[1]->destroyed);
	EXPECT_TRUE(Audio::PollFinishedSamples().empty());
}

TEST_F(AudioSamples, PausesUpdatesAndWrapsLoopPosition)
{
	description.looping = true;
	Play(0x10000);
	ASSERT_EQ(voices.size(), 1u);
	Audio::QueueSampleCommand({Audio::SampleCommandType::Pause, 0x10000});
	Audio::QueueSampleCommand({Audio::SampleCommandType::Update, 0x10000, {}, {0.1f, 0.2f, 0.5f}});
	Audio::FlushSampleCommands();
	EXPECT_FALSE(voices[0]->playing);
	EXPECT_FLOAT_EQ(voices[0]->controls.pitch, 0.5f);
	EXPECT_TRUE(Audio::PollFinishedSamples().empty());
	voices[0]->frames = 112;
	std::uint32_t offset = 0;
	ASSERT_TRUE(Audio::GetSamplePosition(0x10000, offset));
	EXPECT_EQ(offset, 32u);
	Audio::QueueSampleCommand({Audio::SampleCommandType::Resume, 0x10000});
	Audio::FlushSampleCommands();
	EXPECT_TRUE(voices[0]->playing);
}

TEST_F(AudioSamples, StopAndResetDiscardPendingWorkAndFailures)
{
	failVoiceCreation = true;
	Play(0x10000);
	EXPECT_EQ(Audio::PollFinishedSamples(), (std::vector<std::uint32_t>{0x10000}));
	EXPECT_TRUE(Audio::PollFinishedSamples().empty());
	failVoiceCreation = false;
	Play(0x20000);
	Audio::DestroySample(0x10000);
	ASSERT_EQ(voices.size(), 1u);
	EXPECT_FALSE(voices[0]->destroyed);
	Audio::QueueSampleCommand({Audio::SampleCommandType::Create, 0x30000, description});
	Audio::Reset();
	Audio::FlushSampleCommands();
	EXPECT_EQ(voices.size(), 1u);
	EXPECT_TRUE(voices[0]->destroyed);
	EXPECT_TRUE(Audio::PollFinishedSamples().empty());
}

// Opt-in device smoke test: real XAudio2, with a silent output matrix.
TEST_F(AudioSamples, DISABLED_XAudio2PlaysCompletesLoopsAndStops)
{
	Audio::SetSampleVoiceFactory(nullptr);
	Audio::QueueSampleCommand({Audio::SampleCommandType::Create, 0x10000, description, {0, 0, 1}});
	Audio::QueueSampleCommand({Audio::SampleCommandType::Start, 0x10000});
	Audio::FlushSampleCommands();
	std::uint32_t position = 0;
	ASSERT_TRUE(Audio::GetSamplePosition(0x10000, position)) << "XAudio2 could not create the sample voice";
	std::vector<std::uint32_t> finished;
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
	while (finished.empty() && std::chrono::steady_clock::now() < deadline) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		finished = Audio::PollFinishedSamples();
	}
	EXPECT_EQ(finished, (std::vector<std::uint32_t>{0x10000}));
	description.looping = true;
	Audio::QueueSampleCommand({Audio::SampleCommandType::Create, 0x20000, description, {0, 0, 1}});
	Audio::QueueSampleCommand({Audio::SampleCommandType::Start, 0x20000});
	Audio::FlushSampleCommands();
	ASSERT_TRUE(Audio::GetSamplePosition(0x20000, position));
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(Audio::PollFinishedSamples().empty());
	Audio::DestroySample(0x20000);
	EXPECT_FALSE(Audio::GetSamplePosition(0x20000, position));
	Audio::Shutdown();
}

class EdenAudioSamples : public AudioSamples
{
protected:
	std::array<ed_sound_instance, 64> edenInstances{};
	std::array<edSoundInstanceComType, 64> edenCommands{};
	std::array<ed_sound_instance_finished, 64> finished{};
	std::array<ed_sound_voice_position, 48> positions{};
	std::array<uint, 64> deletes{};
	ed_sound_sample sample{};
	void SetUp() override
	{
		AudioSamples::SetUp();
		ASSERT_EQ(pedSoundInstanceListHead, nullptr);
		for (auto& instance : edenInstances) instance.soundInstanceIndex = 0xffff;
		pedSoundInstances = edenInstances.data();
		edSoundInstanceCom = edenCommands.data();
		pedSoundFinishedInstances = finished.data();
		pedSoundVoicePosition = positions.data();
		pedSoundInstancesToDelete = deletes.data();
		edSoundMaxInstances = static_cast<uint>(edenInstances.size());
		edSoundCurrentInstancesNumber = 0;
		edSoundNextFreeInstanceIndex = 0;
		edSoundInstancesToDeleteNb = 0;
		edSoundNbFinishedInstances = 0;
		pedSoundInstanceListHead = pedSoundInstanceListTail = nullptr;
		_edSoundAllocatedVoices[0] = _edSoundAllocatedVoices[1] = 0;
		edSoundParam = {0, 47};
		edSoundGlobalParams = {};
		edSoundGlobalParams.volume = 1;
		edSoundGlobalParams.field_0x5c = 343.5f;
		edSoundGlobalParams.field_0x64 = 1;
		edSoundGlobalParams.outputMode = MONO;
		edSoundGlobalParams.g_DesiredFrameTime_00483824 = 0.02f;
		sample = {description.handle, description.sampleRate, 4, description.dataSize, 16, 32};
	}
	void TearDown() override
	{
		edSoundTerminateAllInstances();
		pedSoundInstances = nullptr;
		edSoundInstanceCom = nullptr;
		pedSoundFinishedInstances = nullptr;
		pedSoundVoicePosition = nullptr;
		pedSoundInstancesToDelete = nullptr;
		edSoundMaxInstances = 0;
		edSoundGlobalParams = {};
		AudioSamples::TearDown();
	}
};

TEST_F(EdenAudioSamples, FlushCompletesPendingSampleTransfer)
{
	sample.soundRamAddress = 0;
	sample.flags = 0;
	ASSERT_NE(edSoundLoadToSoundRamNoWaitDirect(adpcm.data(), static_cast<uint>(adpcm.size()),
		0, &sample, SetFlag_00288910), 0);
	EXPECT_EQ(sample.flags & 4, 0u);
	EXPECT_EQ(sample.soundRamAddress, 0u);

	edSoundFlush();

	EXPECT_NE(sample.flags & 4, 0u);
	Audio::LoadedDataInfo data{};
	ASSERT_TRUE(Audio::LookupLoadedData(sample.soundRamAddress, data));
	EXPECT_EQ(data.size, adpcm.size());
}

TEST_F(EdenAudioSamples, PriorityListRemainsLinkedAfterRemovalAndSlotReuse)
{
	const uint low = edSoundSamplePlay(1.0f, &sample);
	const uint high = edSoundSamplePlay(3.0f, &sample);
	const uint middle = edSoundSamplePlay(2.0f, &sample);
	const uint equal = edSoundSamplePlay(2.0f, &sample);
	auto checkList = [](std::initializer_list<uint> expected) {
		ed_sound_instance* previous = nullptr;
		ed_sound_instance* current = pedSoundInstanceListHead;
		for (uint id : expected) {
			ASSERT_NE(current, nullptr);
			EXPECT_EQ(current->fullSoundInstanceId, id);
			EXPECT_EQ(current->higherPrioritySoundInstance, previous);
			previous = current;
			current = current->lowerPrioritySoundInstance;
		}
		EXPECT_EQ(current, nullptr);
		EXPECT_EQ(pedSoundInstanceListTail, previous);
		EXPECT_EQ(edSoundCurrentInstancesNumber, expected.size());
	};
	checkList({high, equal, middle, low});
	edSoundInstanceStop(middle);
	checkList({high, equal, low});
	edSoundInstanceStop(high);
	checkList({equal, low});
	edSoundInstanceStop(low);
	checkList({equal});
	edSoundNextFreeInstanceIndex = 0xffffffff;
	const uint reused = edSoundSamplePlay(1.0f, &sample);
	EXPECT_EQ(reused & 0xffff, low & 0xffff);
	EXPECT_NE(reused, low);
	checkList({equal, reused});
	edSoundInstanceStop(equal);
	edSoundInstanceStop(reused);
	checkList({});
}

TEST_F(EdenAudioSamples, FlushSpatializesEveryInstanceInPriorityList)
{
	std::array<uint, 43> ids{};
	edsound_3d_data spatial{};
	spatial.position = {10.0f, 0.0f, 0.0f};
	spatial.rotation = {0.0f, 0.0f, 0.0f};
	spatial.field_0x18 = 10.0f;
	spatial.field_0x1c = 100.0f;
	for (unsigned i = 0; i < ids.size(); ++i) {
		ids[i] = edSoundSamplePlay(static_cast<float>(i % 5 + 1), &sample);
		ASSERT_NE(ids[i], 0u);
		edSoundInstanceSet3DData(ids[i], &spatial, nullptr, 0);
		pedSoundInstances[ids[i] & 0xffff].field_0x4c = -1.0f;
	}
	edSoundFlush();
	EXPECT_EQ(voices.size(), ids.size());
	for (uint id : ids) {
		const auto& instance = pedSoundInstances[id & 0xffff];
		EXPECT_EQ(instance.fullSoundInstanceId, id);
		EXPECT_FLOAT_EQ(instance.field_0x4c, 1.0f);
		EXPECT_FLOAT_EQ(instance.field_0x88, 1.0f);
		EXPECT_FLOAT_EQ(instance.field_0x8c, 1.0f);
	}
}

TEST_F(EdenAudioSamples, FlushStartsCompletesAndReusesSlots)
{
	const uint first = edSoundSamplePlay(1, &sample);
	edSoundInstanceSetVolume(0.5f, first);
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	EXPECT_TRUE(edSoundInstanceIsAlive(first));
	EXPECT_FLOAT_EQ(voices[0]->controls.left, 0.5f * std::sqrt(0.5f));
	EXPECT_EQ(edenCommands[first & 0xffff].flags, 0u);
	voices[0]->finished = true;
	edSoundFlush();
	EXPECT_FALSE(edSoundInstanceIsAlive(first));
	EXPECT_EQ(edSoundCurrentInstancesNumber, 0u);
	EXPECT_EQ(edSoundNbFinishedInstances, 1u);
	EXPECT_EQ(_edSoundAllocatedVoices[0], 0u);
	edSoundNextFreeInstanceIndex = first & 0xffff;
	const uint second = edSoundSamplePlay(1, &sample);
	EXPECT_NE(second, first);
	edSoundFlush();
	Audio::DestroySample(first);
	EXPECT_TRUE(edSoundInstanceIsAlive(second));
	EXPECT_EQ(edSoundInstanceStop(second), 0u);
	EXPECT_TRUE(voices[1]->destroyed);
}

TEST_F(EdenAudioSamples, AppliesSpatialAttenuationMasterVolumePauseAndFade)
{
	edsound_3d_data spatial{};
	spatial.position = {2, 0, 0};
	spatial.rotation = {0, 0, 0};
	spatial.field_0x1c = 1;
	const uint id = edSoundSamplePlay(1, &sample);
	edSoundInstanceSet3DData(id, &spatial, nullptr, 0);
	edSoundGlobalParams.volume = 0.5f;
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	EXPECT_FLOAT_EQ(voices[0]->controls.left, 0.125f);
	EXPECT_FLOAT_EQ(voices[0]->controls.right, 0.125f);
	edSoundInstanceSetPause(id, 1);
	edSoundFlush();
	EXPECT_FALSE(voices[0]->playing);
	edSoundInstanceSetPause(id, 0);
	edSoundInstanceSetFrequency(1.5f, id);
	edSoundFlush();
	EXPECT_TRUE(voices[0]->playing);
	EXPECT_FLOAT_EQ(voices[0]->controls.pitch, 1.5f);
	edSoundInstanceFade(-1, -1, 0, -1, 0.02f, id);
	edSoundFlush();
	EXPECT_FALSE(edSoundInstanceIsAlive(id));
	EXPECT_TRUE(voices[0]->destroyed);
}

TEST_F(EdenAudioSamples, FailedStartFinishesAndVoiceBudgetEvictsLowestPriority)
{
	failVoiceCreation = true;
	uint id = edSoundSamplePlay(1, &sample);
	edSoundFlush();
	edSoundFlush();
	EXPECT_FALSE(edSoundInstanceIsAlive(id));
	failVoiceCreation = false;
	id = edSoundSamplePlay(0.1f, &sample);
	for (int index = 0; index < 48; ++index) edSoundSamplePlay(1, &sample);
	edSoundFlush();
	EXPECT_FALSE(edSoundInstanceIsAlive(id));
	EXPECT_EQ(voices.size(), 48u);
	EXPECT_EQ(edSoundCurrentInstancesNumber, 48u);
	edSoundTerminateAllInstances();
	EXPECT_EQ(edSoundCurrentInstancesNumber, 0u);
	EXPECT_TRUE(std::all_of(voices.begin(), voices.end(), [](const auto& voice) { return voice->destroyed; }));
}

TEST_F(EdenAudioSamples, GameCallbackUsesFullIdAndExactCount)
{
	CSoundInstance actorInstance;
	int called = 0;
	// pOwner is callback data in the original callback contract.
	actorInstance.pOwner = reinterpret_cast<CActorSound*>(&called);
	actorInstance.pFinishCallback = [](CSoundInstance*, void* owner) {
		++*reinterpret_cast<int*>(owner);
	};
	actorInstance.soundId = edSoundSamplePlay(1, &sample);
	edSoundInstanceSetUserData(actorInstance.soundId, &actorInstance);
	edSoundGlobalParams.finishedInstancesCallback = CAudioManager_SoundFinishedInstancesCallback;
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	voices[0]->finished = true;
	edSoundFlush();
	EXPECT_EQ(called, 1);
	edSoundFlush();
	EXPECT_EQ(called, 1);
}

TEST_F(EdenAudioSamples, ActorSoundStartUsesSampleOverrideAndReplacesPlayback)
{
	CSoundSample sound;
	sound.InitializeFromSample(0.5f, 0, 1.25f, 0, 1, 1, &sample, 0);
	CSoundInstance slot;
	CActorSound actorSound{};
	actorSound.nbInstances = 1;
	actorSound.aSoundInstances = &slot;
	slot.pOwner = &actorSound;
	SOUND_SPATIALIZATION_PARAM spatial{nullptr};
	actorSound.SoundStart(nullptr, 0, &sound, 1, 2, &spatial);
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	EXPECT_TRUE(slot.IsAlive());
	EXPECT_FLOAT_EQ(voices[0]->controls.pitch, 1.25f);
	const uint first = slot.soundId;
	actorSound.SoundStart(nullptr, 0, &sound, 0, 2, &spatial);
	EXPECT_EQ(slot.soundId, first);
	actorSound.SoundStart(nullptr, 0, &sound, 1, 2, &spatial);
	EXPECT_NE(slot.soundId, first);
	EXPECT_TRUE(voices[0]->destroyed);
	edSoundFlush();
	ASSERT_EQ(voices.size(), 2u);
	voices[1]->finished = true;
	edSoundFlush();
	EXPECT_FALSE(slot.IsAlive());
	sample.flags |= 1;
	EXPECT_TRUE(static_cast<CSound*>(&sound)->IsLooping(1));
}

TEST_F(EdenAudioSamples, StereoAtListenerIsFiniteAndBalanced)
{
	edSoundGlobalParams.outputMode = STEREO;
	edsound_3d_data spatial{};
	spatial.position = spatial.rotation = {0, 0, 0};
	spatial.field_0x18 = 1;
	spatial.field_0x1c = 1;
	const uint id = edSoundSamplePlay(1, &sample);
	edSoundInstanceSet3DData(id, &spatial, nullptr, 0);
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	EXPECT_TRUE(std::isfinite(voices[0]->controls.left));
	EXPECT_TRUE(std::isfinite(voices[0]->controls.right));
	EXPECT_NEAR(voices[0]->controls.left, voices[0]->controls.right, 0.00001f);
	EXPECT_GT(voices[0]->controls.left, 0);
}

TEST_F(EdenAudioSamples, AmbianceAppliesAuthoredGainAndSfxMasterOnce)
{
	CSoundSample sound;
	sound.InitializeFromSample(0.4f, 0, 1, 0, 1, 1, &sample, 0);
	CSoundAmbiance entry{};
	entry.soundInstance.pSound = &sound;
	entry.soundInstance.soundId = edSoundSamplePlay(1, &sample);
	entry.field_0x68 = -1; // Already playing; no random restart timer.
	CAmbiance ambiance{};
	ambiance.aSoundAmbiance = &entry;
	ambiance.nbSoundAmbiance = 1;
	ambiance.field_0x14 = 1;
	edSoundGlobalParams.outputMode = STEREO;
	edSoundGlobalParams.volume = 0.5f;
	ambiance.Play(0.5f);
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	const float expected = 0.4f * 0.5f * 0.5f * std::sqrt(0.5f);
	EXPECT_NEAR(voices[0]->controls.left, expected, 0.000001f);
	EXPECT_NEAR(voices[0]->controls.right, expected, 0.000001f);
	edSoundGlobalParams.volume = 0;
	ambiance.Play(0.5f);
	edSoundFlush();
	EXPECT_EQ(voices[0]->controls.left, 0);
	EXPECT_EQ(voices[0]->controls.right, 0);
}

TEST_F(EdenAudioSamples, SamplesAndStreamsUseSeparateVoices)
{
	Audio::RegisterStream(3, 100, 1000);
	_ed_sound_stream stream{};
	stream.streamBufferId[0] = 3;
	stream.field_0x14 = 1;
	stream.pMem = &stream; // This test only needs a valid prepared-stream token.
	const uint streamId = edSoundStreamCreate_00284500(1, &stream);
	edSoundStream_00283650(streamId);
	const uint sampleId = edSoundSamplePlay(1, &sample);
	edSoundFlush();
	ASSERT_EQ(voices.size(), 1u);
	Audio::StreamInfo streamInfo;
	ASSERT_TRUE(Audio::GetStreamInfo(3, streamInfo));
	EXPECT_TRUE(streamInfo.playing);
	edSoundInstanceStop(sampleId);
	edSoundFlush();
	ASSERT_TRUE(Audio::GetStreamInfo(3, streamInfo));
	EXPECT_TRUE(streamInfo.playing);
	EXPECT_TRUE(edSoundInstanceIsAlive(streamId));
	edSoundInstanceStop(streamId);
	ASSERT_TRUE(Audio::GetStreamInfo(3, streamInfo));
	EXPECT_FALSE(streamInfo.playing);
	Audio::UnregisterStream(3);
}

TEST_F(EdenAudioSamples, CinematicStreamHonorsVolumeMasterPauseAndFade)
{
	Audio::RegisterStream(3, 100, 1000, 2);
	_ed_sound_stream stream{};
	stream.streamBufferId[0] = 3;
	stream.field_0x14 = 2;
	stream.pMem = &stream;
	const uint id = edSoundStreamCreate_00284500(1, &stream);
	edSoundInstanceSetVolume(0.4f, id);
	edSoundGlobalParams.volume = 0.5f;
	edSoundStream_00283650(id);
	Audio::StreamInfo info;
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_FLOAT_EQ(info.volume, 0.2f); // Applied before playback starts.
	edSoundFlush();
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_FLOAT_EQ(info.volume, 0.2f);
	EXPECT_TRUE(voices.empty());
	edSoundInstanceSetPause(id, 1);
	edSoundFlush();
	edSoundInstanceSetVolume(0.8f, id);
	edSoundFlush();
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_FALSE(info.playing);
	EXPECT_FLOAT_EQ(info.volume, 0.4f);
	edSoundInstanceSetPause(id, 0);
	edSoundGlobalParams.volume = 0;
	edSoundFlush();
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_TRUE(info.playing);
	EXPECT_EQ(info.volume, 0);
	edSoundGlobalParams.volume = 0.5f;
	edSoundInstanceFade(-1, -1, 0.4f, -1, 0.02f, id);
	edSoundFlush();
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_FLOAT_EQ(info.volume, 0.2f);
	edSoundInstanceStop(id);
	Audio::UnregisterStream(3);
	Audio::RegisterStream(3, 100, 1000);
	ASSERT_TRUE(Audio::GetStreamInfo(3, info));
	EXPECT_EQ(info.volume, 1);
	Audio::UnregisterStream(3);
}
}
