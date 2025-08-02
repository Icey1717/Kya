#ifndef ACTOR_COMPANION_H
#define ACTOR_COMPANION_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ScenaricCondition.h"
#include "CinematicManager.h"

class CActorHero;
class CActorCompanion;
class CCamera;

struct S_NTF_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

#define COMPANION_BEHAVIOUR_DEFAULT 0x3

#define COMPANION_DEFAULT_STATE_TIPS_ALERT 0x9
#define COMPANION_DEFAULT_STATE_TIPS_SHOW 0xa

#define COMPANION_SUB_OBJ_FLAG_HAS_CAMERA 0x4
#define COMPANION_SUB_OBJ_FLAG_HAS_ACTOR 0x40

struct CompanionAlert
{
	ushort flags_0x0;
	ushort flags_0x2;
	ushort flags_0x4;
	undefined field_0x6;
	undefined field_0x7;
	int field_0x8;
	ushort field_0xc;
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
	ulong levelStringKey;
	float field_0x50;
	int levelStringLen;
	int field_0x58;
	float field_0x5c;
	float field_0x60;
	ScenaricCondition field_0x64;
	S_NTF_SWITCH targetSwitch;
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

	void FUN_001e50a0();
	void FUN_001e5790();
	void FUN_001e58e0();
	void FUN_001e9660(edF32VECTOR4* param_2);

	void _Display_BGText(float param_1, float param_2, float param_3);
	void _Display_Text(float param_1, float param_2, float param_3);

	void CopyNextFormattedChar(char** param_2, char** param_3);

	CompanionAlert* GetActiveAlert();
	CompanionAlert* GetAlert(int index);

	CActorCompanion* pOwner;

	int nbSubObjs;
	CompanionAlert* aSubObjs;

	int field_0x10;
	int activeSubObjId;
	int field_0x18;
	float field_0x1c;
	float displayTime;

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

class CActorCompanion : public CActorAutonomous
{
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

	void BehaviourCompanion_Manage();

	void StateTipsAlert();
	void StateTipsShow();

	void FUN_001e82a0();

	float SV_CMP_MoveToSpecial(float speed, float acceleration, float deceleration, float followTrigLow, CActor* pActor, edF32VECTOR4* pPosition, uint flags);

	CActorHero* pActorHero;
	_rgba field_0x354;
	int field_0x358;
	int materialId_0x35c;
	int field_0x360;

	int field_0x368;
	undefined4 field_0x36c;

	CBehaviourCompanion behaviourCompanion;
};

#endif //ACTOR_COMPANION_H