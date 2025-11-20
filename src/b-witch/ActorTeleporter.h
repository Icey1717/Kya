#ifndef ACTOR_TELEPORTER_H
#define ACTOR_TELEPORTER_H

#include "Types.h"
#include "Actor.h"
#include "ScenaricCondition.h"
#include "CinematicManager.h"

#define TELEPORTER_BEHAVIOUR_DEFAULT 0x2

#define TELEPORTER_STATE_DEFAULT_INACTIVE 0x5
#define TELEPORTER_STATE_DEFAULT_ACTIVE 0x6

struct ParticleInfo;
class CActorTeleporter;

class CBehaviourTeleporterDefault : public CBehaviour
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorTeleporter* pOwner;
};

struct ACTOR_WAYPOINT
{
	S_STREAM_REF<CActor> actorRef;
	S_STREAM_REF<CWayPoint> wayPointRef;
};

struct ACTOR_WAYPOINT_LIST
{
	int nbEntries;
	ACTOR_WAYPOINT aEntries[];

	static void Create(ACTOR_WAYPOINT_LIST** pList, ByteCode* pByteCode);
};

struct S_DESTINATION_ENTRY
{
	undefined4 field_0x0;
	undefined4 field_0x4;
	undefined4 field_0x8;
};

struct S_DESTINATION_LIST
{
	int nbEntries;
	S_DESTINATION_ENTRY aEntries[];

	static void Create(S_DESTINATION_LIST** pList, ByteCode* pByteCode);
};

class CActorTeleporter : public CActor
{
public:
	static StateConfig _gStateCfg_TLP[3];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void StateTeleporterActive();

	void UpdateCurTeleporterState(int levelId, int param_3);
	void NotifyLevelTeleporterChanged();

	bool LevelHasTeleporters();
	int FUN_002ee670();

	edDList_material* GetMySubSectorMaterial(int levelId, int nbAreas);
	void DetectDisabledDestinations(int param_2);
	edDList_material* GetDestinationMaterial(int param_2, int param_3);
	void DisplayDigit(float param_2, float param_3, float param_4, float param_5, int digit);

	ConditionedOperationArray condOpArray;

	CBehaviourTeleporterDefault behaviourTeleporterDefault;

	ulong hash;
	int field_0x168;
	uint field_0x16c;

	S_NTF_SWITCH_ONOFF field_0x170;
	S_NTF_SWITCH_ONOFF field_0x178;
	S_NTF_SWITCH_ONOFF field_0x180;

	S_STREAM_REF<ed_zone_3d> field_0x188;
	S_STREAM_REF<ed_zone_3d> field_0x18c;

	ParticleInfo* field_0x190;
	ParticleInfo* field_0x194;

	ed_g2d_manager g2dManager;

	edDList_material* aMaterials;
	int nbMaterials;

	int cinematicId;

	S_STREAM_REF<ed_zone_3d> field_0x1d4;
	S_STREAM_REF<CActor> field_0x1d8;
	ParticleInfo* field_0x1dc;
	S_STREAM_REF<ed_zone_3d> field_0x1e0;

	edF32MATRIX4 field_0x1f0;

	edF32MATRIX4 field_0x230;

	edF32VECTOR4 field_0x270;

	ed_hash_code snapshotHashCode;
	ed_hash_code* pSnapshotHashCode;

	int activeButtonIndex;
	int field_0x298;
	uint disabledDestinationMask;

	int bOpen;

	int field_0x2a4;
	int field_0x2ac;
	S_DESTINATION_LIST* pDestinationList;
	ACTOR_WAYPOINT_LIST* pActorWaypointList;
	int field_0x2b0;
};

#endif //ACTOR_TELEPORTER_H