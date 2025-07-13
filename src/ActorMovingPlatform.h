#ifndef ACTOR_MOVING_PLATFORM_H
#define ACTOR_MOVING_PLATFORM_H

#include "Types.h"
#include "ActorMovable.h"
#include "CinematicManager.h"
#include "PathManager.h"

#define MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY				0x2
#define MOVING_PLATFORM_BEHAVIOUR_SLAB						0x3
#define MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE	0x4
#define MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER	0x5
#define MOVING_PLATFORM_BEHAVIOUR_DESTROYED					0x6
#define MOVING_PLATFORM_BEHAVIOUR_STAND						0x7
#define MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER			0x8
#define MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW				0x9
#define MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM			0xa

#define MOVING_PLATFORM_FLAG_CAN_BE_DESTROYED				0x10000

class CActorMovingPlatform;

struct S_TRAJ_POS {
	float field_0x0;
	short field_0x4;
	short field_0x6;
	short field_0x8;
};

struct S_STREAM_MPF_NO_FRICTION_ZONE {
	int field_0x0;
	char* field_0x4;
};

class CBehaviourPlatform : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void ChangeManageState(int state) {}
	CActorMovingPlatform* pOwner;
};

class CBehaviourPlatformStand : public CBehaviourPlatform
{
public:
	CBehaviourPlatformStand();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void ChangeManageState(int state);

	int field_0x8;
	int field_0xc;
	CinNamedObject30* pCinData;
	CActor* pTiedActor;
};

class CBehaviourTeleportRandom : public CBehaviourPlatform
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CPathFollow* pPathFollow;
	undefined2 field_0xc;
};

class CBehaviourPlatformTrajectory : public CBehaviourPlatform
{
public:
	CBehaviourPlatformTrajectory();
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CPathFollowReaderAbsolute pathFollowReaderAbs;

	float field_0x28;
	float field_0x2c;

	float goalAmount_0x30;
	float field_0x34;
	S_TRAJ_POS currentFillAmount_0x38;
};

class CSound;
struct ParticleInfo;

class CBehaviourPlatformSlab : public CBehaviourPlatform
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	S_TARGET_ON_OFF_STREAM_REF* pTargetStream;
	S_STREAM_EVENT_CAMERA* streamEventCamera;

	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	S_STREAM_REF<CSound> field_0x20;
	float field_0x24;
	float field_0x28;
	CActor* field_0x2c;
};

class CBehaviourPlatformDestroyed : public CBehaviourPlatform
{
public:
	CBehaviourPlatformDestroyed();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	CPathFollowReaderAbsolute pathFollowReaderAbs;
	CActorAlternateModel alternateModel;

	int altModelId;
	int field_0x40;

	S_TRAJ_POS trajPos;

	edF32VECTOR4 field_0x50;
};

class CBehaviourWeighingMachine : public CBehaviourPlatform
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void ManageFrozen();

	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	CPathFollowReaderAbsolute pathFollowReaderAbs;
	S_TRAJ_POS trajPos;
};

class CBehaviourWeighingMachineMaster : public CBehaviourWeighingMachine
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	S_STREAM_REF<CActor> streamActorRef;

	S_STREAM_EVENT_CAMERA* pStreamEventCamera;

	S_STREAM_NTF_TARGET_ANALOG_LIST* field_0x34;

	float field_0x40;
	float field_0x44;
};

class CBehaviourWeighingMachineSlave : public CBehaviourWeighingMachine
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	float field_0x34;
};

class CBehaviourSelector : public CBehaviourPlatform
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner) { IMPLEMENTATION_GUARD(); }
	virtual void ManageFrozen() { IMPLEMENTATION_GUARD(); }
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType) { IMPLEMENTATION_GUARD(); }
	virtual void InitState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	float field_0x8;
	float field_0xc;
	float field_0x10;

	int field_0x18;

	float field_0x20;
	float field_0x24;
	int field_0x28;

	ParticleInfo* pParticleInfo;
	S_STREAM_REF<CSound> streamRefSound;
};

class CBehaviourSelectorMaster : public CBehaviourSelector
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	S_ACTOR_STREAM_REF* pActorStreamRef;
	S_STREAM_NTF_TARGET_SWITCH_EX_LIST* field_0x30;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
	int field_0x38;
};

class CBehaviourSelectorNew : public CBehaviourSelector
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
};

PACK(
struct AudioSetupParams {
	int field_0x0;
	int field_0x4;
});

