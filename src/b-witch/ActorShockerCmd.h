#ifndef ACTOR_SHOCKER_CMD_H
#define ACTOR_SHOCKER_CMD_H

#include "Types.h"
#include "Actor.h"

class CSequenceParam {
public:
	int field_0x0;
	int count_0x4;
	float* aFloats;
};

class CActorShockerCmd : public CActor {
public:
	CActorShockerCmd(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	int sequenceParamsCount;
	CSequenceParam* aSequenceParams;
};

#endif //ACTOR_SHOCKER_CMD_H