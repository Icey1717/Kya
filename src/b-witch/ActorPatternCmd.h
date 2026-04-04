#ifndef ACTOR_PATTERN_CMD_H
#define ACTOR_PATTERN_CMD_H

#include "Types.h"
#include "Actor.h"
#include "ActorPatternService.h"

#define PATTERN_CMD_BEHAVIOUR_STAND 0x2

#define PATTERN_CMD_STAND_STATE_STAND		0x5
#define PATTERN_CMD_STAND_STATE_CHASE		0x6
#define PATTERN_CMD_STAND_STATE_FIRE 		0x7
#define PATTERN_CMD_STAND_STATE_DECELERATE	0x8
#define PATTERN_CMD_STAND_STATE_UNKNOWN 	0x9

class CActorPatternCmd;
class CActorHero;

class CBehaviourPatternCmd : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorPatternCmd* pOwner;
};

class CBehaviourPatternCmdStand : public CBehaviourPatternCmd
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorPatternCmd : public CActor
{
public:
	static StateConfig _gStateCfg_PTC[5];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Draw();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourStand_Manage();

	void ClearLocalData();

	void StatePatternCmdStand();
	void StatePatternCmdChase();
	void StatePatternCmdFire();
	void StatePatternCmdDecelerate();

	void ComputePositionsTwoPattern(edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5);
	void ChooseTwoPosBetweenFour(edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7);

	int CheckArea();

	S_ACTOR_STREAM_REF* pActorStreamRef;
	CPatternPart patternPart;

	uint field_0x160;
	uint field_0x164;

	float field_0x1f0;
	float field_0x1f4;

	edF32VECTOR4 field_0x200;
	edF32VECTOR4 field_0x210;
	edF32VECTOR4 field_0x220;
	edF32VECTOR4 field_0x230;
	edF32VECTOR4 field_0x240;
	edF32VECTOR4 field_0x250;
	int field_0x260;
	edF32VECTOR4 field_0x270;

	CActorHero* field_0x280;
	int nbStreamRefEntries;
	CBehaviourPatternCmdStand behaviourStand;
};

#endif //ACTOR_PATTERN_CMD_H