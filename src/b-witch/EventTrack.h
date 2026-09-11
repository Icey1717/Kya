#ifndef EVENT_TRACK_H
#define EVENT_TRACK_H

#include "Types.h"
#include "LargeObject.h"
#include "Fx.h"

struct ByteCode;
class CActor;

bool BnkInstallTrack(char* pFileData, int length);

#ifdef PLATFORM_WIN
class CFxHandlePackedHack
{
public:
	int id;
	strd_ptr(CNewFx*) pFx;
};

#define UNPACK_HANDLE(s_track_event_ptr) CFxHandle{(s_track_event_ptr)->fxHandle.id, LOAD_POINTER_CAST(CNewFx*, (s_track_event_ptr)->fxHandle.pFx)}
#else
#define UNPACK_HANDLE(s_track_event_ptr) (s_track_event_ptr)->fxHandle
#endif

PACK(
struct s_track_event
{
	float field_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	uint field_0x8;
	int type;
	uint field_0x10;
	undefined4 field_0x14;
#ifdef PLATFORM_PS2
	CFxHandle fxHandle;
#else
	CFxHandlePackedHack fxHandle;
#endif
	int field_0x20;
});

class CEventTrack
{
public:
	void Add(ByteCode* pByteCode);
	void Play(float param_1, float param_2, undefined8 param_4, CActor* pActor);
	void Stop();
	void Reset();
	void Pause();
	void Resume();

	bool FUN_0019f140();

	void _PlayEvent(s_track_event* pEvent, CActor* pActor);

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
