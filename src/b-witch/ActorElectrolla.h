#ifndef ACTOR_ELECTROLLA_H
#define ACTOR_ELECTROLLA_H

#include "Types.h"
#include "Actor.h"
#include "ActorManager.h"
#include "CinematicManager.h"
#include "Fx_Spark.h"
#include "SharedLights.h"

#define ELECTROLLA_BEHAVIOUR_STAND 2
#define ELECTROLLA_STATE_STAND 5
#define ELECTROLLA_STATE_SCAN 6
#define ELECTROLLA_STATE_ALERT 7
#define ELECTROLLA_STATE_CHARGE 8
#define ELECTROLLA_STATE_DISCHARGE 9
#define ELECTROLLA_STATE_DISCHARGE_END 10
#define ELECTROLLA_STATE_COOLDOWN 11

class CActorElectrolla;

class CBehaviourElectrolla : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);

	CActorElectrolla* pOwner;
};

class CActorElectrolla : public CActor
{
public:
	CActorElectrolla();
	virtual ~CActorElectrolla();

	static StateConfig _gStateCfg_ELE[8];
	static CSharedLights<CLightOmni, 3> _gELE_Lights;

	virtual bool InitDlistPatchable(int patchId);
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual void BehaviourElectrolla_InitState(int newState);

	void BehaviourElectrolla_Manage();

	edF32VECTOR4 field_0x160;
	CFxSparkNoAlloc<3, 12>* field_0x170;
	int field_0x174;
	float field_0x178;
	float field_0x17c;
	float field_0x180;
	float field_0x184;
	float field_0x188;
	float field_0x18c;
	_rgba field_0x190;
	float field_0x194;
	float field_0x198;
	float field_0x19c;
	float field_0x1a0;
	int field_0x1a4;
	CFixedTable<CActor*, 4> field_0x1a8;
	int materialId;
	int dlistPatchId;
	S_NTF_SWITCH_ONOFF field_0x1c4;
	CBehaviourElectrolla behaviourStand;
};

struct _criterion_near_params
{
	CActorsTable* pTable;
	float* aDistances;
	CActor* pActor;
	float nearestDistance;
	float field_0x10;
};

float GetActorsNearWithCriterion(float radius, CActor* pActor, CActorsTable* pTable, ColCallbackFuncPtr* pFunc);

#endif //ACTOR_ELECTROLLA_H
