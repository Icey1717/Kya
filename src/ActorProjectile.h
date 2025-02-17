#ifndef ACTOR_PROJECTILE_H
#define ACTOR_PROJECTILE_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "Fx_Spark.h"

#define PROJECTILE_BEHAVIOUR_STAND 0x3
#define PROJECTILE_BEHAVIOUR_STRAIGHT 0x4
#define PROJECTILE_BEHAVIOUR_PORTABLE 0x5
#define PROJECTILE_BEHAVIOUR_INVENTORY 0x9
#define PROJECTILE_BEHAVIOUR_LAVA_BALL 0xa
#define PROJECTILE_BEHAVIOUR_EXCUSE 0xb
#define PROJECTILE_BEHAVIOUR_INACTIVE 0xc

class CPointPattern
{
public:
	edF32VECTOR4 field_0x10;
	undefined4 field_0x20;
	byte field_0x24;
	byte field_0x25;
};

class CPatternPart
{
public:
	CPatternPart();

	void Create(ByteCode* pByteCode);
	void Init(float param_1, CActor* pOwner);
	void Reset();
	void FUN_003a7cc0(float param_2);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;

	int materialId;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	uint field_0x28;
	edF32VECTOR4 field_0x30;
	undefined4 field_0x40;
	float field_0x48;
	undefined4 field_0x4c;
	undefined4 field_0x50;
	int field_0x5c;

	float field_0x60;
	float field_0x64;
	float field_0x68;
	float field_0x6c;

	float field_0x70;
	undefined4 field_0x74;
	float field_0x78;

	CActor* pOwner;

	int nbPointPatterns;
	CPointPattern* aPointPatterns;
};

class CActorProjectile;

class CBehaviourProjectile : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	int field_0x4;
	CActorProjectile* pOwner;
};

class CBehaviourProjectileStand : public CBehaviourProjectile
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	int field_0xc;
	int materialId;

	CFxSparkNoAlloc<3, 12>* aFxSparks;
};

class CBehaviourInventory : public CBehaviour
{

};

class CBehaviourProjectileLavaBall : public CBehaviour
{

};

class CBehaviourProjectileStraight : public CBehaviourProjectile
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	int field_0xc;

	float field_0x10;
	float field_0x14;
	float field_0x18;
};

class CBehaviourProjectileExcuse : public CBehaviourProjectile
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	uint field_0xc;
	S_STREAM_REF<CActor> actorRef;
};

class CBehaviourProjectilePortable : public CBehaviourProjectile
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	uint field_0xc;
	float field_0x10;
	CActor* field_0x14;
	int field_0x18;
};

class CBehaviourProjectileNew : public CBehaviourProjectile
{
public:
};

struct ProjectileSubObj
{
	float timeToExplode;
	float explosionRange;
	float animDuration;
	uint flags;
	float damage;
	float hitRadius;
	float warnRadius;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	int materialId;
	int field_0x30;
	int field_0x34;
	uint field_0x38;
	uint field_0x3c;
	float field_0x40;
	float field_0x44;
};

static_assert(sizeof(ProjectileSubObj) == 0x48, "ProjectileSubObj size is incorrect");

class CActorProjectile : public CActorAutonomous 
{
public:
	static StateConfig _gStateCfg_PRJ[27];

	CActorProjectile();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Draw();
	virtual void ComputeLighting();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual bool IsLockable();
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	void ClearLocalData();

	void BehaviourProjectile_InitState(int newState);
	void BehaviourProjectile_Manage(CBehaviourProjectileStand* pBehaviourStand);

	void BehaviourProjectileStand_InitState(int newState, CBehaviourProjectileStand* pBehaviourStand);
	void BehaviourProjectileStand_TermState(int oldState, int newState, CBehaviourProjectileStand* pBehaviourStand);

	void AppearToDie();
	void GoToSleep();
	void Die();

	void UpdateSmoke() { IMPLEMENTATION_GUARD_LOG(); }

	void ProjectDirected(float velocity, edF32VECTOR4* pSource, edF32VECTOR4* pTarget, bool param_5, CActor* pFiringActor);

	void StateFlyingDirected(ulong flags, int param_3);
	void StateDie(uint dynFlags, int param_3, int param_4);

	void HitActor(edF32VECTOR4* pSphere, CActor* pHitActor, int explode, int param_5);

	float GetTimeToExplode();

	CBehaviourProjectileStand behaviourProjectileStand;

	CPatternPart patternPart;

	edF32VECTOR4 lightAmbient;
	edF32MATRIX4 lightDirections;
	edF32MATRIX4 lightColors;

	ed_3D_Light_Config lightConfig;

	ProjectileSubObj* field_0x350;
	int field_0x354;
	int* field_0x358;
	int field_0x35c;
	int* field_0x360;

	float field_0x3f0;
	undefined4 field_0x3f4;
	float timeToExplode;
	CActor* field_0x3fc;

	undefined4 field_0x410;

	CActor* pFiringActor;

	CVectorDyn vectorDyn;

	undefined4 field_0x400;
	float field_0x404;
	CActor* field_0x40c;

	undefined4 field_0x540;
	float field_0x570;
	undefined4 field_0x5a8;
};

#endif //ACTOR_PROJECTILE_H