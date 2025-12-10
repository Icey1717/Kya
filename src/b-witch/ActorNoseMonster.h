#ifndef ACTOR_NOSE_MONSTER_H
#define ACTOR_NOSE_MONSTER_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "FireShot.h"
#include "Fx.h"
#include "CinematicManager.h"

#define NOSE_MONSTER_BEHAVIOUR_SLEEP 0x2
#define NOSE_MONSTER_BEHAVIOUR_ATTACK 0x3
#define NOSE_MONSTER_BEHAVIOUR_DEATH 0x4

#define NOSE_MONSTER_STATE_ATTACK_WAIT 0x7
#define NOSE_MONSTER_STATE_ATTACK_SHOOT 0xb
#define NOSE_MONSTER_STATE_ATTACK_BURST_FIRE 0xc
#define NOSE_MONSTER_STATE_ATTACK_PUSTULE_DESTROYED 0xf


class CActorNoseMonster;
class CPathFollow;
struct ed_zone_3d;

class CBehaviourNoseMonster : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	CActorNoseMonster* pOwner;
};

class CBehaviourNoseMonsterSleep : public CBehaviourNoseMonster
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
};

class CBehaviourNoseMonsterAttack : public CBehaviourNoseMonster
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	int field_0x8;
	int targetedPustuleIndex;
};

class CBehaviourNoseMonsterDeath : public CBehaviourNoseMonster
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
};

struct ZonePathEntry
{
	S_STREAM_REF<ed_zone_3d> zoneRef;
	S_STREAM_REF<CPathFollow> pathFollowRef;
};

struct ZonePathEntryList
{
	int nbEntries;
	ZonePathEntry aEntries[];
};

class CActorNoseMonster : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_NoseMonster[13];

	CActorNoseMonster();

	virtual void Create(ByteCode* pByteCode);

	virtual void Init();
	virtual void Term();

	virtual void Reset();
	virtual void CheckpointReset();

	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual StateConfig* GetStateCfg(int state);
	virtual void UpdateLookingAt();

	virtual void CinematicMode_Leave(int behaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourInitState(int newState);

	void ManageBehaviourAttack();

	void ManageMaterial();
	void ManageOrientation();

	void StateNoseMonsterWait();
	void FUN_002f7230();
	void StateNoseMonsterBurstFire();
	void StateNoseMonsterPustuleDestroyed();

	void FUN_002f7760();

	S_STREAM_REF<ed_zone_3d> zoneRef;
	float field_0x354;
	float field_0x358;
	float field_0x35c;
	float field_0x360;
	
	uint field_0x364;
	ZonePathEntryList* pZonePathFollowList;
	uint field_0x36c;
	float field_0x370;
	float field_0x374;
	float field_0x378;
	float field_0x37c;

	uint field_0x380;
	float field_0x384;

	CFireShot fireShot;
	int field_0x630;
	S_NTF_SWITCH_ONOFF ntfSwitch;
	float field_0x63c;
	float shootTimer;

	int bBurstFire;
	uint pustuleBits;

	int field_0x644;
	float field_0x650;
	int field_0x654;
	float field_0x658;
	float field_0x65c;

	float field_0x664;
	float field_0x668;
	float field_0x66c;
	float field_0x670[6];

	float field_0x688;
	float field_0x68c;
	float field_0x690;
	float field_0x694;
	float field_0x698;
	float field_0x69c;
	uint field_0x6a0;
	uint field_0x6a4;
	uint field_0x6a8;

	CBehaviourNoseMonsterAttack behaviourAttack;
	CBehaviourNoseMonsterSleep behaviourSleep;
	CBehaviourNoseMonsterDeath behaviourDeath;

	CFxHandle field_0x6cc[6];

	byte field_0x6fc;
	byte field_0x6fd;
};

#endif //ACTOR_NOSE_MONSTER_H