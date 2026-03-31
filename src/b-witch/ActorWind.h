#ifndef ACTOR_WIND_H
#define ACTOR_WIND_H

#include "Types.h"
#include "Actor.h"
#include "SectorManager.h"
#include "CinematicManager.h"
#include "FxLightEmitter.h"

#define IMPLEMENTATION_GUARD_WIND_FX(x) assert(false);

#define WIND_STATE_IDLE 0x5
#define WIND_STATE_DORMANT 0x6
#define WIND_STATE_FADE_IN 0x7
#define WIND_STATE_ACTIVE 0x8
#define WIND_STATE_FADE_OUT 0x9
#define WIND_STATE_COOLDOWN 0xa

// CFxWind::flags_0x54 bit flags
#define FXWIND_FLAG_ACTIVE 0x01
#define FXWIND_FLAG_SOLID_GRAPHICS_ENABLED 0x02
#define FXWIND_FLAG_CAMERA_ALIGNED 0x04

//The following flags are set from owner actor but never read
#define FXWIND_FLAG_UNUSED_0x08 0x08
#define FXWIND_FLAG_UNUSED_0x10 0x10
#define FXWIND_FLAG_UNUSED_0x20 0x20
#define FXWIND_FLAG_UNUSED_0x40 0x40
#define FXWIND_FLAG_UNUSED_0x80 0x80

#define FXWIND_FLAG_EMITTER_POOL_ACTIVE 0x100

class CActorWind;
class CWayPoint;
class CSoundWind;
class CFxWind;

class CFxWindHandle
{
public:
	CFxWind* pFxWind;
	float field_0x4;
};

struct _EmiNfo
{
	CFxLightEmitter field_0x0[3];
	CFxWind* pFxWind;
	undefined4 field_0x2a4[3];
	float field_0x2b0;
	float field_0x2b4;
};

struct HEAT_FX_PARAM
{
	int field_0x0;
	int field_0x4;
	int field_0x8;
	uint field_0xc;
	int field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	int field_0x20;
};

struct RayMem
{
	RAY_DEF* pRayDef;
	int nbRays;
};

struct _SP_PartNfo
{
	edF32VECTOR4* field_0x0;
	int field_0x4[3];
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
};

struct FASTRAM_MNG
{

};

class CFxEmitterPool
{
public:
	CFxEmitterPool();
	void Manage(CFxWind* pFxWind);
	void _SortRegNfo(int param_2);
	void _PrepareCommonSubPart(_EmiNfo* pEmiInfo, _SP_PartNfo* pSubPartInfo);
	void Draw();

	int field_0x0;
	undefined4 field_0x4;
	float field_0x8;
	undefined4 field_0xc;

	int nbHandles;
	int field_0x14;
	float field_0x18;
	CFxWindHandle aWindHandles[64];
	_EmiNfo aHolders[4];
	float field_0xd20[3];
	float field_0xd2c[3];
	HEAT_FX_PARAM heatFxParam;
	RayMem field_0xd5c[3];
	FASTRAM_MNG field_0xd74;
};

struct NotifyWindParam
{
	edF32VECTOR4 field_0x0;
	float field_0x10;
};

struct _wind_fx_vtx
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
};

struct _wind_fx_draw_state
{
	byte* pAlphaMultiplier;
	float alpha;
	float prevAlpha;
	int index;
	int drawState;
};

class CFxWindSolid
{
public:
	void _Create_SemiCylinder(_wind_fx_vtx* pVtx, int param_3, int param_4, int param_5);
	void _Create_FakePlane(float param_1, float param_2, float param_3, float param_4, _wind_fx_vtx* pVtx, int param_7, int param_8);
	void _Create_Whirl(_wind_fx_vtx* pVtx);
	bool CreatePatchableDlist(int patchId);
	void Draw(edF32MATRIX4* param_2);

	CFxWind* pOwner;
	uint field_0x4;

	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;

	int outerPatchId;
	int whirlPatchId;
	int fakePlanePatchId;
	void* pAlphaData;

	_wind_fx_draw_state outerStateA;
	_wind_fx_draw_state outerStateB;
	_wind_fx_draw_state fakePlaneState;
	_wind_fx_draw_state whirlStateA;
	_wind_fx_draw_state whirlStateB;

	_rgba field_0x8c;
};

struct MaterialManagerCombined
{
	edDList_material material;
	ed_g2d_manager manager;
};

struct edCSound3DPrim
{

};

class CFxWind : public CObject
{
public:
	struct WindAnimST
	{
		int field_0x0;
		float field_0x4;
		float field_0x8;
	};

	static WindAnimST gUseAnimST[5];

	CFxWind();
	virtual ~CFxWind();

	// CObject
	virtual bool IsKindOfObject(ulong kind);
	virtual bool InitDlistPatchable(int patchId);

	void Create(CActorWind* pOwner, int param_3);
	void Init(edF32MATRIX4* pMatrix);
	void SectorChange(int oldSectorId, int newSectorId);
	void Manage(int param_2);
	void Draw();
	void Reset();

	void RetrieveFlags(CActorWind* pWind);
	void _Compute_Matrix(edF32MATRIX4* pCameraMatrix);
	void ChangeVisibleState(int state);

	CActorWind* pOwner;

	edF32MATRIX4 field_0x10;

	uint flags_0x54;
	int field_0x58;
	edF32MATRIX4* field_0x5c;

	edF32MATRIX4 windMatrix;
	edF32MATRIX4 field_0xa0;
	edF32MATRIX4 field_0xe0;

	edF32VECTOR4 field_0x130;
	edF32VECTOR4 field_0x140;
	edF32VECTOR4 field_0x150;

	float field_0x160;

	int field_0x164;
	float distanceToCameraScale;
	float field_0x16c;
	float field_0x170;
	float baseAlpha;
	float perpendicularAlpha;
	float parallelAlpha;

	ed_g2d_manager* field_0x180;
	MaterialManagerCombined aCombinedMaterials[5];

	float field_0x2c4;
	float field_0x2c8;
	float field_0x2cc;
	float field_0x2d0;
	float field_0x2d4;

	CFxWindSolid windSolid;

	int outerPatchId;
	int whirlPatchId;
	int fakePlanePatchId;
	CActorSound* field_0x374;

	float field_0x390;

	edCSound3DPrim field_0x3a0;

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
	virtual ~CActorWind();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void ComputeLighting() {}
	virtual void Reset();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
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

	void ReComputeBoundingSphere(edF32MATRIX4* pMatrix);
	bool IsAValidFxPrimitive(CFxWind* pFx, byte param_3, edF32MATRIX4* param_4, edF32MATRIX4* param_5);

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

	CSoundWind* pSoundWind;

	S_STREAM_REF<ed_zone_3d> activationZone;
	S_ACTOR_STREAM_REF* pActorStreamRef;
	S_STREAM_REF<ed_zone_3d> field_0x1c4;
	_rgba field_0x1c8;

	S_STREAM_REF<CWayPoint> field_0x1cc;

	S_NTF_SWITCH field_0x1d0;
	S_NTF_SWITCH field_0x1d8;
	S_NTF_SWITCH field_0x1e0;

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