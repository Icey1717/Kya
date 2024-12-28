#include "ActorMovable.h"

#include <math.h>

#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "CollisionManager.h"
#include "ActorCheckpointManager.h"
#include "CollisionRay.h"
#include "ActorAutonomous.h"

int INT_00448e04 = 0;

float CDynamic::gMaxSpeed_Horiz = 32.0f;
float CDynamic::gMaxSpeed_Vert = 20.0f;

void S_TILT_STREAM_DEF::Init()
{
	this->field_0x4 = this->field_0x4 * 0.01745329f;
	this->oscConfig.field_0x0 = this->oscConfig.field_0x0 * 100.0f;
	this->oscConfig.field_0x4 = this->oscConfig.field_0x4 * 100.0f;
}


void S_PUSH_STREAM_DEF::Init()
{
	this->oscConfig.field_0x0 = this->oscConfig.field_0x0 * 100.0f;
	this->oscConfig.field_0x4 = this->oscConfig.field_0x4 * 100.0f;
}


void S_TILT_DATA::Init(float param_1, CActor* pActor, S_TILT_STREAM_DEF* pStreamDef)
{
	edObbTREE_DYN* peVar1;
	float fVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	CWayPoint* pMVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	this->oscQuat.field_0x0 = gF32Vertex4Zero;
	this->oscQuat.field_0x10 = gF32Vertex4Zero;

	this->oscValue.field_0x0 = 0.0f;
	this->oscValue.field_0x4 = 0.0f;
	this->field_0x38 = 0.0f;
	(this->field_0x0).x = 0.0f;
	(this->field_0x0).y = 0.0f;
	(this->field_0x0).z = 0.0f;
	(this->field_0x0).w = 0.0f;

	if (pStreamDef->field_0x4 != 0.0f) {

		if (pStreamDef->field_0x0 == -1) {
			pMVar6 = (CWayPoint*)0x0;
		}
		else {
			IMPLEMENTATION_GUARD(
				pMVar6 = (CScene::ptable.g_CWayPointManager_0045169c)->field_0x4 + pStreamDef->field_0x0;)
		}

		if (pMVar6 != (CWayPoint*)0x0) {
			IMPLEMENTATION_GUARD(
				edF32Matrix4FromEulerSoft(&eStack64, &pActor->pCinData->rotationEuler, "XYZ");
			eStack64.rowT = pActor->baseLocation;
			edF32Matrix4GetInverseOrthoHard(&eStack64, &eStack64);
			local_50.xyz = pMVar6->field_0x0;
			local_50.w = 1.0f;
			edF32Matrix4MulF32Vector4Hard(&this->field_0x0, &eStack64, &local_50);
			this->field_0x0.w = 0.0f;
			edF32Vector4SafeNormalize1Hard(&this->field_0x0, &this->field_0x0);)
		}

		if (pActor->pCollisionData == (CCollision*)0x0) {
			fVar8 = 1.0f;
		}
		else {
			peVar1 = pActor->pCollisionData->pObbTree;
			fVar8 = 0.0f;
			local_60.w = 1.0f;
			local_60.x = (peVar1->field_0xb0).x;

			for (iVar5 = 0; iVar5 < 2; iVar5 = iVar5 + 1) {
				local_60.y = (peVar1->field_0xb0).y;

				for (iVar4 = 0; iVar4 < 2; iVar4 = iVar4 + 1) {
					local_60.z = (peVar1->field_0xb0).z;

					for (iVar3 = 0; iVar3 < 2; iVar3 = iVar3 + 1) {
						edF32Matrix4MulF32Vector4Hard(&local_70, &peVar1->matrix_0x70, &local_60);
						if (pMVar6 == (CWayPoint*)0x0) {
							local_70.w = 0.0f;
							fVar7 = edF32Vector4GetDistHard(&local_70);
						}
						else {
							local_80.x = (local_70.y - gF32Vertex4Zero.y) * this->field_0x0.z - this->field_0x0.y * (local_70.z - gF32Vertex4Zero.z);
							local_80.y = (local_70.z - gF32Vertex4Zero.z) * this->field_0x0.x - this->field_0x0.z * (local_70.x - gF32Vertex4Zero.x);
							local_80.z = (local_70.x - gF32Vertex4Zero.x) * this->field_0x0.y - this->field_0x0.x * (local_70.y - gF32Vertex4Zero.y);
							fVar7 = edF32Vector4GetDistHard(&local_80);
						}

						if (fVar7 <= fVar8) {
							fVar7 = fVar8;
						}

						local_60.z = -(peVar1->field_0xb0).z;
						fVar8 = fVar7;
					}

					local_60.y = -(peVar1->field_0xb0).y;
				}

				local_60.x = -(peVar1->field_0xb0).x;
			}
		}

		this->field_0x38 = pStreamDef->field_0x4 / (fVar8 * param_1);
	}
	return;
}

void S_TILT_DATA::Reset()
{
	this->oscQuat.field_0x0 = gF32Vertex4Zero;
	this->oscQuat.field_0x10 = gF32Vertex4Zero;

	(this->oscValue).field_0x0 = 0.0f;
	(this->oscValue).field_0x4 = 0.0f;
	return;
}

bool S_OSCILLATING_VALUE::Update(float param_1, float deltaTime, S_OSCILLATION_CONFIG* pConfig)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar4 = param_1 - this->field_0x0;
	if (fabs(fVar4) < 1e-06f) {
		fVar4 = 0.0f;
	}

	if ((0.1f < fabs(this->field_0x4)) || (0.001f < fabs(fVar4))) {
		fVar2 = this->field_0x4;
		fVar3 = deltaTime * fVar2 * -pConfig->field_0x4 * fVar2;

		if (fVar2 < 0.0f) {
			fVar3 = -fVar3;
		}

		bVar1 = false;
		fVar4 = this->field_0x4 + deltaTime * (pConfig->field_0x0 * fVar4 + fVar3);
		this->field_0x4 = fVar4;
		this->field_0x0 = this->field_0x0 + fVar4 * deltaTime;
	}
	else {
		this->field_0x4 = 0.0f;
		bVar1 = true;
		this->field_0x0 = param_1;
	}

	return bVar1;
}

void S_PUSH_DATA::Init()
{
	(this->oscValue).field_0x0 = 0.0f;
	(this->oscValue).field_0x4 = 0.0f;

	return;
}

void S_PUSH_DATA::Reset()
{
	(this->oscValue).field_0x0 = 0.0f;
	(this->oscValue).field_0x4 = 0.0f;

	return;
}

CActorMovable::CActorMovable()
{
	this->dynamic.weightB = 0.0f;

	return;
}

bool CActorMovable::IsKindOfObject(ulong kind)
{
	return (kind & 3) != 0;
}

void CActorMovable::Create(ByteCode* pByteCode)
{
	float fVar1;

	INT_00448e04 = INT_00448e04 + 1;
	CActor::Create(pByteCode);
	this->dynamic.Reset(this);
	fVar1 = pByteCode->GetF32();
	(this->dynamic).weightB = fVar1;
	this->field_0x1c0.x = 1e+30f;
	return;
}


void CActorMovable::Reset()
{
	CActor::Reset();
	this->dynamic.Reset(this);
	(this->field_0x1c0).x = 1e+30f;

	return;
}

void CActorMovable::CheckpointReset()
{
	CActor::CheckpointReset();

	if ((this->field_0x1c0).x != 1e+30f) {
		CActor::UpdatePosition(&this->field_0x1c0, true);
	}

	return;
}

void CActorMovable::SetState(int newState, int animType)
{
	if (newState != this->actorState) {
		this->dynamic.weightA = 1.0f;
	}

	CActor::SetState(newState, animType);
	return;
}

void CActorMovable::ChangeManageState(int state)
{
	CActor::ChangeManageState(state);

	if (state != 0) {
		this->dynamic.ClearLocalData();
	}

	return;
}

