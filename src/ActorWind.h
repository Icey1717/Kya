#ifndef ACTOR_WIND_H
#define ACTOR_WIND_H

#include "Types.h"
#include "Actor.h"
#include "SectorManager.h"
#include "CinematicManager.h"

#define IMPLEMENTATION_GUARD_WIND_FX(x) 

class CActorWind;
class CWayPoint;

struct NotifyWindParam
{
	edF32VECTOR4 field_0x0;
	float field_0x10;
};

class CFxWind : public CObject
{
public:
	void SectorChange(int oldSectorId, int newSectorId) { IMPLEMENTATION_GUARD_WIND_FX(); }
	void Draw() { IMPLEMENTATION_GUARD_WIND_FX(); }
	void Reset() { IMPLEMENTATION_GUARD_WIND_FX(); }
};

class CBehaviourWind : public CBehaviour 
{
	// CBehaviour
	virtual void Create() {}
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { return 0; }

	CActorWind* pOwner;
};

class CActorWind : public CActor 
{
public:
	// CActor
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void ComputeLighting() {}
	virtual void Reset();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual void ChangeVisibleState(int state);
	virtual void TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);


	void BehaviourWind_Manage();

	float ComputeCurTime();
	int ComputeStateFromTime(float* pTime);
	void ComputeSpeedsInterpoltation();

	void ResetTiming();

	void NotifyActorInWindArea(float param_1, CActor* pNotifyActor);

	CSectorHierarchy sectorObj;
	CBehaviourWind behaviourWind;

	int materialId;

	uint nbFxWind;
	CFxWind* aFxWind;

	uint field_0x160;
	float maxWind;
	float field_0x168;
	float field_0x16c;
	float field_0x170;
	float field_0x174;
	float field_0x178;
	float field_0x17c;
	float field_0x180;
	float field_0x184;
	float field_0x188;
	float field_0x18c;
	uint field_0x190;
	float field_0x194;
	float field_0x198;
	float field_0x19c;
	float field_0x1a0;
	float field_0x1a4;
	S_STREAM_REF<CWayPoint> pWayPointRef;
	float field_0x1ac;
	float field_0x1b0;

	S_STREAM_REF<ed_zone_3d> activationZone;
	S_ACTOR_STREAM_REF* pActorStreamRef;
	S_STREAM_REF<ed_zone_3d> field_0x1c4;
	uint field_0x1c8;

	S_STREAM_REF<CWayPoint> field_0x1cc;

	S_TARGET_STREAM_REF* field_0x1d0;
	S_STREAM_EVENT_CAMERA* field_0x1d4;
	S_TARGET_STREAM_REF* field_0x1d8;
	S_STREAM_EVENT_CAMERA* field_0x1dc;
	S_TARGET_STREAM_REF* field_0x1e0;
	S_STREAM_EVENT_CAMERA* field_0x1e4;

	edF32MATRIX4* field_0x1e8;
	int* aAppliedToActorFlags;
	undefined4 field_0x1f0;
	float field_0x1fc;

	float curWindAlpha;
	float field_0x1f8;

	float field_0x200;
	float field_0x204;
	float field_0x208;
	float field_0x20c;
	float field_0x210;
	float time_0x214;
	uint field_0x218;

	ed_zone_3d* field_0x21c;
};

#endif //ACTOR_WIND_H