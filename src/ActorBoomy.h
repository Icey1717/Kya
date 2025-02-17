#ifndef ACTOR_BOOMY_H
#define ACTOR_BOOMY_H

#include "Types.h"
#include "ActorMovable.h"
#include "Vision.h"
#include "ActorAmbre.h"
#include "Fx_Tail.h"
#include "FxLightEmitter.h"

class CActorBoomy;
class CedMathTCBSpline;
class CCameraGame;

class CBehaviourBoomyLaunch : public CBehaviour
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CActorBoomy* pOwner;
};

struct BoomySubObj_0x1c {
	uint flags;
	float field_0x4;
	float visionRange;
	int field_0xc;
	uint field_0x10;
	int field_0x14;
};

class CActorBoomy : public CActorMovable {
public:
	static CActorBoomy* _gThis;
	static StateConfig _gStateCfg_BMY[7];

	CActorBoomy() {
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual CVision* GetVision();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CActorMovable
	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	void BehaviourBoomyLaunch_Manage();

	void UpdateTypeInfo();

	void ClearLocalData();

	float field_0x1dc;

	CFxTail fxTail;
	CBehaviourBoomyLaunch launchBehaviour;

	CedMathTCBSpline* pSpline;

	CVision vision;

	BoomySubObj_0x1c aBoomyTypeInfo[4];
	int curBoomyTypeId;

	CActorSound* field_0x2b4;
	S_STREAM_REF<CSound> field_0x2b8;
	S_STREAM_REF<CSound> field_0x2bc;
	float field_0x2c0;
	byte field_0x2c4;

	int field_0x294;

	uint field_0x3cc;

	int particleID_0x3d0;
	int particleID_0x3d4;
	int particleID_0x3d8;
	int particleID_0x3dc;

	CActor* field_0x3e0;

	CFxLightEmitter fxLightEmitterA;
	CFxLightEmitter fxLightEmitterB;

	StaticMeshComponent staticMeshComponent;

	CCameraGame* pCamera;

	edF32MATRIX4 field_0x610;

	float field_0x740;
	float field_0x744;

	SPEED_DYN speedDyn;	
};

#endif //ACTOR_BOOMY_H