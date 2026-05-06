#ifndef ACTOR_ACCELERATOS_H
#define ACTOR_ACCELERATOS_H

#include "Types.h"
#include "Actor.h"

#define ACCELERATOS_BEHAVIOUR_STAND 2
#define ACCELERATOS_STATE_STAND 5

class CActorAcceleratos;

class CBehaviourAcceleratos : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CActorAcceleratos* pOwner;
};

class CBehaviourAcceleratosStand : public CBehaviourAcceleratos
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void TermState(int oldState, int newState);
};

PACK(
struct S_ACCELERATOS_CONFIG
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
	S_STREAM_REF<CSound> field_0xc;
	S_STREAM_REF<CSound> field_0x10;
});

static_assert(sizeof(S_ACCELERATOS_CONFIG) == 0x14, "S_ACCELERATOS_CONFIG has wrong size!");

class CActorAcceleratos : public CActor
{
public:
	static StateConfig _gStateCfg_ACC[1];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourAcceleratosStand_Manage();

	S_ACCELERATOS_CONFIG* pConfig;

	CActorSound* field_0x164;
	int field_0x168;
	CActor* field_0x16c;
	CBehaviourAcceleratosStand behaviourStand;
};

#endif //ACTOR_ACCELERATOS_H