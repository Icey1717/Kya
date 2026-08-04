#include "Audio.h"

int NoAudio = 0;

void CAudioManager::SetMusicVolume(float newVolume)
{
	float fVar1;

	if (newVolume < 0.0f) {
		newVolume = 0.0f;
	}

	fVar1 = 1.0f;
	if (newVolume <= 1.0f) {
		fVar1 = newVolume;
	}

	this->musicVolume = fVar1;
	fVar1 = this->field_0xc4 * this->musicVolume * this->field_0x68 * 65535.0f;
	if (this->field_0xd1 != 0) {
		fVar1 = 0.0f;
	}

	if (NoAudio == 0) {
		edMusicSetMasterVolume(static_cast<int>(fVar1));
	}

	return;
}

void CAudioManager::SetSfxVolume(float newVolume)
{
	float fVar1;

	if (newVolume < 0.0f) {
		newVolume = 0.0f;
	}

	fVar1 = 1.0f;
	if (newVolume <= 1.0f) {
		fVar1 = newVolume;
	}

	this->sfxVolume = fVar1;
	fVar1 = this->field_0xc8 * this->sfxVolume;
	if (this->field_0xd0 != 0) {
		fVar1 = 0.0f;
	}

	if (NoAudio == 0) {
		edSoundSetMasterVolume(fVar1);
	}

	return;
}

void CAudioManager::SetOutputMode(AUDIO_MODE newMode)
{
	AUDIO_MODE unaff_s0_lo;
	AUDIO_MODE newMode_00;

	edSoundOutputModeSet(newMode);

	newMode_00 = SURROUND;
	if (((newMode != SURROUND) && (newMode_00 = STEREO, newMode != STEREO)) && (newMode_00 = unaff_s0_lo, newMode == MONO)) {
		newMode_00 = MONO;
	}

	edMusicSetOutputMode(newMode_00);

	return;
}

void CAudioManager::PlayCombatMusic()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

void CAudioManager::StopCombatMusic()
{
	IMPLEMENTATION_GUARD_AUDIO();
}