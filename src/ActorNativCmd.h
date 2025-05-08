#ifndef ACTOR_NATIV_CMD_H
#define ACTOR_NATIV_CMD_H

#include "Types.h"
#include "Actor.h"

struct CTalkParamSubObj {
	undefined4 field_0x0;
	int field_0x4;
};

class CTalkParam 
{
public:
	undefined4 field_0x4;
	CTalkParamSubObj* field_0x8;
	int nbSubObj;
	undefined4 field_0x10;
	undefined4 field_0x14;
	undefined4 field_0x18;
	byte field_0x1c;
	byte field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

struct CActorNativMsgParam
{
	int type;
	int field_0x4;
};

struct CActorNativMsgParamOther
{
	int type;
	int field_0x4;
};

class CAddOnNativ : public CAddOn 
{
public:
	CAddOnNativ();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pActor);
	virtual void Manage();
	virtual void Reset();
	virtual bool Func_0x20();

	int nbSubObjs;
	CAddOnSubObj* aSubObjs;
};

class CActorNativCmd;
class CActorNativ;

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

class CActorNativCmd : public CActor {
public:
	CActorNativCmd(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

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
	void FUN_00394250(CActorNativ* pNativ);

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