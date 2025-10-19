#include "Dynamic.h"

#include "MathOps.h"
#include "TimeController.h"
#include "Actor.h"


// Should be in: D:/Projects/b-witch/ActorServices.cpp
void CVibrationDyn::Init(VibrationParam* pParams)
{
	this->field_0x0 = 0.0f;
	this->field_0x4 = 0.0f;
	this->field_0x8 = 0;
	this->field_0xc = 0;
	this->field_0x10 = 0.0f;
	this->field_0x14 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x1c = 0.0f;
	this->field_0x20 = 0.0f;
	this->field_0x24 = 0.0f;
	this->field_0x28 = 1;
	this->field_0x2c = 1;
	this->field_0x30 = 1;
	this->field_0x31 = 1;
	this->field_0x38 = 1.0f;

	this->field_0x3c = pParams->field_0x0;
	this->field_0x40 = pParams->field_0x4;
	this->field_0x44 = pParams->field_0x8;

	this->pActor = pParams->pActor;

	this->field_0x48 = pParams->field_0xc;
	this->field_0x4c = pParams->field_0x10;
	this->field_0x54 = pParams->field_0x14;

	edF32VECTOR3 actorRotation = this->pActor->rotationEuler.xyz;

	edF32Matrix4FromEulerSoft(&this->rotMatrix, &actorRotation, "XYZ");
}


void CVibrationDyn::UpdateAllFactors(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* pPosition)
{
	int iVar1;
	int iVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	edF32Vector4SubHard(&eStack48, pPosition, &this->pActor->currentLocation);

	this->field_0x30 = 1;
	this->field_0x31 = 1;

	fVar7 = 0.2f;
	fVar6 = (param_1 / this->field_0x40) * (eStack48.y / ((this->field_0x4c * 3.0f) / 4.0f));
	if (0.2f <= fVar6) {
		fVar7 = fVar6;
	}

	edF32Vector4ScaleHard(fVar7, &local_20, param_3);

	if ((this->pActor->flags & 0x1000) != 0) {
		this->pActor->Compute2DOrientationFromAngles();
	}

	local_4 = edF32Vector4DotProductHard(&this->pActor->rotationQuat, &local_20);
	local_40.y = 0.0f;
	local_40.z = -local_20.x;
	local_40.x = local_20.z;

	local_8 = edF32Vector4DotProductHard(&this->pActor->rotationQuat, &local_40);
	local_10 = this->field_0x18;
	this->field_0x18 = this->field_0x18 + local_8;
	local_c = this->field_0x1c;
	this->field_0x1c = this->field_0x1c + local_4;

	if (1.0f < fabs(this->field_0x18) + fabs(this->field_0x10)) {
		if (0.0f < this->field_0x18) {
			iVar1 = 1;
		}
		else {
			iVar1 = -1;
		}
		this->field_0x18 = (1.0f - fabs(this->field_0x10)) * (float)iVar1;
	}

	if (1.0f < fabs(this->field_0x1c) + fabs(this->field_0x14)) {
		if (0.0f < this->field_0x1c) {
			iVar1 = 1;
		}
		else {
			iVar1 = -1;
		}
		this->field_0x1c = (1.0f - fabs(this->field_0x14)) * (float)iVar1;
	}

	fVar7 = 1.0f;
	if (local_10 != 0.0f) {
		fVar7 = fabs(local_10) / (fabs(local_10) + fabs(this->field_0x10));
	}

	fVar6 = fabs(this->field_0x18);
	if (this->field_0x18 == 0.0f) {
		if (0.0f < this->field_0x0) {
			iVar1 = 1;
		}
		else {
			iVar1 = -1;
		}
		this->field_0x20 = (fVar6 * (float)-iVar1 * GetTimer()->cutsceneDeltaTime) / (this->field_0x44 * fVar7);
	}
	else {
		if (0.0f < this->field_0x18) {
			iVar1 = 1;
		}
		else {
			iVar1 = -1;
		}
		this->field_0x20 = (fVar6 * (float)iVar1 * GetTimer()->cutsceneDeltaTime) / (this->field_0x44 * fVar7);
	}

	fVar7 = 1.0f;
	if (local_c != 0.0f) {
		fVar7 = fabs(local_c) / (fabs(local_c) + fabs(this->field_0x14));
	}

	fVar5 = this->field_0x1c;
	if (this->field_0x1c == 0.0f) {
		if (0.0f < this->field_0x4) {
			iVar1 = 1;
		}
		else {
			iVar1 = -1;
		}
		this->field_0x24 = (fVar6 * (float)-iVar1 * GetTimer()->cutsceneDeltaTime) / (this->field_0x44 * fVar7);
	}
	else {
		if (0.0f < this->field_0x1c) {
			iVar1 = 1;
		}
		else {
			iVar1 = -1;
		}
		this->field_0x24 = (fabs(fVar5) * (float)iVar1 * GetTimer()->cutsceneDeltaTime) / (this->field_0x44 * fVar7);
	}

	fVar7 = this->field_0x20;
	if ((fVar7 < 0.0005f) && (-0.0005f < fVar7)) {
		iVar1 = 1;
		if (fVar7 <= 0.0f) {
			iVar1 = -1;
		}
		this->field_0x20 = (float)iVar1 * 0.0005f;
	}

	fVar7 = this->field_0x24;
	if ((fVar7 < 0.0005f) && (-0.0005f < fVar7)) {
		iVar1 = 1;
		if (fVar7 <= 0.0f) {
			iVar1 = -1;
		}
		this->field_0x24 = (float)iVar1 * 0.0005f;
	}

	fVar7 = this->field_0x10;
	if (this->field_0x18 == 0.0f) {
		iVar4 = 1;
		if (fVar7 == 0.0f) {
			iVar4 = 2;
		}
	}
	else {
		if (fVar7 == 0.0f) {
			iVar4 = 0;
		}
		else {
			iVar1 = 1;
			if (fVar7 <= 0.0f) {
				iVar1 = -1;
			}

			iVar3 = 1;
			if (this->field_0x18 <= 0.0f) {
				iVar3 = -1;
			}

			iVar4 = 1;
			if (iVar3 != iVar1) {
				iVar4 = 0;
			}
		}
	}

	fVar7 = this->field_0x14;
	if (this->field_0x1c == 0.0f) {
		iVar1 = 1;
		if (fVar7 == 0.0f) {
			iVar1 = 2;
		}
	}
	else {
		if (fVar7 == 0.0f) {
			iVar1 = 0;
		}
		else {
			iVar1 = 1;
			if (fVar7 <= 0.0f) {
				iVar1 = -1;
			}

			iVar3 = 1;
			if (this->field_0x1c <= 0.0f) {
				iVar3 = -1;
			}

			if (iVar3 == iVar1) {
				iVar1 = 1;
			}
			else {
				iVar1 = 0;
			}
		}
	}

	if (iVar4 != 2) {
		this->field_0x28 = iVar4;
	}

	if (iVar1 != 2) {
		this->field_0x2c = iVar1;
	}

	return;
}



