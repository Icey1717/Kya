#ifndef ACTOR_MINI_GAME_BOX_COUNTER_H
#define ACTOR_MINI_GAME_BOX_COUNTER_H

#include "Types.h"
#include "ActorMiniGame.h"

class CBehaviourMiniGameBoxCounterTraining : public CBehaviourMiniGameTraining
{
public:
	virtual void Manage(); // 0x003c4230
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x003c42f0
	virtual void InitState(int newState); // 0x003c4180
	virtual void TermState(int oldState, int newState); // 0x003c4100
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003c40e0
};

class CBehaviourMiniGameBoxCounterBetting : public CBehaviourMiniGameBetting
{
public:
	virtual void Manage(); // 0x003c3f80
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x003c4040
	virtual void InitState(int newState); // 0x003c3ed0
	virtual void TermState(int oldState, int newState); // 0x003c3e50
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003c3e30
};

class CBehaviourMiniGameBoxCounterMulti : public CBehaviourMiniGameMulti
{
public:
	virtual void Manage(); // 0x003c3cd0
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x003c3d90
	virtual void InitState(int newState); // 0x003c3c20
	virtual void TermState(int oldState, int newState); // 0x003c3ba0
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003c3b80
};

class CActorMiniGameBoxCounter : public CActorMiniGame
{
public:
	static StateConfig _gStateCfg_MBC[1];

	virtual void Create(ByteCode* pByteCode); // 0x003c4980
	virtual void Init(); // 0x003c4950
	virtual void Reset(); // 0x003c4450
	virtual void CheckpointReset(); // 0x003c4420
	virtual CBehaviour* BuildBehaviour(int behaviourType); // 0x003c44c0
	virtual StateConfig* GetStateCfg(int state); // 0x003c4480
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003c4390

	virtual CBehaviourMiniGameBetting* GetBhvBetting(); // 0x003c4a10
	virtual CBehaviourMiniGameTraining* GetBhvTraining(); // 0x003c4a20
	virtual CBehaviourMiniGameMulti* GetBhvMulti(); // 0x003c4a30
	virtual int GetUnity(); // 0x003c49c0
	virtual ulong GetScoreLabelMessageHash(); // 0x003c49d0
	virtual ulong GetHighScoreLabelMessageHash(); // 0x003c49f0
	virtual void SetYouAreChosen(byte bChosen); // 0x003c4710
	virtual bool MustStop(); // 0x003c4690
	virtual float GetExtraHudHeight(); // 0x003c4640
	virtual void DrawExtraHud(float param_1, float param_2); // 0x003c4520

	float timeLimit; // 0x1e0; seconds, -1 disables the countdown.
	float timeRemaining; // 0x1e4

	CBehaviourMiniGameBoxCounterTraining trainingBehaviour; // 0x1e8
	CBehaviourMiniGameBoxCounterBetting bettingBehaviour; // 0x1f8
	CBehaviourMiniGameBoxCounterMulti multiBehaviour; // 0x210
};

#endif //ACTOR_MINI_GAME_BOX_COUNTER_H