bool CActorMovable::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	CCollision* pCol;
	uint uVar1;
	Timer* pTVar2;
	float fVar3;
	edF32VECTOR3 local_90;
	edF32MATRIX4 eStack128;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if ((this->flags & 0x1000) == 0) {
		edF32Matrix4MulF32Vector4Hard(&local_40, m0, &this->rotationQuat);
		local_40.y = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_40, &local_40);
		this->rotationQuat = local_40;
		GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
	}
	else {
		edF32Matrix4FromEulerSoft(&eStack128, &this->rotationEuler.xyz, "XYZ");
		edF32Matrix4MulF32Matrix4Hard(&eStack128, &eStack128, m0);
		edF32Matrix4ToEulerSoft(&eStack128, &local_90, "XYZ");
		this->rotationEuler.xyz = local_90;
	}

	edF32Matrix4MulF32Vector4Hard(&eStack48, m0, &this->currentLocation);
	edF32Vector4SubHard(&local_10, &eStack48, &this->currentLocation);
	edF32Matrix4MulF32Vector4Hard(&this->dynamic.rotationQuat, m0, &this->dynamic.rotationQuat);

	pCol = this->pCollisionData;
	if (pCol == (CCollision*)0x0) {
		UpdatePosition(&eStack48, true);
	}
	else {
		uVar1 = pCol->flags_0x0;
		pCol->flags_0x0 = pCol->flags_0x0 | 0x400000;
		pCol->flags_0x0 = pCol->flags_0x0 & 0xfffeffff;
		pCol->CheckCollisions_MoveActor(this, &eStack48, (CActorsTable*)0x0, pActor, 0);
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffbfffff;

		if ((uVar1 & 0x10000) != 0) {
			pCol->flags_0x0 = pCol->flags_0x0 | 0x10000;
		}
	}

	if (this->pTiedActor == pActor) {
		if ((pActor->pCollisionData == (CCollision*)0x0) || (((pActor->pCollisionData->flags_0x0 & 0x100000) != 0 && ((this->flags & 0x20000) == 0)))) {
			pTVar2 = GetTimer();

			if (pTVar2->cutsceneDeltaTime == 0.0f) {
				local_10.x = 0.0f;
				local_10.y = 0.0f;
				local_10.z = 0.0f;
				local_10.w = 0.0f;
			}
			else {
				fVar3 = 1.0f / pTVar2->cutsceneDeltaTime;
				local_10 = local_10 * fVar3;
			}

			fVar3 = edF32Vector4SafeNormalize0Hard(&eStack32, &local_10);
			if (30.0f < fVar3) {
				edF32Vector4ScaleHard(30.0f, &local_10, &eStack32);
			}

			if (pTVar2->cutsceneDeltaTime == 0.0) {
				local_10.x = 0.0f;
				local_10.y = 0.0f;
				local_10.z = 0.0f;
				local_10.w = 0.0f;
			}
			else {
				fVar3 = 1.0f / pTVar2->cutsceneDeltaTime;
				local_10 = local_10 * fVar3;
			}

			(this->dynamic).field_0x10 = local_10;
		}
		else {
			(this->dynamic).field_0x10.x = 0.0f;
			(this->dynamic).field_0x10.y = 0.0f;
			(this->dynamic).field_0x10.z = 0.0f;
			(this->dynamic).field_0x10.w = 0.0f;
		}
	}

	this->flags = this->flags & 0xffdfffff;
	return true;
}

