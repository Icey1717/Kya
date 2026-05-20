#ifndef ACTOR_BRAZUL_H
#define ACTOR_BRAZUL_H

#include "Types.h"
#include "ActorWolfen.h"

class CActorBrazul;

struct EnemyComponent80Ext : public EnemyComponent80
{
	virtual void SetupObjects(CActor* pOwner) {}
	virtual void Term() {}

	int UpdatePostAnimEffects() { return 0; }
};

struct astruct_19
{
	void Create(int param_2, int param_3, int param_4);
	void Init(CActorBrazul* param_2);
	void Manage(edF32VECTOR4* v0);
	void Reset(float param_1);
	void Draw();

	CActorBrazul* pOwner;
	int field_0x4;

	edF32VECTOR4 field_0x10;

	CActor* field_0x30;
	float field_0x34;
	float field_0x38;
	float field_0x3c;

	CFxSpark field_0x40;

	edF32VECTOR3 field_0x130[3];
	float field_0x154[3];

	CFxHandleExt field_0x3d0;
	CFxHandleExt field_0x3dc;
};

class CBehaviourBrazulA : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourBrazulB : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorBrazul* pOwner;
	S_STREAM_REF<CWayPoint> waypointRef;
	S_NTF_SWITCH_ONOFF ntfSwitch;
};

class CBehaviourFighterBrazul : public CBehaviourFighterWolfen
{
public:
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	// CActorFighter
	virtual void _ManageHit(bool bPlayImpact);

	// CBehaviourFighterWolfen
	virtual void ManageExit();

};

class CActorBrazul : public CActorWolfen
{
public:
	static StateConfig gStateCfg_BRZ[12];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);
	virtual void UpdatePostAnimEffects();
	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual void SetState(int newState, int animType);
	virtual void ChangeManageState(int state);
	virtual bool Can_0x9c();
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void FUN_003e60c0();
	void FUN_003e59e0();
	void FUN_003e62d0(float param_1);
	void FUN_003e5f40(float param_1);
	void FUN_003e87e0(int nextState, int param_3);

	float field_0xfc0;
	float field_0xfc4;
	int field_0xfc8;
	float field_0xfcc;
	CVectorDyn field_0xfd0;

	CPathFollowReader field_0x1050;

	uint field_0x1060;
	edF32VECTOR4* field_0x1064;

	astruct_19 field_0x1070[8];

	float field_0x2ff0;
	float field_0x2ff4;
	float field_0x2ff8;
	float field_0x2ffc;

	CFxHandleExt field_0x3000;
	CFxHandleExt field_0x300c;
	EnemyComponent80Ext field_0x3020;
	CFxHandleExt field_0x30a0;
	CFxHandleExt field_0x30ac[6];

	StaticMeshComponentAdvanced field_0x3100;
	StaticMeshComponentAdvanced field_0x31e0;

	CFxHandleExt field_0x3260;
	CFxHandleExt field_0x326c;
};

#endif //ACTOR_BRAZUL_H