#ifndef _ACTOR_H
#define _ACTOR_H

#include "Types.h"
#include "EdenLib/edCinematic/Sources/Cinematic.h"
#include "ed3D.h"
#include "Animation.h"
#include "EdenLib/edAnim/AnmSkeleton.h"
#include "port/pointer_conv.h"

#include <assert.h>

#define CHAR_TO_UINT64(str) \
	(static_cast<unsigned long long>(str[0]) | \
	(static_cast<unsigned long long>(str[1]) << 8) | \
	(static_cast<unsigned long long>(str[2]) << 16) | \
	(static_cast<unsigned long long>(str[3]) << 24) | \
	(static_cast<unsigned long long>(str[4]) << 32) | \
	(static_cast<unsigned long long>(str[5]) << 40) | \
	(static_cast<unsigned long long>(str[6]) << 48) | \
	(static_cast<unsigned long long>(str[7]) << 56))

#define ACTOR_LOG(level, format, ...) MY_LOG_CATEGORY("Actor", level, format, ##__VA_ARGS__)

struct edNODE;
struct ed_g3d_hierarchy;
struct ed_3d_hierarchy_node;
struct S_BOUNDING_BOX;
struct edCEventMessage;

class CWayPoint;

class CPlayerInput;

class CVision;

class CInventoryInfo;

struct MessageSoccerParams
{
	int field_0x0;
};

struct MessageSoccerParamsDetailed : public MessageSoccerParams
{
	float speed;
	float rotation;
};

struct GetActionMsgParams
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
};

struct _msg_tied_params
{
	edF32VECTOR4* pPosition;
	int bTied;
};

enum ACTOR_MESSAGE {
	MESSAGE_KICKED = 0x2,
	MESSAGE_GET_VISUAL_DETECTION_POINT = 0x7,
	MESSAGE_GET_RUN_SPEED = 0xc,
	MESSAGE_TIED = 0xd,
	MESSAGE_GET_ACTION = 0x12,
	MESSAGE_TRAP_STRUGGLE = 0x14,
	MESSAGE_IN_WIND_AREA = 0x16,
	MESSAGE_ENTER_WIND = 0x17,
	MESSAGE_LEAVE_WIND = 0x18,
	MESSAGE_FIGHT_ACTION_SUCCESS = 0x1b,
	MESSAGE_ENTER_TRAMPO = 0x1d,
	MESSAGE_IMPULSE = 0x1e,
	MESSAGE_ENTER_SHOP = 0x23,
	MESSAGE_LEAVE_SHOP = 0x24,
	MESSAGE_DISABLE_INPUT = 0x25,
	MESSAGE_ENABLE_INPUT = 0x26,
	MESSAGE_TRAP_CAUGHT = 0x31,
	MESSAGE_SOCCER_START = 0x35,
	MESSAGE_REQUEST_CAMERA_TARGET = 0x49,
	MESSAGE_GET_BONE_ID = 0x4d,
	MESSAGE_NATIV_CMD = 0x4e,
	MESSAGE_BOOMY_CHANGED = 0x62,
	MESSAGE_FIGHT_RING_CHANGED = 0x6b,
	MESSAGE_RECEPTACLE_CHANGED = 0x79,
};

#define OBJ_TYPE_MOVABLE	0x2
#define OBJ_TYPE_AUTONOMOUS 0x4
#define OBJ_TYPE_FIGHTER	0x8
#define OBJ_TYPE_HERO		0x10

typedef void* MSG_PARAM;

#define HERO_ACTION_ID_JOKE 0x3
#define HERO_ACTION_ID_ESCAPE_TRAP 0xc

class CActorConeInfluence {
public:
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	int field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	edF32VECTOR4 field_0x20;
};

class CInterface {
public:
	virtual bool Activate(int bActive) { return true; }
	virtual bool CanActivate() { return true; }
	virtual bool IsActive() { return true; }
	virtual bool Manage() { return true; }

	virtual void Draw() = 0;
	virtual void Reset() = 0;
	virtual void SetValue(float value) = 0;
	virtual float GetValue() = 0;
};

class CBehaviour : public CObject
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner) {}
	virtual void Term() {}
	virtual void Manage() {}
	virtual void ManageFrozen() {}
	virtual void SectorChange(int oldSectorId, int newSectorId) {}
	virtual void PauseChange(int bPaused) {}
	virtual void Draw() {}
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType) {}
	virtual void End(int newBehaviourId) {}
	virtual void InitState(int newState) {}
	virtual void TermState(int oldState, int newState) {}
	virtual void GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }
};

