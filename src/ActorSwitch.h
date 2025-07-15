#ifndef ACTOR_SWITCH_H
#define ACTOR_SWITCH_H

#include "Types.h"
#include "Actor.h"
#include "ActorMovable.h"
#include "Fx.h"

struct S_NTF_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

class CActorSwitch;

class CBehaviourSwitch : public CBehaviour
{
public:
	// Needs to be deleted functions.
	virtual void SaveContext(uint*, int) {}
	virtual void LoadContext(uint*, int) {}
	virtual void ChangeManageState(int state) {}

	CActorSwitch* pOwner;
};

class CBehaviourSwitchLever : public CBehaviourSwitch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);

	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SaveContext(uint*, int) {}
	virtual void LoadContext(uint*, int) {}

	CActor* pActor;

	edF32VECTOR4 field_0x10;
	S_OSCILLATING_QUAT field_0x20;

	S_STREAM_REF<CSound> field_0x40;
	S_STREAM_REF<CSound> field_0x44;

};

class CBehaviourSwitchMagic : public CBehaviourSwitch
{
public:
	CBehaviourSwitchMagic();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void ChangeManageState(int state);


	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	int field_0x18;
	CinNamedObject30* field_0x1c;
	CActor* field_0x20;
	int field_0x24;
	int field_0x28;
	edNODE* pHierarchy;
	int field_0x30;
	CinNamedObject30* field_0x34;
	CActor* field_0x38;
	int field_0x3c;
	int field_0x40;
	undefined4 field_0x44;
	CFxDigits fxDigits;
	int field_0x4c;
	CinNamedObject30* field_0x50;
	CActor* field_0x54;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
};

class CBehaviourSwitchWolfenCounter : public CBehaviourSwitch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);

	virtual void Manage();
	virtual void Draw();

	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);

	ParticleInfo* field_0x8;
	S_STREAM_REF<ed_zone_3d> streamRefZone;
	S_STREAM_REF<CActor> streamRefActor;
	int field_0x14;
	float field_0x18;

	edF32MATRIX4 matrix;
};

class CBehaviourSwitchTarget : public CBehaviourSwitch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);

	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SaveContext(uint*, int) {}
	virtual void LoadContext(uint*, int) {}
	virtual void ChangeManageState(int state) {}

	float field_0x8;
	S_OSCILLATION_CONFIG oscConfig;
	S_OSCILLATING_VALUE oscValue;
};

class CBehaviourSwitchNew : public CBehaviourSwitch
{
public:
};

class CBehaviourSwitchMultiCondition : public CBehaviourSwitch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);

	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }

	uint field_0x8;
	uint field_0xc;
	uint field_0x10;
	uint field_0x14;
};

class CBehaviourSwitchSequence : public CBehaviourSwitch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);

	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }

	uint field_0x8;
	uint field_0xc;

	int* field_0x10;
	undefined4 field_0x14;
	uint field_0x18;
};

class CActorSwitch : public CActor {
public:

	static StateConfig _gStateCfg_SWT[5];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourSwitchMagic_Manage(CBehaviourSwitchMagic* pBehaviour);
	void StateSwitchLeverOff2On(CBehaviourSwitchLever* pBehaviour);

	S_NTF_TARGET_STREAM_REF* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
	CActorSound* pActorSound;

	CBehaviourSwitchMultiCondition behaviourSwitchMultiCondition;
	CBehaviourSwitchSequence behaviourSwitchSequence;
};

#endif //ACTOR_SWITCH_H