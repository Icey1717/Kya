#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct ed_event_chunk;
class CActor;

struct ed_event {
	uint flags;
	int pZone; // ed_zone_3d*
	undefined4 field_0x8;
	uint nbColliders;
};

struct _ed_event_collider_test {
	edF32VECTOR4 worldLocation;
	byte messageFlags[4];
	uint flags;
	int pActorRef; // ed_event_actor_ref*
	int field_0x1c; // ed_event*
	int aSendInfo[4]; // EventSendInfo*
};

static_assert(sizeof(_ed_event_collider_test) == 0x30);

struct EventSendInfo
{
	int field_0x0;
	int nbActorIndexes;
};

static_assert(sizeof(EventSendInfo) == 0x8);

struct edCEventMessage {
	uint colliderId;
	_ed_event_collider_test* pEventCollider;
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
	int pLocation; // edf32VECTOR4*
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

	// Indexes into the global actor list
	int nbSendInfo;
	int* aSendInfo; // EventSendInfo**

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

typedef uint e_ed_event_prim3d_type;

void edEventInit(void);
uint edEventAddChunk(void* pFileData, uint mode);
uint edEventGetChunkNbEvents(int chunkIndex);
ed_zone_3d* edEventGetChunkZone(uint chunkId, uint zoneId);
int _edEventComputeZeroVolumeZoneAgainstVertex(ed_event_chunk* pEventChunk, ed_zone_3d* pZone, edF32VECTOR4* param_3, edF32VECTOR4* param_4, uint* param_5);
int edEventComputeZoneAgainstVertex(ed_event_chunk* pEventChunk, ed_zone_3d* pZone, edF32VECTOR4* pLocation, uint mode);
int edEventComputeZoneAgainstVertex(int index, ed_zone_3d* pZone, edF32VECTOR4* param_3, long mode);
int edEventGetChunkZoneNbInclusivePrimitives(int param_1, ed_zone_3d* param_2);
edF32MATRIX4* edEventGetChunkZonePrimitive(uint param_1, ed_zone_3d* param_2, uint param_3, e_ed_event_prim3d_type* param_4);
void edEventInitChunk(int eventChunkID);
void edEventRemoveChunk(int eventChunkID);

#endif // EVENT_MANAGER_H