class CBehaviourInactive : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage() {}
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	int activateMsgId;
	uint flags;
	int activeBehaviourId;
	CActor* pOwner;
};

class CBehaviourStand : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState) {}
	virtual void TermState() {}

	CActor* pOwner;
};

PACK( struct BehaviourEntry {
	int id;
	int pBehaviour; // CBehaviour*

	inline CBehaviour* GetBehaviour() {
		return (CBehaviour*)LOAD_SECTION(pBehaviour);
	}

	inline void SetBehaviour(CBehaviour* pNewBehaviour) {
		pBehaviour = STORE_SECTION(pNewBehaviour);
	}

	// Only valid when loading behaviours.
	inline int GetSize() {
		return pBehaviour;
	}
});

template<int T>
struct BehaviourList {
	int count;
	BehaviourEntry aComponents[T];
};

PACK(
struct KyaUpdateObjA {
	int defaultBehaviourId;
	edF32VECTOR4 boundingSphere;
	float floatFieldB; /* Created by retype action */
	float visibilityDistance;
	float field_0x1c;
	float field_0x20;
	int field_0x24;
	uint hashCode;
	float lodBiases[2];
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	int actorFieldS;
	int animLayerCount;
	int field_0x40;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	uint flags_0x48;
	float lightingFloat_0x4c;
});

struct CinNamedObject30 {
	int meshIndex;
	int textureIndex;
	int collisionDataIndex;
	edF32VECTOR3 position;
	edF32VECTOR3 rotationEuler;
	edF32VECTOR3 scale;
};

enum EActorState {
	AS_None = -1,
};

struct CClusterNode;
struct BoneData;

class CCollision;

struct CShadow;
struct ByteCode;
struct StateConfig {
	StateConfig() {}

	StateConfig(int inA, uint inB) : animId(inA), flags_0x4(inB) {}

	int animId;
	uint flags_0x4;
};

struct MeshTextureHash {
	ulong meshHash;
	ulong textureHash;
};

class CPathFollowReaderAbsolute;
struct S_PATHREADER_POS_INFO;
struct edAnmMacroBlendN;

struct CActorParamsOut
{
	edF32VECTOR4 moveDirection;
	float moveVelocity;
	uint flags;
};

struct CActorParamsIn
{
	uint flags;
	float rotSpeed;
	edF32VECTOR4* pRotation;
};

#define HIT_TYPE_KICK 0x3
#define HIT_TYPE_BOOMY 0x4

struct _msg_hit_param
{
	int projectileType;
	int field_0x4;
	uint flags;
	float damage;
	float field_0x10;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	edF32VECTOR4 field_0x20;
	float field_0x30;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	edF32VECTOR4 field_0x40;
	short field_0x50;
	ushort field_0x52;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	edF32VECTOR4 field_0x60;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	int field_0x74;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	edF32VECTOR4 field_0x80;
};

class CActorAlternateModel
{
public:
	edF32VECTOR4 cachedBoundingSphere;
	ed_3d_hierarchy_node* pHierarchy;
};

struct SOUND_SPATIALIZATION_PARAM
{

};

class CActorSound
{
public:
	void SoundStart(CActor* pActor, int param_3, CSound* pSound, long param_5, int param_6, SOUND_SPATIALIZATION_PARAM* pSoundSpatializationParam) { IMPLEMENTATION_GUARD_AUDIO(); }
	void SoundStop(int) { IMPLEMENTATION_GUARD_AUDIO(); }
	void SetFrequency(float frequency, int) { IMPLEMENTATION_GUARD_AUDIO(); }
};

struct GetPositionMsgParams
{
	int field_0x0;
	edF32VECTOR4 vectorFieldA;
	edF32VECTOR4 vectorFieldB;
};

struct CCineActorConfig;

class CActInstance
{
public:
	CActInstance();

	virtual void SetState(int newState);
	virtual void CheckpointReset();

	void Init(CActor* pOwner, edF32VECTOR4* pPosition, edF32VECTOR4* pBoundSphere, int param_5); 
	void Term();
	void SetVisible(int bVisible);
	void Reset();
	void SetAlive(int bAlive);

	float DistSquared(edF32VECTOR4* pPosition);

	void State_GotoKim();
	void State_Wait();

	void FUN_003982c0();

	uint flags;

	CActor* pOwner;

	edF32VECTOR4 basePosition;
	edF32VECTOR4 currentPosition;
	edF32VECTOR4 field_0x40;

	float field_0x54;
	float field_0x58;
	float field_0x5c;
	int field_0x60;

	edNODE* pNode;
	ed_3d_hierarchy* pHierarchy;

	int state;