int CActorMovable::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int uVar1;

	if (msg == 0x48) {
		uVar1 = 1;
		(this->dynamic).weightA = 1.2f;
	}
	else {
		if (msg == 0x47) {
			(this->dynamic).weightA = 1.0f;
			uVar1 = 1;
		}
		else {
			if (msg == 0x46) {
				(this->dynamic).weightA = 0.5f;
				uVar1 = 1;
			}
			else {
				if (msg == 0x45) {
					uVar1 = 1;
					(this->dynamic).weightA = 0.8f;
				}
				else {
					uVar1 = CActor::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}
	return uVar1;
}

void CActorMovable::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
{
	uint uVar1;
	Timer* pTimer;
	float fVar3;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 originalLocation;
	edF32VECTOR4 translation;
	CCollision* pCol;

	pCol = this->pCollisionData;
	pTimer = GetTimer();
	originalLocation = this->currentLocation;

	(this->dynamic).flags = 0;
	uVar1 = (this->dynamic).field_0x4c;
	edF32Vector4ScaleHard((this->dynamic).speed, &translation, &this->dynamic.rotationQuat);

	fVar3 = pTimer->cutsceneDeltaTime;
	translation = translation * fVar3;

	if ((pCol == (CCollision*)0x0) || (((flags | uVar1) & 0x400) != 0)) {
		edF32Vector4AddHard(&translation, &translation, &this->currentLocation);
		UpdatePosition(&translation, true);
	}
	else {
		pCol->CheckCollisions_TranslateActor(this, &translation, pActorsTable, (CActor*)0x0, 1);
	}

	pTimer = GetTimer();
	edF32Vector4SubHard(&eStack48, &this->currentLocation, &originalLocation);

	fVar3 = edF32Vector4SafeNormalize1Hard(&(this->dynamic).velocityDirectionEuler, &eStack48);
	(this->dynamic).linearAcceleration = fVar3;
	(this->dynamic).linearAcceleration = fVar3 / pTimer->cutsceneDeltaTime;
	eStack48.y = 0.0f;

	fVar3 = edF32Vector4SafeNormalize1Hard(&(this->dynamic).horizontalVelocityDirectionEuler, &eStack48);
	(this->dynamic).horizontalLinearAcceleration = fVar3;
	(this->dynamic).horizontalLinearAcceleration = fVar3 / pTimer->cutsceneDeltaTime;
	return;
}

void CActorMovable::SV_MOV_UpdateSpeedIntensity(float param_1, float param_2)

{
	Timer* pTVar1;
	float fVar2;
	float fVar3;

	pTVar1 = GetTimer();
	fVar3 = (this->dynamic).speed;
	fVar2 = param_2 * pTVar1->cutsceneDeltaTime;
	if (fVar3 < param_1) {
		fVar3 = fVar3 + fVar2;
		(this->dynamic).speed = fVar3;
		if (param_1 < fVar3) {
			(this->dynamic).speed = param_1;
		}
	}
	else {
		fVar3 = fVar3 - fVar2;
		(this->dynamic).speed = fVar3;
		if (fVar3 < param_1) {
			(this->dynamic).speed = param_1;
		}
	}
	return;
}

void CActorMovable::SV_MOV_DecreaseSpeedIntensity(float param_1)
{
	Timer* pTVar1;
	float fVar2;

	pTVar1 = GetTimer();
	fVar2 = (this->dynamic).speed - param_1 * pTVar1->cutsceneDeltaTime;
	(this->dynamic).speed = fVar2;
	if (fVar2 < 0.0f) {
		(this->dynamic).speed = 0.0f;
	}
	return;
}

float CActorMovable::SV_MOV_ComputeDistIdealPos(CPathFollowReader* pPathFollowReader, float param_3, float defaultDelay)
{
	float delay;

	delay = pPathFollowReader->GetDelay();
	if (delay != 0.0f) {
		defaultDelay = pPathFollowReader->GetDelay();
	}

	return defaultDelay;
}

void CActorMovable::SV_MOV_ComputeSpeedAccelerationToFleeActor(float param_1, CActorMovable* pFleeActor, CPathPlane* pInPathPlane, SV_MOV_FLEE_ON_PATH_PARAM* pParams)
{
	CPathFollow* pCVar1;
	bool bVar2;
	edF32VECTOR4* peVar3;
	float* pfVar4;
	int iVar5;
	edF32VECTOR4* peVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	edF32VECTOR4 auStack128;
	edF32VECTOR4 auStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	CPathPlaneOutData local_28;
	CPathPlaneOutData local_18;
	float local_8;
	float local_4;

	fVar7 = pParams->delay;
	fVar9 = fVar7 / (param_1 * 2.0f);
	fVar13 = fVar7 - fVar9;
	fVar9 = fVar7 + fVar9;
	fVar12 = fVar13 - fVar7 / param_1;
	fVar10 = fVar9 + fVar7 / param_1;
	fVar7 = pFleeActor->currentLocation.x - this->currentLocation.x;
	fVar8 = pFleeActor->currentLocation.y - this->currentLocation.y;
	fVar11 = pFleeActor->currentLocation.z - this->currentLocation.z;

	fVar11 = sqrtf(fVar7 * fVar7 + fVar8 * fVar8 + fVar11 * fVar11);
	edF32Vector4SubHard(&eStack64, &this->currentLocation, &pFleeActor->currentLocation);
	peVar3 = pInPathPlane->pathFollowReader.GetWayPoint();
	edF32Vector4SubHard(&eStack80, peVar3, &this->currentLocation);
	fVar8 = edF32Vector4GetDistHard(&eStack80);
	fVar7 = 0.0f;

	if (0.0f < fVar8) {
		edF32Vector4NormalizeHard(&eStack64, &eStack64);
		edF32Vector4NormalizeHard(&eStack80, &eStack80);
		fVar7 = edF32Vector4DotProductHard(&eStack64, &eStack80);
		fVar7 = fVar11 * fVar7;
	}

	fVar8 = fVar10;
	if (fVar11 < fVar10) {
		fVar8 = fVar11;
	}

	fVar11 = fVar10;
	if (fVar7 < fVar10) {
		fVar11 = fVar7;
	}

	fVar8 = (fVar8 + fVar11) / 2.0f;
	fVar7 = edF32Vector4DotProductHard(&eStack64, &(pFleeActor->dynamic).velocityDirectionEuler);
	if ((0.0f < fVar7) && (pParams->field_0x4 == 0)) {
		IMPLEMENTATION_GUARD(
		(**(code**)(((pInPathPlane->pathFollowReader).pPathFollow)->field_0x0 + 0xc))();
		(**(code**)(((pInPathPlane->pathFollowReader).pPathFollow)->field_0x0 + 0x10))();
		fVar7 = edF32Vector4DotProductHard(&eStack64, &eStack80);
		if (fVar7 <= 0.0) {
			pfVar4 = (float*)(**(code**)(((pInPathPlane->pathFollowReader).pPathFollow)->field_0x0 + 0xc))();
			fVar7 = *pfVar4 - this->currentLocation.x;
			fVar8 = pfVar4[1] - this->currentLocation.y;
			fVar11 = pfVar4[2] - this->currentLocation.z;
			if ((1.0 < SQRT(fVar7 * fVar7 + fVar8 * fVar8 + fVar11 * fVar11)) &&
				(pfVar4 = (float*)(**(code**)(((pInPathPlane->pathFollowReader).pPathFollow)->field_0x0 + 0x10))(),
					fVar7 = *pfVar4 - this->currentLocation.x, fVar8 = pfVar4[1] - this->currentLocation.y,
					fVar11 = pfVar4[2] - this->currentLocation.z,
					1.0 < SQRT(fVar7 * fVar7 + fVar8 * fVar8 + fVar11 * fVar11))) {
				(pInPathPlane->pathFollowReader).field_0xc = (pInPathPlane->pathFollowReader).field_0xc ^ 1;
				pPathFollowReader->NextWayPoint((CPathFollowReader*)pInPathPlane);
			}
			pParams->acceleration = 0.0;
			pParams->speed = 50.0;
			return;
		})
	}

	pInPathPlane->InitTargetPos(&this->currentLocation, &local_18);
	pInPathPlane->ExternComputeTargetPosWithPlane(&this->currentLocation, &local_18);
	FUN_00115380(&this->currentLocation, &pInPathPlane->pathFollowReader, local_18.field_0x0, 0, &auStack128, &local_8);
	pInPathPlane->InitTargetPos(&this->currentLocation, &local_28);
	pInPathPlane->ExternComputeTargetPosWithPlane(&this->currentLocation, &local_28);
	FUN_00115380(&pFleeActor->currentLocation, &pInPathPlane->pathFollowReader, local_28.field_0x0, 0, &auStack112, &local_4);

	bVar2 = false;
	if ((local_18.field_0x0 < local_28.field_0x0) || ((local_18.field_0x0 == local_28.field_0x0 && (local_8 <= local_4))))
	{
		bVar2 = true;
	}

	if (local_28.field_0x0 == -1) {
		peVar3 = ((pInPathPlane->pathFollowReader).pPathFollow)->aSplinePoints;
		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = peVar3 + 1;
		}

		peVar6 = ((pInPathPlane->pathFollowReader).pPathFollow)->aSplinePoints;
		if (peVar6 == (edF32VECTOR4*)0x0) {
			peVar6 = &gF32Vertex4Zero;
		}

		edF32Vector4SubHard(&eStack96, peVar3, peVar6);
	}
	else {
		pCVar1 = (pInPathPlane->pathFollowReader).pPathFollow;
		iVar5 = pCVar1->splinePointCount + -1;
		if (local_28.field_0x0 < iVar5) {
			if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar1->aSplinePoints + local_28.field_0x0 + 1;
			}

			peVar6 = ((pInPathPlane->pathFollowReader).pPathFollow)->aSplinePoints;
			if (peVar6 == (edF32VECTOR4*)0x0) {
				peVar6 = &gF32Vertex4Zero;
			}
			else {
				peVar6 = peVar6 + local_28.field_0x0;
			}

			edF32Vector4SubHard(&eStack96, peVar3, peVar6);
		}
		else {
			if (local_28.field_0x0 == iVar5) {
				if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
					peVar3 = &gF32Vertex4Zero;
				}
				else {
					peVar3 = pCVar1->aSplinePoints + local_28.field_0x0;
				}

				peVar6 = ((pInPathPlane->pathFollowReader).pPathFollow)->aSplinePoints;
				if (peVar6 == (edF32VECTOR4*)0x0) {
					peVar6 = &gF32Vertex4Zero;
				}
				else {
					peVar6 = peVar6 + local_28.field_0x0 + -1;
				}

				edF32Vector4SubHard(&eStack96, peVar3, peVar6);
			}
		}
	}

	edF32Vector4NormalizeHard(&eStack96, &eStack96);
	fVar11 = (pFleeActor->dynamic).linearAcceleration;
	fVar7 = edF32Vector4DotProductHard(&(pFleeActor->dynamic).velocityDirectionEuler, &eStack96);
	iVar5 = 0;
	if (bVar2) {
		iVar5 = -2;
	}
	else {
		if (((fVar8 < fVar13) && (0.0f < fVar8)) && (iVar5 = -2, fVar12 + 0.1f <= fVar8)) {
			iVar5 = -1;
		}
		if ((fVar9 < fVar8) && (iVar5 = 2, fVar8 <= fVar10 - 0.1f)) {
			iVar5 = 1;
		}
	}

	if (iVar5 == 0) {
		pParams->speed = 0.0f;
		pParams->acceleration = (this->dynamic).linearAcceleration;
	}
	else {
		if (iVar5 == 2) {
			pParams->acceleration = 0.0f;
			pParams->speed = 15.0f;
		}
		else {
			if (iVar5 == 1) {
				pParams->speed = (((this->dynamic).linearAcceleration - (pFleeActor->dynamic).linearAcceleration) *
						((this->dynamic).linearAcceleration - (pFleeActor->dynamic).linearAcceleration) * 0.4f) / (fVar10 - fVar8);
				pParams->acceleration = fVar7 * fVar11 - 6.0f / fabs(fVar10 - fVar8);
				if (pParams->speed < 3.0f) {
					pParams->speed = 3.0f;
				}
			}
			else {
				if (iVar5 == -1) {
					fVar8 = fVar8 - fVar12;
					fVar9 = (this->dynamic).linearAcceleration - (pFleeActor->dynamic).linearAcceleration;
					pParams->speed = (fVar9 * fVar9 * 0.6f) / fVar8;
					pParams->acceleration = fVar7 * fVar11 + 6.0f / fVar8;
					if (pParams->speed < 3.0f) {
						pParams->speed = 3.0f;
					}
				}
				else {
					if (iVar5 == -2) {
						pParams->speed = 50.0f;
						pParams->acceleration = (pFleeActor->dynamic).linearAcceleration + 15.0f;
					}
				}
			}
		}
	}

	if (pParams->acceleration < 0.0f) {
		pParams->acceleration = 0.0f;
	}

	return;
}

