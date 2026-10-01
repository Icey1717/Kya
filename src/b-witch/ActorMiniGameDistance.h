#ifndef ACTOR_MINI_GAME_DISTANCE_H
#define ACTOR_MINI_GAME_DISTANCE_H

#include "Types.h"
#include "ActorMiniGame.h"
#include "PathFollow.h"

class CBehaviourMiniGameDistanceTraining : public CBehaviourMiniGameTraining
{
public:
	virtual void Manage(); // 0x004081c0
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x00408250
	virtual void InitState(int newState); // 0x004080d0
	virtual void TermState(int oldState, int newState); // 0x00408050
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x00408030
};

class CBehaviourMiniGameDistanceBetting : public CBehaviourMiniGameBetting
{
public:
	virtual void Manage(); // 0x00407f00
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x00407f90
	virtual void InitState(int newState); // 0x00407e10
	virtual void TermState(int oldState, int newState); // 0x00407db0
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x00407d90
};

class CBehaviourMiniGameDistanceMulti : public CBehaviourMiniGameMulti
{
public:
	virtual void Manage(); // 0x00407c60
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType); // 0x00407cf0
	virtual void InitState(int newState); // 0x00407b70
	virtual void TermState(int oldState, int newState); // 0x00407af0
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x00407ad0
};

class CActorMiniGameDistance : public CActorMiniGame
{
public:
	static StateConfig _gStateCfg_MD[1];

	virtual void Create(ByteCode* pByteCode); // 0x00408810
	virtual void Init(); // 0x004087b0
	virtual void Reset(); // 0x00408350
	virtual CBehaviour* BuildBehaviour(int behaviourType); // 0x004083e0
	virtual StateConfig* GetStateCfg(int state); // 0x004083a0
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam); // 0x00408300

	virtual CBehaviourMiniGameBetting* GetBhvBetting(); // 0x004088b0
	virtual CBehaviourMiniGameTraining* GetBhvTraining(); // 0x004088c0
	virtual CBehaviourMiniGameMulti* GetBhvMulti(); // 0x004088d0
	virtual int GetUnity(); // 0x00408860
	virtual ulong GetScoreLabelMessageHash(); // 0x00408870
	virtual ulong GetHighScoreLabelMessageHash(); // 0x00408890
	virtual bool MustStop(); // 0x004082e0

	void StateDistance(); // 0x004079c0
	void InitDistanceState();
	float ComputePathDistance(CPathPlane* pPathPlane, int* pLapCount, int* pPreviousSegment); // 0x00408440

	CPathPlane pathPlaneA; // 0x1e0
	CPathPlane pathPlaneB; // 0x200
	int lapCountA; // 0x220
	int lapCountB; // 0x224
	int previousSegmentA; // 0x228
	int previousSegmentB; // 0x22c

	CBehaviourMiniGameDistanceTraining trainingBehaviour; // 0x230
	CBehaviourMiniGameDistanceBetting bettingBehaviour; // 0x240
	CBehaviourMiniGameDistanceMulti multiBehaviour; // 0x258
};

#endif //ACTOR_MINI_GAME_DISTANCE_H
