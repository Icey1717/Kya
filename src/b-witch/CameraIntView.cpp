#include "CameraIntView.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include "InputManager.h"
#include "LevelScheduler.h"

ECameraType CCameraIntView::GetMode()
{
	return CT_IntView;
}

edF32VECTOR4 edF32VECTOR4_004259f0 = { 0.0f, 1.4f, 0.0f, 0.0f };

void CCameraIntView::Init()
{
	CActor* pCVar5;

	CCameraAimBase::Init();

	this->fov = 0.84f;
	SetDistance(4.0f);
	SetAngleAlpha(0.0f);
	pCVar5 = GetTarget();
	SetAngleBeta(pCVar5->rotationEuler.y);
	this->field_0x50 = edF32VECTOR4_004259f0;
	this->flags_0xc = this->flags_0xc | 0x81;

	this->field_0xe0.Init(0.0f, 14.0f);
	this->field_0xe8.Init(0.0f, 15.0f);

	return;
}

void AccurateMatrix4RotateY(edF32MATRIX4* param_1, edF32MATRIX4* param_2, float param_3)
{
	edF32MATRIX4 local_40;

	local_40.aa = cosf(param_3);
	local_40.ca = sinf(param_3);
	local_40.ac = -local_40.ca;
	local_40.bb = 1.0f;
	local_40.dd = 1.0f;
	local_40.ab = 0.0f;
	local_40.ad = 0.0f;
	local_40.ba = 0.0f;
	local_40.bc = 0.0f;
	local_40.bd = 0.0f;
	local_40.cb = 0.0f;
	local_40.cd = 0.0f;
	local_40.da = 0.0f;
	local_40.db = 0.0f;
	local_40.dc = 0.0f;
	local_40.cc = local_40.aa;
	edF32Matrix4MulF32Matrix4Soft(param_1, param_2, &local_40);

	return;
}

void AccurateMatrix4RotateX(edF32MATRIX4* param_1, edF32MATRIX4* param_2, float param_3)
{
	edF32MATRIX4 local_40;

	local_40.bb = cosf(param_3);
	local_40.bc = sinf(param_3);
	local_40.cb = -local_40.bc;
	local_40.aa = 1.0f;
	local_40.dd = 1.0f;
	local_40.ab = 0.0f;
	local_40.ac = 0.0f;
	local_40.ad = 0.0f;
	local_40.ba = 0.0f;
	local_40.bd = 0.0f;
	local_40.ca = 0.0f;
	local_40.cd = 0.0f;
	local_40.da = 0.0f;
	local_40.db = 0.0f;
	local_40.dc = 0.0f;
	local_40.cc = local_40.bb;
	edF32Matrix4MulF32Matrix4Soft(param_1, param_2, &local_40);

	return;
}

void CCameraIntView::Manage_Pad(float* param_2, float* param_3)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	fVar2 = edFIntervalLERP(this->fov, 0.1f, 0.84f, 0.1f, 1.0f);
	fVar3 = 0.0f;
	fVar4 = 0.0f;
	fVar5 = 0.0f;
	if ((0.0f <= gPlayerInput.aAnalogSticks[0].y) || (1.047198f < this->field_0x100)) {
		if ((0.0f < gPlayerInput.aAnalogSticks[0].y) && (-1.047198f <= this->field_0x100)) {
			fVar3 = edFIntervalLERP(gPlayerInput.aAnalogSticks[0].y, 0.0f, 1.0f, 0.0f, -2.5f);
		}
	}
	else {
		fVar3 = edFIntervalLERP(gPlayerInput.aAnalogSticks[0].y, 0.0f, -1.0f, 0.0f, 2.5f);
	}

	this->field_0xe0.Update(fVar3);

	fVar3 = fVar2 * this->field_0xe0.currentAlpha * CCamera::_gpcam_man->time_0x4;
	*param_2 = fVar3;
	this->field_0x100 = this->field_0x100 + fVar3;
	if (gPlayerInput.aAnalogSticks[0].x < 0.0f) {
		fVar4 = edFIntervalLERP(gPlayerInput.aAnalogSticks[0].x, 0.0f, -1.0f, 0.0f, -3.141593f);
	}
	else {
		if (0.0f < gPlayerInput.aAnalogSticks[0].x) {
			fVar4 = edFIntervalLERP(gPlayerInput.aAnalogSticks[0].x, 0.0f, 1.0f, 0.0f, 3.141593f);
		}
	}

	this->field_0xe8.Update(fVar4);

	fVar2 = fVar2 * -this->field_0xe8.currentAlpha * CCamera::_gpcam_man->time_0x4;
	*param_3 = fVar2;
	fVar3 = edF32Between_2Pi(this->field_0x104 + fVar2);
	this->field_0x104 = fVar3;
	iVar1 = CLevelScheduler::ScenVar_Get(SCN_ABILITY_BINOCULARS);
	if (iVar1 == 0) {
		this->fov = 0.84f;
	}
	else {
		fVar3 = gPlayerInput.aButtons[14].analogValue - gPlayerInput.aButtons[15].analogValue;
		if (fabsf(fVar3) < 0.4f) {
			fVar3 = 0.0f;
		}

		if ((0.0f <= fVar3) || (0.84f < this->fov)) {
			if ((0.0f < fVar3) && (0.1f <= this->fov)) {
				fVar5 = edFIntervalLERP(fVar3, 0.0f, 1.0f, -0.2f, -2.0f);
			}
		}
		else {
			fVar5 = edFIntervalLERP(fVar3, 0.0f, -1.0f, 0.2f, 2.0f);
		}

		fVar3 = this->fov;
		fVar3 = fVar3 + fVar5 * fVar3 * CCamera::_gpcam_man->time_0x4;
		this->fov = fVar3;

		if (fVar3 < 0.1f) {
			this->fov = 0.1f;
		}

		if (0.84f < this->fov) {
			this->fov = 0.84f;
		}
	}

	return;
}

