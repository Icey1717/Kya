#ifndef ACTOR_MINI_GAME_BOOMY_H
#define ACTOR_MINI_GAME_BOOMY_H

#include "Types.h"
#include "ActorMiniGame.h"

struct S_MINI_GAME_BOOMY_TARGET
{
	void Create(ByteCode* pByteCode);
	void Init();
	bool CheckCollision();
	bool CheckHit();

	S_STREAM_REF<CActor> actorRef; 
	S_NTF_SWITCH hitSwitch;
	byte bArmed;
};

struct S_MINI_GAME_BOOMY_SEQUENCE
{
	void Create(ByteCode* pByteCode);
	void Draw(); // 0x003f9f10
	void GetTargetColor(int index, _rgba* pColor); // 0x003fa1b0
	int GetTargetIndex(int index);

	int nbTargets; // 0x0
	int* aTargetIndices; // 0x4
	int curTarget; // 0x8
};

class CBehaviourMiniGameBoomyTraining : public CBehaviourMiniGameTraining
{
public:
	virtual void Manage(); // 0x003fb770
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x003fb840
	virtual void InitState(int newState); // 0x003fb6c0
	virtual void TermState(int oldState, int newState); // 0x003fb640
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003fb620
};

class CBehaviourMiniGameBoomyBetting : public CBehaviourMiniGameBetting
{
public:
	virtual void Manage(); // 0x003fb4b0
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x003fb580
	virtual void InitState(int newState); // 0x003fb400
	virtual void TermState(int oldState, int newState); // 0x003fb3a0
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003fb380
};

class CBehaviourMiniGameBoomyMulti : public CBehaviourMiniGameMulti
{
public:
	virtual void Manage(); // 0x003fb210
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x003fb2e0
	virtual void InitState(int newState); // 0x003fb160
	virtual void TermState(int oldState, int newState); // 0x003fb0e0
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003fb0c0
};

class CActorMiniGameBoomy : public CActorMiniGame
{
public:
	CActorMiniGameBoomy();
	virtual ~CActorMiniGameBoomy();

	static StateConfig _gStateCfg_MB[2];

	virtual void Create(ByteCode* pByteCode); // 0x003fc5a0
	virtual void Init(); // 0x003fc310
	virtual void Reset(); // 0x003fbf50
	virtual void CheckpointReset(); // 0x003fbe60
	virtual CBehaviour* BuildBehaviour(int behaviourType); // 0x003fc080
	virtual StateConfig* GetStateCfg(int state); // 0x003fc040
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x003fbd30

	virtual CBehaviourMiniGameBetting* GetBhvBetting(); // 0x003fc900
	virtual CBehaviourMiniGameTraining* GetBhvTraining(); // 0x003fc910
	virtual CBehaviourMiniGameMulti* GetBhvMulti(); // 0x003fc920
	virtual int GetUnity(); // 0x003fc8b0
	virtual ulong GetScoreLabelMessageHash(); // 0x003fc8c0
	virtual ulong GetHighScoreLabelMessageHash(); // 0x003fc8e0
	virtual bool MustStop(); // 0x003fbc70
	virtual float GetExtraHudHeight(); // 0x003fbbe0
	virtual void DrawExtraHud(float param_1, float param_2); // 0x003fb8d0

	void StateBoomy(); // 0x003fa390
	void DisableUnusedSequenceTargets(); // 0x003faf30
	void ResetTargets(); // New inlined.
	S_MINI_GAME_BOOMY_SEQUENCE* GetCurSequence(); // New inlined.

	int nbTargets; // 0x1e0
	S_MINI_GAME_BOOMY_TARGET* aTargets; // 0x1e4
	int nbPenaltyTargets; // 0x1e8
	S_MINI_GAME_BOOMY_TARGET* aPenaltyTargets; // 0x1ec
	float timeLimit; // 0x1f0
	uint gameFlags; // 0x1f4
	int maxPenalties; // 0x1f8
	undefined4 field_0x1fc;
	int nbSequences; // 0x200
	S_MINI_GAME_BOOMY_SEQUENCE* aSequences; // 0x204
	int curSequence; // 0x208
	undefined4 field_0x20c;
	float timeRemaining; // 0x210
	int penaltyCount; // 0x214
	int hitCount; // 0x218
	int field_0x21c;
	byte bSequenceComplete; // 0x220

	CBehaviourMiniGameBoomyTraining trainingBehaviour; // 0x224
	CBehaviourMiniGameBoomyBetting bettingBehaviour; // 0x234
	CBehaviourMiniGameBoomyMulti multiBehaviour; // 0x24c
};

#endif //ACTOR_MINI_GAME_BOOMY_H
