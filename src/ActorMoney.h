#ifndef ACTOR_MONEY_H
#define ACTOR_MONEY_H

#include "Types.h"
#include "ActorMovable.h"
#include "PathFollow.h"

#define MONEY_BEHAVIOUR_FLOCK 0x2
#define MONEY_BEHAVIOUR_ADD_ON 0x3

class CActorMoney;

class CMnyInstance : public CActInstance
{
public:
	void SetState(int newState);
	float GetAngleRotY();
};

class CBnsInstance : public CActInstance
{

};

class CShadowShared
{
	
};

class CInstantFlares
{
public:
	void Create(float, float, int);
	void Manage(CActInstance* pInstances, int nbInstances);
};

class CBehaviourMoneyFlock : public CBehaviour
{
public:
	CBehaviourMoneyFlock();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool InitDlistPatchable(int);

	S_STREAM_REF<CPathFollow> pathFollow;

	int nbMoneyInstances;
	int nbSharedShadows;
	int field_0x18;

	CActorMoney* pOwner;

	float field_0x20;
	float field_0x24;
	float field_0x28;

	CMnyInstance* aMoneyInstances;
	CShadowShared* aSharedShadows;

	CInstantFlares instantFlares;
};

class CBehaviourMoneyAddOn : public CBehaviourMoneyFlock
{

};

class CActorMoney : public CActorMovable
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeVisibleState(int bVisible);

	uint moneyValue;
	int field_0x1d4;

	CLightConfig lightConfig;

	S_STREAM_REF<CSound> soundRef;
	CActorSound* field_0x280;
	edF32VECTOR4 field_0x288;
};

#endif //ACTOR_MONEY_H