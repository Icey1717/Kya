#ifndef ACTOR_COMMANDER_H
#define ACTOR_COMMANDER_H

#include "Types.h"
#include "Actor.h"
#include "CameraViewManager.h"

class CActorFighter;

class CTeamElt
{
public:
	CTeamElt();
	int field_0x0;
	int field_0x4;
	int field_0x14;
	int enemyIndex;
	CActorFighter* pEnemyActor;
};

class ACommanderComponent_10 {};

class CSquad 
{
public:
	void Create(ByteCode* pByteCode);
};

class CBehaviourCommander : public CBehaviour
{
public:
};

class CBehaviourCommanderBeatUp : public CBehaviourCommander
{
public:
};

class CBehaviourCommanderDefault : public CBehaviourCommander
{
public:
};

class CActorCommander : public CActor {
public:
	CActorCommander();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term() { IMPLEMENTATION_GUARD(); }
	virtual void Manage();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state) { IMPLEMENTATION_GUARD(); }
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { IMPLEMENTATION_GUARD(); }

	void ClearLocalData();

	void AddTracked();
	void RemoveTracked();

	CActorFighter* GetIntruder();

	int CheckDetectArea(edF32VECTOR4* v0);
	int CheckGuardArea(edF32VECTOR4* v0);

	void CheckExorcism();
	void CheckArea_IntruderDetectArea();
	void CheckArea_IntruderGuardArea();

	void CheckArea_SoldiersDetectArea();
	void CheckArea_SoldiersGuardArea();

	bool CheckZone_00170f90(edF32VECTOR4* v0);

	uint detectAreaZoneId;
	uint guardAreaZoneId;

	CSquad squad;
	CCamFigData camFigData;

	CTeamElt* aTeamElt;
	ACommanderComponent_10* aComponent_0x170;

	int count_0x16c;

	uint field_0x180;
	int newPathFindingId;
	int curPathFindingId;
	uint flags_0x18c;

	float field_0x1dc;
	int field_0x1e0;
	float field_0x1e4;
	CActorsTable exorcismActorsTable;

	S_STREAM_REF<CActor> actorRef;

	edF32VECTOR4 targetPosition;

	int bInCombat_0x1b0;

	edF32VECTOR4 targetLocation;

	int count_0x1d0;

	undefined4 field_0x1e8;

	int nbTeams;

	CBehaviourCommanderDefault behaviourCommanderDefault;
};

#endif //ACTOR_COMMANDER_H