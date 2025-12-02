#ifndef ED_SOUND_PLAY_H
#define ED_SOUND_PLAY_H

#include "Types.h"

struct _ed_sound_stream
{
	int streamBufferId;
	int streamFileId;
	void* pMem;
};

struct GlobalSound_FileData
{

};

int _edSoundStreamInit(GlobalSound_FileData* pSoundData, _ed_sound_stream* pSoundStream, char* szPath, ulong param_4, undefined8 param_5, undefined8 param_6);
void _edSoundStreamTerm(_ed_sound_stream* pSoundStream);

int edSoundStreamLoadA(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, char* szPath, undefined8 param_4);
int edSoundStreamLoadB(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, ulong param_3, undefined8 param_4, undefined8 param_5);
void edSoundStreamFree(_ed_sound_stream* pSoundStream);

void edSoundFlush();

uint edSoundInstanceStop(uint instanceId);

#endif // ED_SOUND_PLAY_H
