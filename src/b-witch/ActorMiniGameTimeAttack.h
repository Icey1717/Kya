#ifndef ACTOR_MINI_GAME_TIME_ATTACK_H
#define ACTOR_MINI_GAME_TIME_ATTACK_H

#include "Types.h"
#include "ActorMiniGame.h"

class CBehaviourMiniGameTimeAttackTraining : public CBehaviourMiniGameTraining
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourMiniGameTimeAttackBetting : public CBehaviourMiniGameBetting
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourMiniGameTimeAttackMulti : public CBehaviourMiniGameMulti
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorMiniGameTimeAttack : public CActorMiniGame
{
public:
	static StateConfig _gStateCfg_MTA[1];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual CBehaviourMiniGameBetting* GetBhvBetting();
	virtual CBehaviourMiniGameTraining* GetBhvTraining();
	virtual CBehaviourMiniGameMulti* GetBhvMulti();
	virtual int GetUnity();
	virtual ulong GetScoreLabelMessageHash(); // FUN_003b9ae0
	virtual ulong GetHighScoreLabelMessageHash(); // FUN_003b9b00
	virtual bool MustStop();
	virtual float GetExtraHudHeight(); // FUN_003b96c0
	virtual void DrawExtraHud(float param_1, float param_2); // FUN_003b95b0

	void ConvertScores();

	int field_0x1e0;
	int field_0x1e4;

	// Time-attack behaviour overrides are not decompiled yet.
	CBehaviourMiniGameTimeAttackTraining field_0x1e8;
	CBehaviourMiniGameTimeAttackBetting field_0x1f8;
	CBehaviourMiniGameTimeAttackMulti field_0x210;
};

#endif //ACTOR_MINI_GAME_TIME_ATTACK_H
