#ifndef ED_SOUND_INIT_H
#define ED_SOUND_INIT_H

#include "Types.h"

struct edSoundConfig
{
	int nbMaxInstances;
	uint nbVoices;
	undefined4 field_0x8;
	uint field_0x10;
	uint field_0x14;
};

edSoundConfig* edSoundGetConfig(void);
uint edSoundVoicesNumberGet(void);
void edSoundInit(void);
void edSoundInitFunc(float param_1);

#endif // ED_SOUND_INIT_H
