#ifndef ACTOR_RUNE_H
#define ACTOR_RUNE_H

#include "Types.h"
#include "Actor.h"
#include "Fx.h"

#define RUNE_BEHAVIOUR_DEFAULT 2

#define RUNE_STATE_DEFAULT_WAIT_FOR_RECEPTACLE 0x5
#define RUNE_STATE_DEFAULT_IDLE 0x6
#define RUNE_STATE_DEFAULT_CINEMATIC 0x7
#define RUNE_STATE_DEFAULT_ACTIVATE 0x8

class CActorRune;

class CBehaviourRune : public CBehaviour
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CActorRune* pOwner;
	CFxHandle fxHandle;
};

class CActorRune : public CActor
{
public:
	static StateConfig gStateCfg_RUN[10];

	virtual void Create(ByteCode* pByteCode);
	virtual void Draw();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);

	void BehaviourDefault_Manage();

	float triggerDistance;
	int cinematicId;
	int scenVarId;
	int fxId;
	int materialIdA;
	int materialIdB;

	CBehaviourRune behaviourRune;
};

#endif //ACTOR_RUNE_H