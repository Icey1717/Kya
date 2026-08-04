#ifndef AUDIO_H
#define AUDIO_H

#include "Types.h"
#include "edSound/edSoundPlay.h"

class CSoundSample
{

};

class CAudioManager
{
public:
	CSoundSample* GetSound(uint id)
	{
		return nullptr;
	}

	void SetMusicVolume(float newVolume);
	void SetSfxVolume(float newVolume);

	void SetOutputMode(AUDIO_MODE newMode);

	void PlayCombatMusic();
	void StopCombatMusic();

	float field_0x68;

	float musicVolume;
	float sfxVolume;
	float field_0xc4;
	float field_0xc8;
	float field_0xcc;

	byte field_0xd0;
	byte field_0xd1;
};

extern int NoAudio;

#endif // AUDIO_H
