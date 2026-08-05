#ifndef ED_SOUND_INSTANCE_H
#define ED_SOUND_INSTANCE_H

#include "Types.h"

struct ed_sound_sample;

struct edsound_3d_data
{
	edF32VECTOR3 position;
	edF32VECTOR3 rotation;
	float field_0x18;
	float field_0x1c;
	undefined4 field_0x20;
	byte field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
};

struct ed_sound_instance
{
	union
	{
		uint fullSoundInstanceId;
		ushort soundInstanceId;
	};

	ed_sound_instance* higherPrioritySoundInstance;
	ed_sound_instance* lowerPrioritySoundInstance;
	float priority;
	uint flags;
	ed_sound_sample* pSample; /* Created by retype action */
	edsound_3d_data* p3dData;
	edsound_3d_data data3d;
	float field_0x44;
	float field_0x48;
	float field_0x4c;
	float field_0x50;
	float field_0x54;
	float field_0x58;
	undefined4 field_0x5c;
	undefined4 field_0x60;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined4 field_0x68;
	undefined4 field_0x6c;
	undefined4 field_0x70;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	float field_0x88;
	float field_0x8c;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	float field_0x94;
	float field_0x98;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined4 field_0xa0;
};

struct ed_sound_instance_finished
{
	ed_sound_instance* pSoundInstance;
};

void edSoundInitInstances(int nbInstances);

extern ed_sound_instance* pedSoundInstances;
extern ed_sound_instance_finished* pedSoundFinishedInstances;
extern uint edSoundNbFinishedInstances;

#endif // ED_SOUND_INSTANCE_H
