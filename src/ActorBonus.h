#ifndef ACTOR_BONUS_H
#define ACTOR_BONUS_H

#include "Types.h"
#include "Actor.h"

class CBehaviourBonusBase : public CBehaviour
{

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

class CActorBonus : public CActorMovable {
public:
	CActorBonus(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourBonusFlock field_0x200;
};

#endif //ACTOR_BONUS_H