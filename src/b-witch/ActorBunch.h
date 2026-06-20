#ifndef ACTOR_BUNCH_H
#define ACTOR_BUNCH_H

#include "Types.h"
#include "ActorWolfen.h"

class CActorBunch;

#define BUNCH_BEHAVIOUR_REPEL 0x1a
#define BUNCH_BEHAVIOUR_FENCE 0x1b

#define BUNCH_STATE_JUMP_CENTRE_PREPARE_A 0xbe
#define BUNCH_STATE_JUMP_CENTRE_PREPARE_B 0xbf
#define BUNCH_STATE_JUMP_CENTRE 0xc0

struct S_BUNCH_STREAM_ENTRY
{
	uint field_0x0;
	int behaviourId;
	uint zoneId;
	uint field_0xc;
	uint zoneResult;
};

struct CShockWaveBunch
{
	CShockWaveBunch();
	void Create(uint defaultId);
	void Manage(CActorsTable* pTable);
	void Draw();
	void Term(float param_1);
	void InitState(CActorBunch* pBunch, edF32VECTOR4* param_3, int param_4);

	CActorBunch* pOwner;
	int field_0x4; // state?
	undefined4 field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	StaticMeshComponentAdvanced staticMeshComponentAdvanced;

	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
	float field_0x40;
	float field_0x44;
	float field_0x48;

	CActorsTable actorsTable;

	CFxHandleExt field_0x1d0;
	CFxHandleExt field_0x1dc;
	CFxHandleExt field_0x1e8[3];

	float field_0x20c;
};

class CBehaviourBunch : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	virtual void ManageExit();
	virtual void Reset();
	virtual int GetSpecialState();
	virtual CWayPoint* GetWayPoint();

	CActorBunch* pOwner;
	S_STREAM_REF<CWayPoint> wayPointStreamRef;
};

class CBehaviourBunchRepel : public CBehaviourBunch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void ManageExit();
	virtual void Reset();
	virtual int GetSpecialState();

	CShockWaveBunch* field_0xc;
	uint field_0x10;
	float field_0x14;
	uint* field_0x18;
	uint field_0x1c;

	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
};

class CBehaviourBunchFence : public CBehaviourBunch
{
public:
	virtual void Reset() { IMPLEMENTATION_GUARD_LOG(); }
};

class CBehaviourBunchFighterProjected : public CBehaviourWolfenFighterProjected
{
public:
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourFighterBunch : public CBehaviourFighterWolfen
{
public:
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CBehaviourFighter
	virtual void _ManageHit(bool bPlayImpact);

	// CBehaviourFighterWolfen
	virtual void ManageExit();
};

class CActorBunch : public CActorWolfen
{
public:
	static StateConfig gStateCfg_BUN[14];

	virtual void Create(ByteCode* pByteCode);
	virtual void Term();
	virtual void Manage();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);

	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual bool Can_0x9c();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int Func_0x18c();
	virtual bool AquareAdversaryB(CActor* param_2);

	void FUN_003ded20(bool bPlayImpact);

	// State functions:
	void FUN_003dea20();
	void FUN_003de8e0(int nextState, int param_3);

	float field_0xfc0;
	uint field_0xfc4;
	float field_0xfc8;
	S_BUNCH_STREAM_ENTRY* pBunchStreamEntries;
	uint nbBunchStreamEntries;
	int field_0xfd4;
	float field_0xfd8;
	CVectorDyn field_0xfe0;

	CShockWaveBunch field_0x1060;
};

#endif //ACTOR_BUNCH_H