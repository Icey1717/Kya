#ifndef ACTOR_SHIP_H
#define ACTOR_SHIP_H

#include "Types.h"
#include "Actor.h"
#include "Fx.h"
#include "Vision.h"
#include "CinematicManager.h"

class CActorShip;

struct ShipPathManager
{
	void Create(ByteCode* pByteCode);

	int nbInts;
	int* aInts;
	undefined4 field_0x8;
	int field_0xc;
	undefined4 field_0x10;
	undefined4 field_0x14;
	int field_0x18;
	int nb;
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
	CActorShip();
	virtual void Create(ByteCode* pByteCode);

	S_STREAM_REF<CActor> field_0x180;
	S_STREAM_REF<CActor> field_0x184;
	S_STREAM_REF<CActor> hunterStreamRef;
	uint field_0x18c[4];
	uint field_0x19c;
	uint field_0x1a0;
	uint field_0x1a4;
	uint field_0x1a8;
	S_STREAM_REF<ed_zone_3d> field_0x1ac;
	CVision field_0x1b0;
	float field_0x1b4;
	S_ACTOR_STREAM_REF* careBoyStreamRef;
	ShipPathManager shipPathManager;
	float field_0x210;
	float field_0x214;
	float field_0x218;
	float field_0x21c;
	float field_0x220;
	float field_0x224;
	float field_0x228;
	float field_0x22c;
	float field_0x230;
	int field_0x234;
	int field_0x238;
	S_NTF_SWITCH field_0x23c;
	float field_0x25c;
	float field_0x260;

	CFxHandle field_0x26c;
};

#endif //ACTOR_SHIP_H