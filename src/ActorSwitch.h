#ifndef ACTOR_SWITCH_H
#define ACTOR_SWITCH_H

#include "Types.h"
#include "Actor.h"

struct S_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;
class CActorSound;

class CActorSwitch;

struct ParticleInfo;

class CFxDigits 
{
public:
	void Init(int param_2) {}

	ParticleInfo* field_0x0;
};

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

	virtual void Manage() { IMPLEMENTATION_GUARD(); }
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState) { IMPLEMENTATION_GUARD(); }
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { IMPLEMENTATION_GUARD(); }

	virtual void SaveContext(uint*, int) {}
	virtual void LoadContext(uint*, int) {}

	CActor* pActor;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;

	S_STREAM_REF<CSound> field_0x40;
	S_STREAM_REF<CSound> field_0x44;

};

class CBehaviourSwitchMagic : public CBehaviourSwitch
{
public:
	CBehaviourSwitchMagic();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
	virtual void Draw() { IMPLEMENTATION_GUARD(); }
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId) { IMPLEMENTATION_GUARD(); }
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState) { IMPLEMENTATION_GUARD(); }
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { IMPLEMENTATION_GUARD(); }

	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void ChangeManageState(int state) { IMPLEMENTATION_GUARD(); }


	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	int field_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	int field_0x24;
	int field_0x28;
	edNODE* pHierarchy;
	int field_0x30;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	int field_0x3c;
	int field_0x40;
	undefined4 field_0x44;
	CFxDigits fxDigits;
	int field_0x4c;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
};

class CBehaviourSwitchWolfenCounter : public CBehaviourSwitch
{
public:
	CBehaviourSwitchWolfenCounter() { IMPLEMENTATION_GUARD(); }
};

class CBehaviourSwitchTarget : public CBehaviourSwitch
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);

	virtual void Manage() { IMPLEMENTATION_GUARD(); }
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { IMPLEMENTATION_GUARD(); }

	virtual void SaveContext(uint*, int) {}
	virtual void LoadContext(uint*, int) {}
	virtual void ChangeManageState(int state) {}

	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	undefined4 field_0x18;
};

class CBehaviourSwitchNew : public CBehaviourSwitch
{
public:
	CBehaviourSwitchNew() { IMPLEMENTATION_GUARD(); }
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
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
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

	S_TARGET_STREAM_REF* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
	CActorSound* pActorSound;

	CBehaviourSwitchMultiCondition behaviourSwitchMultiCondition;
	CBehaviourSwitchSequence behaviourSwitchSequence;
};

#endif //ACTOR_SWITCH_H