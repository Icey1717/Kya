#ifndef ACTOR_FX_H
#define ACTOR_FX_H

#include "Types.h"
#include "Actor.h"

class CActorFx : public CActor {
public:
	class CBhvSingle : public CBehaviour {

	};

	class CBhvPath : public CBehaviour {
	};

	class CBhvUnknown : public CBehaviour {
	};

	CActorFx(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBhvSingle bhvSingle;
	CBhvPath bhvPath;
	CBhvUnknown bhvUnknown;
};

#endif //ACTOR_FX_H