edF32VECTOR4 edF32VECTOR4_00425a00 = { 0.0f, 0.0f, 1.0f, 0.0f };

bool CCameraIntView::Manage()
{
	bool bVar1;
	edF32VECTOR3* pfVar2;
	edF32MATRIX4* peVar2;
	float fVar3;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	float local_18;
	float local_14;
	float local_10;
	float local_8;
	float local_4;

	if (GetTarget() == (CActor*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = false;
		if ((this->flags_0xc & 0x100000) == 0) {
			bVar1 = CCamera::Manage();
			if (bVar1 == false) {
				local_30 = edF32VECTOR4_00425a00;
				pfVar2 = GetAngles();
				local_18 = pfVar2->x;
				local_14 = pfVar2->y;
				local_10 = pfVar2->z;
				ComputeTargetOffset(&eStack80);
				ComputeTargetPosition(&local_40);
				edF32Vector4AddHard(&local_40, &local_40, &eStack80);
				if ((CCamera::_gpcam_man->flags & 0x4000000) == 0) {
					Manage_Pad(&local_4, &local_8);
					if (this->field_0x100 < -1.047198f) {
						local_4 = 0.0f;
						this->field_0xe0.currentAlpha = 0.0f;
						this->field_0x100 = -1.047198f;
					}
					if (1.047198f < this->field_0x100) {
						local_4 = 0.0f;
						this->field_0xe0.currentAlpha = 0.0f;
						this->field_0x100 = 1.0471976f;
					}
					local_18 = local_18 + local_4;
					local_14 = local_14 + local_8;
				}

				SetAngleAlpha(local_18);

				fVar3 = edF32Between_2Pi(local_14);
				SetAngleBeta(fVar3);
				SetAngleGamma(local_10);
				fVar3 = GetAngleGamma();
				edF32Matrix4RotateZHard(fVar3, &this->transformationMatrix, &gF32Matrix4Unit);
				fVar3 = GetAngleAlpha();
				peVar2 = &this->transformationMatrix;
				AccurateMatrix4RotateX(peVar2, peVar2, fVar3);
				fVar3 = GetAngleBeta();
				peVar2 = &this->transformationMatrix;
				AccurateMatrix4RotateY(peVar2, peVar2, fVar3);
				edF32Matrix4MulF32Vector4Hard(&this->field_0xd0, &this->transformationMatrix, &local_30);
				this->transformationMatrix.rowT = local_40;
				fVar3 = GetDistance();
				edF32Vector4ScaleHard(fVar3, &local_30, &this->field_0xd0);
				edF32Vector4AddHard(&local_30, &local_30, &this->transformationMatrix.rowT);
				this->lookAt = local_30;
				if ((CCamera::_gpcam_man->flags & 0x4000000) == 0) {
					fVar3 = GetAngleBeta();
					GetTarget()->rotationEuler.y = fVar3;
					SetVectorFromAngles(&GetTarget()->rotationQuat, &GetTarget()->rotationEuler.xyz);
				}

				FUN_00199b80();

				fVar3 = GetAngleYFromVector(&this->transformationMatrix.rowZ);
				fVar3 = edF32Between_2Pi(fVar3);
				SetAngleBeta(fVar3);
				bVar1 = true;
			}
			else {
				bVar1 = false;
			}
		}
	}

	return bVar1;
}

bool CCameraIntView::AlertCamera(int alertType, void* pParams, CCamera* param_4)
{
	CCamera* pCVar1;
	ECameraType EVar2;
	edF32VECTOR3* v0;
	float fVar3;

	if (alertType == 2) {
		pCVar1 = CCamera::_gpcam_man->pActiveCamera;
		EVar2 = pCVar1->GetMode();
		if (EVar2 == CT_SilverBoomy) {
			this->field_0x100 = 0.0f;
			this->field_0x104 = 0.0f;
			fVar3 = pCVar1->GetAngleAlpha();
			SetAngleAlpha(fVar3);
			fVar3 = pCVar1->GetAngleBeta();
			SetAngleBeta(fVar3);
		}
		else {
			this->field_0x100 = 0.0f;
			this->field_0x104 = 0.0f;
			SetAngleAlpha(0.0f);
			fVar3 = pCVar1->GetAngleBeta();
			SetAngleBeta(fVar3);
		}

		this->fov = 0.84f;

		v0 = GetAngles();
		edF32Matrix4FromEulerSoft(&this->transformationMatrix, v0, "ZXY");

		this->field_0xe0.Init(0.0f, 14.0f);
		this->field_0xe8.Init(0.0f, 15.0f);

		CCameraExt::ComputeTargetPosition(&this->transformationMatrix.rowT);
		this->transformationMatrix.rowT.y = this->transformationMatrix.rowT.y + 1.4f;
	}

	CCamera::AlertCamera(alertType, pParams, param_4);

	return false;
}
