#ifndef ACTOR_JAMGUT_H
#define ACTOR_JAMGUT_H

#include "Types.h"
#include "Actor.h"

class CActorJamgut : public CActor {
public:
	CActorJamgut(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_JAMGUT_H