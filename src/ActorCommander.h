#ifndef ACTOR_COMMANDER_H
#define ACTOR_COMMANDER_H

#include "Types.h"
#include "Actor.h"
#include "CameraViewManager.h"
#include "Squad.h"
#include "ChessBoard.h"
#include "CameraFightData.h"

class CActorFighter;
class CActorWolfen;
class CActorCommander;

class ACommanderComponent_10 {};

struct S_TARGET_ON_OFF_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

class CBehaviourCommander : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	virtual bool CanReleaseSemaphore(int, CActorWolfen*);
	virtual CActorFighter* GetFocus();
	virtual void TestSwitch();
	virtual bool UpdateTeamAnim();

	CActorCommander* pOwner;

	CSquadConfig squadConfig;

	S_TARGET_ON_OFF_STREAM_REF* pMagicalSwitch1C_0x10;
	S_STREAM_EVENT_CAMERA* pMagicalSwitch20_0x14;
	int field_0x18;
	float healthRatio;
};

class CBehaviourCommanderBeatUp : public CBehaviourCommander
{
public:
};

class CBehaviourCommanderDefault : public CBehaviourCommander
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void TestSwitch();
	virtual bool UpdateTeamAnim();

	byte field_0x20;
};

class CActorCommander : public CActor
{
public:
	static StateConfig _gStateCfg_CMD[4];

	CActorCommander();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Manage();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { IMPLEMENTATION_GUARD(); }

	virtual void _UpdateSquad();
	virtual void _UpdateSequence();
	virtual void _UpdatePattern();

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

	void _UpdateCamera();

	void StateCommanderDefault();

	bool CanContinueToFight(CActorFighter* pFighter);

	bool CanReleaseSemaphore(CActorWolfen* pWolfen);
	void ReleaseSemaphore(int index, CActorWolfen* pWolfen);
	bool QuerySemaphoreCold(int index, CActorWolfen* pWolfen);
	bool IsValidEnemy(CActorWolfen* pWolfen);

	uint detectAreaZoneId;
	uint guardAreaZoneId;

	CSquad squad;
	CCamFigData camFigData;

	CTeamElt* aTeamElt;
	ACommanderComponent_10* aComponent_0x170;

	int field_0x168;
	int count_0x16c;

	CActor* field_0x174;

	uint field_0x180;
	int newPathFindingId;
	int curPathFindingId;
	uint flags_0x18c;

	int field_0x194;

	float field_0x1d4;
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

	undefined4 field_0x2f4;

	int field_0x6d0;

	CActorFighter* field_0x9f0;
};

#endif //ACTOR_COMMANDER_H