float CActorMovable::FUN_00115380(edF32VECTOR4* param_2, CPathFollowReader* pPathFollowReader, int param_4, long param_5, edF32VECTOR4* pOutPosition, float* param_7)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* v2;
	int index;
	float fVar2;
	float t;
	float fVar3;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	int local_4;

	if (param_4 == -1) {
		param_4 = 0;
	}

	local_4 = pPathFollowReader->GetNextPlace(param_4, 1);
	index = param_4;
	if (local_4 == -1) {
		index = param_4 + -1;
		local_4 = param_4;
	}

	peVar1 = pPathFollowReader->GetWayPoint(local_4);
	v2 = pPathFollowReader->GetWayPoint(index);
	edF32Vector4SubHard(&local_20, peVar1, v2);
	local_20.y = 0.0f;

	if (param_5 == 0) {
		fVar2 = edF32Vector4GetDistHard(&local_20);
	}
	else {
		fVar2 = edF32Vector4SafeNormalize1Hard((edF32VECTOR4*)param_5, &local_20);
	}

	local_40.x = -local_20.z;
	local_40.y = local_20.y;
	local_40.z = local_20.x;
	edF32Vector4NormalizeHard(&local_40, &local_40);
	peVar1 = pPathFollowReader->GetWayPoint(index);
	edF32Vector4SubHard(&eStack48, param_2, peVar1);
	eStack48.y = 0.0f;
	t = edF32Vector4DotProductHard(&eStack48, &local_40);
	if ((pOutPosition != (edF32VECTOR4*)0x0) || (param_7 != (float*)0x0)) {
		edF32Vector4ScaleHard(t, &local_40, &local_40);
		edF32Vector4SubHard(&eStack48, &eStack48, &local_40);

		fVar3 = edF32Vector4DotProductHard(&eStack48, &local_20);
		if (fVar3 < 0.0f) {
			if (pOutPosition != (edF32VECTOR4*)0x0) {
				*pOutPosition = *pPathFollowReader->GetWayPoint(index);
			}

			if (param_7 != (float*)0x0) {
				*param_7 = 0.0f;
			}
		}
		else {
			if (param_7 != (float*)0x0) {
				fVar3 = edF32Vector4GetDistHard(&eStack48);
				*param_7 = fVar3 / fVar2;
			}

			if (pOutPosition != (edF32VECTOR4*)0x0) {
				if (1.0f < *param_7) {
					*pOutPosition = *pPathFollowReader->GetWayPoint(local_4);
				}
				else {
					peVar1 = pPathFollowReader->GetWayPoint(index);
					edF32Vector4AddHard(pOutPosition, &eStack48, peVar1);
				}
			}
		}
	}

	return t;
}

float CActorMovable::SV_MOV_ManageMovOnPath(CPathFollowReader* pPathFollowReader, SV_MOV_PATH_PARAM* pMovPathParams)
{
	int iVar1;
	ed_3d_hierarchy_node* peVar2;
	edF32VECTOR4* pWayPoint;
	edF32VECTOR4* v1;
	Timer* pTVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float puVar8;
	float puVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float unaff_f22;
	edF32VECTOR4 local_160;
	edF32VECTOR4 local_150;
	float local_140;
	float local_13c;
	float local_138;
	float fStack308;
	float local_130;
	float local_12c;
	float local_128;
	float fStack292;
	edF32VECTOR4 local_120;
	edF32MATRIX4 eStack272;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 local_b0;
	CCollisionRay CStack160;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	float local_30;
	float local_2c;
	float local_28;
	edF32VECTOR3 local_20;
	_ray_info_out _Stack16;

	fVar11 = (this->dynamic).linearAcceleration;
	if (fVar11 == 0.0f) {
		fVar11 = 0.5f;
	}

	pWayPoint = pPathFollowReader->GetWayPoint();
	edF32Vector4SubHard(&local_40, pWayPoint, &this->currentLocation);

	fVar5 = edF32Vector4GetDistHard(&local_40);
	if (0.001f < fVar5) {
		if (pMovPathParams->field_0x10 == 1) {
			fVar5 = edF32Vector4NormalizeHard(&local_40, &local_40);
			if (pPathFollowReader->splinePointIndex != 0) {
				pWayPoint = pPathFollowReader->GetWayPoint(pPathFollowReader->field_0x8);
				v1 = pPathFollowReader->GetWayPoint();
				edF32Vector4SubHard(&local_50, v1, pWayPoint);
				fVar6 = edF32Vector4GetDistHard(&local_50);
				pWayPoint = pPathFollowReader->GetWayPoint();
				fVar12 = pWayPoint->x - this->currentLocation.x;
				fVar10 = pWayPoint->z - this->currentLocation.z;
				edF32Vector4NormalizeHard(&local_50, &local_50);

				fVar7 = edF32Vector4DotProductHard(&local_50, &this->dynamic.rotationQuat);
				if ((sqrtf(fVar12 * fVar12 + 0.0f + fVar10 * fVar10) <= fVar6 / 2.0f) || (fVar7 <= 0.96f)) {
					pWayPoint = pPathFollowReader->GetWayPoint();
					edF32Vector4SubHard(&local_40, pWayPoint, &this->currentLocation);
					unaff_f22 = 3.141593f;
				}
				else {
					local_40 = local_50;
					puVar8 = edF32Vector4DotProductHard(&local_40, &this->dynamic.rotationQuat);
					if (1.0f < puVar8) {
						puVar9 = 1.0f;
					}
					else {
						puVar9 = -1.0f;
						if (-1.0 <= puVar8) {
							puVar9 = puVar8;
						}
					}

					fVar10 = acosf(puVar9);
					fVar12 = 0.5f;
					if (fVar5 / fVar11 != 0.0f) {
						fVar12 = fVar5 / fVar11;
					}
					if (0.7853982f < fVar10) {
						fVar12 = fVar12 / 4.0f;
					}
					else {
						if (0.5235988f < fVar10) {
							fVar12 = fVar12 / 3.0f;
						}
						else {
							fVar12 = fVar12 / 2.0f;
						}
					}

					unaff_f22 = fVar10 / fVar12;
					if (unaff_f22 < 0.1f) {
						unaff_f22 = 0.1f;
					}
				}
			}

			edF32Vector4NormalizeHard(&local_40, &local_40);
			CActor::SV_Vector4SLERP(unaff_f22, &this->dynamic.rotationQuat, &local_40);
			pMovPathParams->field_0x14 = local_40.x * (this->dynamic).rotationQuat.z - (this->dynamic).rotationQuat.x * local_40.z;
		}
		else {
			if (pMovPathParams->field_0x10 == 0.0f) {
				edF32Vector4NormalizeHard(&local_40, &local_40);
				(this->dynamic).rotationQuat = local_40;
			}
		}
	}
	if ((this->dynamic).speed < pMovPathParams->acceleration) {
		fVar5 = pMovPathParams->speed;
		pTVar4 = GetTimer();
		fVar5 = (this->dynamic).speed + fVar5 * pTVar4->cutsceneDeltaTime;
		(this->dynamic).speed = fVar5;
		if (pMovPathParams->acceleration < fVar5) {
			(this->dynamic).speed = pMovPathParams->acceleration;
		}
	}
	else {
		fVar5 = pMovPathParams->speed;
		pTVar4 = GetTimer();
		fVar5 = (this->dynamic).speed - fVar5 * pTVar4->cutsceneDeltaTime;
		(this->dynamic).speed = fVar5;
		if (fVar5 < pMovPathParams->acceleration) {
			(this->dynamic).speed = pMovPathParams->acceleration;
		}
	}

	local_60 = this->currentLocation;
	iVar1 = pMovPathParams->field_0xc;
	if (((iVar1 != 2) && (iVar1 != 1)) && (iVar1 == 0)) {
		peVar2 = this->pMeshTransform;
		local_80.w = (peVar2->base).transformA.bd;
		local_80.x = 0.0f - (peVar2->base).transformA.ba;
		local_80.y = 0.0f - (peVar2->base).transformA.bb;
		local_80.z = 0.0f - (peVar2->base).transformA.bc;
		edF32Vector4NormalizeHard(&local_80, &local_80);
		CStack160 = CCollisionRay(2.0f, &this->currentLocation, &local_80);
		fVar5 = CStack160.IntersectScenery(&local_70, &_Stack16);
		if (fVar5 == 1e+30f) {
			peVar2 = this->pMeshTransform;
			local_b0 = (peVar2->base).transformA.rowY;
			edF32Vector4NormalizeHard(&local_b0, &local_b0);
			CStack160.ChangeLeadVector(&local_b0);
			fVar5 = CStack160.IntersectScenery(&local_70, &_Stack16);
			if (fVar5 == 1e+30f) {
				local_70.x = 0.0f - CDynamicExt::gForceGravity.x;
				local_70.y = 0.0f - CDynamicExt::gForceGravity.y;
				local_70.z = 0.0f - CDynamicExt::gForceGravity.z;
				local_70.w = CDynamicExt::gForceGravity.w;
				edF32Vector4NormalizeHard(&local_70, &local_70);
			}
			else {
				edF32Vector4ScaleHard(fVar5 + 0.03f, &eStack192, &local_70);
				edF32Vector4AddHard(&local_60, &this->currentLocation, &eStack192);
				UpdatePosition(&local_60, true);
			}
		}
		else {
			edF32Vector4ScaleHard(-(fVar5 - 0.03f), &eStack208, &local_70);
			edF32Vector4AddHard(&local_60, &this->currentLocation, &eStack208);
			UpdatePosition(&local_60, true);
		}

		if (0.6f < (this->dynamic).speed) {
			peVar2 = this->pMeshTransform;
			local_120 = (peVar2->base).transformA.rowY;
			fVar5 = edF32Vector4DotProductHard(&local_120, &local_70);
			if (fVar5 < 0.5f) {
				fVar12 = 2.0f;
			}
			else {
				fVar12 = 1.0f;
			}

			fVar5 = edFIntervalUnitSrcLERP(fVar5, 4.0f, 1.5f);
			fVar10 = edFIntervalLERP((this->dynamic).linearAcceleration, 5.0f, 25.0f, 1.0f, 1.5f);
			CActor::SV_Vector4SLERP(fVar12 * fVar5 * fVar10, &local_120, &local_70);
			BuildMatrixFromNormalAndSpeed(&eStack272, &local_120, &this->dynamic.rotationQuat);
			edF32Matrix4ToEulerSoft(&eStack272, &local_20, "XYZ");
			this->rotationEuler.xyz = local_20;
			SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
		}
	}

	iVar1 = pMovPathParams->field_0x8;
	if (iVar1 != 0) {
		if (iVar1 == 3) {
			if (0.0f < (this->dynamic).horizontalLinearAcceleration) {
				local_160.x = (this->dynamic).velocityDirectionEuler.x;
				local_160.y = 0.0f;
				local_160.z = (this->dynamic).velocityDirectionEuler.z;
				local_160.w = 0.0f;
				edF32Vector4NormalizeHard(&local_160, &local_160);
				SV_UpdateOrientation(pMovPathParams->rotationSpeed, &local_160);
				pMovPathParams->field_0x14 = local_160.x * this->rotationQuat.z - this->rotationQuat.x * local_160.z;
			}
		}
		else {
			if (iVar1 == 2) {
				if (0.0f < (this->dynamic).speed) {
					local_150.xyz = (this->dynamic).rotationQuat.xyz;
					local_150.w = 0.0f;
					edF32Vector4NormalizeHard(&local_150, &local_150);
					SV_UpdateOrientation(pMovPathParams->rotationSpeed, &local_150);
				}
			}
			else {
				if ((iVar1 == 1) && (fVar5 = edF32Vector4GetDistHard(&this->dynamic.rotationQuat), fVar5 != 0.0f)) {
					pWayPoint = pPathFollowReader->GetWayPointAngles();
					local_128 = pWayPoint->z;
					fStack292 = pWayPoint->w;
					local_130 = pWayPoint->x;
					local_12c = pWayPoint->y;
					local_140 = this->rotationEuler.x;
					local_13c = this->rotationEuler.y;
					local_138 = this->rotationEuler.z;
					fStack308 = this->rotationEuler.w;
					local_130 = fmodf(local_130, 3.141593f);
					local_12c = fmodf(local_12c, 3.141593f);
					local_128 = fmodf(local_128, 3.141593f);
					local_28 = 1.0f;
					if (1.0f <= fVar11 / fVar5) {
						local_28 = fVar11 / fVar5;
					}
					local_30 = (local_130 - local_140) / local_28;
					local_2c = (local_12c - local_13c) / local_28;
					local_28 = (local_128 - local_138) / local_28;
					this->rotationEuler.x = local_140 + local_30;
					this->rotationEuler.y = local_13c + local_2c;
					this->rotationEuler.z = local_138 + local_28;
				}
			}
		}
	}

	iVar1 = pMovPathParams->field_0xc;
	if (iVar1 == 2) {
		pWayPoint = pPathFollowReader->GetWayPoint();
		fVar11 = pWayPoint->x - this->currentLocation.x;
		fVar5 = pWayPoint->y - this->currentLocation.y;
		fVar12 = pWayPoint->z - this->currentLocation.z;
		fVar11 = sqrtf(fVar11 * fVar11 + fVar5 * fVar5 + fVar12 * fVar12);
	}
	else {
		if ((iVar1 == 1) || (iVar1 == 0)) {
			pWayPoint = pPathFollowReader->GetWayPoint();
			fVar11 = pWayPoint->x - this->currentLocation.x;
			fVar5 = pWayPoint->z - this->currentLocation.z;
			fVar11 = sqrtf(fVar11 * fVar11 + 0.0f + fVar5 * fVar5);
		}
		else {
			fVar11 = 0.0f;
		}
	}

	assert(std::isnan((this->dynamic).rotationQuat.x) == false);
	assert(std::isnan((this->dynamic).rotationQuat.y) == false);
	assert(std::isnan((this->dynamic).rotationQuat.z) == false);

	return fVar11;
}

