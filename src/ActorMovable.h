#ifndef ACTOR_MOVABLE_H
#define ACTOR_MOVABLE_H

#include "Types.h"
#include "Actor.h"

class CDynamic
{
public:
	CDynamic();

	void Reset(CActor* pActor);

	static float gMaxSpeed_Horiz;
	static float gMaxSpeed_Vert;

	edF32VECTOR4 rotationQuat;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 velocityDirectionEuler;
	edF32VECTOR4 horizontalVelocityDirectionEuler;

	uint flags;

	float linearJerk;
	float linearAcceleration;
	float speed;
	undefined4 field_0x4c;

	float weightB;
	float weightA;
};


struct S_OSCILLATION_CONFIG {
	float field_0x0;
	float field_0x4;
};

struct S_TILT_STREAM_DEF {
	int field_0x0;
	float field_0x4;
	S_OSCILLATION_CONFIG oscConfig;

	void Init();
};

struct S_PUSH_STREAM_DEF {
	float field_0x0;
	S_OSCILLATION_CONFIG oscConfig;

	void Init();
};

struct S_OSCILLATING_VALUE {
	float field_0x0;
	float field_0x4;

	bool Update(float param_1, float deltaTime, S_OSCILLATION_CONFIG* pConfig);
};

struct S_PUSH_DATA {
	S_OSCILLATING_VALUE oscValue;

	void Init();
};

struct S_OSCILLATING_QUAT {
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;

	bool Update(float param_1, S_OSCILLATION_CONFIG* pConfig, edF32VECTOR4* param_4);
};

struct S_TILT_DATA {
	edF32VECTOR4 field_0x0;
	S_OSCILLATING_QUAT oscQuat;

	S_OSCILLATING_VALUE oscValue;
	float field_0x38;
	S_PUSH_DATA pushData;

	void Init(float param_1, CActor* pActor, S_TILT_STREAM_DEF* pStreamDef);
};

struct CActorMovParamsOut : public CActorParamsOut {
};

struct CActorMovParamsIn : public CActorParamsIn {
	float field_0xc;
	float field_0x14;
	float field_0x18;
};

class CActorMovable : public CActor {
public:

	virtual void Create(ByteCode* pByteCode);
	virtual void SetState(int newState, int animType);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void SV_MOV_UpdateSpeedIntensity(float param_1, float param_2);
	void SV_MOV_DecreaseSpeedIntensity(float param_1);
	bool SV_MOV_UpdatePush(float param_1, S_PUSH_DATA* pPushData, S_PUSH_STREAM_DEF* pPushStreamRef);
	bool SV_MOV_UpdateTilt(float param_1, S_TILT_DATA* pTiltData, S_TILT_STREAM_DEF* pTiltStreamRef);
	void SV_MOV_MoveTo(CActorMovParamsOut* pActorMovParamsOut, CActorMovParamsIn* pActorMovParamsIn, edF32VECTOR4* param_4);

	void ComputeRealMoving(edF32VECTOR4* delta);
	float GetSubjectiveCumulatedWeight();
	float GetCumulatedWeight();

	CDynamic dynamic;
	float field_0x1c0;

	edF32VECTOR3 vector_0x120c;
};

#endif //ACTOR_MOVABLE_H