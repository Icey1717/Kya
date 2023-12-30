#ifndef ACTOR_NATIV_CMD_H
#define ACTOR_NATIV_CMD_H

#include "Types.h"
#include "Actor.h"

class CBehaviourAddOnNativ : public CBehaviourAddOnBase {
public:
	virtual void Create(ByteCode* pByteCode);

	int field_0x10;
};

class CActorNativCmd : public CActor {
public:
	CActorNativCmd(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CBehaviourAddOnNativ addOnA;
	CBehaviourAddOnNativ addOnB;
	CBehaviourAddOnNativ addOnC;
	CBehaviourAddOnNativ addOnD;
};

#endif //ACTOR_NATIV_CMD_H