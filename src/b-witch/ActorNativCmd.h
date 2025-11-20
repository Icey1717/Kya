#ifndef ACTOR_NATIV_CMD_H
#define ACTOR_NATIV_CMD_H

#include "Types.h"
#include "Actor.h"

class CActorNativCmd;
class CActorNativ;

struct CTalkParamSubObj
{
	CActorNativ* pNativ;
	int field_0x4;
};

class CTalkParam 
{
public:
	void FUN_00392e30(edF32VECTOR4* param_2);
	float FUN_00392ad0(int index);
	int FUN_00392860(int index, int param_3);
	int ChooseFinishOrder(int index);

	undefined4 field_0x4;
	CTalkParamSubObj* field_0x8;
	int nbSubObj;
	int field_0x10;
	float totalTime;
	float field_0x18;
	byte field_0x1c;
	byte field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

struct CActorNativMsgParam_0xe
{
	int type;

	int field_0x4;
	int field_0x8;
	int field_0xc;
	uint field_0x10;

	float field_0x14;
	
	edF32VECTOR4* field_0x18;
};

class CAddOnNativ : public CAddOn 
{
public:
	CAddOnNativ();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pActor);
	virtual void Manage();
	virtual void Reset();
	virtual CAddOnSubObj* GetSubObj(uint param_2, int param_3);
	virtual bool Func_0x20(uint param_2, CActor* param_3, int pActor);
	virtual bool Func_0x24(uint param_2, CActor* pActor);
	virtual void ClearCinematic(int index);
	virtual bool Func_0x2c(uint param_2);
	virtual void Func_0x30(uint param_2);

	int nbSubObjs;
	CAddOnSubObj* aSubObjs;
};

class CBehaviourNativCmdStand : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState) {}
	virtual void TermState(int oldState, int newState) {}

	CActorNativCmd* pOwner;
};

class CActorNativCmd : public CActor
{
public:

	static StateConfig _gStateCfg_NVC[2];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ManageDiscussion();
	void ManageNativWithKim();
	void FUN_00393620();
	void FUN_00393470();

	void FUN_00394250(CActorNativ* pNativ);
	void FUN_00394d00(CActorNativ* pNativ);

	edF32VECTOR4* FUN_00393230(CActor* pActor);

	S_ACTOR_STREAM_REF* aNativs;

	int field_0x164;

	int nbTalkParams;
	CTalkParam* aTalkParams;

	int field_0x1d0;

	CAddOnNativ addOnA;
	CAddOnNativ addOnB;
	CAddOnNativ addOnC;
	CAddOnNativ addOnD;

	CBehaviourNativCmdStand behaviourStand;
};

#endif //ACTOR_NATIV_CMD_H