#include "ActorHero_Private.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "TimeController.h"
#include "InputManager.h"
#include "EventManager.h"
#include "CollisionRay.h"

// Should be in: D:/Projects/b-witch/ActorHero_GripClimb.cpp
void CActorHeroPrivate::ResetGripClimbDefaultSettings()
{
	this->field_0x14ac = 2.0f;
	this->field_0x14b0 = 1.22173f;
	this->field_0x14b4 = 0.5235988f;
	this->field_0x14b8 = 1.22173f;
	this->field_0x14bc = 0.3f;
	this->field_0x14c0 = 0.2f;
	this->field_0x14c4 = 0.8726646f;
	this->field_0x14c8 = 1.4f;
	this->field_0x14cc = 5.0f;
	this->field_0x14d0 = 1.5f;
	this->field_0x14d4 = 1.8f;
	this->field_0x14d8 = 12.0f;
	this->field_0x14dc = 20.0f;
	this->field_0x14e0 = 3.0f;
	this->field_0x14e4 = 11.5f;
	this->field_0x14e8 = 8.0f;
	this->field_0x14ec = 5.0f;
	this->field_0x14f0 = 9.5f;
	this->field_0x1514 = 2.0f;
	this->field_0x1510 = 2.0f;
	this->gripHorizontalMoveSpeed = 1.0f;
	this->gripUpMoveSpeed = 4.0f;
	this->field_0x1520 = 12.566371f;
	this->field_0x1524 = 2.0f;
	this->field_0x1528 = 8.0f;
	this->field_0x152c = 50.0f;
	this->field_0x1530 = 7.0f;
	this->field_0x1534 = 0.4f;
	return;
}

S_DETECT_WALL_CFG cfg_wall_far{ -0.2f , -0.5f, 0.0f };
S_DETECT_WALL_CFG cfg_wall_close{ -0.01f , -0.5f, 0.0f };

// Should be in: D:/Projects/b-witch/ActorHero_GripClimb.cpp
bool CActorHeroPrivate::DetectGripablePrecipice()
{
	CActor* pCVar1;
	CActor* pTieActor;
	bool bVar2;
	edF32VECTOR4* v0;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	CCollisionRay CStack112;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_ray_info_out local_10;

	cfg_wall_far.field_0x8 = this->field_0x14b0;
	cfg_wall_close.field_0x8 = this->field_0x14b0;

	fVar3 = this->dynamic.horizontalLinearAcceleration;
	if (((((0.1 < fVar3) && (fVar3 < this->runSpeed)) && (this->field_0x14ac < this->distanceToGround)
		) && ((bVar2 = DetectWall(3, 0x40000010, &this->dynamic.velocityDirectionEuler, &cfg_wall_far, &this->bounceLocation, &local_40, &local_10), bVar2 != false
			|| (bVar2 = DetectWall(3, 0x40000010,
				&this->dynamic.
				velocityDirectionEuler, (S_DETECT_WALL_CFG*)&cfg_wall_close,
				&this->bounceLocation, &local_40, &local_10),
				bVar2 != false)))) &&
		(v0 = &this->bounceLocation, (local_10.hitMaterialFlags & 0x1200) == 0)) {
		this->bounceLocation.y = 0.0f;

		edF32Vector4NormalizeHard(v0, v0);

		float fStack20 = ((this->pCollisionData)->pObbPrim->field_0x90).z - 0.1f;
		local_20 = this->bounceLocation * fStack20;
		local_30.x = local_20.x + local_40.x;
		local_30.z = local_20.z + local_40.z;
		local_30.w = local_20.y + local_40.w;
		local_30.y = this->currentLocation.y + 0.7f;
		local_50.x = 0.0f;
		local_50.y = -1.0f;
		local_50.z = 0.0f;
		local_50.w = 0.0f;
		CCollisionRay CStack112 = CCollisionRay(2.4f, &local_30, &local_50);
		fVar3 = CStack112.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		if (fVar3 == 1e+30f) {
			local_30.x = local_20.x + local_30.x;
			local_30.y = local_20.y + local_30.y;
			local_30.z = local_20.z + local_30.z;
			local_30.w = fStack20 + local_30.w;
			fVar3 = CStack112.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			if (fVar3 == 1e+30f) {
				fVar3 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1f;
				this->field_0x1490 = this->bounceLocation * fVar3;
				fVar3 = this->field_0x1490.y;
				fVar4 = this->field_0x1490.z;
				fVar5 = this->field_0x1490.w;
				this->field_0x1490 = this->field_0x1490 + local_40;
				this->field_0x1490.y = this->currentLocation.y;
				GripObject(local_10.pActor_0x0);
				return true;
			}
		}
	}

	return false;
}


bool CActorHeroPrivate::DetectWall(uint param_2, uint param_3, edF32VECTOR4* param_4, S_DETECT_WALL_CFG* pDetectWallCfg, edF32VECTOR4* param_6, edF32VECTOR4* param_7, _ray_info_out* pRayInfoOut)
{
	bool bVar1;
	float fVar2;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.x = this->currentLocation.x;
	local_10.y = pDetectWallCfg->field_0x0 + this->currentLocation.y;
	local_10.z = this->currentLocation.z;
	local_10.w = 1.0f;

	fVar2 = pDetectWallCfg->field_0x4;
	if (fVar2 < 0.0) {
		fVar2 = -fVar2;
		local_20.x = -param_4->x;
		local_20.z = -param_4->z;
	}
	else {
		local_20.x = param_4->x;
		local_20.z = param_4->z;
	}

	local_20.w = 0.0f;
	local_20.y = 0.0f;
	if (param_4->y != 0.0f) {
		edF32Vector4NormalizeHard(&local_20, &local_20);
	}

	CCollisionRay CStack80 = CCollisionRay(fVar2, &local_10, &local_20);
	fVar2 = CStack80.Intersect(param_2, this, (CActor*)0x0, param_3, &local_30, pRayInfoOut);
	bVar1 = fVar2 != 1e+30f && 0.01f < fVar2;

	if (bVar1) {
		bVar1 = fabs(local_30.y) < cosf(pDetectWallCfg->field_0x8);
	}

	if (bVar1) {
		bVar1 = local_30.x * local_20.x + local_30.y * local_20.y + local_30.z * local_20.z <= 0.0;
	}

	if (bVar1) {
		if (param_6 != (edF32VECTOR4*)0x0) {
			*param_6 = local_30;
		}

		if (param_7 != (edF32VECTOR4*)0x0) {
			*param_7 = local_20 * fVar2;
			*param_7 = *param_7 + local_10;
		}
	}

	return bVar1;
}

int CActorHeroPrivate::DetectClimbCeiling(edF32VECTOR4* v0, CActor** pOutActor)
{
	CActor* pCVar1;
	CActor* pTieActor;
	bool bVar2;
	int bDetectClimbCeiling;
	float intersectionDist;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_ray_info_out local_10;

	if ((this->field_0x1420 == 0) && (bDetectClimbCeiling = EvolutionCanClimb(), bDetectClimbCeiling != 0)) {
		local_20.x = v0->x;
		local_20.z = v0->z;
		local_20.w = v0->w;
		bDetectClimbCeiling = 0;
		local_20.y = v0->y + 1.16f;

		CCollisionRay CStack80 = CCollisionRay(0.8f, &local_20, &g_xVector);
		if (this->field_0x1454 == false) {
			intersectionDist = CStack80.IntersectActors(this, (CActor*)0x0, 0x40000100, &local_30, &local_10);
		}
		else {
			intersectionDist = CStack80.Intersect(3, this, (CActor*)0x0, 0x40000100, &local_30, &local_10);
		}

		if ((((intersectionDist != 1e+30f) && (local_30.y <= 0.64265704f)) &&
			((local_10.pActor_0x0 == (CActor*)0x0 || ((local_10.hitMaterialFlags & 0x800) != 0)))) &&
			(((local_10.hitMaterialFlags & 0x200) == 0 &&
				(local_30.x * g_xVector.x + local_30.y * g_xVector.y + local_30.z * g_xVector.z <= 0.0f)))) {
			this->field_0x14a8 = intersectionDist - 0.4f;

			if (pOutActor == (CActor**)0x0) {
				GripObject(local_10.pActor_0x0);
			}
			else {
				*pOutActor = local_10.pActor_0x0;
			}

			bDetectClimbCeiling = 1;
			this->field_0x1480 = local_30;
		}
	}
	else {
		bDetectClimbCeiling = 0;
	}

	return bDetectClimbCeiling;
}