// Should be in: D:/Projects/b-witch/ActorServices.cpp
int CVibrationDyn::UpdateKineticToPotentialUpDown()
{
	int iVar2;

	if (fabs(this->field_0x24) < fabs(this->field_0x1c)) {
		this->field_0x4 = this->field_0x4 + this->field_0x24;
		this->field_0x1c = this->field_0x1c - this->field_0x24;
		this->field_0x14 = this->field_0x14 + this->field_0x24;
		iVar2 = 1;
	}
	else {
		this->field_0x14 = this->field_0x14 + this->field_0x1c;

		if (this->field_0x31 != 0) {
			iVar2 = 1;
			if (this->field_0x24 <= 0.0f) {
				iVar2 = -1;
			}

			this->field_0x24 = ((float)iVar2 * fabs(this->field_0x14) * GetTimer()->cutsceneDeltaTime) / this->field_0x44;
			this->field_0x31 = 0;
		}

		iVar2 = 0;
		this->field_0x14 = this->field_0x14 * this->field_0x3c;
		this->field_0x1c = 0.0f;
		this->field_0x4 = this->field_0x14;
		this->field_0x24 = this->field_0x24 * -1.0f;
	}

	return iVar2;
}

// Should be in: D:/Projects/b-witch/ActorServices.cpp
int CVibrationDyn::ComputeCurFactor()
{
	bool bVar1;
	bool bVibAComplete;
	bool bVibBComplete;
	undefined4 uVar5;
	uint uVar6;
	int iVar7;

	bVibAComplete = false;
	bVibBComplete = false;

	if (this->field_0x54 < fabs(this->field_0x10) + fabs(this->field_0x18)) {
		if (this->field_0x28 == 0) {
			bVar1 = fabs(this->field_0x20) < fabs(this->field_0x10);

			if (bVar1) {
				this->field_0x0 = this->field_0x0 + this->field_0x20;
				this->field_0x18 = this->field_0x18 + this->field_0x20;
				this->field_0x10 = this->field_0x10 + this->field_0x20;
			}
			else {
				this->field_0x18 = this->field_0x18 + this->field_0x10;
				this->field_0x10 = 0.0f;
				this->field_0x0 = 0.0f;
			}

			this->field_0x28 = (uint)!bVar1;
		}
		else {
			if (fabs(this->field_0x20) < fabs(this->field_0x18)) {
				this->field_0x0 = this->field_0x0 + this->field_0x20;
				this->field_0x18 = this->field_0x18 - this->field_0x20;
				this->field_0x10 = this->field_0x10 + this->field_0x20;
				uVar5 = 1;
			}
			else {
				this->field_0x10 = this->field_0x10 + this->field_0x18;
				if (this->field_0x30 != 0) {
					iVar7 = 1;

					if (this->field_0x20 <= 0.0f) {
						iVar7 = -1;
					}

					this->field_0x20 = ((float)iVar7 * fabs(this->field_0x10) * GetTimer()->cutsceneDeltaTime) / this->field_0x44;
					this->field_0x30 = 0;
				}

				uVar5 = 0;
				this->field_0x10 = this->field_0x10 * this->field_0x3c;
				this->field_0x18 = 0.0f;
				this->field_0x0 = this->field_0x10;
				this->field_0x20 = this->field_0x20 * -1.0f;
			}

			this->field_0x28 = uVar5;
		}
	}
	else {
		bVibAComplete = true;
	}

	if (this->field_0x54 < fabs(this->field_0x14) + fabs(this->field_0x1c)) {
		if (this->field_0x2c == 0) {
			bVar1 = fabs(this->field_0x24) < fabs(this->field_0x14);
			if (bVar1) {
				this->field_0x4 = this->field_0x4 + this->field_0x24;
				this->field_0x1c = this->field_0x1c + this->field_0x24;
				this->field_0x14 = this->field_0x14 + this->field_0x24;
			}
			else {
				this->field_0x1c = this->field_0x1c + this->field_0x14;
				this->field_0x14 = 0.0f;
				this->field_0x4 = 0.0f;
			}

			this->field_0x2c = (int)!bVar1;
		}
		else {
			this->field_0x2c = UpdateKineticToPotentialUpDown() & 0xff;
		}
	}
	else {
		bVibBComplete = true;
	}

	int bFinished = 0;

	if ((bVibAComplete) && (bVibBComplete)) {
		bFinished = 1;
	}

	return bFinished;
}

