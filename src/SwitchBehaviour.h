#ifndef _SWITCH_BEHAVIOUR_H
#define _SWITCH_BEHAVIOUR_H

#include "Types.h"

struct ByteCode;

class CCondition
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner) {}
	virtual void Begin(CActor* pOwner) {}
	virtual void Manage(CActor* pOwner) {}
	virtual int InterpretMessage(CActor* pOwner, CActor* pSender, int msg, void* pMsgParam) { return 0; }

	byte field_0x4;
};

class CondMessage : public CCondition
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual int InterpretMessage(CActor* pOwner, CActor* pSender, int msg, void* pMsgParam);

	int field_0x8;
};

class CondSeeActorInArea : public CCondition
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner);
	virtual void Manage(CActor* pOwner);

	S_STREAM_REF<CActor> pActor;
	S_STREAM_REF<ed_zone_3d> pZone;
	uint flags;
	uint zoneResult;
};

class CondActorInArea : public CCondition
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner);
	virtual void Manage(CActor* pOwner);

	S_STREAM_REF<CActor> pActor;
	S_STREAM_REF<ed_zone_3d> pZone;
	uint flags;
	uint zoneResult;
};

class CondSeeActor : public CCondition
{
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
	virtual void Init(CActor* pOwner) { IMPLEMENTATION_GUARD(); }
	virtual void Manage(CActor* pOwner) { IMPLEMENTATION_GUARD(); }
};

class CExecute
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner) {}
	virtual int Execute(CActor* pOwner) {}
};

class CExecuteSwitchBehaviour : public CExecute
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual int Execute(CActor* pOwner);

	int behaviourId;
};

class CCoupleConditionExecution
{
public:
	virtual CCondition* GetCondition() = 0;
	virtual CExecute* GetExecution() = 0;

	byte flags;
};

template<typename TCondition, typename TExecute>
class TCouple_Condition_Execution : public CCoupleConditionExecution
{
public:
	virtual CCondition* GetCondition() { return &condition; }
	virtual CExecute* GetExecution() { return &execution; }

	TCondition condition;
	TExecute execution;
};

class CSwitchBehaviour
{
public:
	CSwitchBehaviour();
	void Create(ByteCode* pByteCode);
	void Init(CActor* pOwner);
	void Begin(CActor* pOwner);
	int InterpretMessage(CActor* pOwner, CActor* pSender, int msg, void* pMsgParam);

	int Test(CActor* pOwner);
	int Execute(CActor* pOwner);

	int nbCondSeeActorSwitches;
	int nbCondActorInAreaSwitches;
	int nbCondSeeInAreaSwitches;
	int nbCondMsgSwitches;
	int nbTotalSwitches;

	CCoupleConditionExecution** apSwitches;
};

#endif // _SWITCH_BEHAVIOUR_H