bool CActorHeroPrivate::DetectClimbWall(int param_2, CActor** pOutActor, float* param_4)
{
	CCollision* pCVar1;
	CActor* pCVar2;
	CActor* pTieActor;
	bool bDetectClimbWall;
	bool bVar3;
	int iVar4;
	edF32VECTOR4* peVar5;
	float fVar6;
	float fVar7;
	float unaff_f20;
	float fVar8;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	S_DETECT_WALL_CFG local_20;
	_ray_info_out local_10;

	if (((this->field_0x1420 != 0) || (this->field_0x1454 == false)) || (iVar4 = EvolutionCanClimb(), iVar4 == 0)) {
		return false;
	}

	pCVar1 = this->pCollisionData;
	bDetectClimbWall = false;
	if (((pCVar1->flags_0x4 & 1) != 0) &&
		(fabs(pCVar1->aCollisionContact[0].location.y) < cosf(this->field_0x14b8))) {
		local_30.w = pCVar1->aCollisionContact[0].location.w;
		local_30.x = 0.0f - pCVar1->aCollisionContact[0].location.x;
		local_30.z = 0.0f - pCVar1->aCollisionContact[0].location.z;
		local_30.y = 0.0f;

		edF32Vector4SafeNormalize1Hard(&local_30, &local_30);

		bVar3 = true;

		if (0.0f <= local_30.x * this->rotationQuat.x + local_30.y * this->rotationQuat.y + local_30.z * this->rotationQuat.z) goto LAB_0013d298;
	}

	bVar3 = false;

LAB_0013d298:
	local_20.field_0x0 = 1.6f;
	fVar7 = 0.4f;
	local_20.field_0x4 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 1.0f;
	local_20.field_0x8 = this->field_0x14b8;

	if (param_2 == 0) {
		peVar5 = &eStack64;
	}
	else {
		peVar5 = &this->field_0x1470;
	}

	if (bVar3) {
		bDetectClimbWall = DetectWall(3, 0x40000100, &local_30, &local_20, peVar5, &eStack80, &local_10);
		if (((local_10.pActor_0x0 != (CActor*)0x0) && ((local_10.hitMaterialFlags & 0x400) == 0)) ||
			((local_10.hitMaterialFlags & 0x200) != 0)) {
			bDetectClimbWall = false;
		}

		if (bDetectClimbWall != false) {
			edF32Vector4SubHard(&eStack96, &this->currentLocation, &eStack80);
			eStack96.y = 0.0f;
			fVar8 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
			fVar6 = edF32Vector4DotProductHard(&eStack96, &this->field_0x1470);
			unaff_f20 = fVar6 - fVar8;
			if (fVar7 < unaff_f20) {
				bDetectClimbWall = false;
			}
		}
	}

	if (bDetectClimbWall == false) {
		bDetectClimbWall = DetectWall(3, 0x40000100, &this->rotationQuat, &local_20, peVar5, &eStack80, &local_10);
		if (((local_10.pActor_0x0 != (CActor*)0x0) && ((local_10.hitMaterialFlags & 0x400) == 0)) || ((local_10.hitMaterialFlags & 0x200) != 0)) {
			bDetectClimbWall = false;
		}

		if (bDetectClimbWall != false) {
			edF32Vector4SubHard(&eStack96, &this->currentLocation, &eStack80);
			eStack96.y = 0.0f;
			fVar8 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
			fVar6 = edF32Vector4DotProductHard(&eStack96, &this->field_0x1470);
			unaff_f20 = fVar6 - fVar8;
			if (fVar7 < unaff_f20) {
				bDetectClimbWall = false;
			}
		}
	}

	if (bDetectClimbWall != false) {
		if (param_2 == 0) {
			UpdateNormal(8.0f, &this->field_0x1470, &eStack64);
		}

		if (pOutActor == (CActor**)0x0) {
			GripObject(local_10.pActor_0x0);
		}
		else {
			*pOutActor = local_10.pActor_0x0;
		}

		if (param_4 != (float*)0x0) {
			*param_4 = unaff_f20;
		}
	}

	return bDetectClimbWall;
}

