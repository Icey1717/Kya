#ifndef ACTOR_D_C_A_H
#define ACTOR_D_C_A_H

#include "Types.h"
#include "Actor.h"

// A.K.A AIRCANON

class CActorDCA : public CActor 
{
public:

	class CBhvDefault : public CBehaviour
	{

	};

	class CBhvControlled : public CBhvDefault
	{

	};

	class CBhvIAControlled : public CBhvDefault
	{

	};

	class CBhvTrajectory : public CBhvDefault
	{

	};

	CActorDCA(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBhvDefault behaviourDefault;
	CBhvControlled behaviourControlled;
	CBhvIAControlled behaviourAiControlled;
};

#endif //ACTOR_D_C_A_H