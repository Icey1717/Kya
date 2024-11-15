#ifndef ACTOR_AMBRE_H
#define ACTOR_AMBRE_H

#include "Types.h"
#include "Actor.h"

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

	void Init(ed_3D_Scene* pScene, ed_g3d_manager* pMeshManager, ed_3d_hierarchy_setup* pHierarchySetup, char* szString);

	edNODE* pMeshTransformParent;
	ed_3d_hierarchy_node* pMeshTransformData;

	int meshIndex;
	int textureIndex;

	edF32MATRIX4 perspectiveMatrix;
};

class CActorAmbre : public CActor {
public:
	static StateConfig _gStateCfg_AMB[6];

	CActorAmbre();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);

	void BehaviourStand_Manage(CBehaviourAmbre* pBehaviour);

	CFxHandle effectsStructC;
	StaticMeshComponent staticMesh;

	int field_0x1d0;

	uint field_0x1200;
	undefined4 field_0x1204;
	uint field_0x120c;
	uint field_0x1218;
	float field_0x1208;
	float field_0x1210;
	float field_0x1214;
	float field_0x121c;
	float field_0x1220;

	S_TARGET_STREAM_REF* pMagicalSwitch1CBase_0x1224;

	S_STREAM_EVENT_CAMERA* field_0x1228;

	CBehaviourAmbre behaviourAmber;
};

#endif //ACTOR_AMBRE_H