bool CVibrationDyn::MakeVibrate(edF32VECTOR3* param_2)
{
	int bFinished;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 local_d0;
	edF32MATRIX4 eStack192;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	bFinished = ComputeCurFactor();

	if (bFinished == 0) {
		edF32Matrix4SetIdentityHard(&eStack64);
		local_d0.y = 0.0f;
		local_d0.x = 1.0f;
		local_e0.z = 1.0f;
		local_d0.z = 0.0f;
		local_d0.w = 0.0f;
		local_e0.x = 0.0f;
		local_e0.y = 0.0f;
		local_e0.w = 0.0f;

		edF32Matrix4BuildFromVectorAndAngle(this->field_0x48 * this->field_0x4, &eStack128, &local_d0);
		edF32Matrix4BuildFromVectorAndAngle(this->field_0x48 * this->field_0x0, &eStack192, &local_e0);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &this->rotMatrix, &eStack64);
		edF32Matrix4MulF32Matrix4Hard(&eStack128, &eStack192, &eStack128);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack128, &eStack64);
		edF32Matrix4ToEulerSoft(&eStack64, param_2, "XYZ");
	}
	else {
		this->field_0x0 = 0.0f;
		this->field_0x4 = 0.0f;
		this->field_0x18 = 0.0f;
		this->field_0x1c = 0.0f;
		this->field_0x10 = 0.0f;
		this->field_0x14 = 0.0f;
		this->field_0x20 = 0.0f;
		this->field_0x24 = 0.0f;
		this->field_0x38 = 1.0f;
		this->field_0x28 = 1;
		this->field_0x2c = 1;

		edF32Matrix4ToEulerSoft(&this->rotMatrix, param_2, "XYZ");
	}

	return bFinished != 0;
}


void CScalarDyn::BuildFromDistTime(float dist, float time)
{
	float fVar1;

	this->field_0xc = 0.0f;
	this->field_0x10 = 0.0f;
	fVar1 = dist / (time * time);
	this->field_0x14 = fVar1;
	this->field_0x24 = fVar1;
	this->flags = 0;
	this->field_0x4 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x20 = 0.0f;
	this->field_0x1c = 0.0f;
	this->duration = time;

	return;
}