int CActorHeroPrivate::DetectClimbCeilingFromGrip(CActor** pOutActor, edF32VECTOR4* pPosition)
{
	int bClimbCeilingFromGrip;
	float fVar1;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	bClimbCeilingFromGrip = EvolutionCanClimb();
	if (bClimbCeilingFromGrip == 0) {
		bClimbCeilingFromGrip = 0;
	}
	else {
		fVar1 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1f;
		local_10.x = this->rotationQuat.x * fVar1 + this->currentLocation.x;
		local_10.z = this->rotationQuat.z * fVar1 + this->currentLocation.z;
		local_10.w = this->rotationQuat.w * fVar1 + this->currentLocation.w;
		local_10.y = (this->rotationQuat.y * fVar1 + this->currentLocation.y) - 0.2f;

		bClimbCeilingFromGrip = DetectClimbCeiling(&local_10, pOutActor);
		if (bClimbCeilingFromGrip != 0) {
			local_20.x = this->currentLocation.x;
			local_20.z = this->currentLocation.z;
			local_20.w = this->currentLocation.w;
			local_20.y = this->currentLocation.y + 1.16f;

			CCollisionRay CStack64 = CCollisionRay(((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1f, &local_20, &this->rotationQuat);
			fVar1 = CStack64.Intersect(3, this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			if (fVar1 != 1e+30f) {
				bClimbCeilingFromGrip = 0;
			}
		}

		if ((pPosition != (edF32VECTOR4*)0x0) && (bClimbCeilingFromGrip != 0)) {
			*pPosition = local_10;
			pPosition->y = pPosition->y + this->field_0x14a8;
		}
	}

	return bClimbCeilingFromGrip;
}

// Should be in: D:/Projects/b-witch/ActorHero_GripClimb.cpp
void CActorHeroPrivate::SetGripState()
{
	int curState;

	if (this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y < -8.0f) {
		SetState(STATE_HERO_GRIP_C, 0xffffffff);
	}
	else {
		curState = this->actorState;
		if (((curState == STATE_HERO_JUMP_2_3_STAND) || (curState == STATE_HERO_JUMP_2_3_RUN)) || (curState == 199)) {
			SetState(STATE_HERO_GRIP_GRAB, 0xffffffff);
		}
		else {
			SetState(STATE_HERO_GRIP_B, 0xffffffff);
		}
	}
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_GripClimb.cpp
void CActorHeroPrivate::UngripAllObjects()
{
	CActor* pCVar1;
	CActor* pTieActor;
	bool bVar2;

	pCVar1 = this->pGrippedActor;

	if (pCVar1 != (CActor*)0x0) {
		pTieActor = this->pTiedActor;

		if (pCVar1 == pTieActor) {
			TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);

			if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) && (bVar2 = CCollision::IsVertexAboveAndAgainstObbTree(&(this->pCollisionData)->highestVertex,
				pTieActor->pCollisionData->pObbTree), bVar2 != false)) {
				TieToActor(pTieActor, 0, 0, (edF32MATRIX4*)0x0);
			}
		}

		this->pGrippedActor = (CActor*)0x0;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_GripClimb.cpp
bool CActorHeroPrivate::CanBounceAgainstWall()
{
	CCollision* pCVar1;
	bool bVar2;
	int iVar3;
	long lVar4;
	edF32VECTOR4* v0;
	float local_20;
	float local_1c;
	float local_18;
	float local_10;
	float local_c;
	float local_8;

	if ((this->field_0x1455 == 0) && (iVar3 = EvolutionBounceCanJump(), iVar3 != 0)) {
		IMPLEMENTATION_GUARD(
			pCVar1 = this->pCollisionData;
		bVar2 = false;
		if ((pCVar1->flags_0x4 & 1) != 0) {
			if (edFCosinus[(int)(fabs(this->field_0x14c4 * 1303.797) + 0.5) & 0x1fff] <=
				fabs(pCVar1->aCollisionContact[0].location.y)) {
				bVar2 = false;
			}
			else {
				local_10 = 0.0;
				local_c = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.2;
				local_8 = this->field_0x14c4;
				local_20 = *(float*)&this->field_0x14c8;
				local_1c = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.2;
				local_18 = this->field_0x14c4;
				lVar4 = DetectWall(this, 3, 0x40000080, (float*)&this->rotationQuat,
					&local_10, 0, 0, 0);
				if ((lVar4 == 0) ||
					(lVar4 = DetectWall(this, 3, 0x40000080, (float*)&this->rotationQuat,
						&local_20, 0, 0, 0), lVar4 == 0)) {
					bVar2 = false;
				}
				else {
					v0 = &this->bounceLocation;
					this->bounceLocation.x = pCVar1->aCollisionContact[0].location.x;
					this->bounceLocation.y = 0.0;
					this->bounceLocation.z = pCVar1->aCollisionContact[0].location.z;
					this->bounceLocation.w = 0.0;
					edF32Vector4NormalizeHard(v0, v0);
					bVar2 = true;
				}
			}
		})
	}
	else {
		bVar2 = false;
	}

	return bVar2;
}

uint CActorHeroPrivate::CanGrip(int param_2, edF32VECTOR4* pRotation)
{
	CCollision* pCVar1;
	CActor* pCVar2;
	CActor* pTieActor;
	bool bVar3;
	uint uVar4;
	uint uVar5;
	float fVar6;
	edF32VECTOR4 local_10;

	if (this->field_0x1420 != 0) {
		return 0;
	}

	pCVar1 = this->pCollisionData;
	uVar4 = 0;

	if (((pCVar1->flags_0x4 & 1) != 0) && (fabs(pCVar1->aCollisionContact[0].location.y) < cosf(this->field_0x14b0))) {
		local_10.w = pCVar1->aCollisionContact[0].location.w;
		local_10.x = 0.0f - pCVar1->aCollisionContact[0].location.x;
		local_10.z = 0.0f - pCVar1->aCollisionContact[0].location.z;
		local_10.y = 0.0;
		edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
		bVar3 = true;

		if (0.0f <= local_10.x * pRotation->x + local_10.y * pRotation->y + local_10.z * pRotation->z) goto LAB_0013bd58;
	}

	bVar3 = false;
LAB_0013bd58:
	if (bVar3) {
		uVar4 = DetectGripEdge(1, &this->currentLocation, &local_10, (float*)0x0, (float*)0x0, &this->bounceLocation);
	}

	if (uVar4 == 0) {
		uVar4 = DetectGripEdge(0, &this->currentLocation, pRotation, (float*)0x0, (float*)0x0, &this->bounceLocation);
	}

	if ((uVar4 != 0) && (param_2 != 0)) {
		fVar6 = this->dynamic.speed;
		bVar3 = this->dynamic.rotationQuat.x * fVar6 * this->bounceLocation.x + this->dynamic.rotationQuat.y * fVar6 * this->bounceLocation.y +
			this->dynamic.rotationQuat.z * fVar6 * this->bounceLocation.z < 0.0f;

		uVar4 = (uint)bVar3;

		if ((!bVar3) && (pCVar2 = this->pGrippedActor, pCVar2 != (CActor*)0x0)) {
			pTieActor = this->pTiedActor;
			if (pCVar2 == pTieActor) {
				TieToActor((CActor*)0x0, 0, 0x1, (edF32MATRIX4*)0x0);
				if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
					(uVar5 = CCollision::IsVertexAboveAndAgainstObbTree(&(this->pCollisionData)->highestVertex, pTieActor->pCollisionData->pObbTree), uVar5 != 0)) {
					TieToActor(pTieActor, 0, 0, (edF32MATRIX4*)0x0);
				}
			}
			this->pGrippedActor = (CActor*)0x0;
		}
	}

	return uVar4;
}

int CActorHeroPrivate::DetectGripEdge(int param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, float* param_5, float* param_6, edF32VECTOR4* param_7)
{
	CCollision* pCVar1;
	CActor* pCVar2;
	CActor* pTieActor;
	uint uVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	CCollisionRay CStack176;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	CCollisionRay CStack96;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_ray_info_out local_10;

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge param_2: {}, param_3: {}, param_4: {}", param_2, param_3->ToString(), param_4->ToString());

	iVar4 = 0;

	fVar5 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1;

	local_20.x = param_4->x * fVar5 + param_3->x;
	local_20.y = param_3->y + 1.75f;
	local_20.z = param_4->z * fVar5 + param_3->z;
	local_20.w = param_4->w * fVar5 + param_3->w;

	local_40.x = 0.0;
	local_40.y = -1.0f;
	local_40.z = 0.0;
	local_40.w = 0.0;

	CStack96 = CCollisionRay(0.35f, &local_20, &local_40);

	fVar5 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, 0x40000010, &local_30, &local_10);

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar5: {}", fVar5);

	if ((((fVar5 == 1e+30f) || (fVar5 <= 0.01f)) || (local_30.y < cosf(this->field_0x14b4))) ||
		(0.0f < local_30.x * local_40.x + local_30.y * local_40.y + local_30.z * local_40.z)) {
		fVar5 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.3f;

		local_20.x = param_4->x * fVar5 + param_3->x;
		local_20.y = param_3->y + 1.75f;
		local_20.z = param_4->z * fVar5 + param_3->z;
		local_20.w = param_4->w * fVar5 + param_3->w;

		fVar5 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, 0x40000010, &local_30, &local_10);
		ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar5: {}", fVar5);
	}

	if (((fVar5 != 1e+30f) && (0.01f < fVar5)) && (((local_10.hitMaterialFlags & 0x1200) == 0 && ((cosf(this->field_0x14b4) <= local_30.y &&
		(local_30.x * local_40.x + local_30.y * local_40.y + local_30.z * local_40.z <= 0.0f)))))) {
		local_90.x = param_3->x;
		fVar5 = local_20.y + local_40.y * fVar5;
		local_90.y = fVar5 - 0.01f;
		local_90.z = param_3->z;
		local_90.w = 1.0f;
		local_70.x = param_4->x;
		local_70.y = 0.0f;
		local_70.z = param_4->z;
		local_70.w = 0.0f;

		if (param_4->y != 0.0f) {
			edF32Vector4NormalizeHard(&local_70, &local_70);
		}

		CStack176 = CCollisionRay(((this->pCollisionData)->pObbPrim->field_0x90).z + 0.5f, &local_90, &local_70);
		fVar6 = CStack176.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, 0x40000010, &local_80, (_ray_info_out*)0x0);

		ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar6: {}", fVar6);

		if ((((fVar6 == 1e+30f) || (fVar6 <= 0.01f)) ||
			(cosf(this->field_0x14b0) <= fabs(local_80.y))) ||
			(0.0f < local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z)) {
			local_90.y = fVar5 - 0.1f;
			fVar6 = CStack176.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, 0x40000010, &local_80, (_ray_info_out*)0x0);
		}

		if (((fVar6 != 1e+30f) && (0.01 < fVar6)) &&
			(((local_10.hitMaterialFlags & 0x1200) == 0 &&
				((fabs(local_80.y) < cosf(this->field_0x14b0) && (local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z <= 0.0f)))))) {
			fVar7 = ((this->pCollisionData)->pObbPrim->field_0x90).z - 0.2f;

			local_20.x = param_4->x * fVar7 + param_3->x;
			local_20.y = param_3->y + 1.8f;
			local_20.z = param_4->z * fVar7 + param_3->z;
			local_20.w = param_4->w * fVar7 + param_3->w;

			fVar7 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar7: {}", fVar7);


			if (fVar7 == 1e+30f) {
				if (param_2 != 0) {
					pCVar1 = this->pCollisionData;
					if (((pCVar1->flags_0x4 & 1) != 0) && (fabs(pCVar1->aCollisionContact[0].location.y) < cosf(this->field_0x14b0))) {
						local_80.xyz = pCVar1->aCollisionContact[0].location.xyz;
					}
				}

				local_80.y = 0.0f;

				edF32Vector4NormalizeHard(param_7, &local_80);

				if (param_5 != (float*)0x0) {
					*param_5 = fVar5 - (param_3->y + 1.7f);
				}

				if (param_6 != (float*)0x0) {
					*param_6 = fVar6 - ((this->pCollisionData)->pObbPrim->field_0x90).z;
				}

				GripObject(local_10.pActor_0x0);

				iVar4 = 1;
			}
		}
	}

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge return {}", iVar4);

	return iVar4;
}

bool CActorHeroPrivate::DetectMultipleWallBounces()
{
	int iVar1;
	edF32VECTOR4* v0;
	undefined4 uVar2;
	float fVar3;
	undefined4 uVar4;
	undefined4 uVar5;
	CCollisionRay CStack64;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	iVar1 = EvolutionBounceCanJump();
	if (iVar1 != 0) {
		IMPLEMENTATION_GUARD();
	}

	return false;
}

void CActorHeroPrivate::GetClimbVector(edF32VECTOR4* pOutVector, edF32VECTOR4* pInVector)
{
	float fVar6 = this->field_0x1470.x;
	float fVar7 = this->field_0x1470.y;
	float fVar8 = this->field_0x1470.z;
	float fVar9 = gF32Vector4UnitY.y * fVar8 - fVar7 * gF32Vector4UnitY.z;
	float fVar10 = gF32Vector4UnitY.z * fVar6 - fVar8 * gF32Vector4UnitY.x;
	float fVar11 = gF32Vector4UnitY.x * fVar7 - fVar6 * gF32Vector4UnitY.y;
	fVar7 = this->field_0x1470.x;
	fVar8 = this->field_0x1470.y;
	float fVar5 = this->field_0x1470.z;
	fVar6 = -pInVector->z;
	float fVar2 = pInVector->y;
	float fVar1 = pInVector->x;
	pOutVector->x = (fVar8 * fVar11 - fVar10 * fVar5) * fVar2 + fVar9 * fVar1 + this->field_0x1470.x * fVar6;
	pOutVector->y = (fVar5 * fVar9 - fVar11 * fVar7) * fVar2 + fVar10 * fVar1 + this->field_0x1470.y * fVar6;
	pOutVector->z = (fVar7 * fVar10 - fVar9 * fVar8) * fVar2 + fVar11 * fVar1 + this->field_0x1470.z * fVar6;
	pOutVector->w = in_vf0x * fVar2 + in_vf0x * fVar1 + this->field_0x1470.w * fVar6;
}

