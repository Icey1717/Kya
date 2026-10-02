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

struct S_SAVE_CLASS_MINI_GAME
{
	int field_0x0;
	S_MINI_GAME_SCORE defaultScore;
	S_MINI_GAME_SCORE multiScores[5];
	undefined4 field_0x34[4];
	S_MINI_GAME_SCORE trainingScores[5];
};

static_assert(sizeof(S_SAVE_CLASS_MINI_GAME) == 0x6c);

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
	CBehaviourMiniGameBetting();
	virtual ~CBehaviourMiniGameBetting();

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

struct S_MINI_GAME_SCORE_LIST
{
	int nbScores;
	S_MINI_GAME_SCORE* aScores;
};

class CBehaviourMiniGameTraining : public CBehaviourMiniGame
{
public:
	CBehaviourMiniGameTraining();
	virtual ~CBehaviourMiniGameTraining();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual void SaveContext(void* pData, uint mode, uint maxSize);

	S_MINI_GAME_SCORE_LIST scoreList;
};

class CBehaviourMiniGameMulti : public CBehaviourMiniGame
{
public:
	CBehaviourMiniGameMulti();
	virtual ~CBehaviourMiniGameMulti();

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
	S_MINI_GAME_SCORE_LIST scoreList;
};

class edCTextFormat;

class CActorMiniGame : public CActor
{
public:
	CActorMiniGame();
	static StateConfig _gStateCfg_MIG[4];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual CBehaviourMiniGameBetting* GetBhvBetting();
	virtual CBehaviourMiniGameTraining* GetBhvTraining();
	virtual CBehaviourMiniGameMulti* GetBhvMulti();
	virtual int GetUnity();
	virtual ulong GetScoreLabelMessageHash();
	virtual ulong GetHighScoreLabelMessageHash();
	virtual void SetYouAreChosen(byte param_2);
	virtual bool MustStop();
	virtual float GetExtraHudHeight();
	virtual void DrawExtraHud(float param_1, float param_2);
	virtual void DrawScoreUnitLabel(float param_1, float param_2);

	void FUN_003ace00();
	void FUN_003ad480();
	bool FUN_003ab830();
	void NextFinalAction();
	void PrevFinalAction();
	void UpdateCurHighScoreName(char* pName);
	void DrawHighScoreArray(float param_1, float param_2, float param_3, float param_4, S_MINI_GAME_SCORE_LIST* pList);
	void FormatScore(float param_1, edCTextFormat* pFormat, char* param_4, int unity, int param_6);
	void FormatScore(float param_1, edCTextFormat* pFormat, int param_4);

	char* FUN_003ace10();

	void FUN_003a9f70();
	void FUN_003a9d80();
	void StateMiniGameStandInit();
	void StateMiniGameStand(CBehaviourMiniGame* pBehaviour, int param_3);
	void FUN_003a9f30();
	void FUN_003a9f80();
	void FUN_003ac030(float param_1, int param_3, int param_4);
	void FUN_003ac170(uint param_2);
	void FUN_003a9d60();
	void FUN_003a9f50();
	byte FUN_003ac880(float param_1);

	ulong field_0x160;
	uint field_0x168;
	uint field_0x16c;
	S_ACTOR_STREAM_REF* field_0x170;

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
	byte field_0x1c4;

	int field_0x1cc;
	float field_0x1d0;
	byte field_0x1d4;
	int field_0x1d8;
	undefined4 field_0x1dc;
};

#endif //ACTOR_MINI_GAME_H
