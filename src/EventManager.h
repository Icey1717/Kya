#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct ed_event_chunk;

struct ed_event {
	uint flags;
	int pZone; // ed_zone_3d*
	undefined4 field_0x8;
	uint field_0xc;
};

struct _ed_event_collider_test {
	edF32VECTOR4 field_0x0;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	int field_0x14;
	int field_0x18; // ?
	int field_0x1c; // ed_event*
	int field_0x20[4]; // ?
};

struct edCEventMessage {
	uint field_0x0;
	_ed_event_collider_test* field_0x4;
	ed_event_chunk* pEventChunk;
};

typedef CActor* (EventCallbackGetActor)(int);
typedef edF32VECTOR4* (EventGetActorPositionFunc)(CActor*);
typedef void (EventSendAllMsgs)(edCEventMessage*, int);

struct CEventManagerInternal {
	EventCallbackGetActor* pGetActorPointer;
	EventGetActorPositionFunc* pGetActorPositionVector;
	void* pGetActorIdent;
	EventSendAllMsgs* pSendAllMessages;
};

struct _edCluster {
	char a[0x34];
};


struct ed_event_actor_ref {
	int pActor; // CActor*
	uint pLocation; // edf32VECTOR4*
};

static_assert(sizeof(ed_event_actor_ref) == 0x8);

struct ed_event_alloc_10 {
	uint clusterCount;
	ed_event_actor_ref* field_0x4;
	undefined4 field_0x8;
	_edCluster* aClusters;
};

struct ed_event_chunk {
	void* pFileData;
	int nbMatrices;
	byte* field_0x8;
	edF32MATRIX4* aMatrices;
	uint field_0x10;
	int* field_0x14; // 
	int count_0x18;
	int* field_0x1c;

	int nbActorRefs;
	ed_event_actor_ref* aActorRefs;

	uint nbZones;
	ed_zone_3d* aZones;

	uint nbEvents;
	int* aEvents; // ed_event**

	uint count_0x38;
	ed_event_alloc_10* field_0x3c;

	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
};

struct EventQueue {
	edCEventMessage* aEntries;
	int totalCount;
	int inUseCount;
};

struct CEventManager : public CObjectManager {
public:
	CEventManager();

	virtual void Level_Init() {};
	virtual void Level_Term();

	virtual void Level_ClearAll() {}
	virtual void Level_Manage();
	virtual void Level_ManagePaused() {};
	virtual void Level_Draw() {};

	virtual void Level_Reset();

	virtual void Level_CheckpointReset();

	int activeChunkId;
	int nbEvents;

	static CEventManagerInternal callbackFunctions;
};

void edEventInit(void);
uint edEventAddChunk(void* pFileData, uint mode);
uint edEventGetChunkNbEvents(int eventIndex);
ed_zone_3d* edEventGetChunkZone(uint chunkId, uint zoneId);

#endif // EVENT_MANAGER_H