bool CActorHeroPrivate::CheckClimbZone(edF32VECTOR4* pInVector)
{
	edF32VECTOR4 local_c0;
	GetClimbVector(&local_c0, pInVector);

	int iVar7 = 0;
	int iVar8 = 0;
	while (true) {
		S_ZONE_STREAM_REF* pSVar3 = this->pClimbZoneStreamRef;
		if (pSVar3 == (S_ZONE_STREAM_REF*)0x0) {
			iVar8 = 0;
		}
		else {
			iVar8 = pSVar3->entryCount;
		}

		if (iVar8 <= iVar7) {
			return false;
		}

		uint uVar9;
		ed_zone_3d* peVar2 = (this->pClimbZoneStreamRef)->aEntries[iVar7].Get();
		if ((peVar2 != (ed_zone_3d*)0x0) && (uVar9 = edEventComputeZoneAgainstVertex(CScene::ptable.g_EventManager_006f5080->activeChunkId, peVar2, &local_c0, 0), (uVar9 & 1) != 0))
			break;

		iVar7 = iVar7 + 1;
	}

	return true;
}

bool CActorHeroPrivate::SetNextClimbState(int param_2, int param_3)
{
	ed_zone_3d* peVar1;
	S_ZONE_STREAM_REF* pSVar2;
	CEventManager* pCVar3;
	bool bVar4;
	Timer* pTVar5;
	int iVar6;
	int iVar7;
	uint uVar8;
	edF32VECTOR4* v0;
	bool bVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float in_vf0x;
	float fVar14;
	float fVar15;
	edF32VECTOR4 eStack304;
	edF32VECTOR4 local_120;
	edF32VECTOR4 local_110;
	edF32VECTOR4 local_f0;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CPlayerInput* pInput;

	local_30 = this->field_0x1470;

	if (local_30.y != 0.0f) {
		local_30.y = 0.0f;
		edF32Vector4NormalizeHard(&local_30, &local_30);
	}

	local_20.y = 0.0f;
	local_20.w = 0.0f;
	local_20.x = -local_30.z;
	local_20.z = local_30.x;
	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		local_40 = gF32Vector4Zero;
	}
	else {
		edF32Vector4ScaleHard(pInput->aAnalogSticks[0].magnitude, &local_40, &pInput->lAnalogStick);
	}
	fVar10 = edF32Vector4DotProductHard(&local_40, &local_20);
	fVar10 = -fVar10;
	fVar11 = edF32Vector4DotProductHard(&local_40, &local_30);
	pCVar3 = CScene::ptable.g_EventManager_006f5080;
	pInput = this->pPlayerInput;
	fVar11 = -fVar11;
	bVar9 = true;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		uVar8 = 0;
	}
	else {
		uVar8 = pInput->pressedBitfield & 0x10;
	}

	if (uVar8 == 0) {
		if (0.6f <= fVar10) {
			if ((param_3 == 0) && (param_2 == 0)) {
				SetState(0xb8, 0xffffffff);
			}
			else {
				if (param_2 == 0) {
					SetState(0xb6, 0xffffffff);
				}
				else {
					local_80.x = 0.2f;
					local_80.y = 0.0f;
					local_80.z = 0.0f;
					local_80.w = 0.0f;
					if (this->field_0x1454 == false) {
						bVar4 = true;
					}
					else
					{
						bVar4 = CheckClimbZone(&local_80);
					}

					bVar9 = true;
					if (bVar4) {
						if (param_3 == 0) {
							SetState(0xb0, 0xffffffff);
						}
						else {
							SetState(0xaf, 0xffffffff);
						}
					}
					else {
						SetState(0xab, 0xffffffff);
					}
				}
			}
		}
		else {
			if (fVar10 <= -0.6f) {
				if ((param_3 == 0) || (param_2 != 0)) {
					if (param_2 == 0) {
						SetState(0xb7, 0xffffffff);
					}
					else {
						local_b0.x = -0.2f;
						local_b0.y = 0.0f;
						local_b0.z = 0.0f;
						local_b0.w = 0.0f;
						if (this->field_0x1454 == false) {
							bVar4 = true;
						}
						else
						{
							bVar4 = CheckClimbZone(&local_b0);
						}

						bVar9 = true;
						if (bVar4) {
							if (param_3 == 0) {
								SetState(0xad, 0xffffffff);
							}
							else {
								SetState(0xae, 0xffffffff);
							}
						}
						else {
							SetState(0xac, 0xffffffff);
						}
					}
				}
				else {
					SetState(0xb5, 0xffffffff);
				}
			}
			else {
				if (0.6f <= fVar11) {
					local_e0.x = 0.0f;
					local_e0.z = 0.0f;
					local_e0.y = 0.2f;
					local_e0.w = 0.0f;
					if (this->field_0x1454 == false) {
						bVar4 = true;
					}
					else
					{
						bVar4 = CheckClimbZone(&local_e0);
					}
					bVar9 = true;
					if (bVar4) {
						if (param_3 == 0) {
							SetState(0xb2, 0xffffffff);
						}
						else {
							SetState(0xb1, 0xffffffff);
						}
					}
					else {
						bVar9 = false;
					}
				}
				else {
					if (fVar11 <= -0.6) {
						local_110.x = 0.0f;
						local_110.z = 0.0f;
						local_110.y = -0.2f;
						local_110.w = 0.0f;
						if (this->field_0x1454 == false) {
							bVar4 = true;
						}
						else
						{
							bVar4 = CheckClimbZone(&local_110);
						}

						bVar9 = true;
						if (bVar4) {
							if (param_3 == 0) {
								SetState(0xb4, 0xffffffff);
							}
							else {
								SetState(0xb3, 0xffffffff);
							}
						}
						else {
							bVar9 = false;
						}
					}
					else {
						bVar9 = false;
					}
				}
			}
		}
	}
	else {
		this->dynamic.speed = 0.0f;

		local_50.x = this->rotationQuat.x;
		local_50.z = this->rotationQuat.z;
		local_50.w = this->rotationQuat.w;
		local_50.y = 0.0f;

		edF32Vector4NormalizeHard(&local_50, &local_50);
		this->rotationQuat = local_50;

		local_10.x = this->field_0x1470.x;
		local_10.y = 0.0f;
		local_10.z = this->field_0x1470.z;
		local_10.w = 0.0f;
		edF32Vector4NormalizeHard(&local_10, &local_10);

		if (0.6f <= fabs(fVar10)) {
			this->dynamic.speed = 6.0f;
			local_60.y = 500.0f;
			local_60.x = 0.0f;
			local_60.z = 0.0f;
			local_60.w = 0.0f;
			pTVar5 = GetTimer();
			edF32Vector4ScaleHard(0.02f / pTVar5->cutsceneDeltaTime, &eStack304, &local_60);
			v0 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(v0, v0, &eStack304);
			fVar11 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar11;
			this->field_0xf00 = this->currentLocation;

			if (0.0f < fVar10) {
				this->dynamic.rotationQuat.x = local_10.z;
				this->dynamic.rotationQuat.y = 0.0f;
				this->dynamic.rotationQuat.z = -local_10.x;
				SetState(0xba, 0xffffffff);
			}
			else {
				this->dynamic.rotationQuat.x = -local_10.z;
				this->dynamic.rotationQuat.y = 0.0f;
				this->dynamic.rotationQuat.z = local_10.x;
				SetState(0xb9, 0xffffffff);
			}
		}
		else {
			if (fVar11 < -0.6) {
				this->bounceLocation = local_10;

				bVar4 = DetectMultipleWallBounces();
				if (bVar4 == false) {
					SetState(0xcf, 0xffffffff);
				}
				else {
					SetState(0xcd, 0xffffffff);
				}
			}
			else {
				if (0.6 < fVar11) {
					this->field_0x1455 = 0;
					SetJumpCfg(0.1, this->runSpeed, this->field_0x1158, this->field_0x1150, this->field_0x1154, 0, (edF32VECTOR4*)0x0);
					SetState(0x79, 0xffffffff);
				}
				else {
					SetState(ChooseStateFall(0), 0xffffffff);
				}
			}
		}
	}

	return bVar9;
}

