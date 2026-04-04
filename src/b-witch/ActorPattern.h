#ifndef ACTOR_PATTERN_H
#define ACTOR_PATTERN_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"
#include "ActorPatternService.h"
#include "Fx.h"

#define PATTERN_BEHAVIOUR_ALONE	0x3
#define PATTERN_BEHAVIOUR_MULTIPLE 0x4

#define PATTERN_STATE_ALONE_CHASE 0xa
#define PATTERN_STATE_ALONE_FIRE_FIRST 0xc
#define PATTERN_STATE_ALONE_FIRE_SECOND 0xd
#define PATTERN_STATE_ALONE_COMEBACK_ALT 0x12
#define PATTERN_STATE_ALONE_COMEBACK 0x13

class CActorPattern;
class CActorPatternCmd;

class CBehaviourPattern: public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorPattern* pOwner;
};

class CBehaviourPatternAlone : public CBehaviourPattern
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CPatternPart patternPart;
	float field_0x90;
	float field_0x94;
	float field_0x98;
};

class CBehaviourPatternMultiple : public CBehaviourPattern
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorPattern : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_PAT[15];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourPatternAlone_InitState(int newState);
	void BehaviourPatternAlone_Manage(CBehaviourPatternAlone* pBehaviour);

	void BehaviourPatternMultiple_InitState(int newState);
	void BehaviourPatternMultiple_Manage();

	void ClearLocalData();

	void ComputeInvincibility();
	void ManageFx();

	void StatePatternComeBack();
	void FUN_003a1ad0();
	void StatePatternChase(CBehaviourPatternAlone* pBehaviour);
	void StatePatternFireFirst(CBehaviourPatternAlone* pBehaviour);
	void StatePatternFireSecond(CBehaviourPatternAlone* pBehaviour);
	void FUN_003a1370();
	void StatePatternComeBackAlt();

	void ComputeFireFirstPoints(edF32VECTOR4* param_2, CBehaviourPatternAlone* pBehaviour);
	void ComputeFireSecondPoints(edF32VECTOR4* param_2, CBehaviourPatternAlone* pBehaviour);

	float field_0x350;
	float field_0x354;
	float field_0x358;
	float field_0x35c;
	float field_0x364;
	float field_0x360;
	float field_0x368;
	float field_0x36c;
	float field_0x370;

	uint boneId;

	bool bInvincible;
	CActorPatternCmd* field_0x37c;
	CLightConfig lightConfig;
	byte field_0x430;
	edF32VECTOR4 field_0x440;
	edF32VECTOR4 field_0x450;
	CFxHandle field_0x460;
	CFxHandle field_0x468;
	CAddOnGenerator field_0x470;

	CBehaviourPatternAlone behaviourAlone;

	CBehaviourPatternMultiple behaviourMultiple;
};

#endif //ACTOR_PATTERN_H