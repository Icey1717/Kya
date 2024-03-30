#ifndef ACTOR_MOVING_PLATFORM_H
#define ACTOR_MOVING_PLATFORM_H

#include "Types.h"
#include "ActorMovable.h"
#include "CinematicManager.h"
#include "PathManager.h"

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
	virtual void Create(ByteCode* pByteCode) {}
};

class CBehaviourPlatformStand : public CBehaviourPlatform
{
public:
	CBehaviourPlatformStand();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();

	CActorMovingPlatform* pOwner;
	int field_0x8;
	int field_0xc;
	CinNamedObject30* pCinData;
	CActor* pTiedActor;
};

class CBehaviourPlatformTrajectory : public CBehaviourPlatform
{
public:
	CBehaviourPlatformTrajectory();
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void Manage();
	
	CActorMovingPlatform* pOwner;
	CPathFollowReaderAbsolute pathFollowReaderAbs;

	float field_0x28;
	float field_0x2c;

	float goalAmount_0x30;
	float field_0x34;
	S_TRAJ_POS currentFillAmount_0x38;
};

struct S_STREAM_NTF_TARGET_ONOFF : public S_STREAM_NTF_TARGET_BASE {
	void Reset();

	undefined4 field_0x0;
	char field_0x18[0xc];
};

static_assert(sizeof(S_STREAM_NTF_TARGET_ONOFF) == 0x1c);

struct S_MOVING_PLATFORM_TARGET_STREAM {
	int entryCount;
	S_STREAM_NTF_TARGET_ONOFF aEntries[];
};

class CBehaviourPlatformSlab : public CBehaviourPlatform
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CActorMovingPlatform* pOwner;
	S_MOVING_PLATFORM_TARGET_STREAM* pTargetStream;
	S_STREAM_EVENT_CAMERA* streamEventCamera;

	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	int field_0x20;
	float field_0x24;
	float field_0x28;
	CActor* field_0x2c;
};

class CActorAlternateModel {};

class CBehaviourPlatformDestroyed : public CBehaviourPlatform
{
public:
	CBehaviourPlatformDestroyed();
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage() { IMPLEMENTATION_GUARD(); }

	CPathFollowReaderAbsolute pathFollowReaderAbs;
	CActorAlternateModel alternateModel;

	int field_0x3c;
	int field_0x40;
};

class CBehaviourWeighingMachine : public CBehaviourPlatform
{
public:
	//virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
};

class CBehaviourWeighingMachineSlave : public CBehaviourWeighingMachine
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage() { IMPLEMENTATION_GUARD(); }

	CPathFollowReaderAbsolute pathFollowReaderAbs;
};

class CBehaviourTeleportRandom : public CBehaviourWeighingMachine
{
public:
	//virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
};

class CBehaviourSelector : public CBehaviour
{
public:
	//virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
};

class CBehaviourSelectorSlave : public CBehaviourSelector
{
public:
	//virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
};

class CBehaviourSelectorNew : public CBehaviourSelector
{
public:
	//virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
};

PACK(
struct CActorMovingPlatform_SubObj {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
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
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
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

struct S_BRIDGE_ZONE_STREAM {
	int entryCount;
	S_STREAM_REF<ed_zone_3d> aEntries[];
};

static_assert(sizeof(S_STREAM_REF<ed_zone_3d>) == 0x4);

PACK(
struct S_BRIDGE_CAMERA_STREAM_ENTRY {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	S_STREAM_NTF_TARGET_BASE streamTarget;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	S_STREAM_EVENT_CAMERA streamCameraEvent;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
});

static_assert(sizeof(S_BRIDGE_CAMERA_STREAM_ENTRY) == 0x48);

struct S_BRIDGE_CAMERA_STREAM {
	int entryCount;
	S_BRIDGE_CAMERA_STREAM_ENTRY aEntries[];
};

class CActorMovingPlatform : public CActorMovable
{
public:
	static AnimResult gStateCfg_MPF[13];

	CActorMovingPlatform();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual AnimResult* GetStateCfg(int state);

	bool Slab_MoveAndDetectCarriedObject(CBehaviourPlatformSlab* pBehaviour, int param_3);
	void ManageNoFrictionZones(int param_2);

	void Platform_UpdatePosition(edF32VECTOR4* pPosition, int param_3, CActorsTable* pActorsTable);
	void Platform_UpdateMatrixOnTrajectory(CPathFollowReaderAbsolute* pPathFollowerAbs, int param_3, int param_4, S_TRAJ_POS* pTrajPos, CActorsTable* pActorsTable, edF32VECTOR4* param_7);

	void ForceCarriedStuff();

	void BehaviourTrajectory_Manage(CBehaviourPlatformTrajectory* pBehaviour);
	float BehaviourTrajectory_ComputeTime(CBehaviourPlatformTrajectory* pBehaviour);

	void BehaviourSlab_Manage(CBehaviourPlatformSlab* pBehaviour);

	void GenericManage(int param_2, int param_3, int param_4, int param_5);

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

	S_BRIDGE_ACTOR_STREAM* field_0x244;
	S_BRIDGE_ZONE_STREAM* field_0x248;
	S_BRIDGE_CAMERA_STREAM* field_0x254;
};

#endif //ACTOR_MOVING_PLATFORM_H