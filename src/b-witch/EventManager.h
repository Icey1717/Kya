#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct ed_event_chunk;
class CActor;

struct ed_event {
	uint flags;
	strd_ptr(ed_zone_3d*) pZone;
	undefined4 field_0x8;
	uint nbColliders;
};

// ed_event::flags bits
#define ED_EVENT_FLAG_START_ACTIVE 0x01 // Serialised: event activates on load
#define ED_EVENT_FLAG_ONE_SHOT     0x02 // Event deactivates (clears ACTIVE) after first fire
#define ED_EVENT_FLAG_ACTIVE       0x80 // Runtime: event is computed each frame

struct _ed_event_collider_test {
	edF32VECTOR4 worldLocation;
	byte messageFlags[4];
	uint flags;
	strd_ptr(ed_event_actor_ref*) pActorRef;
	strd_ptr(ed_event*) field_0x1c;
	int aSendInfo[4]; // EventSendInfo*
};

static_assert(sizeof(_ed_event_collider_test) == 0x30);

// _ed_event_collider_test::flags – recomputed each frame by _edEventComputeEvent.
// Bits 0 and 2 are set when the actor is inside the zone; bits 1 and 3 when outside.
// Bits 2 and 3 are "transition" bits and are cleared the frame after a state change.
// Each bit index directly corresponds to the send-info slot that fires when it is set:
//   slot 0 (INSIDE)       – fires every frame the actor is inside  (sustained)
//   slot 1 (OUTSIDE)      – fires every frame the actor is outside (sustained)
//   slot 2 (JUST_ENTERED) – fires only on the frame the actor enters the zone
//   slot 3 (JUST_EXITED)  – fires only on the frame the actor exits the zone
#define ED_COLLIDER_FLAG_INSIDE       0x01u
#define ED_COLLIDER_FLAG_OUTSIDE      0x02u
#define ED_COLLIDER_FLAG_JUST_ENTERED 0x04u
#define ED_COLLIDER_FLAG_JUST_EXITED  0x08u

#define ED_COLLIDER_FLAGS_STATE_INSIDE  (ED_COLLIDER_FLAG_INSIDE  | ED_COLLIDER_FLAG_JUST_ENTERED) // 0x05
#define ED_COLLIDER_FLAGS_STATE_OUTSIDE (ED_COLLIDER_FLAG_OUTSIDE | ED_COLLIDER_FLAG_JUST_EXITED)  // 0x0a
// Mask applied when the zone state has not changed: clears the one-frame transition bits.
#define ED_COLLIDER_FLAGS_CLEAR_TRANSITIONS (~(ED_COLLIDER_FLAG_JUST_ENTERED | ED_COLLIDER_FLAG_JUST_EXITED)) // 0xfffffff3

// _ed_event_collider_test::messageFlags[4] – one control byte per send-info slot.
#define ED_MSG_FLAG_INIT_ENABLED 0x01 // Serialised: slot should be re-enabled on reset
#define ED_MSG_FLAG_ONE_SHOT     0x02 // Slot disables itself after firing once
#define ED_MSG_FLAG_ENABLED      0x80 // Runtime: slot is currently active

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
	strd_ptr(CActor*) pActor;
	strd_ptr(edf32VECTOR4*) pLocation;
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

	virtual char* ProfileGetName();

	int activeChunkId;
	int nbEvents;

	static CEventManagerInternal callbackFunctions;
};

typedef uint e_ed_event_prim3d_type;

// Values for e_ed_event_prim3d_type (pEventData[0]) passed to ReceiveEvent / InterpretEvent.
// Values EVENT_PRIM_AUDIO_FIRST–EVENT_PRIM_AUDIO_LAST are forwarded to CAudioManager::ReceiveEvent.
#define EVENT_PRIM_KILL               0x1  // Kill/death — sub-type in pEventData[1]
#define EVENT_PRIM_SECTOR_SWITCH      0x2  // Switch sector (immediate)
#define EVENT_PRIM_SECTOR_SWITCH_ANIM 0x3  // Switch sector (with transition)
#define EVENT_PRIM_TELEPORT           0x5  // Level teleport
#define EVENT_PRIM_AUDIO_FIRST        0x6  // First audio event index
#define EVENT_PRIM_AUDIO_LAST         0x10 // Last audio event index
#define EVENT_PRIM_ZONE_ACTIVATE      0x12 // Zone trigger: send MESSAGE_ACTIVATE to actor
#define EVENT_PRIM_ZONE_DEACTIVATE    0x13 // Zone trigger: send MESSAGE_DEACTIVATE to actor
#define EVENT_PRIM_ZONE_TOGGLE        0x14 // Zone trigger: send MESSAGE_TOGGLE to actor
#define EVENT_PRIM_ZONE_MESSAGE       0x15 // Zone trigger: relay ACTOR_MESSAGE (pEventData[1]) to actor
#define EVENT_PRIM_ZONE_PROJECTILE    0x16 // Zone trigger: hit actor with a projectile (MESSAGE_KICKED)

// Kill sub-types — used in pEventData[1] when pEventData[0] == EVENT_PRIM_KILL.
#define EVENT_PRIM_KILL_FALL  0 // Fall to death
#define EVENT_PRIM_KILL_DROWN 1 // Drown

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