PACK(
struct CActorMovingPlatform_SubObj {
	AudioSetupParams field_0x0;
	AudioSetupParams field_0x8;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	uint field_0x1c;
	uint field_0x20;
	uint flags_0x24;
	int field_0x28;
	float field_0x2c;
	S_TILT_STREAM_DEF tiltStreamDef;
	S_PUSH_STREAM_DEF pushStreamDef;
});

static_assert(sizeof(CActorMovingPlatform_SubObj) == 0x4c);

struct S_BRIDGE_ACTOR_STREAM_ENTRY {
	S_STREAM_REF<CActor> actorRef;
	int carryMethod;
};

static_assert(sizeof(S_BRIDGE_ACTOR_STREAM_ENTRY) == 0x8);

struct S_BRIDGE_ACTOR_STREAM {
	int entryCount;
	S_BRIDGE_ACTOR_STREAM_ENTRY aEntries[];
};

struct S_BRIDGE_CAMERA_STREAM_ENTRY
{
	int field_0x0;
	uint field_0x4;
	S_STREAM_NTF_TARGET_SWITCH streamTarget;
	S_STREAM_EVENT_CAMERA streamCameraEvent;
	int field_0x44;
};

static_assert(sizeof(S_BRIDGE_CAMERA_STREAM_ENTRY) == 0x48);

struct S_BRIDGE_CAMERA_STREAM {
	int entryCount;
	S_BRIDGE_CAMERA_STREAM_ENTRY aEntries[];
};

class CActorMovingPlatform : public CActorMovable
{
public:
	static StateConfig gStateCfg_MPF[13];

	CActorMovingPlatform();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void TermBehaviour(int behaviourId, CBehaviour* pBehaviour);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual bool IsLockable();
	virtual void CinematicMode_Manage();
	virtual void CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition);
	virtual void FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* pEventData);

	bool Slab_MoveAndDetectCarriedObject(CBehaviourPlatformSlab* pBehaviour, int param_3);
	void ManageNoFrictionZones(int param_2);

	bool StateTrajectory(float currentFillAmount, CBehaviourPlatformTrajectory* pBehaviour, bool param_4);

	void Platform_UpdateMatrix(edF32MATRIX4* pMatrix, int param_3, CActorsTable* pActorTable);
	void Platform_UpdatePosition(edF32VECTOR4* pPosition, int param_3, CActorsTable* pActorsTable);
	int Platform_UpdateMatrixOnTrajectory(CPathFollowReaderAbsolute* pPathFollowerAbs, int param_3, int param_4, S_TRAJ_POS* pTrajPos, CActorsTable* pActorsTable, edF32VECTOR4* param_7);

	void ForceCarriedStuff();

	void BehaviourTrajectory_Switch(CBehaviourPlatformTrajectory* pBehaviour, int msg);
	void BehaviourTrajectory_Manage(CBehaviourPlatformTrajectory* pBehaviour);
	float BehaviourTrajectory_ComputeTime(CBehaviourPlatformTrajectory* pBehaviour);

	void BehaviourSlab_Manage(CBehaviourPlatformSlab* pBehaviour);
	void BehaviourSelectorMaster_Manage(CBehaviourSelectorMaster* pBehaviour);

	void GenericManage(int param_2, int param_3, int param_4, int param_5);

	void StateSwitchSlabOff2On(CBehaviourPlatformSlab* pBehaviour);
	bool StateWeighingMaster(CBehaviourWeighingMachineMaster* pBehaviour);

	void Die(int bTriggerSwitches);

	// New
	void TriggerSwitches(int conditionType);

	int noFrictionZoneCount;
	S_STREAM_MPF_NO_FRICTION_ZONE* aNoFrictionZones;

	S_TILT_DATA* pTiltData;

	CActorMovingPlatform_SubObj* pProperties;

	CinNamedObject30* field_0x1ec;
	CActor* field_0x1f0;
	CinNamedObject30* field_0x1f4;
	CActor* field_0x1f8;

	uint movingPlatformFlags;
	edF32MATRIX4 field_0x200;

	S_BRIDGE_ACTOR_STREAM* pActorStream;
	S_ZONE_STREAM_REF* pZoneStream;
	S_BRIDGE_CAMERA_STREAM* pCameraStream;

	CActorSound* pActorSound;
	undefined4 field_0x1e4;
	CSound* field_0x1e8;
};

#endif //ACTOR_MOVING_PLATFORM_H