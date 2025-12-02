#ifndef EVENT_TRACK_H
#define EVENT_TRACK_H

#include "Types.h"
#include "LargeObject.h"

struct ByteCode;
class CActor;

bool BnkInstallTrack(char* pFileData, int length);

PACK(
struct s_track_event {
	float field_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	uint field_0x8;
	int type;
	uint field_0x10;
	undefined4 field_0x14;
	int field_0x18;
	strd_ptr(int*) field_0x1c;
	int field_0x20;
});

class CEventTrack {
public:
	void Add(ByteCode* pByteCode);
	void Play(float param_1, float param_2, undefined8 param_4, CActor* pActor);
	void Stop();
	void Reset();
	void Pause();
	void Resume();

	bool FUN_0019f140();

	ushort eventCount;
	s_track_event* pTrackEvent;
};

class CTrackManager : public CObjectManager {
public:
	CTrackManager();

	virtual void Level_Init();
	virtual void Level_Term() {};

	virtual void Level_ClearAll();
	virtual void Level_Manage() {};
	virtual void Level_ManagePaused() {};

	CEventTrack* GetTrack(int index);

	int trackCount;
	CEventTrack* aTracks;
};

#endif // EVENT_TRACK_H