bool CActorMovable::SV_MOV_UpdatePush(float param_1, S_PUSH_DATA* pPushData, S_PUSH_STREAM_DEF* pPushStreamRef)
{
	bool bVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;

	if (pPushStreamRef->field_0x0 == 0.0f) {
		bVar1 = false;
	}
	else {
		fVar3 = GetSubjectiveCumulatedWeight();
		fVar4 = param_1;
		if (fVar3 <= param_1) {
			fVar4 = fVar3;
		}
		fVar3 = pPushStreamRef->field_0x0;
		fVar4 = (fVar4 * fVar3) / param_1;
		if (fVar4 <= fVar3) {
			fVar3 = fVar4;
		}
		if ((((pPushData->oscValue).field_0x0 != 0.0f) || (fVar3 != 0.0f)) ||
			(bVar1 = false, (pPushData->oscValue).field_0x4 != 0.0f)) {
			pTVar2 = Timer::GetTimer();
			pPushData->oscValue.Update(fVar3, pTVar2->cutsceneDeltaTime, &pPushStreamRef->oscConfig);
			bVar1 = true;
		}
	}
	return bVar1;
}

float _edQuatToAngAxis(edF32VECTOR4* param_1, float* param_2, edF32VECTOR3* param_3)
{
	float fVar1;
	float fVar2;
	float puVar3;
	float puVar4;
	float fVar3;

	fVar3 = sqrtf(param_1->z * param_1->z + param_1->x * param_1->x + param_1->y * param_1->y);

	if (1e-06f < fVar3) {
		puVar3 = param_1->w;
		if (1.0f < puVar3) {
			puVar4 = 1.0f;
		}
		else {
			puVar4 = -1.0f;
			if (-1.0f <= puVar3) {
				puVar4 = puVar3;
			}
		}

		fVar2 = acosf(puVar4);
		fVar1 = 1.0f / fVar3;
		*param_2 = fVar2 * 2.0f;

		param_3->x = param_1->x * fVar1;
		param_3->y = param_1->y * fVar1;
		param_3->z = param_1->z * fVar1;
	}
	else {
		*param_2 = 0.0f;
		param_3->x = 0.0f;
		fVar3 = 0.0f;
		param_3->y = 0.0f;
		param_3->z = 0.0f;
	}

	return fVar3;
}

void _edQuatFromAngAxis(float t0, edF32VECTOR4* v0, edF32VECTOR3* v1)

{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar2 = gF32Vertex4Zero.w;
	fVar1 = gF32Vertex4Zero.z;
	fVar3 = gF32Vertex4Zero.y;
	fVar4 = sqrtf(v1->z * v1->z + v1->x * v1->x + v1->y * v1->y);
	if (fVar4 < 1e-06f) {
		v0->x = gF32Vertex4Zero.x;
		v0->y = fVar3;
		v0->z = fVar1;
		v0->w = fVar2;
	}
	else {
		fVar3 = sinf(t0 * 0.5f);
		fVar3 = fVar3 / fVar4;
		v0->x = v1->x * fVar3;
		v0->y = v1->y * fVar3;
		v0->z = v1->z * fVar3;
		fVar3 = cosf(t0 * 0.5f);
		v0->w = fVar3;
	}
	return;
}

