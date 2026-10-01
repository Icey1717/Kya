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

struct S_MINI_GAME_BET
{
	int cost;
	int reward;
	byte bAvailable;
};

class CBehaviourMiniGame : public CBehaviour
{
public:
	CActorMiniGame* pOwner;
};

class CBehaviourMiniGameBetting : public CBehaviourMiniGame
{
public:
	int field_0x8;
	int nbBets;
	S_MINI_GAME_BET* aBets;
	int curBet;
};

class CBehaviourMiniGameTraining : public CBehaviourMiniGame
{
public:
	int nbScores;
	S_MINI_GAME_SCORE* aScores;
};

class CBehaviourMiniGameMulti : public CBehaviourMiniGame
{
public:
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
	CActorMiniGame(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual int GetScoreType() { return 0; }

	void FUN_003ace00();
	void NextFinalAction();
	void PrevFinalAction();
	void SetScoreName(char* pName);
	virtual CBehaviourMiniGameBetting* GetBhvBetting();
	virtual CBehaviourMiniGameTraining* GetBhvTraining();
	virtual CBehaviourMiniGameMulti* GetBhvMulti();
	virtual int GetUnity();
	virtual void SetYouAreChosen(byte param_2);
	virtual bool MustStop();

	// Recovered fields used by the organizer; remaining ranges retain their PS2 offsets.
	ulong field_0x160;
	uint field_0x168;
	uint field_0x16c;
	undefined field_0x170[0xc];
	int field_0x17c;
	S_MINI_GAME_SCORE* field_0x180;
	float field_0x184;
	char field_0x188[4];
	S_STREAM_REF<CWayPoint> field_0x18c;
	int field_0x190;
	
	S_NTF_SWITCH field_0x19c;
	ulong field_0x1a8;
	int field_0x1b0;
	int field_0x1b4;
	byte field_0x1b8;
	bool bMustStop;
	CActorMiniGamesOrganizer* field_0x1c0;
	undefined field_0x1c4[8];
	int field_0x1cc;
	float field_0x1d0;
	byte field_0x1d4;
	int field_0x1d8;
	undefined field_0x1dc[4];
	float field_0x1e0;
};

#endif //ACTOR_MINI_GAME_H
