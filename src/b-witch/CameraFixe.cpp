#include "CameraFixe.h"
#include "MathOps.h"

CCameraFixe::CCameraFixe(ByteCode* pByteCode)
	: CCamera(pByteCode)
{
	this->flags_0xc = this->flags_0xc | 0x20000;
	return;
}

ECameraType CCameraFixe::GetMode()
{
	return (ECameraType)0xe;
}

void CCameraFixe::Init()
{
	int iVar1;
	edF32MATRIX4* peVar2;
	edF32MATRIX4* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	CCamera::Init();

	this->field_0xb0 = this->transformationMatrix;
	this->field_0xf0 = this->lookAt;

	return;
}

bool CCameraFixe::Manage()
{
	edF32MATRIX4 eStack64;
	CActor* pActor;

	if ((this->flags_0xc & 0x800000) == 0) {
		this->flags_0xc = this->flags_0xc | 0x800000;

		pActor = this->pOtherTarget;
		if (pActor != (CActor*)0x0) {
			pActor->SV_ComputeDiffMatrixFromInit(&eStack64);
			edF32Matrix4MulF32Matrix4Hard(&this->transformationMatrix, &this->field_0xb0, &eStack64);
			edF32Matrix4MulF32Vector4Hard(&this->lookAt, &eStack64, &this->field_0xf0);
			FUN_00199b80();
		}
	}

	return true;
}

float CCameraFixe::GetAngleGamma()
{
	return this->angleGamma;
}

void CCameraFixe::SetAngleGamma(float angle)
{
	this->angleGamma = angle;
}