	edF32VECTOR3 field_0x64;
	edF32VECTOR3 field_0x70;

	float field_0x7c;
	undefined4 field_0x84;
	undefined4 field_0x88;

	float field_0x90;
	float angleRotY;
	float field_0x98;
};

class CActor : public CObject
{
public:
	CActor();

	// CActor Interface
	virtual bool IsKindOfObject(ulong kind);
	virtual bool InitDlistPatchable(int);

	virtual void Create(ByteCode* pByteCode);
	virtual void Destroy();

	virtual void Init();
	virtual void Term();

	virtual void Manage();
	virtual void Draw();
	virtual void ComputeLighting();

	virtual void Reset();
	virtual void PreCheckpointReset() {}
	virtual void CheckpointReset();

	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void PauseChange(int bIsPaused);

	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);

	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void TermBehaviour(int behaviourId, CBehaviour* pBehaviour);

	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);

	virtual uint IsLookingAt();
	virtual void SetLookingAtOn();
	virtual void SetLookingAtOff();
	virtual void UpdateLookingAt();

	virtual void UpdateAnimEffects();
	virtual void UpdatePostAnimEffects();

	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual void SetState(int newState, int animType);

	virtual void ChangeManageState(int state);
	virtual void ChangeDisplayState(int state);
	virtual void ChangeVisibleState(int bVisible);

	virtual bool IsLockable();
	virtual bool IsProjectionAim();
	virtual bool CanPassThrough();
	virtual bool Can_0x9c();

	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);

	virtual int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);

	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual void CinematicMode_Manage() { return; }
	virtual void CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition);
	virtual void CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int);
	virtual void CinematicMode_InterpolateTo(CCineActorConfig* pConfig, edF32MATRIX4* param_3, edF32MATRIX4* param_4);
	virtual bool CinematicMode_InterpreteCinMessage(float, float, int param_2, int param_3);

	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);

	virtual CPlayerInput* GetInputManager(int, int);

	virtual bool IsMakingNoise();

	virtual void TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5);

	virtual void Func_0xd4(ed_zone_3d* pZone) {}
	virtual void Func_0xd8(ed_zone_3d* pZone) {}

	virtual CVision* GetVision() { IMPLEMENTATION_GUARD(); }
	virtual int GetNumVisualDetectionPoints();
	virtual void GetVisualDetectionPoint(edF32VECTOR4* pOutPoint, int index);

	virtual CInventoryInfo* GetInventoryInfo() { return NULL; }

	virtual void FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);
	// End Interface

	void PreInit();
	void SetScaleVector(float x, float y, float z);
	bool SV_IsWorldBoundingSphereIntersectingSphere(edF32VECTOR4* param_2);
	bool SV_IsWorldBoundingSphereIntersectingBox(S_BOUNDING_BOX* pBoundingBox);
	void EvaluateManageState();
	void EvaluateDisplayState();

	uint GetStateFlags(int state);

	void UpdateClusterNode();

	void SetScale(float x, float y, float z);
	void SnapOrientation(float x, float y, float z);

	void LoadBehaviours(ByteCode* pByteCode);

	void SetupModel(int count, MeshTextureHash* aHashes);

	void SV_SetModel(int meshIndex, int textureIndex, int count, MeshTextureHash* aHashes);
	void SV_SetModel(ed_g3d_manager* pMeshInfo, int count, MeshTextureHash* aHashes, ed_g2d_manager* pTextureInfo);
	void SV_InstanciateMaterialBank();
	bool SV_UpdateOrientationToPosition2D(float speed, edF32VECTOR4* pOrientation);
	bool SV_IsOrientation2DInRange(float param_1, edF32VECTOR4* param_3);
	bool SV_IAmInFrontOfThisActor(CActor* pOther);

	void SV_RestoreOrgModel(CActorAlternateModel* pActorAlternateModel);
	void SV_SwitchToModel(CActorAlternateModel* pAlternateModel, ed_g3d_manager* p3dManager, edF32VECTOR4* pBoundingSphere);
	void SV_SwitchToModel(CActorAlternateModel* pAlternateModel, int meshIndex, int materialIndex, edF32VECTOR4* pBoundingSphere);


	void SetLocalBoundingSphere(float radius, edF32VECTOR4* pLocation);
	void ComputeWorldBoundingSphere(edF32VECTOR4* v0, edF32MATRIX4* m0);

	void UpdateVisibility();
	void FUN_00101110(CActor* pOtherActor);

	CBehaviour* GetBehaviour(int behaviourId);

	void ResetActorSound();

	void SetupClippingInfo();
	void SetupLodInfo();
	void SetupDefaultPosition();
	void SetupLighting();

	void PreReset();

	edF32VECTOR4* GetBottomPosition();
	edF32VECTOR4* GetTopPosition();

	void RestoreInitData();
	void UpdatePosition(edF32VECTOR4* v0, bool bUpdateCollision);
	void UpdatePosition(edF32MATRIX4* pPosition, int bUpdateCollision);

	void LinkToActor(CActor* pLinkedActor, uint key, int param_4);
	void UnlinkFromActor();

	void SetAlpha(byte alpha);
	void ToggleMeshAlpha();

	void SetBFCulling(byte bActive);

	void PlayAnim(int inAnimType);
	int GetIdMacroAnim(int inAnimType);

	void SV_GetActorTargetPos(CActor* pOtherActor, edF32VECTOR4* pTargetPos);

	int DoMessage(CActor* pReceiver, ACTOR_MESSAGE type, MSG_PARAM flags);

	// #HACK
	void SkipToNextActor(ByteCode* pByteCode);
	void SkipToNextActorNoBase(ByteCode* pByteCode);

	bool SV_PatchMaterial(ulong originalHash, ulong newHash, ed_g2d_manager* pMaterial);

	void SV_GetActorHitPos(CActor* pOtherActor, edF32VECTOR4* v0);
	float SV_GetDirectionalAlignmentToTarget(edF32VECTOR4* v0);
	void SV_BuildAngleWithOnlyY(edF32VECTOR3* v0, edF32VECTOR3* v1);

	static int SV_InstallMaterialId(int materialId);

	float SV_AttractActorInAConeAboveMe(CActor* pActor, CActorConeInfluence* pActorConeInfluence);

	void SV_LookTo(CActorParamsOut* pActorParamsOut, CActorParamsIn* pActorParamsIn);

	void FUN_00115ea0(uint param_2);

	void ComputeAltitude();
	void ComputeRotTransMatrix(edF32MATRIX4* pOutMatrix);
	void ComputeLocalMatrix(edF32MATRIX4* m0, edF32MATRIX4* m1);
	void Compute2DOrientationFromAngles();

	int ReceiveEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* pEventData);

	bool ColWithAToboggan();
	bool ColWithLava();
	bool ColWithCactus();

	bool PlayWaitingAnimation(float param_1, float param_2, int specialAnimType, int regularAnimType, byte idleLoopsToPlay);

	bool UpdateNormal(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);

	void RestartCurAnim();

	void SV_GetGroundPosition(edF32VECTOR4* v0);
	void SV_GetActorColCenter(edF32VECTOR4* pColCenter);
	float SV_GetCosAngle2D(edF32VECTOR4* pToLocation);
	bool SV_Vector4SLERP(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	void SV_GetBoneDefaultWorldPosition(uint boneId, edF32VECTOR4* pOutPosition);
	void SV_GetBoneWorldPosition(int boneIndex, edF32VECTOR4* pOutPosition);
	void SV_UpdatePosition_Rel(edF32VECTOR4* pPosition, int param_3, int param_4, CActorsTable* pActorsTable, edF32VECTOR4* param_6);
	void SV_ComputeDiffMatrixFromInit(edF32MATRIX4* m0);
	bool SV_UpdateOrientation(float param_1, edF32VECTOR4* pOrientation);
	bool SV_UpdateOrientation2D(float speed, edF32VECTOR4* pNewOrientation, int mode);
	void SV_UpdatePercent(float param_1, float param_2, float* pValue);
	bool SV_UpdateValue(float target, float speed, float* pValue);
	void SV_UpdateMatrix_Rel(edF32MATRIX4* m0, int param_3, int param_4, CActorsTable* pActorsTable, edF32VECTOR4* v0);
	void SV_InheritMatrixFromTiedToActor(edF32MATRIX4* m0);
	bool SV_AmICarrying(CActor* pOther);
	void SV_SetOrientationToPosition2D(edF32VECTOR4* pPosition);
	int SV_UpdateMatrixOnTrajectory_Rel(float param_1, CPathFollowReaderAbsolute* pPathFollowReaderAbs, int param_4, int param_5, CActorsTable* pActorsTable, edF32MATRIX4* pMatrix, edF32VECTOR4* param_8, S_PATHREADER_POS_INFO* pPathReaderPosInfo);
	static void SV_Blend3AnimationsWith2Ratios(float r1, float r2, edAnmMacroBlendN* param_3, uint param_4, uint param_5, uint param_6);
	static void SV_Blend4AnimationsWith2Ratios(float r1, float r2, edAnmMacroBlendN* param_3, uint param_4, uint param_5, uint param_6, uint param_7);


	void UpdateShadow(edF32VECTOR4* pLocation, int bInAir, ushort param_4);
	CActor* GetCollidingActor();

	float GetPosition_00117db0();
	void GetPosition_00101130(edF32VECTOR4* pOutPosition);

	CActorSound* CreateActorSound(int soundType);

	CActor* GetLinkFather();

	void UpdateBoundingSphere(CActInstance* pInstances, int nbInstances);

#ifdef DEBUG_FEATURES
	// #Debug
	char name[64];
#endif

	uint flags;
	byte state_0x10;
	byte field_0x11;
	void* aComponents;

	int dlistPatchId;
	uint actorFieldS;

	int actorManagerIndex;

	edNODE* pMeshNode;
	ed_3d_hierarchy_node* p3DHierNode;
	ed_3d_hierarchy_node* pMeshTransform;

	edF32VECTOR4 rotationEuler;
	edF32VECTOR4 rotationQuat;
	edF32VECTOR4 scale;

	ed_g3d_hierarchy* pHier;

	// pMaterial bank from SV_InstanciateMaterialBank
	void* pMBNK;

	CinNamedObject30 namedObjSectionStart;
	CinNamedObject30* pCinData;

	KyaUpdateObjA otherSectionStart;
	KyaUpdateObjA* subObjA;

	ed_3d_hierarchy_setup hierarchySetup;

	edF32VECTOR4 baseLocation;
	edF32VECTOR4 sphereCentre;
	edF32VECTOR4 currentLocation;
	edF32VECTOR3 previousLocation;

	undefined* field_0x110;
	edF32VECTOR3 vector_0x120;
	edF32VECTOR3 vector_0x12c;

	ACTOR_CLASS typeID;
	int prevBehaviourId;
	int curBehaviourId;

	int prevAnimType;
	int currentAnimType;

	float distanceToCamera;
	float distanceToGround;

	undefined4 field_0x58;

	uint lightingFlags;
	float lightingFloat_0xe0;
	float field_0xf0;
	ushort field_0xf4;

	float timeInAir;
	float idleTimer;
	int numIdleLoops;

	EActorState actorState;
	EActorState prevActorState;

	CClusterNode* pClusterNode;
	CAnimation* pAnimationController;
	CCollision* pCollisionData;
	CShadow* pShadow;

	CBehaviourStand standBehaviour;

	CActor* pTiedActor;

	static StateConfig gStateCfg_ACT[5];
	static uint _gBehaviourFlags_ACT[2];

	float lodBiases[4];
};

