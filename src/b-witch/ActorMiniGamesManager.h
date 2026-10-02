#ifndef ACTOR_MINI_GAME_MANAGER_H
#define ACTOR_MINI_GAME_MANAGER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGamesManager;
class CActorMiniGame;

#define MINI_GAMES_MANAGER_BEHAVIOUR_STAND 2
#define MINI_GAMES_MANAGER_STATE_STAND 5

class CBehaviourMiniGamesManager : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorMiniGamesManager* pOwner;
};

class CBehaviourMiniGamesManagerStand : public CBehaviourMiniGamesManager
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorMiniGamesManager : public CActor
{
public:
	CActorMiniGamesManager();
	virtual ~CActorMiniGamesManager();

	static StateConfig _gStateCfg_MGM[1];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ClearLocalData();
	bool IsBetAvailable(int cost, int reward);
	bool PlaceBet(int cost);

	int FUN_003ad9f0(int param_2);

	CBehaviourMiniGamesManagerStand behaviourStand;
	S_ACTOR_STREAM_REF* pOrganizerStreamRefs;
	S_STREAM_REF<CActor> actorRef;
	int nextMiniGameOrder;
	int field_0x174;
	CActorMiniGame** aMiniGames;
	int nbMaxMiniGames;
	int nbMiniGames;
};

#endif //ACTOR_MINI_GAME_MANAGER_H
