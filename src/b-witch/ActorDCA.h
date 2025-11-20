#ifndef ACTOR_D_C_A_H
#define ACTOR_D_C_A_H

#include "Types.h"
#include "Actor.h"
#include "Dynamic.h"

// A.K.A AIRCANON

#define DCA_BEHAVIOUR_DEFAULT 0x2
#define DCA_BEHAVIOUR_CONTROLLED 0x3
#define DCA_BEHAVIOUR_AI_CONTROLLED 0x4
#define DCA_BEHAVIOUR_TRAJECTORY 0x5


class CCameraDCA;
struct ProjectileSubObj;

struct S_SHOT_DATA
{
	edF32VECTOR4 field_0x0;
	float field_0x10;
	float timeToExplode;
	float field_0x18;
};

class CFireShot
{
public:
	CFireShot();
	void Create(ByteCode* pByteCode);
	void Init();
	void Reset();

	void ManageShots();

	bool Project(float velocity, edF32VECTOR4* pPosition, edF32VECTOR4* pDirection, CActor* pFiringActor);
	bool ProjectDirected(float velocity, edF32VECTOR4* pSource, edF32VECTOR4* pTarget, CActor* pFiringActor);

	uint GetProjectileIndex(edF32VECTOR4* pLocation, bool param_3);
	CActor* _ComputeNewShotNoRelease(float velocity, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection, S_SHOT_DATA** pShotData, CActor* pFiringActor, bool param_7);

	S_SHOT_DATA aShots[20];

	S_ACTOR_STREAM_REF* pActorStreamRef;
	int field_0x290;
	undefined4 field_0x294;
	uint field_0x298;
	uint field_0x29c;
};

struct CONTROLLED_PARAMS
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float fov;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
};

class CActorDCA : public CActor 
{
public:

	static StateConfig _gStateCfg_DCA[3];

	class CBhvDefault : public CBehaviour
	{
	public:
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void InitState(int newState);
		virtual void TermState(int oldState, int newState);
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		virtual edF32VECTOR4* GetLocation();
		virtual bool GetShouldDoSomething();

		CActorDCA* pOwner;
	};

	class CBhvControlled : public CBhvDefault
	{
	public:
		virtual void Create(ByteCode* pByteCode);
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void End(int newBehaviourId);
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		CONTROLLED_PARAMS controlledParams;
	};

	class CBhvIAControlled : public CBhvControlled
	{
		virtual void Create(ByteCode* pByteCode);
		virtual void Manage();
	};

	class CBhvTrajectory : public CBhvDefault
	{
		virtual void Create(ByteCode* pByteCode);
		virtual void Init(CActor* pOwner);
		virtual void Manage();
		virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
		virtual void End(int newBehaviourId);
		virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

		// CBhvDefault
		virtual edF32VECTOR4* GetLocation();
		virtual bool GetShouldDoSomething();

		uint field_0x8;

		byte field_0xc;

		float field_0x10;
		S_STREAM_REF<CActor> actorRef;
		float field_0x18;
		float field_0x1c;
	};

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);

	void UpdateAimAngle();

	void BehaviourDefault_InitState(int state);
	void BehaviourDefault_Manage(CBhvDefault* pBehaviour);
	void BehaviourControlled_Manage(CBhvControlled* pBehaviour, CONTROLLED_PARAMS* pParams);

	float aimAdjustSpeed;
	edF32VECTOR3 aimInputDirection;
	edF32VECTOR3 aimDirection;
	edF32VECTOR4 projectAimLocation;
	edF32VECTOR4 projectAimDirection;

	CActor* pControlledByActor;
	undefined4 field_0x1a4;

	CFireShot fireShot;

	CBhvDefault behaviourDefault;
	CBhvControlled behaviourControlled;
	CBhvIAControlled behaviourAiControlled;

	CCameraDCA* pCamera;

	uint field_0x450;
	uint field_0x454;

	byte bIsFiring;
	byte field_0x459;

	byte field_0x45a;
	byte field_0x45b;

	undefined4 field_0x45c;

	float field_0x460;
	float field_0x464;

	CVectorDyn vectorDyn;

	float field_0x4f0;
	byte field_0x4f8;
	CActorSound* field_0x4fc;

	S_STREAM_REF<CSound> field_0x500;
	S_STREAM_REF<CSound> field_0x504;
	S_STREAM_REF<CSound> field_0x508;
	S_STREAM_REF<CSound> field_0x50c;
};

#endif //ACTOR_D_C_A_H