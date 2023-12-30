#ifndef _ACTOR_H
#define _ACTOR_H

#include "Types.h"
#include "EdenLib/edCinematic/Sources/Cinematic.h"
#include "ed3D.h"
#include "Animation.h"
#include "EdenLib/edAnim/AnmSkeleton.h"
#include "port/pointer_conv.h"


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
	virtual bool Begin(CActor* pOwner, int newState, int newAnimationType) {}
	virtual void End(int newBehaviourId) {}
	virtual void InitState(int newState) {}
	virtual void TermState() {}
	virtual void GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites);
	virtual bool InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

struct CBehaviourStand : public CBehaviour
{
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner);
	virtual bool Begin(CActor* pOwner, int newState, int newAnimationType);
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
	int mainComponentID_0x0;
	edF32VECTOR4 boundingSphere;
	float floatFieldB; /* Created by retype action */
	float floatField;
	float field_0x1c;
	float field_0x20;
	int field_0x24;
	int hashCode;
	float field_0x2c;
	float field_0x30;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	int count_0x3c;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	uint flags_0x48;
	float field_0x4c;
});

struct CinNamedObject30 {
	int meshIndex;
	int textureIndex;
	int collisionDataIndex;
	edF32VECTOR3 position;
	edF32VECTOR3 vectorThree_0x18;
	float floatFieldA;
	float floatFieldB;
	float floatFieldC;
};

enum EActorState {
	AS_None = -1,
};

struct CClusterNode;
struct AnimMatrixData;

struct CollisionData;
struct CShadow;
struct ByteCode;

struct MeshTextureHash {
	ulong meshHash;
	ulong textureHash;
};

class CActor : public CObject {
public:
	byte field_0xd;
	uint flags;
	void* aComponents;

	int dlistPatchId;
	uint actorFieldS;

	int actorManagerIndex;

	edNODE* pMeshNode;
	ed_3d_hierarchy_node* p3DHierNode;
	ed_3d_hierarchy_node* pMeshTransform;

	edF32VECTOR3 rotationEuler;
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

	ACTOR_CLASS typeID;
	int prevBehaviourId;
	int curBehaviourId;

	int prevAnimType;
	int currentAnimType;

	float adjustedMagnitude;
	float distanceToGround;

	undefined4 field_0x58;
	float field_0xf0;

	float timeA;
	float idleTimer;
	int numIdleLoops;

	EActorState actorState;
	EActorState prevActorState;

	CClusterNode* pClusterNode;
	CAnimation* pAnimationController;
	CollisionData* pCollisionData;
	CShadow* pShadow;

	void* pMBNK;
	undefined4 field_0xcc;

	CBehaviourStand standBehaviour;

	CActor* actorFieldG;

	CActor();

	void PreInit();
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
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void ChangeManageState(int state);
	virtual void ChangeDisplayState(int state);
	virtual void SetState(int newState, int animType);
	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition);
	virtual void CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int);
	virtual uint IsLookingAt();
	virtual void UpdateAnimEffects();
	virtual void UpdatePostAnimEffects();
	virtual void ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);
	virtual bool InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual bool CinematicMode_InterpreteCinMessage(float, float, int param_2, int param_3);

	void ChangeVisibleState(int bVisible);

	CBehaviour* GetBehaviour(int behaviourId);

	void SetupClippingInfo();
	void SetupDefaultPosition();

	void RestoreInitData();
	void UpdatePosition(edF32VECTOR4* v0, bool bUpdateCollision);
	void UpdatePosition(edF32MATRIX4* pPosition, int bUpdateCollision);

	void PlayAnim(int inAnimType);

	// #HACK
	void SkipToNextActor(ByteCode* pByteCode);

	bool SV_PatchMaterial(ulong originalHash, ulong newHash, ed_g2d_manager* pMaterial);
};

class CActorMovable : public CActor {
public:
	CActorMovable() {
		IMPLEMENTATION_GUARD_LOG();
	}

	virtual void Create(ByteCode* pByteCode);

	float field_0x1c0;
};

class CActorAutonomous : public CActorMovable {
public:
	CActorAutonomous() {
		IMPLEMENTATION_GUARD_LOG();
	}

	virtual void Create(ByteCode* pByteCode);
};


class CAddOnGenerator_SubObj {
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
