#ifndef ACTOR_BUNCH_H
#define ACTOR_BUNCH_H

#include "Types.h"
#include "ActorWolfen.h"

class CActorBunch;

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
	void Manage(CActorsTable* pOutTable);

	undefined4 field_0x0;
	undefined4 field_0x4;
	undefined4 field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	StaticMeshComponentAdvanced staticMeshComponentAdvanced;

	float field_0x40;
	float field_0x44;

	CActorsTable actorsTable;

	CFxHandleExt field_0x1d0;
	CFxHandleExt field_0x1dc;
	CFxHandleExt field_0x1e8[3];

	float field_0x20c;
};

class CBehaviourBunch : public CBehaviour
{
public:
	virtual void Reset();

	CActorBunch* pOwner;
};

class CBehaviourBunchRepel : public CBehaviourBunch
{
public:
	virtual void Reset() { IMPLEMENTATION_GUARD_LOG(); }
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

	float field_0xfc0;
	uint field_0xfc4;
	float field_0xfc8;
	S_BUNCH_STREAM_ENTRY* pBunchStreamEntries;
	uint nbBunchStreamEntries;
	int field_0xfd4;
	float field_0xfd8;

	CShockWaveBunch field_0x1060;
};

#endif //ACTOR_BUNCH_H