bool CActorMovable::SV_MOV_UpdateTilt(float param_1, S_TILT_DATA* pTiltData, S_TILT_STREAM_DEF* pTiltStreamRef)
{
	CActorMovable* pOther;
	CCollision* pCVar1;
	bool bVar2;
	Timer* pTVar3;
	float puVar5;
	float puVar4;
	int iVar4;
	S_TIED_ACTOR_ENTRY* pCVar7;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	edF32VECTOR4 eStack160;
	float local_90;
	float fStack140;
	float fStack136;
	float fStack132;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32MATRIX4 local_60;
	edF32VECTOR4 local_20;
	edF32VECTOR3 eStack16;
	float local_4;

	fVar6 = 0.0f;
	if (pTiltData->field_0x38 == 0.0f) {
		bVar2 = false;
	}
	else {
		iVar4 = 0;
		pCVar7 = (this->pCollisionData)->pTiedActorEntry;
		local_20 = gF32Vertex4Zero;

		if (pCVar7 != (S_TIED_ACTOR_ENTRY*)0x0) {
			edF32Matrix4FromEulerSoft(&local_60, (edF32VECTOR3*)&this->rotationEuler, "XYZ");
			local_60.rowT = this->currentLocation;

			edF32Matrix4GetInverseOrthoHard(&local_60, &local_60);
			edF32Matrix4MulF32Vector4Hard(&local_70, &local_60, &g_xVector);

			while (pCVar7 != (S_TIED_ACTOR_ENTRY*)0x0) {
				pOther = (CActorMovable*)pCVar7->pActor;
				pCVar7 = pCVar7->pPrev;
				if (((pOther->pCollisionData != (CCollision*)0x0) &&
					(bVar2 = pOther->IsKindOfObject(2), bVar2 != false)) && (fVar5 = pOther->GetSubjectiveCumulatedWeight(), 0.0f < fVar5)) {
					fVar7 = param_1;

					if (fVar5 <= param_1) {
						fVar7 = fVar5;
					}

					pCVar1 = pOther->pCollisionData;
					fVar5 = (pCVar1->highestVertex).x;
					fVar8 = (pCVar1->highestVertex).y;
					fVar9 = (pCVar1->highestVertex).z;
					fVar10 = (pCVar1->highestVertex).w;
					local_80.x = local_60.aa * fVar5 + local_60.ba * fVar8 + local_60.ca * fVar9 + local_60.da * fVar10;
					local_80.y = local_60.ab * fVar5 + local_60.bb * fVar8 + local_60.cb * fVar9 + local_60.db * fVar10;
					local_80.z = local_60.ac * fVar5 + local_60.bc * fVar8 + local_60.cc * fVar9 + local_60.dc * fVar10;
					fStack132 = local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z;
					local_90 = local_70.x * fStack132;
					fStack140 = local_70.y * fStack132;
					fStack136 = local_70.z * fStack132;
					fStack132 = local_70.w * fStack132;
					local_80.x = local_80.x - local_90;
					local_80.y = local_80.y - fStack140;
					local_80.z = local_80.z - fStack136;
					local_80.w = (local_60.ad * fVar5 + local_60.bd * fVar8 + local_60.cd * fVar9 + local_60.dd * fVar10) - fStack132;

					if (pTiltStreamRef->field_0x0 == -1) {
						local_80.w = 0.0f;
						fVar5 = edF32Vector4GetDistHard(&local_80);

						(pTiltData->field_0x0).x = local_80.y * local_70.z - local_70.y * local_80.z;
						(pTiltData->field_0x0).y = local_80.z * local_70.x - local_70.z * local_80.x;
						(pTiltData->field_0x0).z = local_80.x * local_70.y - local_70.x * local_80.y;
						(pTiltData->field_0x0).w = in_vf0x;

						edF32Vector4SafeNormalize1Hard(&pTiltData->field_0x0, &pTiltData->field_0x0);
					}
					else {
						IMPLEMENTATION_GUARD(
						fVar5 = GetDistFromRay(&local_80, &gF32Vertex4Zero, &pTiltData->field_0x0);

						fVar8 = (pTiltData->field_0x0).x;
						fVar9 = (pTiltData->field_0x0).y;
						fVar10 = (pTiltData->field_0x0).z;

						local_90 = local_80.y * fVar10 - fVar9 * local_80.z;
						fStack140 = local_80.z * fVar8 - fVar10 * local_80.x;
						fStack136 = local_80.x * fVar9 - fVar8 * local_80.y;
						fStack132 = in_vf0x;

						if (0.0f < local_90 * local_70.x + fStack140 * local_70.y + fStack136 * local_70.z) {
							fVar5 = -fVar5;
						})
					}

					fVar8 = pTiltStreamRef->field_0x4;
					fVar5 = pTiltData->field_0x38 * fVar5 * fVar7;

					if ((fVar5 <= fVar8) && (fVar8 = -fVar8, fVar8 <= fVar5)) {
						fVar8 = fVar5;
					}

					if (pTiltStreamRef->field_0x0 == -1) {
						_edQuatFromAngAxis(fVar8, &eStack160, &pTiltData->field_0x0.xyz);
						edQuatMul(&local_20, &local_20, &eStack160);
					}
					else {
						fVar6 = fVar6 + fVar8;
					}

					iVar4 = iVar4 + 1;
				}
			}
		}
		if (pTiltStreamRef->field_0x0 == -1) {
			if ((((pTiltData->oscQuat).field_0x0.w != 1.0f) || (local_20.w != 1.0f)) || (bVar2 = false, (pTiltData->oscQuat).field_0x10.w != 1.0f)) {
				if (1 < iVar4) {
					_edQuatToAngAxis(&local_20, &local_4, &eStack16);
					local_4 = edF32Between_Pi(local_4);
					fVar6 = pTiltStreamRef->field_0x4;

					if (fVar6 < fabs(local_4)) {
						bVar2 = local_4 <= 0.0f;
						local_4 = fVar6;

						if (bVar2) {
							local_4 = -fVar6;
						}

						_edQuatFromAngAxis(local_4, &local_20, &eStack16);
					}
				}

				pTVar3 = Timer::GetTimer();
				pTiltData->oscQuat.Update(pTVar3->cutsceneDeltaTime, &pTiltStreamRef->oscConfig, &local_20);
				bVar2 = true;
			}
		}
		else {
			if (((pTiltData->oscValue.field_0x0 != 1.0f) || (fVar6 != 0.0f)) || (bVar2 = false, pTiltData->oscValue.field_0x4 != 1.0f)) {
				IMPLEMENTATION_GUARD(
				fVar5 = pTiltStreamRef->field_0x4;
				if (fVar6 <= fVar5) {
					fVar5 = fVar6;
				}
				pTVar3 = Timer::GetTimer();
				S_OSCILLATING_VALUE::Update
				(fVar5, pTVar3->cutsceneDeltaTime, (S_OSCILLATING_VALUE*)&pTiltData->field_0x30,
					&pTiltStreamRef->oscConfig);
				fVar6 = (float)pTiltData->field_0x30;
				if (3.140593f < fabs(fVar6)) {
					puVar5 = 1.0f;
					if (fVar6 < 0.0f) {
						puVar5 = -1.0f;
					}
					fVar6 = puVar5 * 3.140593f;
				}
				_edQuatFromAngAxis(fVar6, (edF32VECTOR4*)&pTiltData->oscQuat, (edF32VECTOR3*)pTiltData);
				fVar6 = pTiltData->field_0x34;
				if (3.140593f < fabs(fVar6)) {
					puVar4 = 1.0f;
					if (fVar6 < 0.0f) {
						puVar4 = -1.0f;
					}
					fVar6 = (float)puVar4 * 3.140593f;
				}
				_edQuatFromAngAxis(fVar6, &(pTiltData->oscQuat).field_0x10, (edF32VECTOR3*)pTiltData);
				bVar2 = true;)
			}
		}
	}
	return bVar2;
}