void CScalarDyn::BuildFromSpeedDist(float param_1, float param_2, float distance)
{
	if (distance == 0.0f) {
		this->field_0xc = param_1;
		this->field_0x14 = 0.0f;
		this->field_0x10 = 0.0f;
		this->duration = 0.0f;
		this->field_0x4 = 0.0f;
		this->field_0x18 = 0.0f;
		this->flags = 1;
		this->field_0x20 = param_2;
		this->field_0x1c = 0.0f;
		this->field_0x24 = 0.0f;
	}
	else {
		this->field_0xc = param_1;
		this->field_0x10 = 0.0f;
		this->field_0x14 = (param_2 * param_2 - param_1 * param_1) / (distance * 2.0f);
		this->field_0x20 = param_1;
		this->field_0x24 = this->field_0x14;
		this->flags = 0;
		this->field_0x4 = 0.0f;
		this->field_0x18 = 0.0f;
		this->field_0x1c = 0.0f;
		this->duration = (param_2 - param_1) / this->field_0x14;
	}

	return;
}

void CScalarDyn::BuildFromSpeedDistTime(float param_1, float param_2, float distance, float time)
{
	float fVar1;
	float fVar2;

	fVar2 = 1.0f / time;
	fVar1 = (fVar2 / time) * 6.0f * ((param_1 + param_2) - distance * 2.0f * fVar2);
	this->field_0xc = param_1;
	this->field_0x10 = fVar1;
	this->field_0x14 = (distance * 2.0f * (fVar2 / time) - param_1 * 2.0f * fVar2) - (time / 3.0f) * fVar1;
	this->field_0x20 = param_1;
	this->field_0x24 = this->field_0x14;
	this->flags = 0;
	this->field_0x4 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x1c = 0.0f;
	this->duration = time;

	return;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
void CScalarDyn::Reset()
{
	this->field_0x24 = 0.0f;
	this->field_0x20 = 0.0f;
	this->field_0x1c = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x14 = 0.0f;
	this->field_0x10 = 0.0f;
	this->field_0xc = 0.0f;
	this->duration = 0.0f;
	this->field_0x4 = 0.0f;
	this->flags = 1;
	return;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
bool CScalarDyn::IsFinished()
{
	return (this->flags & 1) != 0;
}

void CScalarDyn::Integrate(float param_1, float param_2)
{
	float fVar1;
	float fVar2;

	if ((param_2 != 0.0f) && (param_1 != 0.0f)) {
		if (this->flags == 0) {
			fVar1 = this->field_0x4 + param_1;
			if ((this->duration <= fVar1) || (fabs(fVar1 - this->duration) < 1e-06f)) {
				fVar1 = this->duration;
				this->flags = 2;
			}
			else {
				this->flags = 0;
			}

			fVar2 = this->field_0x4;
			this->field_0x1c = (this->field_0x10 * (fVar1 * fVar1 * fVar1 - fVar2 * fVar2 * fVar2)) / 6.0f;
			this->field_0x1c = this->field_0x1c + (this->field_0x14 * (fVar1 * fVar1 - fVar2 * fVar2)) / 2.0f;
			this->field_0x1c = this->field_0x1c + this->field_0xc * (fVar1 - this->field_0x4);
		}
		else {
			this->flags = 1;
		}

		this->field_0x18 = this->field_0x18 + this->field_0x1c;
		this->field_0x4 = this->field_0x4 + param_1;
		fVar1 = this->field_0x1c / param_2;
		this->field_0x20 = fVar1;
		this->field_0x24 = fVar1 / param_2;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
void CScalarDyn::Integrate(float param_1)
{
	Integrate(param_1, param_1);
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
float CScalarDyn::GetInstantSpeed()
{
	float fVar1;

	fVar1 = this->field_0x4;
	return this->field_0xc + (fVar1 * this->field_0x10 * fVar1) / 2.0f + this->field_0x14 * fVar1;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
bool CScalarDyn::OnLastValidSample()
{
	return (this->flags & 2) != 0;
}

void CScalarDyn::BuildFromSpeedTime(float param_1, float param_2, float param_3)
{
	this->field_0xc = param_1;
	this->field_0x10 = 0.0f;
	this->field_0x14 = (param_2 - param_1) / param_3;
	this->field_0x20 = param_1;
	this->field_0x24 = this->field_0x14;
	this->flags = 0;
	this->field_0x4 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x1c = 0.0f;
	this->duration = param_3;

	return;
}

void CScalarDyn::BuildFromDistTimeNoAccel(float dist, float time)
{
	this->field_0x14 = 0.0f;
	this->field_0x10 = 0.0f;
	this->field_0xc = dist / time;
	this->flags = 0;
	this->field_0x4 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x24 = 0.0f;
	this->field_0x20 = 0.0f;
	this->field_0x1c = 0.0f;
	this->duration = time;

	return;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
void CScalarDyn::Stop()
{
	this->field_0xc = 0.0f;
	this->field_0x10 = 0.0f;
	this->field_0x14 = 0.0f;
	this->field_0x24 = 0.0f;
	this->field_0x20 = 0.0f;
	this->field_0x1c = 0.0f;
	this->flags = 1;

	return;
}

void CVectorDyn::Integrate(float param_1, float param_2)
{
	float fVar1;
	float fVar2;

	if ((param_2 != 0.0f) && (param_1 != 0.0f)) {
		if (this->field_0x0 == 0) {
			fVar1 = this->field_0x8 + param_1;

			if (this->field_0x4 != 0) {
				if ((this->field_0xc <= fVar1) || (fabs(fVar1 - this->field_0xc) < 1e-06f)) {
					fVar1 = this->field_0xc;
					this->field_0x0 = 2;
				}
				else {
					this->field_0x0 = 0;
				}
			}

			fVar2 = fVar1 * fVar1 - this->field_0x8 * this->field_0x8;
			(this->field_0x50).x = ((this->field_0x20).x * fVar2) / 2.0f;
			(this->field_0x50).x = (this->field_0x50).x + (this->field_0x10).x * (fVar1 - this->field_0x8);
			(this->field_0x50).y = ((this->field_0x20).y * fVar2) / 2.0f;
			(this->field_0x50).y = (this->field_0x50).y + (this->field_0x10).y * (fVar1 - this->field_0x8);
			(this->field_0x50).z = ((this->field_0x20).z * fVar2) / 2.0f;
			(this->field_0x50).z = (this->field_0x50).z + (this->field_0x10).z * (fVar1 - this->field_0x8);
			(this->field_0x50).w = 0.0f;
		}
		else {
			this->field_0x0 = 1;
		}

		edF32Vector4AddHard(&this->field_0x30, &this->field_0x30, &this->field_0x50);
		this->field_0x8 = this->field_0x8 + param_1;
		edF32Vector4ScaleHard(1.0f / param_2, &this->field_0x60, &this->field_0x50);
		edF32Vector4ScaleHard(1.0f / param_2, &this->field_0x70, &this->field_0x60);
	}

	assert(std::isnan(this->field_0x50.x) == false);
	assert(std::isnan(this->field_0x50.y) == false);
	assert(std::isnan(this->field_0x50.z) == false);

	assert(std::isnan(this->field_0x60.x) == false);
	assert(std::isnan(this->field_0x60.y) == false);
	assert(std::isnan(this->field_0x60.z) == false);

	return;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
void CVectorDyn::Integrate(float param_1)
{
	Integrate(param_1, param_1);
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
void CVectorDyn::Reset()
{
	this->field_0x0 = 1;
	this->field_0xc = 0.0f;
	this->field_0x8 = 0.0f;

	(this->field_0x10).x = 0.0f;
	(this->field_0x10).y = 0.0f;
	(this->field_0x10).z = 0.0f;
	(this->field_0x10).w = 0.0f;

	(this->field_0x20).x = 0.0f;
	(this->field_0x20).y = 0.0f;
	(this->field_0x20).z = 0.0f;
	(this->field_0x20).w = 0.0f;

	(this->field_0x50).x = 0.0f;
	(this->field_0x50).y = 0.0f;
	(this->field_0x50).z = 0.0f;
	(this->field_0x50).w = 0.0f;

	(this->field_0x60).x = 0.0f;
	(this->field_0x60).y = 0.0f;
	(this->field_0x60).z = 0.0f;
	(this->field_0x60).w = 0.0f;

	(this->field_0x70).x = 0.0f;
	(this->field_0x70).y = 0.0f;
	(this->field_0x70).z = 0.0f;
	(this->field_0x70).w = 0.0f;

	(this->field_0x30).x = 0.0f;
	(this->field_0x30).y = 0.0f;
	(this->field_0x30).z = 0.0f;
	(this->field_0x30).w = 0.0f;
	return;
}

// Should be in: D:/Projects/b-witch/ActorAutonomousServices.cpp
bool CVectorDyn::IsFinished()
{
	return (this->field_0x0 & 1) != 0;
}

void CVectorDyn::BuildFromAccelDistAmplitude(float param_1, edF32VECTOR4* pGravity, edF32VECTOR4* param_4, byte param_5)
{
	float fVar1;
	float angle;
	float fVar2;
	float puVar3;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	fVar1 = edF32Vector4SafeNormalize0Hard(&local_60, pGravity);

	if (1.0f < local_60.y) {
		puVar3 = 1.0f;
	}
	else {
		puVar3 = -1.0f;
		if (-1.0f <= local_60.y) {
			puVar3 = local_60.y;
		}
	}

	angle = acosf(puVar3);

	local_50.y = 0.0f;
	local_50.w = 0.0f;
	local_50.z = -local_60.x;
	local_50.x = local_60.z;

	edF32Vector4SafeNormalize0Hard(&local_50, &local_50);
	edF32Matrix4FromAngAxisSoft(angle, &eStack64, &local_50);
	edF32Matrix4MulF32Vector4Hard(&local_70, &eStack64, param_4);

	if (local_70.y < 0.0f) {
		param_1 = param_1 - local_70.y;
	}

	fVar2 = -sqrtf(param_1 * fVar1 * 2.0f);
	(this->field_0x10).y = fVar2;
	this->field_0x40 = -fVar2 / fVar1;
	float oaeuoaeuoeau = ((local_70.y + param_1) * 2.0f) / fVar1;
	float aoeuoaeu = sqrtf(oaeuoaeuoeau);
	this->field_0xc = this->field_0x40 + sqrtf(((local_70.y + param_1) * 2.0f) / fVar1);

	assert(std::isnan(this->field_0xc) == false);

	(this->field_0x10).x = local_70.x / this->field_0xc;
	(this->field_0x10).z = local_70.z / this->field_0xc;
	(this->field_0x10).w = 0.0f;
	(this->field_0x20).x = pGravity->x;
	(this->field_0x20).y = pGravity->y;
	(this->field_0x20).z = pGravity->z;
	(this->field_0x20).w = pGravity->w;

	edF32Matrix4FromAngAxisSoft(-angle, &eStack64, &local_50);
	edF32Matrix4MulF32Vector4Hard(&this->field_0x10, &eStack64, &this->field_0x10);

	this->field_0x4 = param_5;
	this->field_0x0 = 0;
	this->field_0x8 = 0.0f;

	(this->field_0x50).x = 0.0f;
	(this->field_0x50).y = 0.0f;
	(this->field_0x50).z = 0.0f;
	(this->field_0x50).w = 0.0f;

	(this->field_0x60).x = 0.0f;
	(this->field_0x60).y = 0.0f;
	(this->field_0x60).z = 0.0f;
	(this->field_0x60).w = 0.0f;

	(this->field_0x70).x = 0.0f;
	(this->field_0x70).y = 0.0f;
	(this->field_0x70).z = 0.0f;
	(this->field_0x70).w = 0.0f;

	(this->field_0x30).x = 0.0f;
	(this->field_0x30).y = 0.0f;
	(this->field_0x30).z = 0.0f;
	(this->field_0x30).w = 0.0f;

	return;
}


CVertexDyn::CVertexDyn()
	: CDynBase()
{
	this->field_0x0 = 0.01f;

	return;
}

void CVertexDyn::Init(float param_1, edF32VECTOR4* pPosition)
{
	this->field_0x20 = *pPosition;
	this->field_0x30 = this->field_0x20;
	this->field_0x10 = param_1;
	this->field_0x1c = 0.0f;

	return;
}

void CVertexDyn::MoveTo(edF32VECTOR4* pPosition)
{
	float fVar2;
	float fVar4;
	edF32VECTOR4 eStack16;

	this->field_0x1c = 0;

	if (this->field_0xc != 0.0f) {
		edF32Vector4SubHard(&eStack16, pPosition, &this->field_0x30);
		fVar2 = edF32Vector4GetDistHard(&eStack16);
		if (fVar2 < this->field_0xc) {
			return;
		}
	}

	edF32Vector4SubHard(&eStack16, pPosition, &this->field_0x20);

	fVar2 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
	if (fVar2 <= this->field_0x0) {
		this->field_0x10 = 0.0f;
		this->field_0x20 = *pPosition;
		this->field_0x30 = this->field_0x20;
	}
	else {
		this->field_0x1c = 1;

		fVar4 = this->field_0x10;
		if (fVar2 <= ((0.0f - fVar4) / -this->field_0x18) * (fVar4 + 0.0f) * 0.5f) {
			if (fVar4 < 0.0f) {
				fVar2 = this->field_0x10 + this->field_0x18 * GetTimer()->lastFrameTime;
				this->field_0x10 = fVar2;
				if (0.0f < fVar2) {
					this->field_0x10 = 0.0f;
				}
			}
			else {
				fVar2 = this->field_0x10 - this->field_0x18 * GetTimer()->lastFrameTime;
				this->field_0x10 = fVar2;
				if (fVar2 < 0.0f) {
					this->field_0x10 = 0.0f;
				}
			}
		}
		else {
			if (fVar4 < this->field_0x14) {
				this->field_0x10 = this->field_0x10 + this->field_0x18 * GetTimer()->lastFrameTime;
				fVar2 = this->field_0x14;
				if (fVar2 < this->field_0x10) {
					this->field_0x10 = fVar2;
				}
			}
			else {
				this->field_0x10 = this->field_0x10 - this->field_0x18 * GetTimer()->lastFrameTime;
				fVar2 = this->field_0x14;
				if (this->field_0x10 < fVar2) {
					this->field_0x10 = fVar2;
				}
			}
		}

		edF32Vector4ScaleHard(this->field_0x10 * GetTimer()->lastFrameTime, &eStack16, &eStack16);
		edF32Vector4AddHard(&this->field_0x20, &this->field_0x20, &eStack16);
		(this->field_0x20).w = 1.0f;
	}

	return;
}

void CVertexDyn::FUN_00213b90(edF32MATRIX4* param_2)
{
	edF32Matrix4MulF32Vector4Hard(&this->field_0x20, param_2, &this->field_0x20);
	edF32Matrix4MulF32Vector4Hard(&this->field_0x30, param_2, &this->field_0x30);
	return;
}

// Should be in: D:/Projects/b-witch/CameraFightData.cpp
void CValueDyn::MoveTo(float param_1)
{
	float fVar2;
	float lastFrameTime;
	float fVar4;
	float fVar5;

	lastFrameTime = GetTimer()->lastFrameTime;
	this->field_0x1c = 0;

	if ((this->field_0xc == 0.0f) || (this->field_0xc <= fabs(param_1 - this->field_0x8))) {
		fVar2 = param_1 - this->field_0x4;
		if (fabs(fVar2) <= this->field_0x0) {
			this->field_0x4 = param_1;
			this->field_0x8 = param_1;
			this->field_0x10 = 0.0f;
		}
		else {
			this->field_0x1c = 1;
			fVar4 = this->field_0x18;
			fVar5 = this->field_0x10;

			if (fabs(fVar2) < ((0.0f - fVar5) / (fVar4 * -2.0f)) * (fVar5 + 0.0f) * 0.5f) {
				if (fVar5 < 0.0f) {
					fVar5 = fVar5 + lastFrameTime * 2.0f * fVar4;
					this->field_0x10 = fVar5;

					if (0.0f < fVar5) {
						this->field_0x10 = 0.0f;
					}
				}
				else {
					fVar5 = fVar5 - lastFrameTime * 2.0f * fVar4;
					this->field_0x10 = fVar5;

					if (fVar5 < 0.0f) {
						this->field_0x10 = 0.0f;
					}
				}
			}
			else {
				if (0.0f <= fVar2) {
					fVar5 = this->field_0x14;
				}
				else {
					fVar5 = -this->field_0x14;
				}

				fVar4 = this->field_0x10;
				if (fVar4 < fVar5) {
					this->field_0x10 = fVar4 + lastFrameTime * 2.0f * this->field_0x18;
					if (0.0f <= fVar2) {
						fVar5 = this->field_0x14;
					}
					else {
						fVar5 = -this->field_0x14;
					}

					if (fVar5 < this->field_0x10) {
						if (0.0f <= fVar2) {
							fVar2 = this->field_0x14;
						}
						else {
							fVar2 = -this->field_0x14;
						}
						this->field_0x10 = fVar2;
					}
				}
				else {
					this->field_0x10 = fVar4 - lastFrameTime * 2.0f * this->field_0x18;
					if (0.0f <= fVar2) {
						fVar5 = this->field_0x14;
					}
					else {
						fVar5 = -this->field_0x14;
					}

					if (this->field_0x10 < fVar5) {
						if (0.0f <= fVar2) {
							fVar2 = this->field_0x14;
						}
						else {
							fVar2 = -this->field_0x14;
						}

						this->field_0x10 = fVar2;
					}
				}
			}

			this->field_0x4 = this->field_0x4 + lastFrameTime * this->field_0x10;
		}
	}

	return;
}

CDynBase::CDynBase()
{
	this->field_0x0 = 0.01f;
	this->field_0xc = 0.0f;
	this->field_0x14 = 0.0f;
	this->field_0x10 = 0.0f;
	this->field_0x18 = 0.0f;
	this->field_0x8 = 0.0f;
	this->field_0x4 = 0.0f;

	return;
}

void CDynBase::Init(float param_1, float param_2)
{
	this->field_0x4 = param_1;
	this->field_0x8 = param_1;
	this->field_0x10 = param_2;
	this->field_0x1c = 0;

	return;
}

void SPEED_DYN::Init(float param_1, float param_2)
{
	this->currentAlpha = param_1;
	this->field_0x4 = param_2;
}

// Should be in: D:/Projects/b-witch/MiscFunctions.cpp
float SPEED_DYN::UpdateLerp(float target)
{
	Timer* pTimeController;
	float adjustedDelta;
	float delta;

	adjustedDelta = this->field_0x4;
	delta = target - this->currentAlpha;
	pTimeController = GetTimer();
	adjustedDelta = adjustedDelta * pTimeController->cutsceneDeltaTime;

	if (fabs(delta) < this->field_0x4) {
		pTimeController = GetTimer();
		adjustedDelta = fabs(delta * pTimeController->cutsceneDeltaTime);
	}

	if (adjustedDelta < fabs(delta)) {
		if (delta < 0.0f) {
			adjustedDelta = -adjustedDelta;
		}
		this->currentAlpha = this->currentAlpha + adjustedDelta;
	}
	else {
		this->currentAlpha = target;
	}

	return this->currentAlpha;
}

// Should be in: D:/Projects/b-witch/MiscFunctions.cpp
void SPEED_DYN::Update(float param_1)
{
	float fVar2;
	float fVar3;

	fVar3 = this->field_0x4;
	fVar2 = param_1 - this->currentAlpha;
	fVar3 = fVar3 * GetTimer()->cutsceneDeltaTime;

	if (fVar3 < fabs(fVar2)) {
		if (fVar2 < 0.0f) {
			fVar3 = -fVar3;
		}
		this->currentAlpha = this->currentAlpha + fVar3;
	}
	else {
		this->currentAlpha = param_1;
	}

	return;
}


CAngleDyn::CAngleDyn()
	: CDynBase()
{
	this->field_0x0 = 0.01f;

	return;
}

void CAngleDyn::MoveTo(float angle, int mode)
{
	float fVar2;
	float fVar3;
	float unaff_f20;

	fVar2 = edF32Between_2Pi(this->field_0x4);
	this->field_0x4 = fVar2;
	fVar2 = edF32Between_2Pi(angle);
	this->field_0x1c = 0;

	if ((this->field_0xc == 0.0f) || (unaff_f20 = edF32GetAnglesDelta(this->field_0x8, fVar2), this->field_0xc <= fabs(unaff_f20))) {
		if (mode == 1) {
			fVar3 = edF32Between_0_2Pi(fVar2 - this->field_0x4);
			unaff_f20 = fVar3 - 6.283185f;
		}
		else {
			if (mode == 2) {
				unaff_f20 = edF32Between_0_2Pi(fVar2 - this->field_0x4);
			}
			else {
				if (mode == 0) {
					unaff_f20 = edF32GetAnglesDelta(this->field_0x4, fVar2);
				}
			}
		}

		if (fabs(unaff_f20) <= this->field_0x0) {
			this->field_0x4 = fVar2;
			this->field_0x8 = fVar2;
			this->field_0x10 = 0.0f;
		}
		else {
			this->field_0x1c = 1;
			fVar2 = this->field_0x10;

			if (fabs(unaff_f20) <= ((0.0f - fVar2) / -this->field_0x18) * (fVar2 + 0.0f) * 0.5f) {
				if (fVar2 < 0.0f) {
					fVar2 = this->field_0x10 + this->field_0x18 * GetTimer()->lastFrameTime * 2.0f;
					this->field_0x10 = fVar2;

					if (0.0f < fVar2) {
						this->field_0x10 = 0.0f;
					}
				}
				else {
					fVar2 = this->field_0x10 - this->field_0x18 * GetTimer()->lastFrameTime * 2.0f;
					this->field_0x10 = fVar2;

					if (fVar2 < 0.0f) {
						this->field_0x10 = 0.0f;
					}
				}
			}
			else {
				if (0.0f <= unaff_f20) {
					fVar2 = this->field_0x14;
				}
				else {
					fVar2 = -this->field_0x14;
				}

				if (this->field_0x10 < fVar2) {
					this->field_0x10 = this->field_0x10 + this->field_0x18 * GetTimer()->lastFrameTime * 2.0f;

					if (0.0f <= unaff_f20) {
						fVar2 = this->field_0x14;
					}
					else {
						fVar2 = -this->field_0x14;
					}

					if (fVar2 < this->field_0x10) {
						if (0.0f <= unaff_f20) {
							fVar2 = this->field_0x14;
						}
						else {
							fVar2 = -this->field_0x14;
						}

						this->field_0x10 = fVar2;
					}
				}
				else {
					this->field_0x10 = this->field_0x10 - this->field_0x18 * GetTimer()->lastFrameTime * 2.0f;

					if (0.0f <= unaff_f20) {
						fVar2 = this->field_0x14;
					}
					else {
						fVar2 = -this->field_0x14;
					}

					if (this->field_0x10 < fVar2) {
						if (0.0f <= unaff_f20) {
							fVar2 = this->field_0x14;
						}
						else {
							fVar2 = -this->field_0x14;
						}

						this->field_0x10 = fVar2;
					}
				}
			}

			this->field_0x4 = this->field_0x4 + this->field_0x10 * GetTimer()->lastFrameTime;
		}
	}

	return;
}

