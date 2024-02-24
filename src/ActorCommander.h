#ifndef ACTOR_COMMANDER_H
#define ACTOR_COMMANDER_H

#include "Types.h"
#include "Actor.h"
#include "CameraViewManager.h"

class CTeamElt {
public:
	int enemyIndex;
};

class ACommanderComponent_10 {};

class CSquad {
public:
	void Create(ByteCode* pByteCode);
};

class CActorCommander : public CActor {
public:
	CActorCommander(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	uint eventIndex_0x178;
	uint eventID_0x17c;
	CSquad squad;
	CCamFigData camFigData;

	CTeamElt* aTeamElt;
	ACommanderComponent_10* aComponent_0x170;

	int count_0x16c;
	int count_0x2ec;
};

#endif //ACTOR_COMMANDER_H