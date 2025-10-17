#include "CameraCinematic.h"
#include "MathOps.h"


CCameraCinematic::CCameraCinematic()
{
	this->fov = 0.6f;
	this->transformationMatrix.rowT = gF32Vertex4Zero;
	this->lookAt = gF32Vertex4Zero;

	SetDistance(1.0f);

	this->field_0x8 = -100;
	this->flags_0xc = (this->flags_0xc) | 0x37c;
	this->switchMode = SWITCH_MODE_B;

	this->field_0x98 = 0.8f;
	this->field_0x94 = SWITCH_MODE_A;
	this->field_0x9c = 0.8f;

	this->field_0xb0.x = 0.0f;
	this->field_0xb0.y = 0.0f;
	this->field_0xb0.z = 0.0f;
	return;
}


void CCameraCinematic::SetTransform(edF32MATRIX4* transformMatrix)
{
	float fVar1;
	int iVar2;
	edF32VECTOR4 lookAt;

	this->transformationMatrix = *transformMatrix;
	edF32Matrix4ToEulerSoft(transformMatrix, &this->field_0xb0, "ZXY");
	edF32Vector4AddHard(&lookAt, &this->transformationMatrix.rowT, &transformMatrix->rowZ);
	this->lookAt = lookAt;
	return;
}

ECameraType CCameraCinematic::GetMode(void)
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_Cinematic: 0x{:x}", (int)CT_Cinematic);
	return CT_Cinematic;
}

void CCameraCinematic::Init()
{
	CCamera::Init();
	this->flags_0xc = this->flags_0xc | 0x20000;
}

float CCameraCinematic::GetAngleAlpha()
{
	return (this->field_0xb0).alpha;
}

float CCameraCinematic::GetAngleBeta()
{
	return (this->field_0xb0).beta;
}

float CCameraCinematic::GetAngleGamma()
{
	return (this->field_0xb0).gamma;
}