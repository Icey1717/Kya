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

struct MessageSoccerParams
{
	int field_0x0;
};

struct MessageSoccerParamsDetailed : public MessageSoccerParams
{
	float speed;
	float rotation;
};

struct MessageKickedParams
{
	int field_0x0;
	float field_0xc;
	edF32VECTOR4 impulseDirection;
	float field_0x30;
	float field_0x44;
};

enum ACTOR_MESSAGE {
	MESSAGE_KICKED = 0x2,
	MESSAGE_GET_ACTION = 0x12,
	MESSAGE_TRAP_STRUGGLE = 0x14,
	MESSAGE_ENTER_TRAMPO = 0x1d,
	MESSAGE_IMPULSE = 0x1e,
	MESSAGE_TRAP_CAUGHT = 0x31,
	MESSAGE_SOCCER_START = 0x35,
};

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
	virtual float GetValue() = 0;
	virtual void SetValue(float value) = 0;
};

struct SPEED_DYN {
	void Init(float param_1, float param_2);
	float UpdateLerp(float target);

	float field_0x0;
	float field_0x4;
};

struct ActorMessage_7 {
	int field_0x0;

	edF32VECTOR4 field_0x20;
};

class CVision {
public:
	void Create(CActor* pActor, ByteCode* pByteCode);
};

class CBehaviour 
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner) {}
	virtual void Term() {}
	virtual void Manage() {}
	virtual void SectorChange(int oldSectorId, int newSectorId) {}
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

class CObject {
public:
	CObject() : objectId(-1) { }

	int objectId;

	virtual bool IsKindOfObject(ulong kind) { return false; }
	virtual bool InitDlistPatchable() { return false; }
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
	int hashCode;
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

class CScalarDyn {
public:
	void BuildFromSpeedDist(float param_1, float param_2, float param_3);
	void Reset();
	bool IsFinished();
	void Integrate(float param_1);
	void Integrate(float param_1, float param_2);
	float GetInstantSpeed();
	bool OnLastValidSample();

	uint flags;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
};

class CVectorDyn {
public:
	void Integrate(float param_1, float param_2);
	void Integrate(float param_1);
	void Reset();
	bool IsFinished();
	void BuildFromAccelDistAmplitude(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, byte param_5);

	undefined4 field_0x0;
	byte field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	float field_0x8;
	float field_0xc;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
	float field_0x40;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	edF32VECTOR4 field_0x50;
	edF32VECTOR4 field_0x60;
	edF32VECTOR4 field_0x70;
};

class CPathFollowReaderAbsolute;
struct S_PATHREADER_POS_INFO;
struct edAnmMacroBlendN;

struct CActorParamsOut {
	edF32VECTOR4 vectorField;
	float floatField;
	uint flags;
};

struct CActorParamsIn {
	uint flags;
	float field_0x4;
	edF32VECTOR4* field_0x8;
};

struct HitMessageParams {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	float field_0xc;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
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
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
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
	undefined field_0x60;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
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

class CActorAlternateModel {};

class CActorSound;

class CActor : public CObject {
public:
	CActor();