class CAddOnGenerator_SubObj 
{
public:
	void Create(ByteCode* pByteCode);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
};

class CAddOnGenerator {
public:
	void Create(CActor* pActor, ByteCode* pByteCode);
	void Init(int) { IMPLEMENTATION_GUARD_ADD_ON(); }
	void Term() { IMPLEMENTATION_GUARD_ADD_ON(); }

	void Generate(edF32VECTOR4* pPosition) { IMPLEMENTATION_GUARD_ADD_ON(); }

	CAddOnGenerator_SubObj subObj;
};

class CCinematic;

class CAddOnSubObj
{
public:
	void SetCinematic(CCinematic* pCinematic);
	int PickCinematic();

	uint field_0x0;
	int* aCinematicIds;
	int nbCinematics;
	CCinematic* pCinematic;
	int lastPlayedCinematicId;
	float field_0x14;
};

class CAddOn
{
public:
	CAddOn();
	virtual void Create(ByteCode* pByteCode) = 0;
	virtual void Init(CActor* pActor);
	virtual void Manage();
	virtual void Reset();
	virtual CAddOnSubObj* GetSubObj(uint param_2, int pActor) = 0;
	virtual bool Func_0x20(uint param_2, CActor* param_3, int pActor) = 0;
	virtual bool Func_0x24(uint param_2, CActor* pActor) = 0;
	virtual bool Func_0x34(uint param_2, CActor* pActor);

	CCinematic* GetCinematic();

	CActor* pOwner;
	CAddOnSubObj* pSubObj;
	byte field_0xc;
	byte field_0xd;
};

struct ActorAndWaypoint {
	S_STREAM_REF<CActor> pActor;
	S_STREAM_REF<CWayPoint> pWaypoint;
};

struct S_ACTOR_STREAM_REF {
	int entryCount;
	S_STREAM_REF<CActor> aEntries[];
};

class CEmotionInfo {
public:
	void DoAnimation(float, float, CActor*);

	int macroAnimId;
	int field_0x4;
	int field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
};

#endif // _ACTOR_H
