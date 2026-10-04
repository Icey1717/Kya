#include <gtest/gtest.h>
#include "EventTrack.h"
#include "FxComposite.h"
#include "FxSound.h"
#include "FxParticle.h"
#include "edParticles/edParticles.h"

namespace {
class CTrackTestSound : public CFxNewSound
{
public:
	bool IsLooped() override { return looped; }
	void Stop(float) override { ++stopCount; }
	bool looped = false;
	int stopCount = 0;
};

class CTrackTestSample : public CSound
{
public:
	bool IsLooping(uint selector) override { lastSelector = selector; return looped; }
	bool looped = false;
	uint lastSelector = 0;
};

void SetEventHandle(s_track_event& event, CNewFx* pFx)
{
	event.type = 0x39;
	event.fxHandle.id = pFx->id;
	event.fxHandle.pFx = STORE_POINTER(pFx);
	event.field_0x20 = 1;
}
}

TEST(EventTrack, AnimationTransitionPreservesOneShotSound)
{
	CTrackTestSound sound;
	s_track_event event{};
	SetEventHandle(event, &sound);
	CEventTrack track;
	track.eventCount = 1;
	track.pTrackEvent = &event;
	track.Stop();
	EXPECT_EQ(sound.stopCount, 0);
	EXPECT_EQ(event.fxHandle.id, 0);
	EXPECT_EQ(event.fxHandle.pFx, 0);
	EXPECT_EQ(event.field_0x20, 0);
}

TEST(EventTrack, AnimationTransitionStopsLoopingSound)
{
	CTrackTestSound sound;
	sound.looped = true;
	s_track_event event{};
	SetEventHandle(event, &sound);
	CEventTrack track;
	track.eventCount = 1;
	track.pTrackEvent = &event;
	track.Stop();
	EXPECT_EQ(sound.stopCount, 1);
	EXPECT_EQ(event.fxHandle.id, 0);
}

TEST(EventTrack, CompositeLoopCheckUsesValidChildLoopState)
{
	CTrackTestSound sound;
	CFxNewComposite composite;
	composite.nbComponentParticles = 1;
	composite.aFxHandles[0].pFx = &sound;
	composite.aFxHandles[0].id = sound.id;
	EXPECT_FALSE(composite.IsLooped());
	sound.looped = true;
	EXPECT_TRUE(composite.IsLooped());
	composite.aFxHandles[0].id = sound.id + 1;
	EXPECT_FALSE(composite.IsLooped());
}

TEST(EventTrack, SoundEffectLoopCheckUsesSoundAndMaterialSelector)
{
	CTrackTestSample sample;
	CFxSoundScenaricData data{};
	data.soundRef.pObj = STORE_POINTER(&sample);
	CFxNewSound sound;
	sound.Instanciate(&data, FX_MATERIAL_SELECTOR_NONE);
	EXPECT_FALSE(sound.IsLooped());
	EXPECT_EQ(sample.lastSelector, static_cast<uint>(FX_MATERIAL_SELECTOR_NONE));
	sample.looped = true;
	EXPECT_TRUE(sound.IsLooped());
	data.soundRef.pObj = 0;
	EXPECT_FALSE(sound.IsLooped());
}

TEST(EventTrack, ParticleLoopCheckUsesAllParticleGroups)
{
	CFxNewParticle particle;
	particle.pManager = nullptr;
	EXPECT_FALSE(particle.IsLooped());
	_ed_particle_manager manager{};
	_ed_particle_group groups[2]{};
	manager.aGroups.pData = groups;
	manager.nbGroups = 1;
	manager.nbTotalGroups = 2;
	particle.pManager = &manager;
	EXPECT_TRUE(particle.IsLooped());
	groups[1].particleGroupFlags = 4;
	EXPECT_FALSE(particle.IsLooped());
	manager.nbTotalGroups = 0;
	EXPECT_TRUE(particle.IsLooped());
	particle.pManager = nullptr;
}
