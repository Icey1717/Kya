#ifndef ACTOR_BONUS_H
#define ACTOR_BONUS_H

#include "Types.h"
#include "ActorMovable.h"
#include "ActorBonusServices.h"
#include "Fx_Tail.h"
#include "PathFollow.h"

#define BONUS_BEHAVIOUR_TURN 0x3
#define BONUS_BEHAVIOUR_PATH 0x4
#define BONUS_BEHAVIOUR_FLOCK 0x5
#define BONUS_BEHAVIOUR_ADD_ON 0x6

class CActorBonus;
class CShadowShared;
class CBehaviourBonusTurn;
class CBehaviourBonusPath;
class CBehaviourBonusFlock;

class CBnsInstance : public CActInstance
{
public:
	virtual void SetState(int newState);
	virtual float GetAngleRotY();
	virtual void CheckpointReset();

	void InitBns(CActor* pOwner, edF32VECTOR4* pPosition, edF32VECTOR4* pBoundSphere, int instanceIndex);
	bool InitDlistPatchable(int bIsFlare);
	void BehaviourTurn_Manage(CBehaviourBonusTurn* pBehaviour);
	void BehaviourPath_Manage(CBehaviourBonusPath* pBehaviour);

	void BehaviourAddOn_Manage(CBehaviourBonusFlock* pBehaviour);
	void BehaviourFlock_Manage(CBehaviourBonusFlock* pBehaviour);

	void MoveOnPath(float param_1, float speed, CBehaviourBonusPath* pBehaviour);

	void State_Turn(CBehaviourBonusTurn* pBehaviour);

	float field_0x9c;
	float angleRotYalt;
	float field_0xa4;
	SV_MOV_FLEE_ON_PATH_PARAM field_0xa8;
};

class CBehaviourBonusBase : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual void Func_0x4c();
	virtual void KillInstance(CBnsInstance* pInstance) = 0;
	virtual void ChangeVisibleState(int state);
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	float UpdateGroundDistanceAndShadow(edF32VECTOR4* pOrigin);

	CActorBonus* pOwner;

	float field_0x8;
	float field_0xc;
	float field_0x10;
};

class CBehaviourBonusAlone : public CBehaviourBonusBase
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool InitDlistPatchable(int patchId);

	virtual void KillInstance(CBnsInstance* pInstance);
	virtual void ChangeVisibleState(int state);
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	float field_0x14;

	CBnsInstance actInstance;

	CFxTail fxTail;

	float field_0x1b0;
	float field_0x1b4;
	float field_0x1b8;

	int bonusFlarePatchId;
	int bonusAnimPatchId;

	int field_0x1c4;
};

class CBehaviourBonusTurn : public CBehaviourBonusAlone
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	float field_0x1d0;
	float field_0x1d4;
	undefined4 field_0x1d8;
	uint field_0x1dc;
};

class CBehaviourBonusPath : public CBehaviourBonusAlone
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CPathPlane pathPlane;

	undefined4 field_0x1f0;
	float field_0x1f4;
	float field_0x1f8;
	float field_0x1fc;
	float field_0x200;
	uint field_0x204;
};

class CBehaviourBonusFlock : public CBehaviourBonusBase
{
public:
	CBehaviourBonusFlock();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool InitDlistPatchable(int patchId);

	virtual void KillInstance(CBnsInstance* pInstance);
	virtual void ChangeVisibleState(int state);

	S_STREAM_REF<CPathFollow> pathFollowRef;
	CShadowShared* aShadowShared;
	int nbSharedShadows;
	float field_0x18;
	int field_0x20;
	int flarePatchId;
	int animPatchId;
	int field_0x2c;
	undefined4 field_0x38;

	CBnsInstance* aBnsInstances;
	int nbInstances;
};

class CBehaviourBonusAddOn : public CBehaviourBonusFlock
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual void Func_0x4c();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	void Allocate(int nbNewInstances);
	CBnsInstance** Generate(edF32VECTOR4* pPosition, CAddOnGenerator_SubObj* pSubObj, int nbToSpawn, CBnsInstance** pInstance);
};

class COmniLight { };

template<class T, int count>
class CSharedLights
{
public:
	void Init(int param_1, int param_2, uint param_4, uint param_5, uint param_6) { IMPLEMENTATION_GUARD_LIGHT(); }
	void Term() { field_0x0 = 0; }

	undefined4 field_0x0;
};

class CActorBonus : public CActorMovable
{
public:
	static CSharedLights<COmniLight, 3> _gBNS_Lights;
	static StateConfig _gStateCfg_BNS[6];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void ComputeLighting() {}
	virtual void CheckpointReset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual void ChangeVisibleState(int state);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual void CinematicMode_Manage();
	virtual void CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition);

	CBehaviourBonusFlock behaviourBonusFlock;

	int flareMaterialId;
	int animMaterialId;
	CActorSound* field_0x1d8;
	S_STREAM_REF<CSound> field_0x1dc;
	CFxTail* pFxTail;
	edF32VECTOR4 vector_0x1e0;
};

#endif //ACTOR_BONUS_H