void CActorMovable::SV_MOV_MoveTo(CActorMovParamsOut* pActorMovParamsOut, CActorMovParamsIn* pActorMovParamsIn, edF32VECTOR4* pMoveToPosition)
{
	uint uVar1;
	edF32VECTOR4* peVar2;
	Timer* pTVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	uVar1 = pActorMovParamsIn->flags;
	if ((uVar1 & 0x10) == 0) {
		if ((uVar1 & 0x800) == 0) {
			edF32Vector4SubHard(&pActorMovParamsOut->moveDirection, pMoveToPosition, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&pActorMovParamsOut->moveDirection, &this->currentLocation, pMoveToPosition);
		}

		if ((pActorMovParamsIn->flags & 4) == 0) {
			pActorMovParamsOut->moveDirection.y = 0.0f;
		}

		pActorMovParamsOut->moveVelocity = edF32Vector4SafeNormalize0Hard(&pActorMovParamsOut->moveDirection, &pActorMovParamsOut->moveDirection);

		if (((pActorMovParamsIn->flags & 1) == 0) && (pActorMovParamsOut->moveVelocity != 0.0f)) {
			if (pActorMovParamsIn->pRotation == (edF32VECTOR4*)0x0) {
				pActorMovParamsIn->pRotation = &pActorMovParamsOut->moveDirection;
			}

			SV_LookTo(pActorMovParamsOut, pActorMovParamsIn);
		}

		uVar1 = pActorMovParamsIn->flags;

		if (((uVar1 & 0x200) == 0) &&
			(((uVar1 & 0x1000) == 0 || (pActorMovParamsIn->field_0x18 <= pActorMovParamsOut->moveVelocity)))) {
			if ((uVar1 & 0x400) == 0) {
				(this->dynamic).rotationQuat = pActorMovParamsOut->moveDirection;
				(this->dynamic).speed = pActorMovParamsIn->speed;
			}
			else {
				(this->dynamic).rotationQuat = pActorMovParamsOut->moveDirection;

				fVar6 = pActorMovParamsIn->speed;
				fVar4 = pActorMovParamsIn->acceleration;
				pTVar3 = GetTimer();
				fVar5 = (this->dynamic).speed;
				fVar4 = fVar4 * pTVar3->cutsceneDeltaTime;
				if (fVar5 < fVar6) {
					fVar5 = fVar5 + fVar4;
					(this->dynamic).speed = fVar5;
					if (fVar6 < fVar5) {
						(this->dynamic).speed = fVar6;
					}
				}
				else {
					fVar5 = fVar5 - fVar4;
					(this->dynamic).speed = fVar5;
					if (fVar5 < fVar6) {
						(this->dynamic).speed = fVar6;
					}
				}
			}
		}
	}
	else {
		if ((uVar1 & 0x800) == 0) {
			edF32Vector4SubHard(&pActorMovParamsOut->moveDirection, pMoveToPosition, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&pActorMovParamsOut->moveDirection, &this->currentLocation, pMoveToPosition);
		}

		if ((pActorMovParamsIn->flags & 4) == 0) {
			pActorMovParamsOut->moveDirection.y = 0.0;
		}

		fVar4 = edF32Vector4SafeNormalize0Hard(&pActorMovParamsOut->moveDirection, &pActorMovParamsOut->moveDirection);
		pActorMovParamsOut->moveVelocity = fVar4;

		if (fVar4 != 0.0f) {
			pActorMovParamsIn->pRotation = &pActorMovParamsOut->moveDirection;
			SV_LookTo(pActorMovParamsOut, pActorMovParamsIn);

			uVar1 = pActorMovParamsIn->flags;
			if ((uVar1 & 4) == 0) {
				(this->dynamic).rotationQuat = this->rotationQuat;
			}
			else {
				if ((uVar1 & 8) == 0) {
					peVar2 = pActorMovParamsIn->pRotation;
					(this->dynamic).rotationQuat = *peVar2;
				}
				else {
					(this->dynamic).rotationQuat = this->rotationQuat;
				}
			}
		}

		uVar1 = pActorMovParamsIn->flags;
		if (((uVar1 & 0x200) == 0) && (((uVar1 & 0x1000) == 0 || (pActorMovParamsIn->field_0x18 <= pActorMovParamsOut->moveVelocity)))) {
			if ((uVar1 & 0x400) == 0) {
				(this->dynamic).speed = pActorMovParamsIn->speed;
			}
			else {
				fVar6 = pActorMovParamsIn->speed;
				fVar4 = pActorMovParamsIn->acceleration;
				pTVar3 = GetTimer();
				fVar5 = (this->dynamic).speed;
				fVar4 = fVar4 * pTVar3->cutsceneDeltaTime;

				if (fVar5 < fVar6) {
					fVar5 = fVar5 + fVar4;
					(this->dynamic).speed = fVar5;

					if (fVar6 < fVar5) {
						(this->dynamic).speed = fVar6;
					}
				}
				else {
					fVar5 = fVar5 - fVar4;
					(this->dynamic).speed = fVar5;

					if (fVar5 < fVar6) {
						(this->dynamic).speed = fVar6;
					}
				}
			}
		}
	}

	return;
}

void CActorMovable::SV_MOV_MoveCloserTo(float speed, edF32VECTOR4* param_3)
{
	Timer* pTVar1;
	float fVar2;
	edF32VECTOR4 local_10;

	edF32Vector4SubHard(&local_10, param_3, &this->currentLocation);
	local_10.y = 0.0f;

	fVar2 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
	pTVar1 = GetTimer();

	if (fVar2 < speed * pTVar1->cutsceneDeltaTime) {
		pTVar1 = GetTimer();
		speed = fVar2 / pTVar1->cutsceneDeltaTime;
	}

	if (fVar2 != 0.0f) {
		(this->dynamic).rotationQuat = local_10;
	}

	(this->dynamic).speed = speed;
	return;
}

float CActorMovable::SV_MOV_GetAccelerationFromDistAndSpeed(float param_1, float param_2, float param_3)
{
	float fVar1;

	fVar1 = (param_3 * 2.0f) / (param_1 + param_2);
	if (0.0f < fVar1) {
		fVar1 = fabs(param_2 - param_1) / fVar1;
	}
	else {
		fVar1 = 500.0f;
	}

	return fVar1;
}

void CActorMovable::SV_MOV_AddSpeedVector(edF32VECTOR4* pSpeedVector)
{
	float fVar1;
	edF32VECTOR4 local_10;

	edF32Vector4ScaleHard((this->dynamic).speed, &local_10, &this->dynamic.rotationQuat);
	edF32Vector4AddHard(&local_10, &local_10, pSpeedVector);

	fVar1 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
	(this->dynamic).speed = fVar1;

	if (0.001f < fVar1) {
		(this->dynamic).rotationQuat = local_10;
	}

	return;
}

float CActorMovable::FUN_00120250(float param_1)
{
	Timer* pTVar1;
	float fVar2;

	pTVar1 = GetTimer();
	fVar2 = (this->dynamic).linearAcceleration * pTVar1->cutsceneDeltaTime;
	if (param_1 <= fVar2) {
		param_1 = fVar2;
	}

	return param_1;
}

void CActorMovable::ComputeRealMoving(edF32VECTOR4* delta)
{
	Timer* pTVar1;
	float fVar2;
	edF32VECTOR4 eStack16;

	pTVar1 = GetTimer();
	edF32Vector4SubHard(&eStack16, &this->currentLocation, delta);
	fVar2 = edF32Vector4SafeNormalize1Hard(&(this->dynamic).velocityDirectionEuler, &eStack16);
	(this->dynamic).linearAcceleration = fVar2;
	(this->dynamic).linearAcceleration = (this->dynamic).linearAcceleration / pTVar1->cutsceneDeltaTime;

	eStack16.y = 0.0f;

	fVar2 = edF32Vector4SafeNormalize1Hard(&(this->dynamic).horizontalVelocityDirectionEuler, &eStack16);
	(this->dynamic).horizontalLinearAcceleration = fVar2;
	(this->dynamic).horizontalLinearAcceleration = (this->dynamic).horizontalLinearAcceleration / pTVar1->cutsceneDeltaTime;
	return;
}

float CActorMovable::GetSubjectiveCumulatedWeight()

{
	float fVar1;
	float subjectiveCumulatedWeight;
	CCollision* pCol;

	pCol = this->pCollisionData;

	subjectiveCumulatedWeight = (this->dynamic).weightA * (this->dynamic).weightB;

	if (pCol != (CCollision*)0x0) {
		fVar1 = pCol->GetSubjectiveCarriedWeight();
		subjectiveCumulatedWeight = subjectiveCumulatedWeight + fVar1;
	}

	return subjectiveCumulatedWeight;
}

float CActorMovable::GetCumulatedWeight()
{
	float fVar1;
	float fVar2;
	CCollision* pCol;

	fVar2 = (this->dynamic).weightB;
	pCol = this->pCollisionData;
	if (pCol != (CCollision*)0x0) {
		fVar1 = pCol->GetCarriedWeight();
		fVar2 = fVar2 + fVar1;
	}
	return fVar2;
}

CDynamic::CDynamic()
{
	weightB = 0.0f;
}

void CDynamic::Reset(CActor* pActor)
{
	float fVar1;
	float fVar2;
	float fVar3;

	this->field_0x4c = 0;
	this->flags = 0;
	

	this->velocityDirectionEuler = gF32Vector4UnitZ;
	this->horizontalVelocityDirectionEuler = gF32Vector4UnitZ;
	this->field_0x10 = gF32Vector4Zero;

	this->speed = 0.0f;
	this->linearAcceleration = 0;
	this->horizontalLinearAcceleration = 0;
	this->weightA = 1.0f;
	if (pActor == (CActor*)0x0) {
		(this->rotationQuat).x = 1.0f;
		(this->rotationQuat).y = 0.0f;
		(this->rotationQuat).z = 0.0f;
		(this->rotationQuat).w = 0.0f;
	}
	else {
		this->rotationQuat = pActor->rotationQuat;
	}
	return;
}

