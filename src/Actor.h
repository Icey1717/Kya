#ifndef _ACTOR_H
#define _ACTOR_H

#include "Types.h"
#include "EdenLib/edCinematic/Sources/Cinematic.h"
#include "ed3D.h"
#include "Animation.h"
#include "EdenLib/edAnim/AnmSkeleton.h"
#include "port/pointer_conv.h"

#include <assert.h>

#define ACTOR_LOG(level, format, ...) MY_LOG_CATEGORY("Actor", level, format, ##__VA_ARGS__)

struct edNODE;
struct CActor;
struct ed_g3d_hierarchy;
struct ed_3d_hierarchy_node;


enum ACTOR_MESSAGE {};
typedef void* MSG_PARAM;

struct CBehaviour 
{
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner) {}
	virtual void Manage() {}
	virtual void SectorChange(int oldSectorId, int newSectorId) {}
	virtual void Draw() {}
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType) {}
	virtual void End(int newBehaviourId) {}
	virtual void InitState(int newState) {}
	virtual void TermState(int, int) {}
	virtual void GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites);
	virtual bool InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

struct CBehaviourStand : public CBehaviour
{
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
	CObject() {
		objectId - -1;
	}

	int objectId;

	virtual bool IsKindOfObject(ulong kind) { return false; }
	virtual bool InitDlistPatchable() { return false; }
};

PACK(
struct KyaUpdateObjA {
	int defaultBehaviourId;
	edF32VECTOR4 boundingSphere;
	float floatFieldB; /* Created by retype action */
	float floatField;
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
struct AnimMatrixData;

class CCollision;

class CActorsTable
{
public:
	int entryCount;
	CActor* aEntries[59];
};

struct CShadow;
struct ByteCode;
struct AnimResult {
	AnimResult() {}

	AnimResult(int inA, uint inB) : field_0x0(inA), flags_0x4(inB) {}

	int field_0x0;
	uint flags_0x4;
};

struct MeshTextureHash {
	ulong meshHash;
	ulong textureHash;
};

class CActor : public CObject {
public:
	// #Debug
	char name[64];

	byte field_0xd;
	uint flags;
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

	float adjustedMagnitude;
	float distanceToGround;

	undefined4 field_0x58;

	uint lightingFlags;
	float lightingFloat_0xe0;
	ushort field_0xf0;

	float timeA;
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

	static AnimResult gStateCfg_ACT[5];
	static uint _gBehaviourFlags_ACT[2];

	float lodBiases[4];

	CActor();

	void PreInit();
	void SetScaleVector(float x, float y, float z);
	bool SV_IsWorldBoundingSphereIntersectingSphere(edF32VECTOR4* param_2);
	void EvaluateManageState();
	void EvaluateDisplayState();

	void SetScale(float x, float y, float z);

	void LoadBehaviours(ByteCode* pByteCode);

	void SetupModel(int count, MeshTextureHash* aHashes);

	void SV_SetModel(int meshIndex, int textureIndex, int count, MeshTextureHash* aHashes);
	void SV_SetModel(ed_g3d_manager* pMeshInfo, int count, MeshTextureHash* aHashes, ed_g2d_manager* pTextureInfo);


	virtual bool IsKindOfObject(ulong kind);
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage();
	virtual void Draw();
	virtual void ComputeLighting();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void ChangeManageState(int state);
	virtual void ChangeDisplayState(int state);
	virtual AnimResult* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);
	virtual void SetState(int newState, int animType);
	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition);
	virtual void CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int);
	virtual uint IsLookingAt();
	virtual void UpdateAnimEffects();
	virtual void UpdatePostAnimEffects();
	virtual bool ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);
	virtual bool InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual bool CinematicMode_InterpreteCinMessage(float, float, int param_2, int param_3);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual void Reset();

	void ComputeWorldBoundingSphere(edF32VECTOR4* v0, edF32MATRIX4* m0);
	void ChangeVisibleState(int bVisible);

	void UpdateVisibility();
	void FUN_00101110(CActor* pOtherActor);

	CBehaviour* GetBehaviour(int behaviourId);

	void SetupClippingInfo();
	void SetupLodInfo();
	void SetupDefaultPosition();
	void SetupLighting();

	void PreReset();

	void RestoreInitData();
	void UpdatePosition(edF32VECTOR4* v0, bool bUpdateCollision);
	void UpdatePosition(edF32MATRIX4* pPosition, int bUpdateCollision);

	void PlayAnim(int inAnimType);
	int GetIdMacroAnim(int inAnimType);

	bool DoMessage(CActor* pReceiver, ACTOR_MESSAGE type, MSG_PARAM flags);

	// #HACK
	void SkipToNextActor(ByteCode* pByteCode);

	bool SV_PatchMaterial(ulong originalHash, ulong newHash, ed_g2d_manager* pMaterial);

	void SV_BuildAngleWithOnlyY(edF32VECTOR3* v0, edF32VECTOR3* v1);

	void FUN_00115ea0(uint param_2);

	void ComputeAltitude();

	void TieToActor(CActor* pTieActor, int carryMethod, int param_4);
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

	CAddOnGenerator_SubObj subObj;
};

class CBehaviourAddOnBase {
public:
	virtual void Create(ByteCode* pByteCode) = 0;
};

struct ActorAndWaypoint {
	CActor* pActor;
	struct ManagerC_24* pWaypoint;
};

#endif // _ACTOR_H
