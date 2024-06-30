#ifndef ACTOR_TRAP_H
#define ACTOR_TRAP_H

#include "Types.h"
#include "Actor.h"
#include "ActorAutonomous.h"
#include "ActorMovingPlatform.h"

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

	void StateTrapCatch_2_2(int param_2, int param_3);

	uint flags;
	int field_0xc;
	S_ACTOR_STREAM_REF* pActorStream;
	uint field_0x14;
	uint field_0x18;
	uint field_0x1c;
	int field_0x20;
	int field_0x24;
	uint field_0x28;
	S_STREAM_REF<CActor> actorRef;
	float field_0x30;
	float field_0x34;
	CPathFollowReader pathFollowReader;
	S_TRAP_STREAM_REF* field_0x48;
	float field_0x4c;
	uint field_0x50;
	undefined4 field_0x54;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined4 field_0x5c;
	float field_0x60;
	CActor* field_0x64;
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

class CActorTrap : public CActorAutonomous
{
public:
	static AnimResult gStateCfg_TRP[14];

	CActorTrap();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Draw();
	virtual void Reset();

	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual AnimResult* GetStateCfg(int state);

	CBehaviourTrapStand_ActorTrap trapStandActorTrap;

	undefined4 field_0x2dc;
	undefined4 field_0x2e0;
	undefined4 field_0x2e4;
	undefined4 field_0x2e8;
};

#endif //ACTOR_TRAP_H