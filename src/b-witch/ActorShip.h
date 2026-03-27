#ifndef ACTOR_SHIP_H
#define ACTOR_SHIP_H

#include "Types.h"
#include "Actor.h"
#include "Fx.h"
#include "Vision.h"
#include "CinematicManager.h"

#define SHIP_BEHAVIOUR_FLY 0x2

#define SHIP_FLY_STATE_PHASE_1 0x5
#define SHIP_FLY_STATE_FLAME_THROWER 0x6
#define SHIP_FLY_STATE_THROWING 0x7
#define SHIP_FLY_STATE_CLOSING 0x8
#define SHIP_FLY_STATE_GOBING 0x9

class CActorShip;

struct ACTIVE_PATHS_CONTAINER
{
	uint field_0x0;
	int field_0x4;
	CFxHandle aActivePaths[32];
};

struct CFxArrayManager
{
	CFxArrayManager();
	void Create(ByteCode* pByteCode);
	bool CheckForKimTouch(float param_1);
	bool CheckForFireShotTouch(CActor* pActor);

	int nbInts;
	int nb;
	int field_0x8;
	uint flags;
	int* aInts;
	ACTIVE_PATHS_CONTAINER* aActivePathContainers;
	int field_0x18;
};

class CBehaviourShip : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorShip* pOwner;
};

class CBehaviourShipFLY : public CBehaviourShip
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorShip : public CActor
{
public:
	static StateConfig _gStateCfg_SHP[8];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage();
	virtual void Reset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ClearLocalData();
	void SetShipAnimations(int state);
	void BehaviourShipFLY_InitState(int newState);
	void BehaviourShipFLY_Manage();

	void ManageFlameThrower(int param_2);

	S_ACTOR_STREAM_REF* careBoyStreamRef;
	CFxArrayManager fxArrayManager;
	S_STREAM_REF<CActor> jawCareBoyStreamRef;
	S_STREAM_REF<CActor> airCannonStreamRef;
	S_STREAM_REF<CActor> hunterStreamRef;
	uint field_0x18c[4];
	uint field_0x19c;
	uint field_0x1a0;
	uint field_0x1a4;
	uint field_0x1a8;
	S_STREAM_REF<ed_zone_3d> field_0x1ac;
	CVision vision;
	float field_0x210;
	float field_0x214;
	float field_0x218;
	float field_0x21c;
	float field_0x220;
	float field_0x224;
	float kimTouchRange;
	float field_0x22c;
	float field_0x230;
	int field_0x234;
	int field_0x238;
	S_NTF_SWITCH field_0x23c;
	CBehaviourShipFLY behaviourFly;
	int nbCareBoys;
	CActor* field_0x24c;
	float field_0x250;
	float field_0x258;
	float field_0x25c;
	float field_0x260;

	CFxHandle field_0x264;
	CFxHandle field_0x26c;

	float field_0x300;
	byte field_0x304;
	byte field_0x305;
	int field_0x308;
	int field_0x30c;
	CActor* field_0x310;
	edF32VECTOR4 field_0x320;
	edF32VECTOR3 field_0x330;
};

#endif //ACTOR_SHIP_H