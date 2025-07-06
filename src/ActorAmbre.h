#ifndef ACTOR_AMBRE_H
#define ACTOR_AMBRE_H

#include "Types.h"
#include "Actor.h"
#include "Fx_Spark.h"
#include "Fx.h"

struct S_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

class CActorAmbre;

class CBehaviourAmbre : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual bool UnknownFunction() { return false; }

	CActorAmbre* pOwner;
};

class CBehaviourAmbreUnknown : public CBehaviourAmbre
{
public:
	virtual bool UnknownFunction() { return true; }
};

class StaticMeshComponent
{
public:
	StaticMeshComponent();

	virtual void Reset();
	virtual void SetHidden(ed_3D_Scene* pScene);
	virtual void SetVisible(ed_3D_Scene* pScene);
	virtual bool HasMesh();

	void Init(ed_3D_Scene* pScene, ed_g3d_manager* pMeshManager, ed_3d_hierarchy_setup* pHierarchySetup, char* szString);
	void Term(ed_3D_Scene* pScene);

	edNODE* pMeshTransformParent;
	ed_3d_hierarchy_node* pMeshTransformData;

	int meshIndex;
	int textureIndex;

	edF32MATRIX4 perspectiveMatrix;
};

struct AmberSparkProps
{
	uint boneId;
	float field_0x4;
	float field_0x8;
};

class CActorAmbre : public CActor {
public:
	static StateConfig _gStateCfg_AMB[6];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);

	void BehaviourStand_Manage(CBehaviourAmbre* pBehaviour);

	CFxSparkNoAlloc<4, 12> aFxSparks[3];
	AmberSparkProps aFxSparkProps[3];

	CFxHandleExt effectsStructC;
	StaticMeshComponent staticMesh;

	int field_0x1d0;

	CFxHandle effectsStructD;

	S_TARGET_STREAM_REF* pMagicalSwitch1CBase_0x1224;

	S_STREAM_EVENT_CAMERA* field_0x1228;

	CBehaviourAmbre behaviourAmber;
};

#endif //ACTOR_AMBRE_H