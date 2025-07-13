#ifndef ACTOR_COMPANION_H
#define ACTOR_COMPANION_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ScenaricCondition.h"

class CActorHero;
class CActorCompanion;
class CCamera;

struct S_NTF_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

#define COMPANION_BEHAVIOUR_DEFAULT 0x3

#define COMPANION_SUB_OBJ_FLAG_HAS_CAMERA 0x4
#define COMPANION_SUB_OBJ_FLAG_HAS_ACTOR 0x40

struct CBehaviourCompanionSubObj_80 {
	ushort flags_0x0;
	ushort flags_0x2;
	ushort flags_0x4;
	undefined field_0x6;
	undefined field_0x7;
	int field_0x8;
	undefined2 field_0xc;
	undefined field_0xe;
	undefined field_0xf;
	int field_0x10;
	int field_0x14;
	float field_0x18;
	S_STREAM_REF<CActor> pActor;
	float field_0x20;
	float field_0x24;
	float field_0x28;
	undefined4 field_0x2c;
	S_STREAM_REF<CCamera> pCamera;
	short field_0x34[8];
	int field_0x44;
	ulong field_0x48;
	float field_0x50;
	int field_0x54;
	int field_0x58;
	float field_0x5c;
	float field_0x60;
	ScenaricCondition field_0x64;
	S_NTF_TARGET_STREAM_REF* field_0x68;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
	ConditionedOperationArray field_0x70;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
};

class CBehaviourCompanion : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorCompanion* pOwner;

	int nbSubObjs;
	CBehaviourCompanionSubObj_80* aSubObjs;

	int field_0x10;
	int activeSubObjId;
	int field_0x18;
	float field_0x1c;
	float field_0x20;

	edF32VECTOR4 field_0x30;
	edF32VECTOR4 field_0x40;

	int field_0x50;
	undefined4 field_0x54;
	undefined4 field_0x58;
	undefined4 field_0x5c;
	undefined4 field_0x60;
	undefined4 field_0x64;

	undefined4 field_0x68;
};

class CActorCompanion : public CActorAutonomous {
public:

	static StateConfig _gStateCfg_CMP[9];
	static CActorCompanion* _gThis;

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	CActorHero* pActorHero;
	float field_0x354;
	int field_0x358;
	int field_0x35c;
	int field_0x360;

	undefined4 field_0x36c;

	CBehaviourCompanion behaviourCompanion;
};

#endif //ACTOR_COMPANION_H