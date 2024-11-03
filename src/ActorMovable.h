#ifndef ACTOR_MOVABLE_H
#define ACTOR_MOVABLE_H

#include "Types.h"
#include "Actor.h"
#include "PathFollow.h"
#include "Path.h"

struct SV_MOV_FLEE_ON_PATH_PARAM
{
	float delay;
	int field_0x4;
	float acceleration;
	float speed;
};

class CDynamic
{
public:
	CDynamic();

	void Reset(CActor* pActor);
	void ClearLocalData();

	static float gMaxSpeed_Horiz;
	static float gMaxSpeed_Vert;

	edF32VECTOR4 rotationQuat;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 velocityDirectionEuler;
	edF32VECTOR4 horizontalVelocityDirectionEuler;

	uint flags;

	float horizontalLinearAcceleration;
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
	void Reset();
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
	void Reset();
};

struct CActorMovParamsOut : public CActorParamsOut {
};

struct CActorMovParamsIn : public CActorParamsIn {
	float field_0xc;
	float field_0x14;
	float field_0x18;
};

struct SV_MOV_PATH_PARAM
{
	float acceleration;
	float speed;
	int field_0x8;

	int field_0xc;
	int field_0x10;
	float field_0x14;
	float rotationSpeed;
};

class CActorMovable : public CActor {
public:
	CActorMovable();

	virtual bool IsKindOfObject(ulong kind);
	virtual void Create(ByteCode* pByteCode);
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SetState(int newState, int animType);
	virtual void ChangeManageState(int state);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	void SV_MOV_UpdateSpeedIntensity(float param_1, float param_2);
	void SV_MOV_DecreaseSpeedIntensity(float param_1);
	float SV_MOV_ComputeDistIdealPos(CPathFollowReader* pPathFollowReader, float param_3, float defaultDelay);
	void SV_MOV_ComputeSpeedAccelerationToFleeActor(float param_1, CActorMovable* pFleeActor, CPathPlane* pInPathPlane, SV_MOV_FLEE_ON_PATH_PARAM* pParams);
	float FUN_00115380(edF32VECTOR4* param_2, CPathFollowReader* pPathFollowReader, int param_4, long param_5, edF32VECTOR4* pOutPosition, float* param_7);
	float SV_MOV_ManageMovOnPath(CPathFollowReader* pPathFollowReader, SV_MOV_PATH_PARAM* pMovPathParams);
	bool SV_MOV_UpdatePush(float param_1, S_PUSH_DATA* pPushData, S_PUSH_STREAM_DEF* pPushStreamRef);
	bool SV_MOV_UpdateTilt(float param_1, S_TILT_DATA* pTiltData, S_TILT_STREAM_DEF* pTiltStreamRef);
	void SV_MOV_MoveTo(CActorMovParamsOut* pActorMovParamsOut, CActorMovParamsIn* pActorMovParamsIn, edF32VECTOR4* param_4);
	void SV_MOV_MoveCloserTo(float param_1, edF32VECTOR4* param_3);
	float SV_MOV_GetAccelerationFromDistAndSpeed(float param_1, float param_2, float param_3);
	void SV_MOV_AddSpeedVector(edF32VECTOR4* pSpeedVector);

	float FUN_00120250(float param_1);

	void ComputeRealMoving(edF32VECTOR4* delta);
	float GetSubjectiveCumulatedWeight();
	float GetCumulatedWeight();

	CDynamic dynamic;
	edF32VECTOR3 vector_0x120c;
	edF32VECTOR4 field_0x1c0;
};

#endif //ACTOR_MOVABLE_H