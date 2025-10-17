#ifndef ACTOR_HELPER_SIGN_H
#define ACTOR_HELPER_SIGN_H

#include "Types.h"
#include "Actor.h"

class CActorHelperSign : public CActor {
public:
	CActorHelperSign(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_HELPER_SIGN_H