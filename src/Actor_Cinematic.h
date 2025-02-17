#ifndef _ACTOR_CINEMATIC_H
#define _ACTOR_CINEMATIC_H

#include "Types.h"
#include "Actor.h"
#include "EdenLib/edCinematic/Sources/Cinematic.h"
#include "EdenLib/edCinematic/Sources/CinScene.h"
#include "CinematicManager.h"

struct ed_g2d_manager;
struct ed_g3d_manager;
struct ed_3D_Scene;
struct CActorCinematic;

struct CBehaviourCinematic : public CBehaviour {
	CActorCinematic* pOwner;
	CBWCinActor cinActor;
	int leaveCinematicBehaviourId;

	CEmotionInfo field_0x144;

	int field_0x178;

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void Manage();
	virtual void InitState(int newState);

	bool CinematicMode_InterpreteCinMessage(int param_2, int param_3);
};

struct CActorCinematic : public CActor {
	CActorCinematic();

	void PatchMaterialForCutscene(ed_g3d_manager* pG3D, ed_g2d_manager* pG2D);
	void Create(const edCinGameInterface::ACTORV_CREATIONtag* pGameInterface, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D, ed_3D_Scene* pScene);

	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init();
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_Leave(int behaviourId);

	CAnimation animationController;
	edAnmLayer anmLayers[2];

	BehaviourList<1> components;
	CBehaviourCinematic behaviourCinematic;

	ed_3d_hierarchy_node field_0x580;

	char cinematicName[32];
};

#endif // _ACTOR_CINEMATIC_H