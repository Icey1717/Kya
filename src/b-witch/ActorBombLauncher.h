#ifndef ACTOR_BOMB_LAUNCHER_H
#define ACTOR_BOMB_LAUNCHER_H

#include "Types.h"
#include "Actor.h"
#include "FireShot.h"

class CActorBombLauncher;

class CBehaviourBombLauncher : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorBombLauncher* pOwner;
};

class CBehaviourBombLauncherStand : public CBehaviourBombLauncher
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};


class CActorBombLauncher : public CActor
{
public:
	static StateConfig gStateCfg_BLA[2];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CBehaviourBombLauncherStand behaviourStand;
	float field_0x168;
	CFireShot fireShot;
};


#endif //ACTOR_BOMB_LAUNCHER_H
