#ifndef ACTOR_ATON_H
#define ACTOR_ATON_H

#include "Types.h"
#include "Actor.h"

class CPathPlaneArray {
public:
	void Create(ByteCode* pByteCode);

	int field_0x0;
};

class CBehaviourAddOnAton : public CBehaviourAddOnBase {
public:
	virtual void Create(ByteCode* pByteCode);

	int field_0x10;
	int field_0x1c;
};

class CBehaviourAton : public CBehaviour
{

};

class CBehaviourAtonEscape : public CBehaviourAton
{

};

class CActorAton : public CActorAutonomous {
public:
	CActorAton(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CPathPlaneArray pathPlaneArray;
	CBehaviourAddOnAton behaviourAddOn;
	CBehaviourAtonEscape behaviourAtonEscape;
};

#endif //ACTOR_ATON_H