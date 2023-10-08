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

	virtual void Init(CActor* pOwner);
};

struct CActorCinematic : public CActor {
	CActorCinematic();
	void Create(const edCinGameInterface::ACTORV_CREATIONtag* pGameInterface, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D, ed_3D_Scene* pScene);
	virtual void Init();

	ComponentList<1> components;
	CBehaviourCinematic behaviourCinematic;
	char name[32];
};

#endif // _ACTOR_CINEMATIC_H