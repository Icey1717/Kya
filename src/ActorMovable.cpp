#include "ActorMovable.h"

#include <math.h>

#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "CollisionManager.h"
#include "ActorCheckpointManager.h"

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
}

CActorMovable::CActorMovable()
{
	this->dynamic.weightB = 0.0f;
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
	this->field_0x1c0 = 1e+30f;
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
		GetAnglesFromVector(&this->rotationEuler, &this->rotationQuat);
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
		pCol->CheckCollisions_MoveActor((CActor*)this, &eStack48, (CActorsTable*)0x0, pActor, 0);
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

void CActorMovable::SV_MOV_MoveTo(CActorMovParamsOut* pActorMovParamsOut, CActorMovParamsIn* pActorMovParamsIn, edF32VECTOR4* param_4)
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
			edF32Vector4SubHard(&pActorMovParamsOut->vectorField, param_4, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&pActorMovParamsOut->vectorField, &this->currentLocation, param_4);
		}

		if ((pActorMovParamsIn->flags & 4) == 0) {
			pActorMovParamsOut->vectorField.y = 0.0f;
		}

		fVar4 = edF32Vector4SafeNormalize0Hard(&pActorMovParamsOut->vectorField, &pActorMovParamsOut->vectorField);
		pActorMovParamsOut->floatField = fVar4;

		if (((pActorMovParamsIn->flags & 1) == 0) && (pActorMovParamsOut->floatField != 0.0f)) {
			if (pActorMovParamsIn->field_0x8 == (edF32VECTOR4*)0x0) {
				pActorMovParamsIn->field_0x8 = &pActorMovParamsOut->vectorField;
			}

			SV_LookTo(pActorMovParamsOut, pActorMovParamsIn);
		}

		uVar1 = pActorMovParamsIn->flags;

		if (((uVar1 & 0x200) == 0) &&
			(((uVar1 & 0x1000) == 0 || (pActorMovParamsIn->field_0x18 <= pActorMovParamsOut->floatField)))) {
			if ((uVar1 & 0x400) == 0) {
				(this->dynamic).rotationQuat = pActorMovParamsOut->vectorField;
				(this->dynamic).speed = pActorMovParamsIn->field_0xc;
			}
			else {
				(this->dynamic).rotationQuat = pActorMovParamsOut->vectorField;

				fVar6 = pActorMovParamsIn->field_0xc;
				fVar4 = pActorMovParamsIn->field_0x14;
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
			edF32Vector4SubHard(&pActorMovParamsOut->vectorField, param_4, &this->currentLocation);
		}
		else {
			edF32Vector4SubHard(&pActorMovParamsOut->vectorField, &this->currentLocation, param_4);
		}

		if ((pActorMovParamsIn->flags & 4) == 0) {
			pActorMovParamsOut->vectorField.y = 0.0;
		}

		fVar4 = edF32Vector4SafeNormalize0Hard(&pActorMovParamsOut->vectorField, &pActorMovParamsOut->vectorField);
		pActorMovParamsOut->floatField = fVar4;

		if (fVar4 != 0.0f) {
			pActorMovParamsIn->field_0x8 = &pActorMovParamsOut->vectorField;
			SV_LookTo(pActorMovParamsOut, pActorMovParamsIn);

			uVar1 = pActorMovParamsIn->flags;
			if ((uVar1 & 4) == 0) {
				(this->dynamic).rotationQuat = this->rotationQuat;
			}
			else {
				if ((uVar1 & 8) == 0) {
					peVar2 = pActorMovParamsIn->field_0x8;
					(this->dynamic).rotationQuat = *peVar2;
				}
				else {
					(this->dynamic).rotationQuat = this->rotationQuat;
				}
			}
		}

		uVar1 = pActorMovParamsIn->flags;
		if (((uVar1 & 0x200) == 0) && (((uVar1 & 0x1000) == 0 || (pActorMovParamsIn->field_0x18 <= pActorMovParamsOut->floatField)))) {
			if ((uVar1 & 0x400) == 0) {
				(this->dynamic).speed = pActorMovParamsIn->field_0xc;
			}
			else {
				fVar6 = pActorMovParamsIn->field_0xc;
				fVar4 = pActorMovParamsIn->field_0x14;
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