void CActorHeroPrivate::StateHeroClimbStand(int param_2, int param_3)
{
	bool bVar1;
	int iVar3;
	uint uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	float local_4;
	CCollision* pCol;
	CPlayerInput* pInput;

	local_4 = 0.0f;
	bVar1 = DetectClimbWall(0, (CActor**)0x0, &local_4);
	local_20.z = 3.0f;
	if (fabs(local_4) < GetTimer()->cutsceneDeltaTime * 3.0f) {
		local_20.z = local_4 / GetTimer()->cutsceneDeltaTime;
	}
	else {
		if (local_4 <= 0.0f) {
			local_20.z = -3.0f;
		}
	}

	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 | 0x40;

	local_20.x = 0.0f;
	local_20.y = 0.0f;
	local_20.w = 0.0f;

	GetClimbVector(&local_30, &local_20);

	fVar6 = edF32Vector4SafeNormalize0Hard(&local_30, &local_30);
	if (fVar6 != 0.0f) {
		this->dynamic.rotationQuat = local_30;
	}

	this->dynamic.speed = fVar6;
	local_40.w = this->field_0x1470.w;
	local_40.x = 0.0f - this->field_0x1470.x;
	local_40.y = 0.0f - this->field_0x1470.y;
	local_40.z = 0.0f - this->field_0x1470.z;
	SV_UpdateOrientation(this->field_0x1040, &local_40);
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 & 0xffffffbf;
	if (bVar1 == false) {
		this->dynamic.speed = 0.0;
		local_50.x = this->rotationQuat.x;
		local_50.z = this->rotationQuat.z;
		local_50.w = this->rotationQuat.w;
		local_50.y = 0.0f;

		edF32Vector4NormalizeHard(&local_50, &local_50);

		this->rotationQuat = local_50;

		iVar3 = ChooseStateFall(0);
		SetState(iVar3, 0xffffffff);
	}
	else {
		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar4 = 0;
		}
		else {
			uVar4 = pInput->pressedBitfield & 0x10;
		}

		if ((uVar4 == 0) && (param_3 != 0)) {
			pInput = this->pPlayerInput;
			if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar6 = 0.0f;
			}
			else {
				fVar6 = pInput->aAnalogSticks[0].magnitude;
			}

			if (fVar6 <= 0.6f) {
				this->effort = 0.0f;

				if (param_2 == 0) {
					SetState(0xaa, 0xffffffff);
				}
				else {
					SetState(0xa9, 0xffffffff);
				}
			}
		}
		else {
			SetNextClimbState(0, param_2);
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroClimbMove(float param_1, float param_2, int param_4, int param_5, int param_6)
{
	CAnimation* pAnimationController;
	ed_zone_3d* pZone;
	S_ZONE_STREAM_REF* pSVar1;
	CPlayerInput* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	CEventManager* pCVar5;
	bool bVar6;
	uint uVar8;
	int iVar9;
	int iVar10;
	float puVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	float local_4;

	pAnimationController = this->pAnimationController;
	IncreaseEffort(1.0f);
	local_4 = 0.0;
	bVar6 = DetectClimbWall(0, (CActor**)0x0, &local_4);
	local_20.z = 3.0f;
	if (fabs(local_4) < GetTimer()->cutsceneDeltaTime * 3.0f) {
		local_20.z = local_4 / GetTimer()->cutsceneDeltaTime;
	}
	else {
		if (local_4 <= 0.0f) {
			local_20.z = -3.0f;
		}
	}

	local_20.x = param_1;
	local_20.y = param_2;
	local_20.w = 0.0f;
	GetClimbVector(&local_30, &local_20);

	fVar13 = edF32Vector4SafeNormalize0Hard(&local_30, &local_30);
	if (fVar13 != 0.0f) {
		this->dynamic.rotationQuat = local_30;
	}
	this->dynamic.speed = fVar13;

	local_40.w = this->field_0x1470.w;
	local_40.x = 0.0f - this->field_0x1470.x;
	local_40.y = 0.0f - this->field_0x1470.y;
	local_40.z = 0.0f - this->field_0x1470.z;

	SV_UpdateOrientation(this->field_0x1040, &local_40);

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);
	pCVar5 = CScene::ptable.g_EventManager_006f5080;
	if (param_1 != 0.0f) {
		if (0.0f <= param_1) {
			puVar11 = 1.0f;
		}
		else {
			puVar11 = -1.0f;
		}

		param_1 = puVar11 * 0.2f;
	}

	fVar13 = 0.0f;
	if (param_2 != 0.0f) {
		if (0.0f <= param_2) {
			puVar11 = 1.0f;
		}
		else {
			puVar11 = -1.0f;
		}

		fVar13 = 0.2f;
		param_2 = puVar11 * 0.2f;
	}

	if (this->field_0x1454 == false) {
		bVar4 = true;
	}
	else {
		local_60.x = param_1;
		local_60.y = param_2;
		local_60.z = 0.0f;
		local_60.w = 0.0f;

		bVar4 = CheckClimbZone(&local_60);
	}

	if (bVar4) {
		if ((param_2 <= 0.0) ||
			(iVar9 = CActorHeroPrivate::DetectGripEdge(1, &this->currentLocation, &this->rotationQuat, (float*)0x0 , (float*)0x0, &this->bounceLocation), iVar9 == 0)) {
			fVar13 = 0.0f;

			if ((param_2 <= 0.0f) || (iVar9 = DetectClimbCeiling(&this->currentLocation, (CActor**)0x0), iVar9 == 0)) {
				if (bVar6 == false) {
					this->dynamic.speed = 0.0;
					local_a0.x = this->rotationQuat.x;
					local_a0.z = this->rotationQuat.z;
					local_a0.w = this->rotationQuat.w;
					local_a0.y = 0.0f;

					edF32Vector4NormalizeHard(&local_a0, &local_a0);
					this->rotationQuat = local_a0;

					SetState(ChooseStateFall(0), 0xffffffff);
				}
				else {
					pCVar2 = this->pPlayerInput;
					if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						uVar8 = 0;
					}
					else {
						uVar8 = pCVar2->pressedBitfield & 0x10;
					}

					if (uVar8 == 0) {
						peVar3 = (pAnimationController->anmBinMetaAnimator).aAnimData;
						if (peVar3->animPlayState == 1) {
							if (param_5 == 0) {
								if (param_6 == 0) {
									if ((peVar3->currentAnimDesc).animType ==
										pAnimationController->currentAnimType_0x30) {
										bVar6 = (peVar3->field_0xcc & 2) != 0;
									}
									else {
										bVar6 = false;
									}
								}
								else {
									if ((peVar3->currentAnimDesc).animType ==
										pAnimationController->currentAnimType_0x30) {
										bVar6 = (peVar3->field_0xcc & 8) != 0;
									}
									else {
										bVar6 = false;
									}
								}
							}
							else {
								this->pAnimationController->GetAnimType_00242330(0);
								fVar14 = this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 0);
								bVar6 = fVar13 * 0.5f <= fVar14;
							}

							if (bVar6) {
								if ((0.0f <= param_2) || (((this->pCollisionData)->flags_0x4 & 2) == 0)) {
									bVar6 = SetNextClimbState(1, param_4);
									if (bVar6 == false) {
										if (param_4 == 0) {
											SetState(0xaa, 0xffffffff);
										}
										else {
											SetState(0xa9, 0xffffffff);
										}
									}
								}
								else {
									this->dynamic.speed = 0.0f;
									local_b0.x = this->rotationQuat.x;
									local_b0.z = this->rotationQuat.z;
									local_b0.w = this->rotationQuat.w;
									local_b0.y = 0.0f;
									edF32Vector4NormalizeHard(&local_b0, &local_b0);
									this->rotationQuat = local_b0;
									SetState(0x73, 0xffffffff);
								}
							}
						}
					}
					else {
						SetNextClimbState(1, param_4);
					}
				}
			}
			else {
				this->dynamic.speed = 0.0f;
				local_90.x = this->rotationQuat.x;
				local_90.z = this->rotationQuat.z;
				local_90.w = this->rotationQuat.w;
				local_90.y = 0.0f;
				edF32Vector4NormalizeHard(&local_90, &local_90);
				this->rotationQuat = local_90;
				SetState(0x110, 0xffffffff);
			}
		}
		else {
			this->dynamic.speed = 0.0f;
			local_80.x = this->rotationQuat.x;
			local_80.z = this->rotationQuat.z;
			local_80.w = this->rotationQuat.w;
			local_80.y = 0.0f;
			edF32Vector4NormalizeHard(&local_80, &local_80);
			this->rotationQuat = local_80;

			if (this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y < -8.0f) {
				SetState(0xbd, 0xffffffff);
			}
			else {
				iVar9 = this->actorState;
				if (((iVar9 == 0x79) || (iVar9 == 0x7c)) || (iVar9 == 199)) {
					SetState(0xca, 0xffffffff);
				}
				else {
					SetState(0xbc, 0xffffffff);
				}
			}
		}
	}
	else {
		pAnimationController->GetAnimType_00242330(0);

		if (this->timeInAir < fVar13 * 0.5f) {
			param_4 = param_4 != 0 ^ 1;
		}

		if (param_4 == 0) {
			SetState(0xaa, 0xffffffff);
		}
		else {
			SetState(0xa9, 0xffffffff);
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroClimbJump()
{
	CActor* pCVar1;
	CActor* pTieActor;
	bool bVar2;
	int iVar3;
	uint uVar4;
	edF32VECTOR4* peVar5;
	float fVar6;
	edF32VECTOR4 local_10;
	CAnimation* pAnim;
	CCollision* pCol;
	edAnmLayer* pLayer;

	pAnim = this->pAnimationController;
	pCol = this->pCollisionData;

	IncreaseEffort(2.0f);

	MoveInAir(0.1f, 6.0f, this->field_0x1158, this->airNoInputSpeed, 0.0f);

	ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

	if ((pCol->flags_0x4 & 2) != 0) {
		Landing();

		iVar3 = ChooseStateLanding(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y);
		SetState(iVar3, 0xffffffff);
		return;
	}

	peVar5 = &this->rotationQuat;
	if (this->field_0x1420 != 0) {
		uVar4 = 0;
		goto LAB_00139d58;
	}

	pCol = this->pCollisionData;
	uVar4 = 0;

	if (((pCol->flags_0x4 & 1) == 0) ||
		(cosf(this->field_0x14b0) <= fabs(pCol->aCollisionContact[0].location.y))) {
	LAB_00139bf8:
		bVar2 = false;
	}
	else {
		local_10.w = pCol->aCollisionContact[0].location.w;
		local_10.x = 0.0f - pCol->aCollisionContact[0].location.x;
		local_10.z = 0.0f - pCol->aCollisionContact[0].location.z;
		local_10.y = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
		bVar2 = true;
		if (local_10.x * peVar5->x +
			local_10.y * this->rotationQuat.y +
			local_10.z * this->rotationQuat.z < 0.0f)
			goto LAB_00139bf8;
	}

	if (bVar2) {
		uVar4 = DetectGripEdge(1, &this->currentLocation, &local_10, (float*)0x0, (float*)0x0, &this->bounceLocation);
	}

	if (uVar4 == 0) {
		uVar4 = DetectGripEdge(0, &this->currentLocation, peVar5, (float*)0x0, (float*)0x0, &this->bounceLocation);
	}

	if (uVar4 != 0) {
		fVar6 = this->dynamic.speed;
		bVar2 = this->dynamic.rotationQuat.x * fVar6 * this->bounceLocation.x + this->dynamic.rotationQuat.y * fVar6 *
			this->bounceLocation.y + this->dynamic.rotationQuat.z * fVar6 * this->bounceLocation.z < 0.0f;

		uVar4 = (uint)bVar2;
		if (!bVar2) {
			UngripAllObjects();
		}
	}
LAB_00139d58:
	if (uVar4 == 0) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(ChooseStateFall(0), 0xffffffff);
		}
	}
	else {
		if (this->dynamic.linearAcceleration *
			this->dynamic.velocityDirectionEuler.y < -8.0f) {
			SetState(STATE_HERO_GRIP_C, 0xffffffff);
		}
		else {
			iVar3 = this->actorState;
			if (((iVar3 == STATE_HERO_JUMP_2_3_STAND) || (iVar3 == STATE_HERO_JUMP_2_3_RUN)) || (iVar3 == STATE_HERO_JUMP_2_3_GRIP)) {
				SetState(STATE_HERO_GRIP_GRAB, 0xffffffff);
			}
			else {
				SetState(STATE_HERO_GRIP_B, 0xffffffff);
			}
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroGrip(float param_1, int nextState, int param_4)
{
	CCollision* pCollision;
	CPlayerInput* pPlayerInput;
	CAnimation* pAnim;
	edAnmLayer* pAnmLayer;
	bool bVar5;
	int iVar7;
	undefined4 uVar8;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 lAnalogStick;
	edF32VECTOR4 normalizedBounceLocation;
	edF32VECTOR4 invertedBounce2D;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	float local_20;
	float local_1c;
	float local_18;
	undefined4 local_14;
	float local_8;
	float local_4;

	IncreaseEffort(1.0f);

	local_8 = 0.0f;
	local_4 = 0.0f;

	pCollision = this->pCollisionData;
	if (((pCollision->flags_0x4 & 1) != 0) && (fabs(pCollision->aCollisionContact[0].location.y) < cosf(this->field_0x14b0))) {
		local_30.w = pCollision->aCollisionContact[0].location.w;
		local_30.x = 0.0f - pCollision->aCollisionContact[0].location.x;
		local_30.z = 0.0f - pCollision->aCollisionContact[0].location.z;
		local_30.y = 0.0f;

		edF32Vector4SafeNormalize1Hard(&local_30, &local_30);

		bVar5 = true;
		if (0.0f <= local_30.x * this->rotationQuat.x + local_30.y * this->rotationQuat.y + local_30.z * this->rotationQuat.z) goto LAB_00139008;
	}

	bVar5 = false;
LAB_00139008:
	iVar7 = 0;
	if (bVar5) {
		iVar7 = DetectGripEdge(1, &this->currentLocation, &local_30, &local_4, &local_8, &eStack96);
	}

	if (iVar7 == 0) {
		iVar7 = DetectGripEdge(0, &this->currentLocation, &this->rotationQuat, &local_4, &local_8, &eStack96);
	}

	if (iVar7 == 0) {
		local_70.w = this->bounceLocation.w;
		local_70.xyz = 0.0f - this->bounceLocation.xyz;
		iVar7 = DetectGripEdge(0, &this->currentLocation, &local_70, &local_4, &local_8, &eStack96);
	}

	if (iVar7 == 0) {
		local_20 = 0.0f;
	}
	else {
		local_20 = param_1;
		UpdateNormal(8.0f, &this->bounceLocation, &eStack96);
		param_1 = local_20;
	}

	normalizedBounceLocation = this->bounceLocation;

	if (normalizedBounceLocation.y != 0.0f) {
		normalizedBounceLocation.y = 0.0f;
		edF32Vector4NormalizeHard(&normalizedBounceLocation, &normalizedBounceLocation);
	}

	invertedBounce2D.y = 0.0f;
	invertedBounce2D.w = 0.0f;
	invertedBounce2D.x = -normalizedBounceLocation.z;
	invertedBounce2D.z = normalizedBounceLocation.x;

	pPlayerInput = this->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		lAnalogStick = gF32Vector4Zero;
	}
	else {
		edF32Vector4ScaleHard(pPlayerInput->aAnalogSticks[PAD_STICK_LEFT].magnitude, &lAnalogStick, &pPlayerInput->lAnalogStick);
	}

	fVar10 = edF32Vector4DotProductHard(&lAnalogStick, &invertedBounce2D);
	fVar11 = edF32Vector4DotProductHard(&lAnalogStick, &normalizedBounceLocation);

	fVar11 = -fVar11;
	local_1c = 3.0f;

	if (fabs(local_4) < GetTimer()->cutsceneDeltaTime * 3.0f) {
		local_1c = local_4 / GetTimer()->cutsceneDeltaTime;
	}
	else {
		if (local_4 <= 0.0f) {
			local_1c = -3.0f;
		}
	}

	local_18 = 3.0f;

	if (fabs(local_8) < GetTimer()->cutsceneDeltaTime * 3.0f) {
		local_18 = local_8 / GetTimer()->cutsceneDeltaTime;
	}
	else {
		if (local_8 <= 0.0f) {
			local_18 = -3.0f;
		}
	}

	local_14 = 0;
	fVar13 = this->bounceLocation.x;
	fVar14 = this->bounceLocation.y;
	fVar15 = this->bounceLocation.z;
	fVar16 = g_xVector.y * fVar15 - fVar14 * g_xVector.z;
	fVar17 = g_xVector.z * fVar13 - fVar15 * g_xVector.x;
	fVar18 = g_xVector.x * fVar14 - fVar13 * g_xVector.y;
	fVar14 = this->bounceLocation.x;
	fVar15 = this->bounceLocation.y;
	fVar12 = this->bounceLocation.z;
	fVar13 = -local_18;
	local_b0.x = (fVar15 * fVar18 - fVar17 * fVar12) * local_1c + fVar16 * local_20 + this->bounceLocation.x * fVar13;
	local_b0.y = (fVar12 * fVar16 - fVar18 * fVar14) * local_1c + fVar17 * local_20 + this->bounceLocation.y * fVar13;
	local_b0.z = (fVar14 * fVar17 - fVar16 * fVar15) * local_1c + fVar18 * local_20 + this->bounceLocation.z * fVar13;
	local_b0.w = in_vf0x * local_1c + in_vf0x * local_20 + this->bounceLocation.w * fVar13;

	fVar13 = edF32Vector4SafeNormalize0Hard(&local_b0, &local_b0);

	if (fVar13 != 0.0f) {
		this->dynamic.rotationQuat = local_b0;
	}

	this->dynamic.speed = fVar13;
	local_c0.w = this->bounceLocation.w;
	local_c0.xyz = 0.0f - this->bounceLocation.xyz;
	SV_UpdateOrientation(this->field_0x1040, &local_c0);

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) || (1.7f <= this->distanceToGround)) {
		if (iVar7 == 0) {
			if (param_1 == 0.0f) {
				this->dynamic.speed = 0.0f;
				uVar8 = ChooseStateFall(1);
				SetState(uVar8, 0xffffffff);
			}
			else {
				if (param_1 < 0.0) {
					local_30.x = this->bounceLocation.z;
					local_30.z = -this->bounceLocation.x;
				}
				else {
					local_30.x = -this->bounceLocation.z;
					local_30.z = this->bounceLocation.x;
				}

				local_30.w = 0.0f;
				local_30.y = 0.0f;
				fVar10 = 0.1f - ((this->pCollisionData)->pObbPrim->field_0x90).z;
				fVar11 = -((this->pCollisionData)->pObbPrim->field_0x90).z - 0.1f;

				local_50 = this->bounceLocation * fVar11;

				local_40.x = local_30.x * fVar10 + local_50.x + this->currentLocation.x;
				local_40.y = fVar10 * 0.0f + local_50.y + this->currentLocation.y;
				local_40.z = local_30.z * fVar10 + local_50.z + this->currentLocation.z;
				local_40.w = fVar10 * 0.0f + local_50.w + this->currentLocation.w;

				iVar7 = DetectGripEdge(0, &local_40, &local_30, (float*)0x0, (float*)0x0, &this->field_0x1460);

				if (iVar7 == 0) {
					fVar10 = 0.1f - ((this->pCollisionData)->pObbPrim->field_0x90).z;
					fVar11 = -((this->pCollisionData)->pObbPrim->field_0x90).z - 0.3f;
					local_50 = this->bounceLocation * fVar11;

					local_40.x = local_30.x * fVar10 + local_50.x + this->currentLocation.x;
					local_40.y = local_30.y * fVar10 + local_50.y + this->currentLocation.y;
					local_40.z = local_30.z * fVar10 + local_50.z + this->currentLocation.z;
					local_40.w = local_30.w * fVar10 + local_50.w + this->currentLocation.w;
					iVar7 = DetectGripEdge(0, &local_40, &local_30, (float*)0x0, (float*)0x0, &this->field_0x1460);
				}

				if (iVar7 == 0) {
					this->dynamic.speed = 0.0f;
					uVar8 = ChooseStateFall(0);
					SetState(uVar8, 0xffffffff);
				}
				else {
					this->field_0x1490 = local_40;
					if (0.0f < param_1) {
						SetState(0xc4, 0xffffffff);
					}
					else {
						SetState(0xc2, 0xffffffff);
					}
				}
			}
		}
		else {
			if (param_4 == 0) {
				if (nextState != -1) {
					if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
						SetState(nextState, 0xffffffff);
						return;
					}
				}
				if ((nextState == -1) || (0.2f <= this->timeInAir)) {
					if (0.8f <= -fVar10) {
						SetState(STATE_HERO_GRIP_RIGHT, 0xffffffff);
					}
					else {
						if (-fVar10 <= -0.8f) {
							SetState(STATE_HERO_GRIP_LEFT, 0xffffffff);
						}
						else {
							if (fVar11 < 0.8f) {
								uint jumpMask;

								pPlayerInput = this->pPlayerInput;
								if ((pPlayerInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									jumpMask = 0;
								}
								else {
									jumpMask = pPlayerInput->pressedBitfield & PAD_BITMASK_CROSS;
								}

								if (jumpMask == 0) {
									if (-0.8f < fVar11) {
										SetState(STATE_HERO_GRIP_HANG_IDLE, 0xffffffff);
										this->effort = 0.0f;
										return;
									}

									this->dynamic.speed = 0.0f;
									SetState(ChooseStateFall(0), 0xffffffff);
									return;
								}
							}

							SetState(STATE_HERO_GRIP_UP, 0xffffffff);
						}
					}
				}
			}
			else {
				pPlayerInput = this->pPlayerInput;
				if ((pPlayerInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar10 = 0.0f;
				}
				else {
					fVar10 = pPlayerInput->aAnalogSticks[PAD_STICK_LEFT].magnitude;
				}

				if (fVar10 <= 0.6f) {
					SetState(STATE_HERO_GRIP_HANG_IDLE, 0xffffffff);
				}
			}
		}
	}
	else {
		this->dynamic.speed = 0.0f;
		SetState(STATE_HERO_JUMP_3_3_STAND, 0xffffffff);
	}

	return;
}

void CActorHeroPrivate::StateHeroGripAngle(int nextState, int param_3)
{
	CCamera* pCVar1;
	CCameraManager* iVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	float local_8;
	float local_4;

	local_8 = 0.0f;
	local_4 = 0.0f;
	DetectGripEdge(1, &this->currentLocation, &this->rotationQuat, &local_4, &local_8, &local_20);
	local_30 = this->field_0x1490 - this->currentLocation;

	if (param_3 != 0) {
		fVar5 = -((this->pCollisionData)->pObbPrim->field_0x90).z - 0.1f;
		local_30 = local_30 - this->bounceLocation * fVar5;
	}

	fVar6 = edF32Vector4SafeNormalize0Hard(&this->dynamic.rotationQuat, &local_30);
	fVar4 = fVar6 / GetTimer()->cutsceneDeltaTime;
	fVar5 = 4.0f;
	if (fVar4 < 4.0f) {
		fVar5 = fVar4;
	}
	this->dynamic.speed = fVar5;

	local_20.w = this->field_0x1460.w;
	local_20.x = 0.0f - this->field_0x1460.x;
	local_20.y = 0.0f - this->field_0x1460.y;
	local_20.z = 0.0f - this->field_0x1460.z;

	SV_UpdateOrientation(this->field_0x1040, &local_20);

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	iVar3 = (CCameraManager*)CScene::GetManager(MO_Camera);
	pCVar1 = this->pCameraViewBase_0x15b0;
	if (iVar3->pActiveCamera == pCVar1) {
		pCVar1->AlertCamera(2, 1, (CCamera*)0x0);
	}

	if (((fVar6 <= 0.001f) || (0.5f <= this->timeInAir)) && (SetState(nextState, 0xffffffff), param_3 == 0)) {
		this->bounceLocation = this->field_0x1460;
	}

	return;
}



void CActorHeroPrivate::StateHeroGripUp(float param_1, float param_2, int nextState, int param_5)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	undefined4 local_10;
	float local_c;
	undefined4 local_8;
	undefined4 local_4;

	pCVar1 = this->pAnimationController;

	IncreaseEffort(2.0f);

	local_10 = 0;
	local_8 = 0;
	local_4 = 0;

	fVar4 = this->bounceLocation.x;
	fVar5 = this->bounceLocation.y;
	fVar6 = this->bounceLocation.z;
	fVar7 = g_xVector.y * fVar6 - fVar5 * g_xVector.z;
	fVar8 = g_xVector.z * fVar4 - fVar6 * g_xVector.x;
	fVar9 = g_xVector.x * fVar5 - fVar4 * g_xVector.y;
	fVar4 = this->bounceLocation.x;
	fVar5 = this->bounceLocation.y;
	fVar6 = this->bounceLocation.z;
	local_20.x = (fVar5 * fVar9 - fVar8 * fVar6) * param_1 + fVar7 * 0.0f + this->bounceLocation.x * -0.0f;
	local_20.y = (fVar6 * fVar7 - fVar9 * fVar4) * param_1 + fVar8 * 0.0f + this->bounceLocation.y * -0.0f;
	local_20.z = (fVar4 * fVar8 - fVar7 * fVar5) * param_1 + fVar9 * 0.0f + this->bounceLocation.z * -0.0f;
	local_20.w = in_vf0x * param_1 + in_vf0x * 0.0f + this->bounceLocation.w * -0.0f;
	local_c = param_1;

	fVar4 = edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
	if (fVar4 != 0.0f) {
		this->dynamic.rotationQuat = local_20;
	}

	this->dynamic.speed = fVar4;

	local_30.w = this->bounceLocation.w;
	local_30.xyz = 0.0 - this->bounceLocation.xyz;

	SV_UpdateOrientation(this->field_0x1040, &local_30);

	if ((param_5 != 0) || (((this->pCollisionData)->flags_0x4 & 1) != 0)) {
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;

		this->dynamicExt.field_0x6c = 0.0f;
	}

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	if ((param_2 <= 0.0f) || (this->timeInAir <= param_2)) {
		if (0.0f < param_2) {
			return;
		}

		if (!pCVar1->IsCurrentLayerAnimEndReached(0)) {
			return;
		}
	}

	SetState(nextState, 0xffffffff);
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_GripClimb.cpp
void CActorHeroPrivate::StateHeroGripUpToJumpInit()
{
	this->dynamic.rotationQuat = this->rotationQuat;
	this->dynamic.speed = 2.5f;
	SetJumpCfg(0.1f, this->field_0x104c, this->field_0x1158 * 0.5f, this->field_0x1150 * 0.7f, this->field_0x1154 * 0.7f, 0, (edF32VECTOR4*)0x0);
	return;
}

void CActorHeroPrivate::StateHeroBounceSomersault1()
{
	edF32VECTOR4* v0;
	float fVar5;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	IncreaseEffort(2.0f);

	local_10.w = this->bounceLocation.w;
	local_10.x = 0.0f - this->bounceLocation.x;
	local_10.y = 0.0f - this->bounceLocation.y;
	local_10.z = 0.0f - this->bounceLocation.z;
	SV_UpdateOrientation2D(this->field_0x1040, &local_10, 0);
	this->dynamic.rotationQuat.x = 0.0f;
	this->dynamic.rotationQuat.y = 1.0f;
	this->dynamic.rotationQuat.z = 0.0f;
	this->dynamic.speed = 3.3f;

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		local_20.y = this->field_0x14e4 * 50.0f;
		local_20.x = 0.0f;
		local_20.z = 0.0f;
		local_20.w = 0.0f;
		edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack48, &local_20);
		v0 = this->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(v0, v0, &eStack48);
		fVar5 = edF32Vector4GetDistHard
		(this->dynamicExt.aImpulseVelocities);
		this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar5;
		fVar5 = edFIntervalLERP(this->field_0x14f4, 1.0f, this->field_0x14cc, this->field_0x14ec, this->field_0x14f0);
		this->dynamic.rotationQuat = this->bounceLocation;
		this->dynamic.speed = fVar5;
		this->field_0x14f4 = this->field_0x14cc;

		SetState(STATE_HERO_BOUNCE_SOMERSAULT_2, 0xffffffff);
	}
	return;
}

