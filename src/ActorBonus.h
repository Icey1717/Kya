#ifndef ACTOR_BONUS_H
#define ACTOR_BONUS_H

#include "Types.h"
#include "ActorMovable.h"

#define BONUS_BEHAVIOUR_TURN 0x3
#define BONUS_BEHAVIOUR_PATH 0x4
#define BONUS_BEHAVIOUR_FLOCK 0x5
#define BONUS_BEHAVIOUR_ADD_ON 0x6

class CBehaviourBonusBase : public CBehaviour
{
public:
	virtual void ChangeVisibleState(int state) {}
};

class CBehaviourBonusFlock : public CBehaviourBonusBase
{

};

class CBehaviourBonusAlone : public CBehaviourBonusBase
{

};

class CCBehaviourBonusAddOn : public CBehaviourBonusFlock
{

};

class CBehaviourBonusPath : public CBehaviourBonusBase
{

};

class CBehaviourBonusTurn : public CBehaviourBonusBase
{

};

class COmniLight { };

template<class T, int count>
class CSharedLights
{
public:
	void Init(int param_1, int param_2, uint param_4, uint param_5, uint param_6) { IMPLEMENTATION_GUARD_LIGHT(); }
	void Term() { field_0x0 = 0; }

	undefined4 field_0x0;
};

class CActorBonus : public CActorMovable {
public:
	CActorBonus() {
		IMPLEMENTATION_GUARD_ACTOR()
	}

	static CSharedLights<COmniLight, 3> _gBNS_Lights;
	static StateConfig _gStateCfg_BNS[6];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void ComputeLighting() {}
	virtual void CheckpointReset();
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual void ChangeVisibleState(int state);

	CBehaviourBonusFlock behaviourBonusFlock;

	S_STREAM_REF<CSound> field_0x1dc;
};

#endif //ACTOR_BONUS_H