void CDynamic::ClearLocalData()
{
	this->field_0x4c = 0;
	this->flags = 0;

	this->velocityDirectionEuler = gF32Vector4UnitZ;
	this->horizontalVelocityDirectionEuler = gF32Vector4UnitZ;
	this->field_0x10 = gF32Vector4Zero;

	this->speed = 0.0f;
	this->linearAcceleration = 0.0f;
	this->horizontalLinearAcceleration = 0.0f;
	this->weightA = 1.0f;

	return;
}

bool S_OSCILLATING_QUAT::Update(float param_1, S_OSCILLATION_CONFIG* pConfig, edF32VECTOR4* param_4)
{
	bool bVar1;
	float fVar2;
	float puVar4;
	float puVar5;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float puVar7;
	float puVar8;
	float puVar9;
	float puVar10;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR3 local_20;
	edF32VECTOR3 local_10;
	float puVar3;

	edQuatInverse(&local_30, &this->field_0x0);
	edQuatMul(&local_30, &local_30, param_4);

	fVar5 = (this->field_0x10).x;
	fVar6 = (this->field_0x10).y;
	fVar2 = (this->field_0x10).z;
	fVar2 = sqrtf(fVar2 * fVar2 + fVar5 * fVar5 + fVar6 * fVar6);

	if (1e-06f < fVar2) {
		puVar7 = (this->field_0x10).w;
		if (1.0f < puVar7) {
			puVar8 = 1.0f;
		}
		else {
			puVar8 = -1.0f;
			if (-1.0f <= puVar7) {
				puVar8 = puVar7;
			}
		}

		fVar5 = acosf(puVar8);
		fVar2 = 1.0f / fVar2;
		fVar5 = fVar5 * 2.0;

		local_10.x = (this->field_0x10).x * fVar2;
		local_10.y = (this->field_0x10).y * fVar2;
		local_10.z = (this->field_0x10).z * fVar2;
	}
	else {
		local_10.x = 0.0f;
		local_10.y = 0.0f;
		local_10.z = 0.0f;
		fVar5 = 0.0f;
	}

	fVar2 = sqrtf(local_30.z * local_30.z + local_30.x * local_30.x + local_30.y * local_30.y);

	if (1e-06f < fVar2) {
		if (1.0f < local_30.w) {
			puVar3 = 1.0f;
		}
		else {
			puVar3 = -1.0f;

			if (-1.0f <= local_30.w) {
				puVar3 = local_30.w;
			}
		}

		fVar6 = acosf((float)puVar3);
		fVar2 = 1.0f / fVar2;
		fVar6 = fVar6 * 2.0;
		local_20.x = local_30.x * fVar2;
		local_20.y = local_30.y * fVar2;
		local_20.z = local_30.z * fVar2;
	}
	else {
		local_20.x = 0.0f;
		local_20.y = 0.0f;
		local_20.z = 0.0f;

		fVar6 = 0.0f;
	}

	fVar2 = fmodf(fVar5, 6.283185f);
	if (fVar2 < -3.141593f) {
		fVar2 = fVar2 + 6.283185f;
	}
	else {
		if (3.141593f <= fVar2) {
			fVar2 = fVar2 - 6.283185f;
		}
	}

	fVar3 = fmodf(fVar6, 6.283185f);
	fVar4 = gF32Vertex4Zero.w;
	fVar6 = gF32Vertex4Zero.z;
	fVar5 = gF32Vertex4Zero.y;

	if (fVar3 < -3.141593f) {
		fVar3 = fVar3 + 6.283185f;
	}
	else {
		if (3.141593f <= fVar3) {
			fVar3 = fVar3 - 6.283185f;
		}
	}

	if ((0.05f < fabs(fVar2)) || (0.01f < fabs(fVar3))) {
		fVar5 = param_1 * pConfig->field_0x0 * fVar3;

		if (3.140593f < fabs(fVar5)) {
			if (0.0f <= fVar5) {
				puVar4 = 1.0f;
			}
			else {
				puVar4 = -1.0f;
			}
			fVar5 = puVar4 * 3.140593f;
		}

		fVar6 = sqrtf(local_20.z * local_20.z + local_20.x * local_20.x + local_20.y * local_20.y);

		if (fVar6 < 1e-06f) {
			local_40.x = gF32Vertex4Zero.x;
			local_40.y = gF32Vertex4Zero.y;
			local_40.z = gF32Vertex4Zero.z;
			local_40.w = gF32Vertex4Zero.w;
		}
		else {
			fVar4 = sinf(fVar5 * 0.5f);
			fVar4 = fVar4 / fVar6;
			local_40.z = local_20.z * fVar4;
			local_40.x = local_20.x * fVar4;
			local_40.y = local_20.y * fVar4;
			local_40.w = cosf(fVar5 * 0.5f);
		}

		fVar5 = param_1 * param_1 * fVar2 * -pConfig->field_0x4 * fVar2;

		if (fVar2 < 0.0f) {
			fVar5 = -fVar5;
		}

		if (3.140593f < fabs(fVar5)) {
			if (0.0f <= fVar5) {
				puVar5 = 1.0f;
			}
			else {
				puVar5 = -1.0f;
			}

			fVar5 = puVar5 * 3.140593f;
		}

		fVar2 = sqrtf(local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y);

		if (fVar2 < 1e-06f) {
			local_50.x = gF32Vertex4Zero.x;
			local_50.y = gF32Vertex4Zero.y;
			local_50.z = gF32Vertex4Zero.z;
			local_50.w = gF32Vertex4Zero.w;
		}
		else {
			fVar6 = sinf(fVar5 * 0.5f);
			fVar6 = fVar6 / fVar2;
			local_50.z = local_10.z * fVar6;
			local_50.x = local_10.x * fVar6;
			local_50.y = local_10.y * fVar6;
			local_50.w = cosf(fVar5 * 0.5f);
		}

		edQuatMul(&eStack96, &local_40, &local_50);
		edQuatMul(&this->field_0x10, &this->field_0x10, &eStack96);

		if ((this->field_0x10).w < 0.999999f) {
			edQuatNormalize(&this->field_0x10, &this->field_0x10);
		}
		else {
			this->field_0x10 = gF32Vertex4Zero;
		}

		fVar2 = sqrtf((this->field_0x10).z * (this->field_0x10).z +
			(this->field_0x10).x * (this->field_0x10).x + (this->field_0x10).y * (this->field_0x10).y);

		if (1e-06f < fVar2) {
			puVar9 = (this->field_0x10).w;

			if (1.0f < puVar9) {
				puVar10 = 1.0f;
			}
			else {
				puVar10 = -1.0f;
				if (-1.0f <= puVar9) {
					puVar10 = puVar9;
				}
			}

			fVar5 = acosf(puVar10);
			fVar2 = 1.0f / fVar2;
			fVar5 = fVar5 * 2.0f;

			local_10.x = (this->field_0x10).x * fVar2;
			local_10.y = (this->field_0x10).y * fVar2;
			local_10.z = (this->field_0x10).z * fVar2;
		}
		else {
			local_10.x = 0.0f;
			local_10.y = 0.0f;
			local_10.z = 0.0f;
			fVar5 = 0.0f;
		}

		fVar2 = sqrtf(local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y);

		if (fVar2 < 1e-06f) {
			local_70 = gF32Vertex4Zero;
		}
		else {
			fVar6 = sinf(fVar5 * param_1 * 0.5f);
			fVar6 = fVar6 / fVar2;
			local_70.z = local_10.z * fVar6;
			local_70.x = local_10.x * fVar6;
			local_70.y = local_10.y * fVar6;
			local_70.w = cosf(fVar5 * param_1 * 0.5f);
		}

		edQuatMul(&this->field_0x0, &this->field_0x0, &local_70);

		if ((this->field_0x0).w < 0.999999) {
			edQuatNormalize(&this->field_0x0, &this->field_0x0);
		}
		else {
			this->field_0x0 = gF32Vertex4Zero;
		}

		bVar1 = false;
	}
	else {
		bVar1 = true;
		this->field_0x10 = gF32Vertex4Zero;
		this->field_0x0 = *param_4;
	}

	return bVar1;
}