void CActorHeroPrivate::StateHeroBounceSomersault2()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	edF32VECTOR4 local_10;

	pCVar1 = this->pAnimationController;
	IncreaseEffort(1.0f);
	local_10.w = this->bounceLocation.w;
	local_10.x = 0.0f - this->bounceLocation.x;
	local_10.y = 0.0f - this->bounceLocation.y;
	local_10.z = 0.0f - this->bounceLocation.z;
	SV_UpdateOrientation2D(this->field_0x1040, &local_10, 0);

	MoveInAir(0.1f, this->field_0x14e8, this->field_0x1158 * 0.5f, this->airNoInputSpeed, 0.0f);

	ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(STATE_HERO_FALL_BOUNCE, 0xffffffff);
	}

	return;
}

void CActorHeroPrivate::StateHeroCeilingClimb(int param_2, int param_3, int param_4)
{
	ed_zone_3d* pZone;
	S_ZONE_STREAM_REF* pSVar1;
	bool bVar2;
	bool bVar3;
	uint uVar5;
	int iVar6;
	int iVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	float local_30;
	float local_2c;
	float local_28;
	float local_24;
	edF32VECTOR4 local_20;
	float fStack8;
	float local_4;
	CAnimation* pAnim;
	CCollision* pCol;
	CEventManager* pEventManager;
	CPlayerInput* pInput;
	edAnmLayer* pLayer;

	IncreaseEffort(1.0f);

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar8 = 0.0f;
	}
	else {
		fVar8 = pInput->aAnalogSticks[0].magnitude;
	}

	if ((fVar8 < 0.1f) || (param_2 == 0)) {
		SV_MOV_UpdateSpeedIntensity(0.0f, this->field_0x152c);
	}
	else {
		BuildHorizontalSpeedVector(this->field_0x1524, this->field_0x1528, this->field_0x1534, this->field_0x1530, this->field_0x1074);
	}

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	fVar8 = 3.0f;

	fVar9 = this->field_0x14a8;
	if (fabs(fVar9) < GetTimer()->cutsceneDeltaTime * 3.0f) {
		fVar8 = fVar9 / GetTimer()->cutsceneDeltaTime;
	}
	else {
		if (fVar9 <= 0.0f) {
			fVar8 = -3.0f;
		}
	}
	this->dynamicExt.field_0x6c = fVar8;
	local_24 = this->field_0x1480.w;
	local_30 = 0.0f - this->field_0x1480.x;
	local_2c = 0.0f - this->field_0x1480.y;
	local_28 = 0.0f - this->field_0x1480.z;
	this->dynamicExt.normalizedTranslation.x = local_30;
	this->dynamicExt.normalizedTranslation.y = local_2c;
	this->dynamicExt.normalizedTranslation.z = local_28;
	this->dynamicExt.normalizedTranslation.w = local_24;
	local_20 = this->currentLocation;

	if (this->dynamic.speed <= 0.1f) {
		pCol = this->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 | 0x40;
	}

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 & 0xffffffbf;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	bVar3 = false;
	if (this->field_0x1454 != false) {
		iVar7 = 0;
		while (true) {
			pSVar1 = this->pClimbZoneStreamRef;
			if (pSVar1 == (S_ZONE_STREAM_REF*)0x0) {
				iVar6 = 0;
			}
			else {
				iVar6 = pSVar1->entryCount;
			}

			if (iVar6 <= iVar7) {
				bVar2 = false;
				goto LAB_00137f08;
			}

			pZone = (this->pClimbZoneStreamRef)->aEntries[iVar7].Get();

			if ((pZone != (ed_zone_3d*)0x0) &&
				(uVar5 = edEventComputeZoneAgainstVertex
				(pEventManager->activeChunkId, pZone,
					&this->currentLocation, 0),
					(uVar5 & 1) != 0)) break;
			iVar7 = iVar7 + 1;
		}

		bVar2 = true;
	LAB_00137f08:
		if (!bVar2) {
			this->dynamic.speed = 0.0f;
			UpdatePosition(&local_20, true);
			bVar3 = true;
		}
	}

	iVar7 = DetectClimbCeiling(&this->currentLocation, (CActor**)0x0);
	if (iVar7 == 0) {
		local_50.w = this->dynamic.rotationQuat.w;
		local_50.x = 0.0 - this->dynamic.rotationQuat.x;
		local_50.y = 0.0 - this->dynamic.rotationQuat.y;
		local_50.z = 0.0 - this->dynamic.rotationQuat.z;
		edF32Vector4SafeNormalize1Hard(&local_50, &local_50);
		fVar8 = -this->pCollisionData->pObbPrim->field_0x90.z;
		local_40.x = local_50.x * fVar8 + this->currentLocation.x;
		local_40.z = local_50.z * fVar8 + this->currentLocation.z;
		local_40.w = local_50.w * fVar8 + this->currentLocation.w;
		local_40.y = local_50.y * fVar8 + this->currentLocation.y + 0.3400001f;

		iVar7 = DetectGripEdge(0, &local_40, &local_50, &local_4, &fStack8, &this->bounceLocation);
		if (iVar7 == 0) {
			SetState(ChooseStateFall(0), 0xffffffff);
		}
		else {
			this->field_0x1490 = local_40;
			this->field_0x1490.y = this->field_0x1490.y + local_4;
			SetState(STATE_HERO_CEILING_2_GRIP, 0xffffffff);
		}
	}
	else {
		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar5 = 0;
		}
		else {
			uVar5 = pInput->pressedBitfield & 0x10;
		}

		if (uVar5 == 0) {
			if (param_3 == 0) {
				if (param_2 == 0) {
					pInput = this->pPlayerInput;
					if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar8 = 0.0f;
					}
					else {
						fVar8 = pInput->aAnalogSticks[0].magnitude;
					}

					if ((fVar8 <= 0.3f) || (bVar3)) {
						if (param_4 != 0) {
							if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
								SetState(0x10f, 0xffffffff);
							}
						}
					}
					else {
						SetState(STATE_HERO_CEILING_CLIMB_C, 0xffffffff);
					}
				}
				else {
					if (bVar3) {
						SetState(STATE_HERO_CEILING_CLIMB_D, 0xffffffff);
					}
					else {
						if (this->dynamic.speed <= 0.1f) {
							SetState(STATE_HERO_CEILING_CLIMB_A, 0xffffffff);
						}
						else {
							bVar3 = DetectClimbWall(1, (CActor**)0x0, (float*)0x0);
							if ((bVar3 == false) ||
								(0.0f <= this->field_0x1470.x *
									this->dynamic.rotationQuat.x +
									this->field_0x1470.y *
									this->dynamic.rotationQuat.y +
									this->field_0x1470.z *
									this->dynamic.rotationQuat.z)) {
								SetState(STATE_HERO_CEILING_CLIMB_C, 0xffffffff);
							}
							else {
								SetState(STATE_HERO_CLIMB_STAND_C, 0xffffffff);
							}
						}
					}
				}
			}
			else {
				pInput = this->pPlayerInput;
				if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar8 = 0.0f;
				}
				else {
					fVar8 = pInput->aAnalogSticks[0].magnitude;
				}

				if (fVar8 <= 0.6f) {
					SetState(STATE_HERO_CEILING_CLIMB_A, 0xffffffff);
				}
			}
		}
		else {
			SetState(ChooseStateFall(0), 0xffffffff);
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroGrip2Ceiling()
{
	IMPLEMENTATION_GUARD();
}

void CActorHeroPrivate::StateHeroCeiling2Grip()
{
	IMPLEMENTATION_GUARD();
}

void CActorHeroPrivate::GripObject(CActor* pOtherActor)
{
	CActor* pOldGrippedActor;
	CActor* pTieActor;
	bool bVar2;

	pOldGrippedActor = this->pGrippedActor;

	if (pOldGrippedActor != pOtherActor) {
		UngripAllObjects();

		this->pGrippedActor = pOtherActor;

		pOldGrippedActor = this->pGrippedActor;
		if (pOldGrippedActor != (CActor*)0x0) {
			TieToActor(pOldGrippedActor, 0, 1, (edF32MATRIX4*)0x0);
		}
	}

	return;
}

