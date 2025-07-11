#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "Types.h"

class CActor;

struct VibrationParam
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	CActor* pActor;
};

class CVibrationDyn
{
public:
	void Init(VibrationParam* pParams);
	void UpdateAllFactors(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* pPosition);
	int UpdateKineticToPotentialUpDown();
	int ComputeCurFactor();
	bool MakeVibrate(edF32VECTOR3* param_2);

	float field_0x0;
	float field_0x4;
	undefined4 field_0x8;
	undefined4 field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	undefined4 field_0x28;
	int field_0x2c;
	byte field_0x30;
	byte field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	CActor* pActor;
	float field_0x38;
	float field_0x3c;
	float field_0x40;
	float field_0x44;
	float field_0x48;
	float field_0x4c;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	float field_0x54;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;

	edF32MATRIX4 rotMatrix;
};

class CScalarDyn
{
public:
	void BuildFromDistTime(float dist, float time);
	void BuildFromSpeedDist(float param_1, float param_2, float distance);
	void BuildFromSpeedDistTime(float param_1, float param_2, float distance, float time);
	void BuildFromSpeedTime(float param_1, float param_2, float param_3);
	void BuildFromDistTimeNoAccel(float dist, float time);
	void Reset();
	bool IsFinished();
	void Integrate(float param_1);
	void Integrate(float param_1, float param_2);
	float GetInstantSpeed();
	bool OnLastValidSample();
	void Stop();

	uint flags;
	float field_0x4;
	float duration;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
};

class CVectorDyn
{
public:
	void Integrate(float param_1, float param_2);
	void Integrate(float param_1);
	void Reset();
	bool IsFinished();
	void BuildFromAccelDistAmplitude(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, byte param_5);

	undefined4 field_0x0;
	byte field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	float field_0x8;
	float field_0xc;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
	float field_0x40;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	edF32VECTOR4 field_0x50;
	edF32VECTOR4 field_0x60;
	edF32VECTOR4 field_0x70;
};

class CDynBase
{
public:
	CDynBase();

	void Init(float param_1, float param_2);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	int field_0x1c;
};

class CValueDyn : public CDynBase
{
public:
	void MoveTo(float param_1);

	edF32VECTOR4 field_0x20;
	float field_0x30;
};

class CVertexDyn : public CDynBase
{
public:
	CVertexDyn();

	void Init(float param_1, edF32VECTOR4* pPosition);
	void MoveTo(edF32VECTOR4* pPosition);
	void FUN_00213b90(edF32MATRIX4* param_2);

	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
};

class CAngleDyn : public CDynBase
{
public:
	CAngleDyn();

	void MoveTo(float angle, int mode);
};

struct SPEED_DYN
{
	void Init(float param_1, float param_2);
	float UpdateLerp(float target);
	void Update(float param_1);

	float currentAlpha;
	float field_0x4;
};

#endif // DYNAMIC_H