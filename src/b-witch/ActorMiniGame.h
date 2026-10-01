#ifndef ACTOR_MINI_GAME_H
#define ACTOR_MINI_GAME_H

#include "Types.h"
#include "Actor.h"
#include "CinematicManager.h"

class CActorMiniGamesOrganizer;
class CActorMiniGame;

struct S_MINI_GAME_SCORE
{
	float score;
	char name[4];
};

class CHighScoreArray
{
public:
	static char _STRING_Init[4];
};

struct S_MINI_GAME_BET
{
	int cost;
	int reward;
	byte bAvailable;
};

class CBehaviourMiniGame : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage() override = 0;
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorMiniGame* pOwner;
};

class CBehaviourMiniGameBetting : public CBehaviourMiniGame
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	int field_0x8;
	int nbBets;
	S_MINI_GAME_BET* aBets;
	int curBet;
};

class CBehaviourMiniGameTraining : public CBehaviourMiniGame
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	int nbScores;
	S_MINI_GAME_SCORE* aScores;
};

class CBehaviourMiniGameMulti : public CBehaviourMiniGame
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void AddOnePlayer();
	void SubOnePlayer();

	int nbPlayers;
	int winner;
	int nbScores;
	S_MINI_GAME_SCORE* aScores;
};

class CActorMiniGame : public CActor
{
public:
	CActorMiniGame();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int GetScoreType() { return 0; }

	void FUN_003ace00();
	void NextFinalAction();
	void PrevFinalAction();
	void UpdateCurHighScoreName(char* pName);
	virtual CBehaviourMiniGameBetting* GetBhvBetting();
	virtual CBehaviourMiniGameTraining* GetBhvTraining();
	virtual CBehaviourMiniGameMulti* GetBhvMulti();
	virtual int GetUnity();
	virtual void SetYouAreChosen(byte param_2);
	virtual bool MustStop();

	ulong field_0x160;
	uint field_0x168;
	uint field_0x16c;

	int field_0x174;
	float* field_0x178;

	int nbScores;
	S_MINI_GAME_SCORE* aScores;

	S_MINI_GAME_SCORE defaultScore;

	S_STREAM_REF<CWayPoint> wayPointRef;
	int field_0x190;
	
	S_NTF_SWITCH field_0x19c;
	ulong field_0x1a8;
	int field_0x1b0;
	int field_0x1b4;
	byte field_0x1b8;
	byte field_0x1b9;
	bool bMustStop;
	int field_0x1bc;
	CActorMiniGamesOrganizer* field_0x1c0;

	int field_0x1cc;
	float field_0x1d0;
	byte field_0x1d4;
	int field_0x1d8;
	undefined4 field_0x1c0;
	float field_0x1e0;
};

#endif //ACTOR_MINI_GAME_H