	virtual bool IsKindOfObject(ulong kind);
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void ComputeLighting();
	virtual void Reset();
	virtual void PreCheckpointReset() {}
	virtual void CheckpointReset();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void TermBehaviour(int behaviourId, undefined8) { IMPLEMENTATION_GUARD(); }
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual void ChangeDisplayState(int state);
	virtual void ChangeVisibleState(int bVisible);
	virtual bool IsLockable();
	virtual uint GetBehaviourFlags(int state);
	virtual void SetState(int newState, int animType);
	virtual uint IsLookingAt();
	virtual void SetLookingAtOn();
	virtual void SetLookingAtOff();
	virtual void UpdateLookingAt();
	virtual void UpdateAnimEffects();
	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual void CinematicMode_Manage() { return; }
	virtual void CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition);
	virtual void CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int);
	virtual void TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5);
	virtual CVision* GetPerception() { IMPLEMENTATION_GUARD(); }
	virtual void UpdatePostAnimEffects();
	virtual bool Can_0x9c();
	virtual void AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);
	virtual void FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);
	virtual bool CinematicMode_InterpreteCinMessage(float, float, int param_2, int param_3);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual CPlayerInput* GetInputManager(int, int);

	void PreInit();
	void SetScaleVector(float x, float y, float z);
	bool SV_IsWorldBoundingSphereIntersectingSphere(edF32VECTOR4* param_2);
	bool SV_IsWorldBoundingSphereIntersectingBox(S_BOUNDING_BOX* pBoundingBox);
	void EvaluateManageState();
	void EvaluateDisplayState();

	void SetScale(float x, float y, float z);
	void SnapOrientation(float x, float y, float z);

	void LoadBehaviours(ByteCode* pByteCode);

	void SetupModel(int count, MeshTextureHash* aHashes);

	void SV_SetModel(int meshIndex, int textureIndex, int count, MeshTextureHash* aHashes);
	void SV_SetModel(ed_g3d_manager* pMeshInfo, int count, MeshTextureHash* aHashes, ed_g2d_manager* pTextureInfo);
	void SV_InstanciateMaterialBank();
	bool SV_UpdateOrientationToPosition2D(float speed, edF32VECTOR4* pOrientation);

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

	void PlayAnim(int inAnimType);
	int GetIdMacroAnim(int inAnimType);

	int DoMessage(CActor* pReceiver, ACTOR_MESSAGE type, MSG_PARAM flags);

	// #HACK
	void SkipToNextActor(ByteCode* pByteCode);

	bool SV_PatchMaterial(ulong originalHash, ulong newHash, ed_g2d_manager* pMaterial);

	void SV_BuildAngleWithOnlyY(edF32VECTOR3* v0, edF32VECTOR3* v1);

	static int SV_InstallMaterialId(int materialId);

	float SV_AttractActorInAConeAboveMe(CActor* pActor, CActorConeInfluence* pActorConeInfluence);

	void SV_LookTo(CActorParamsOut* pActorParamsOut, CActorParamsIn* pActorParamsIn);

	void FUN_00115ea0(uint param_2);

	void ComputeAltitude();

	int ReceiveEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* pEventData);

	bool ColWithAToboggan();
	bool ColWithLava();
	bool ColWithCactus();

	bool PlayWaitingAnimation(float param_1, float param_2, int specialAnimType, int regularAnimType, byte idleLoopsToPlay);

	bool UpdateNormal(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);

	void RestartCurAnim();

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
	int SV_UpdateMatrixOnTrajectory_Rel(float param_1, CPathFollowReaderAbsolute* pPathFollowReaderAbs, int param_4, int param_5, CActorsTable* pActorsTable, edF32MATRIX4* pMatrix, edF32VECTOR4* param_8, S_PATHREADER_POS_INFO* param_9);
	static void SV_Blend3AnimationsWith2Ratios(float r1, float r2, edAnmMacroBlendN* param_3, uint param_4, uint param_5, uint param_6);
	static void SV_Blend4AnimationsWith2Ratios(float r1, float r2, edAnmMacroBlendN* param_3, uint param_4, uint param_5, uint param_6, uint param_7);


	void UpdateShadow(edF32VECTOR4* pLocation, int bInAir, ushort param_4);
	CActor* GetCollidingActor();

	float FUN_00117db0();

	CActorSound* CreateActorSound(int soundType);

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
	void* field_0x98;

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

	void* pMBNK;

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
	void Init(int) { IMPLEMENTATION_GUARD_LOG(); }

	CAddOnGenerator_SubObj subObj;
};

class CAddOn {
public:
	CAddOn();
	virtual void Create(ByteCode* pByteCode) = 0;

	CActor* pOwner;
	undefined4 field_0x8;
};

struct ActorAndWaypoint {
	S_STREAM_REF<CActor> pActor;
	S_STREAM_REF<CWayPoint> pWaypoint;
};

struct S_ACTOR_STREAM_REF {
	int entryCount;
	S_STREAM_REF<CActor> aEntries[];
};

struct S_ZONE_STREAM_REF {
	int entryCount;
	S_STREAM_REF<ed_zone_3d> aEntries[];
};

#endif // _ACTOR_H
