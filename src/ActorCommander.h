#ifndef ACTOR_COMMANDER_H
#define ACTOR_COMMANDER_H

#include "Types.h"
#include "Actor.h"
#include "CameraViewManager.h"

class CActorFighter;
class CActorWolfen;

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

class CChessboard
{
public:
};

struct SquadSubObj_0x20
{
	CActorWolfen* pWolfen;
	CActorWolfen* field_0x4;
	undefined4 field_0x8;
	uint flags;
	undefined4 field_0x10;
};

struct SquadSubObj_0xb0
{
	undefined4 field_0x0;
	float field_0x4;
	int field_0x8;
	uint field_0xc;

	SquadSubObj_0x20 field_0x10[5];
};

class CSquad 
{
public:
	void Create(ByteCode* pByteCode);

	void Clear();

	int NbElt();

	void ClearSquadDeadAndNonFightingElt();

	void RemoveFighter(CTeamElt* pTeamElt);

	CFixedTable<CTeamElt*, 5> eltTable;
	CChessboard chessboard;

	SquadSubObj_0xb0 aSubObjs[2];
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

	virtual void _UpdateSquad();

	void ClearLocalData();

	bool BeginFightIntruder(CActor* pInstigator, CActor* pIntruder);
	void EndFightIntruder(CActorFighter* pIntruder);

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

	int CheckZone_00170f90(edF32VECTOR4* v0);

	CTeamElt* GetTeamElt(CActor* pActor);

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

	int field_0x194;

	float field_0x1dc;
	int field_0x1e0;
	float field_0x1e4;
	CActorsTable exorcismActorsTable;

	S_STREAM_REF<CActor> actorRef;

	edF32VECTOR4 targetPosition;

	int bInCombat_0x1b0;

	edF32VECTOR4 targetGroundPosition;

	int count_0x1d0;

	undefined4 field_0x1e8;

	int nbTeams;

	CBehaviourCommanderDefault behaviourCommanderDefault;
};

#endif //ACTOR_COMMANDER_H