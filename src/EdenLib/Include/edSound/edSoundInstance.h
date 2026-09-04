#ifndef ED_SOUND_INSTANCE_H
#define ED_SOUND_INSTANCE_H

#include "Types.h"

struct ed_sound_sample;
struct _ed_sound_stream;

struct edsound_3d_data
{
	edF32VECTOR3 position;
	edF32VECTOR3 rotation;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	byte field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
};

struct edsound_listener
{
	edF32VECTOR3 field_0x0;
	edF32VECTOR3 field_0xc;
	edF32VECTOR3 field_0x18;
	edF32VECTOR3 field_0x24;
};

struct ed_sound_instance
{
	union
	{
		uint fullSoundInstanceId;
		struct
		{
			ushort soundInstanceIndex;
			ushort generation;
		};
		//ushort soundInstanceId;
	};

	ed_sound_instance* higherPrioritySoundInstance;
	ed_sound_instance* lowerPrioritySoundInstance;
	float priority;
	uint flags;

	union
	{
		ed_sound_sample* pSample;		// flags & 0x10 == 0
		_ed_sound_stream* pSoundStream;	// flags & 0x10 != 0
	};

	edsound_3d_data* p3dData;
	edsound_3d_data data3d;
	float volume;
	float frequency;
	float field_0x4c;
	float field_0x50;
	float targetVolume;
	float targetFrequency;
	undefined4 fadeType;
	float duration;
	byte field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	float field_0x68;
	undefined4 field_0x6c;
	undefined4 field_0x70;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	uint voiceIndices[2];
	uint field_0x80[2];
	float field_0x88;
	float field_0x8c;
	undefined field_0x90;
	undefined field_0x91;
	ushort field_0x92;
	float field_0x94;
	float field_0x98;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	void* field_0xa0;
};

struct ed_sound_instance_finished
{
	ed_sound_instance* pSoundInstance;
	void* field_0x4;

};

struct edSoundInstanceComType
{
	uint flags;
	uint soundInstanceId;
};

void edSoundInitInstances(int nbInstances);
ed_sound_instance* edSoundInstanceCreate(float priority, int bForce);
void edSoundInstanceDeleteLessPrioritary();
void edSoundInitInstance(ed_sound_instance* pInstance);
void edSoundInstancesComputeFade(ed_sound_instance* soundInstance);
void edSoundInstanceDeleteAllFromCurrentLessPrioritary(ed_sound_instance* pInstance);
void edSoundInstanceDelete(ed_sound_instance* pInstance);
uint _edSoundInstanceCheckFinished(ed_sound_instance* pSoundInstance);
void _edSoundInstanceListInstanceRemove(ed_sound_instance* pInstance);
void _edSoundInstanceListInstanceInsert(float priority, ed_sound_instance* newSoundInstance, long param_3);
bool edSoundInstanceFinish(ed_sound_instance* pInstance, int param_2);
bool edSoundInstanceIsAlive(uint soundID);

extern ed_sound_instance* pedSoundInstances;
extern ed_sound_instance_finished* pedSoundFinishedInstances;
extern uint edSoundNbFinishedInstances;
extern ed_sound_instance* pedSoundInstanceListHead;
extern uint* pedSoundInstancesToDelete;
extern int edSoundInstancesToDeleteNb;
extern edSoundInstanceComType* edSoundInstanceCom;

#endif // ED_SOUND_INSTANCE_H
