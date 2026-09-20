#ifndef ACTOR_AMBRE_H
#define ACTOR_AMBRE_H

#include "Types.h"
#include "Actor.h"
#include "CinematicManager.h"
#include "Fx_Spark.h"
#include "StaticMeshComponent.h"
#include "Fx.h"

struct S_NTF_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

class CActorAmbre;

class CBehaviourAmbre : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual bool UnknownFunction() { return false; }

	CActorAmbre* pOwner;
};

class CBehaviourAmbreUnknown : public CBehaviourAmbre
{
public:
	virtual bool UnknownFunction() { return true; }
};

struct AmberSparkProps
{
	uint boneId;
	float field_0x4;
	float field_0x8;
};

class CActorAmbre : public CActor
{
public:
	static StateConfig _gStateCfg_AMB[6];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);

	void BehaviourStand_Manage(CBehaviourAmbre* pBehaviour);

	CFxSparkNoAlloc<4, 16> aFxSparks[3];
	AmberSparkProps aFxSparkProps[3];

	CFxHandleExt effectsStructC;
	StaticMeshComponent staticMesh;

	int field_0x1d0;

	CFxHandle effectsStructD;

	S_NTF_SWITCH ntfSwitch;

	CBehaviourAmbre behaviourAmber;
};

#endif //ACTOR_AMBRE_H