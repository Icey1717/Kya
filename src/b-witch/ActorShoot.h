#ifndef ACTOR_SHOOT_H
#define ACTOR_SHOOT_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorAmbre.h"
#include "ActorBonusServices.h"
#include "FireShot.h"

#define SHOOT_BEHAVIOUR_FIRE 0x3
#define SHOOT_BEHAVIOUR_FIRE_WAVE 0x4

class CActorHero;
class CActorShoot;

class CBehaviourShoot : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorShoot* pOwner;
};

class CBehaviourShootFire : public CBehaviourShoot
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void Reset();

	float field_0x8;

	CFireShot fireshot;

	uint field_0x2b0;
	float field_0x2b4;
};

class CBehaviourShootFireWave : public CBehaviourShoot
{
public:
};

class CActorShoot : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_SHT[10];

	~CActorShoot();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void ComputeLighting();
	virtual void Reset();

	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ClearLocalData();

	CAddOnGenerator addOnGenerator;
	ed_3D_Light_Config lightingConfig;
	CBehaviourShootFire behaviourShootFire;
	CBehaviourShootFireWave behaviourShootFireWave;
	StaticMeshComponent staticMeshComponent;

	uint field_0x350;
	uint field_0x354;

	float field_0x3cc;
	float field_0x3d4;

	uint field_0x3f0;
	float field_0x3f4;
	float field_0x3f8;
	float field_0x3fc;

	undefined4 field_0x400;
	undefined4 field_0x440;

	CActorHero* field_0x438;
	byte field_0x43c;

	edF32MATRIX4 lightDirection;
	edF32MATRIX4 lightColor;
	edF32VECTOR4 lightAmbient;

	ed_3d_hierarchy_setup altHierarchySetup;

	bool field_0x43e;
	byte field_0x43f;
};

#endif //ACTOR_SHOOT_H