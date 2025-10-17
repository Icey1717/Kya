#ifndef ACTOR_EVENT_GENERATOR_H
#define ACTOR_EVENT_GENERATOR_H

#include "Types.h"
#include "Actor.h"

#include "CameraViewManager.h"

struct ByteCode;

class CCamera;
class CActorFighter;

class PTMF_Ext
{
public:
	enum EVC_PHASE
	{
		EVC_PHASE_CREATE = 0,
		EVC_PHASE_INIT = 1,
		EVC_PHASE_MANAGE = 2,
		EVC_PHASE_3 = 3,
		EVC_PHASE_RESET = 4,
		EVC_PHASE_7 = 7,
	};

	typedef int (PTMF_Ext::* PTMF_TYPE)(PTMF_Ext::EVC_PHASE, undefined8, void*);

	int __ptmf_scall_NOT(EVC_PHASE phase, uint param_3, void* pData);

	int PTMF_Ext::CheckComboA(EVC_PHASE phase, undefined8 param_3, void* pData);
	int ManageActorA(EVC_PHASE phase, undefined8 param_3, void* pData);
	int ManageActorB(EVC_PHASE phase, undefined8 param_3, void* pData);
	int ManageMsgReceived(EVC_PHASE phase, undefined8 param_3, void* pData);
	int DummyTempFunc(EVC_PHASE phase, undefined8 param_3, void* pData);

	uint field_0x0;

	PTMF_TYPE ptmf;

	ushort flagA;
	ushort flagB;
	void* pData;
};

struct ManageMsgAParams
{
	S_STREAM_REF<CActor> actorRef;
	uint msgId;
};

struct ManageMsgBParams
{
	S_STREAM_REF<CActor> actorRef;
	int bActive;
};

class CActorEventGenerator;

class CBehaviourEventGen : public CBehaviour
{
public:
	enum EVG_PHASE
	{
		EVG_PHASE_CREATE = 0,
		EVG_PHASE_INIT = 1,
		EVG_PHASE_BEGIN = 2,
		EVG_PHASE_MANAGE = 3,
		EVG_PHASE_DRAW = 4,

	};

	struct BHVR_PTMF;

	typedef int (CBehaviourEventGen::* PTMF_TYPE)(BHVR_PTMF*, EVG_PHASE, void*);

	struct BHVR_PTMF
	{
		PTMF_TYPE ptmf;
		ushort flagA;
		ushort flagB;
	};

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	uint FUN_0038fba0();

	int ManageCamera(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int ManageCinematic(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int ManageMessage(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int ManageSlowDown(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int ManageEarthQuakeA(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int ManageEarthQuakeB(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int ManageEarthQuakeC(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int DrawA(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);
	int DrawB(BHVR_PTMF* pPMTF, EVG_PHASE phase, void* pData);

	void RegisterFunction(PTMF_TYPE* pFunc, uint flag);

	CActorEventGenerator* pOwner;

	float field_0x8;
	int field_0xc;
	float field_0x10;

	S_STREAM_REF<CCamera> cameraRef;
	int cinematicId;

	S_STREAM_REF<CActor> msgActorRef;
	int msgId;
	uint msgParams;

	float slowDownfield_0x28;
	float slowDownfield_0x2c;
	float slowDownfield_0x30;
	float slowDownfield_0x34;
	float slowDownfield_0x38;

	CAM_QUAKE field_0x40;

	CAM_QUAKE field_0x70;

	uint field_0xa0;

	_rgba field_0xb0;
	float field_0xb4;
	float field_0xb8;
	
	_rgba field_0xbc;
	_rgba field_0xc0;
	float field_0xc4;
	float field_0xc8;
	float field_0xcc;
	float field_0xd0;

	CAM_QUAKE field_0xe0;
	float field_0x110;
	float field_0x114;
	float field_0x118;
	float field_0x11c;

	BHVR_PTMF field_0x120[0x10];
	int nbRegisteredMemberFunctions;
};

class CActorEventGenerator : public CActor
{
public:
	static CActorEventGenerator* gGlobalEvG;
	static CActorEventGenerator* gActiveEvG;

	CActorEventGenerator() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void Register12(PTMF_Ext::PTMF_TYPE* p12, void* pData);

	void BehaviourEventGenerator_Manage();

	int field_0x160;
	uint flags_0x164;
	uint field_0x168;

	PTMF_Ext field_0x170[0x10];

	S_STREAM_REF<ed_zone_3d> zoneRef;
	int nbRegisteredMemberFunctions;

	float field_0x308;

	CActorFighter** field_0x538;
	ManageMsgAParams field_0x550;
	S_STREAM_REF<CActor> field_0x578;
	ManageMsgBParams field_0x580;

	CBehaviourEventGen behaviourEventGen;
};

#endif //ACTOR_EVENT_GENERATOR_H