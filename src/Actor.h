#ifndef _ACTOR_H
#define _ACTOR_H

#include "Types.h"
#include "EdenLib/edCinematic/Sources/Cinematic.h"

struct edNODE;
struct CActor;
struct ed_g3d_hierarchy;

struct CBehaviour {
	virtual void Init(CActor* pOwner) {}
	virtual void GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites);
};

struct ComponentEntry {
	int id;
	CBehaviour* pComponent;
};

template<int T>
struct ComponentList {
	int count;
	ComponentEntry aComponents[T];
};

struct CObject {
	undefined4 field_0x4;
};

struct CActor : public CObject {
	int field_0x0;
	char field_0xd;
	uint flags;
	void* aComponents;

	int dlistPatchId;
	uint actorFieldS;

	edNODE* pMeshNode;

	edF32VECTOR3 rotationEuler;
	edF32VECTOR4 rotationQuat;
	edF32VECTOR4 scale;

	ed_g3d_hierarchy* pHier;

	int typeID;

	float adjustedMagnitude;

	CActor();

	void PreInit();
	void EvaluateDisplayState();

	void SetScale(float x, float y, float z);

	virtual void Init();
	virtual void ChangeDisplayState(int state);
	virtual void SetState(int newState, int AnimationType);
	virtual void CinematicMode_Enter(bool param_2);
	virtual void CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int);
};

#endif // _ACTOR_H
