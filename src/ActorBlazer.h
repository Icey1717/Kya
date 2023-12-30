#ifndef ACTOR_BLAZER_H
#define ACTOR_BLAZER_H

#include "Types.h"
#include "Actor.h"

class CVision {
public:
	void Create(CActor* pActor, ByteCode* pByteCode);
};

class CPathFollowReader {
public:
	void Create(ByteCode* pByteCode);

	int field_0x0;
};

class CActorBlazer : public CActor {
public:
	CActorBlazer(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CVision vision;
	CAddOnGenerator addOnGenerator;
	CPathFollowReader pathFollowReader;
};

#endif //ACTOR_BLAZER_H