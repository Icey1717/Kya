#ifndef ACTOR_TRAP_H
#define ACTOR_TRAP_H

#include "Types.h"
#include "Actor.h"
#include "ActorMovable.h"
#include "ActorMovingPlatform.h"

#define TRAP_STATE_IDLE				0x6
#define TRAP_STATE_CATCH_1_1		0x7
#define TRAP_STATE_CATCH_2_2		0x8
#define TRAP_STATE_CATCH_STAND_1_2	0x9
#define TRAP_STATE_CATCH_STAND_2_2	0xa
#define TRAP_STATE_CATCH_STRUGGLE	0xd

#define TRAP_BEHAVIOUR_STAND	2
#define TRAP_BEHAVIOUR_INACTIVE	3

class CActorTrap;

struct S_TRAP_STREAM_ENTRY {
	S_STREAM_NTF_TARGET_ONOFF onoff;
	S_STREAM_EVENT_CAMERA eventCamera;
};

static_assert(sizeof(S_TRAP_STREAM_ENTRY) == 0x3c, "Invalid S_TRAP_STREAM_REF size");

struct S_TRAP_STREAM_REF {
	uint entryCount;
	S_TRAP_STREAM_ENTRY aEntries[];
};

class CBehaviourTrap : public CBehaviour 
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner);

	CActorTrap* pOwner;
};

class CBehaviourTrapStand : public CBehaviourTrap 
{
public:
	virtual void Init(CActor* pOwner);
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int state, int);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	int DetectActor();

	void StateTrapCatchStand_1_2(int param_2, int nextState);
	void StateTrapCatchStand_2_2(int nextState);
	void StateTrapCatch_2_2(int param_2, int nextState);
	void StateTrapCatchStruggle(int param_2, int param_3, int param_4);

	uint flags;
	int defaultBehaviourId;
	S_ACTOR_STREAM_REF* pActorStream;
	uint field_0x14;
	uint field_0x18;
	uint field_0x1c;
	int field_0x20;
	int field_0x24;
	uint catchEffectId;
	S_STREAM_REF<CActor> actorRef;
	float speed;
	float acceleration;
	CPathFollowReader pathFollowReader;
	S_TRAP_STREAM_REF* field_0x48;
	float field_0x4c;
	uint field_0x50;
	undefined4 escapeAttempts;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined4 field_0x5c;
	float field_0x60;
	CActor* pCaughtActor;
	bool field_0x68;
	bool field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	CinNamedObject30* field_0x6c;
	CActor* field_0x70;
	CinNamedObject30* field_0x74;
	CActor* field_0x78;
};

class CBehaviourTrapStand_ActorTrap : public CBehaviourTrapStand 
{
public:
	CBehaviourTrapStand_ActorTrap();
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	void BehaviourTrapStand_Manage();
	void CreateForTrap(ByteCode* pByteCode);
};

class CActorTrap : public CActorMovable
{
public:
	static StateConfig gStateCfg_TRP[14];

	CActorTrap();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();

	virtual void ComputeLighting();

	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	CBehaviourTrapStand_ActorTrap trapStandActorTrap;

	undefined4 field_0x2dc;
	undefined4 field_0x2e0;
	undefined4 field_0x2e4;
	undefined4 field_0x2e8;
};

#endif //ACTOR_TRAP_H