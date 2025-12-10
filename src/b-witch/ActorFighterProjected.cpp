#include "ActorFighter.h"

#include "MathOps.h"
#include "TimeController.h"
#include "CollisionRay.h"
#include "InputManager.h"
#include "ActorWind.h"

static float FLOAT_00448b04 = 10.0f;

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitFly()
{
	int iVar2;
	float fVar3;
	float fVar5;
	edF32MATRIX4 eStack336;
	CActorsTable local_110;

	local_110.nbEntries = 0;
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &this->field_0x740, &this->dynamicExt.normalizedTranslation);

	this->field_0x7d0 = (this->fighterAnatomyZones).field_0x0.y + this->currentLocation.y;

	ManageDyn(4.0f, 0x8000120, &local_110);

	this->field_0x7b0 = this->field_0x7b0 + this->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
	fVar3 = this->field_0x7b4;
	this->field_0x7b4 = fVar3 - FLOAT_00448b04 * fVar3 * fVar3 * GetTimer()->cutsceneDeltaTime * GetTimer()->cutsceneDeltaTime;

	edF32Matrix4FromAngAxisSoft(-this->field_0x7b0, &eStack336, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack336, &this->field_0x760, &eStack336);
	_SV_DYN_SetRotationAroundMassCenter(&eStack336);

	this->field_0x8e4 = _SV_HIT_ProcessActorsCollisions(0.0f, &this->field_0x8f0, &this->field_0x900, &this->field_0x740, &local_110, &this->actorsExcludeTable, true, 6, 0);

	if ((((this->pCollisionData)->flags_0x4 & 7) == 0) && (this->field_0x8e4 == 0)) {
		this->field_0x750 = this->field_0x740;
	}
	else {
		_InterpretCollisions(0);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitWakingUpInit()
{
	edF32VECTOR4* v0;
	float puVar3;
	float puVar4;
	float fVar2;
	edF32VECTOR4 eStack256;
	edF32VECTOR4 eStack240;
	edF32MATRIX4 eStack224;
	edF32MATRIX4 eStack160;
	edF32VECTOR4 eStack96;
	edF32VECTOR3 local_50;
	edF32MATRIX4 eStack64;

	edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
	edF32Matrix4FromAngAxisSoft(-(this->field_0x7dc * 1.570796f), &eStack64, &this->field_0x760.rowX);
	edF32Matrix4MulF32Matrix4Hard(&this->field_0x760, &this->field_0x760, &eStack64);
	edF32Matrix4ToEulerSoft(&this->field_0x760, &local_50, "XYZ");
	this->rotationEuler.xyz = local_50;

	SV_GetBoneWorldPosition(0x53538f8b, &eStack96);
	ComputeRotTransMatrix(&eStack160);
	ComputeLocalMatrix(&eStack224, &eStack160);
	edF32Matrix4MulF32Vector4Hard(&eStack256, &eStack224, this->field_0x810 + ((int)this->field_0x7dc + 1 >> 1));
	edF32Vector4SubHard(&eStack240, &eStack96, &eStack256);
	v0 = &this->currentLocation;
	edF32Vector4AddHard(v0, v0, &eStack240);
	edF32Vector4CrossProductHard(&this->field_0x7a0, &(this->field_0x760).rowY, &g_xVector);
	edF32Vector4SafeNormalize0Hard(&this->field_0x7a0, &this->field_0x7a0);

	puVar3 = edF32Vector4DotProductHard(&(this->field_0x760).rowY, &g_xVector);
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
	this->field_0x7b4 = fVar2;
	this->field_0x7b0 = 0.0f;

	int macroAnim = GetIdMacroAnim(this->currentAnimType);

	if (macroAnim < 0) {
		fVar2 = 0.0f;
	}
	else {
		fVar2 = this->pAnimationController->GetAnimLength(macroAnim, 1);
	}

	this->field_0x7b4 = this->field_0x7b4 / fVar2;

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitFlyToSlide()
{
	Timer* pTVar1;
	int iVar3;
	uint uVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	float fVar8;
	float fVar9;
	edF32MATRIX4 eStack528;
	edF32VECTOR4 local_1d0;
	edF32MATRIX4 local_1c0;
	edF32VECTOR4 eStack352;
	edF32VECTOR4 eStack336;
	edF32VECTOR4 eStack320;
	edF32VECTOR4 local_130;
	CActorsTable local_120;
	_ray_info_out _Stack16;

	local_120.nbEntries = 0;
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &this->field_0x740, &this->dynamicExt.normalizedTranslation);

	this->field_0x7d0 = (this->fighterAnatomyZones).field_0x0.y + this->currentLocation.y;

	ManageDyn(4.0f, 0x8000120, &local_120);

	edF32Vector4AddHard(&local_130, &this->fighterAnatomyZones.field_0x0, &this->currentLocation);
	local_130.w = 1.0f;

	fVar5 = edF32Vector4DotProductHard(&this->field_0x740, &this->field_0x740);
	if (fVar5 == 0.0f) {
		edF32Vector4GetNegHard(&eStack336, &g_xVector);
	}
	else {
		edF32Vector4NormalizeHard(&eStack336, &this->field_0x740);
	}

	_SV_VOLDYN_GetCollisionPosFromVector(&local_130, &eStack336);

	edF32Vector4ScaleHard(-0.1f, &eStack320, &eStack336);
	edF32Vector4AddHard(&local_130, &local_130, &eStack320);

	CCollisionRay CStack384 = CCollisionRay(1.2f, &local_130, &eStack336);
	fVar5 = CStack384.Intersect(RAY_FLAG_ACTOR_SCENERY, (CActor*)0x0, (CActor*)0x0, 0x40000008, &eStack352, &_Stack16);
	if (fVar5 != 1e+30f) {
		uVar4 = _Stack16.hitMaterialFlags & 0xf;
		if (uVar4 == 0) {
			uVar4 = CScene::_pinstance->defaultMaterialIndex;
		}

		if (CCollisionManager::_material_table[uVar4].field_0x8 <= fabs(eStack352.y)) {
			uVar4 = _Stack16.hitMaterialFlags & 0xf;
			if (uVar4 == 0) {
				uVar4 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (-CCollisionManager::_material_table[uVar4].field_0x8 < eStack352.y) {
				edF32Matrix4FromEulerSoft(&local_1c0, &this->rotationEuler.xyz, "XYZ");
				fVar9 = edF32Vector4DotProductHard(&eStack352, &local_1c0.rowZ);
				if (fVar9 < 0.0f) {
					this->field_0x7dc = -1.0f;
					edF32Vector4ScaleHard(this->field_0x7dc, &eStack352, &eStack352);
				}
				else {
					this->field_0x7dc = 1.0f;
				}

				edF32Vector4CrossProductHard(&local_1d0, &local_1c0.rowZ, &eStack352);
				edF32Vector4SafeNormalize0Hard(&local_1d0, &local_1d0);
				puVar6 = edF32Vector4DotProductHard(&local_1c0.rowZ, &eStack352);
				if (1.0f < puVar6) {
					puVar7 = 1.0f;
				}
				else {
					puVar7 = -1.0f;
					if (-1.0f <= puVar6) {
						puVar7 = puVar6;
					}
				}

				fVar9 = acosf(puVar7);
				fVar8 = edF32Vector4DotProductHard(&local_1d0, &this->field_0x7a0);
				if ((fabs(fVar8) < 0.99f) || (0.08726647f < fabs(fVar9 - this->field_0x7b0))) {
					edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
					this->field_0x7a0 = local_1d0;

					this->field_0x7b0 = fVar9;
					this->field_0x7b4 = fVar5;
				}

				fVar5 = edFIntervalUnitDstLERP(fVar5, 0.2f, 1.1f);
				edF32Matrix4FromAngAxisSoft(-(this->field_0x7b0 * (1.0f - fVar5)), &local_1c0, &this->field_0x7a0);
				edF32Matrix4MulF32Matrix4Hard(&local_1c0, &this->field_0x760, &local_1c0);
				_SV_DYN_SetRotationAroundMassCenter(&local_1c0);

				goto LAB_00306b50;
			}
		}
	}

	this->field_0x7b0 = this->field_0x7b0 + this->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
	fVar9 = GetTimer()->cutsceneDeltaTime;
	fVar5 = this->field_0x7b4;
	this->field_0x7b4 = fVar5 - FLOAT_00448b04 * fVar5 * fVar5 * GetTimer()->cutsceneDeltaTime * fVar9;
	edF32Matrix4FromAngAxisSoft(-this->field_0x7b0, &eStack528, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack528, &this->field_0x760, &eStack528);
	_SV_DYN_SetRotationAroundMassCenter(&eStack528);
	this->field_0x7dc = 0.0f;
LAB_00306b50:

	this->field_0x8e4 = _SV_HIT_ProcessActorsCollisions(0.0f, &this->field_0x8f0, &this->field_0x900, &this->field_0x740, &local_120, &this->actorsExcludeTable, 1, 6, 0);
	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) || (this->field_0x7dc == 0.0f)) {
		if (((((this->pCollisionData)->flags_0x4 & 7) == 0) && (this->field_0x8e4 == 0)) ||
			(this->timeInAir <= 0.15f)) {
			this->field_0x750 = this->field_0x740;
			if ((((this->hitFlags & 4U) == 0) && (15.0f < this->dynamicExt.field_0x6c)) &&
				(this->dynamicExt.normalizedTranslation.y < 0.0f)) {
				SetState(FIGHTER_PROJECTED_HIT_FLY, -1);
			}
		}
		else {
			_InterpretCollisions(0);
		}
	}
	else {
		edProjectVectorOnPlane(0.0f, &this->field_0x740, &this->field_0x740, &eStack352, 0);
		iVar3 = _SV_ANM_GetTwoSidedAnim(0x71, (int)this->field_0x7dc);
		SetState(FIGHTER_PROJECTED_HIT_SLIDE, iVar3);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitSlide()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar6;
	edF32VECTOR4 eStack16;

	this->field_0x740 = this->dynamic.velocityDirectionEuler;

	edF32Vector4ScaleHard(this->dynamic.linearAcceleration, &this->field_0x740, &this->field_0x740);
	ManageDyn(4.0f, this->field_0x7e0 | 0x53, (CActorsTable*)0x0);
	pCVar1 = this->pAnimationController;

	if (pCVar1->IsCurrentLayerAnimEndReached(0)) {
		edF32Vector4ScaleHard(this->field_0x7dc, &eStack16, &this->pCollisionData->aCollisionContact[1].location);
		if ((0.8f <= this->dynamic.linearAcceleration) || (fVar6 = edF32Vector4DotProductHard(&eStack16, &g_xVector), fabs(fVar6) <= 0.8f)) {
			PlayAnim(_SV_ANM_GetTwoSidedAnim(0x73, (int)this->field_0x7dc));
		}
		else {
			if (IsAlive() == false) {
				if ((this->field_0x738 & 4) == 0) {
					SetState(0x62, _SV_ANM_GetTwoSidedAnim(0x80, (int)this->field_0x7dc));
				}
				else {
					ProcessDeath();
				}
			}
			else {
				SetState(0x58, _SV_ANM_GetTwoSidedAnim(0x75, (int)this->field_0x7dc));
			}
		}
	}

	_InterpretSlides();

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_Proj_GetPossibleExit()
{
	int iVar1;

	iVar1 = this->actorState;
	if (iVar1 != -1) {
		GetStateCfg(iVar1);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_InterpretSlides()
{
	CCollision* pCVar1;
	uint uVar3;
	edF32VECTOR4* peVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 local_40;

	this->field_0x7b0 = this->field_0x7b0 + this->field_0x7b4 * GetTimer()->cutsceneDeltaTime;
	edF32Matrix4FromEulerSoft(&local_40, &this->rotationEuler.xyz, "XYZ");
	edF32Matrix4FromAngAxisSoft(-this->field_0x7b0, &eStack128, &this->field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&eStack128, &local_40, &eStack128);

	_SV_DYN_SetRotationAroundMassCenter(&eStack128);

	pCVar1 = this->pCollisionData;
	if ((pCVar1->flags_0x4 & 2) == 0) {
		this->field_0x7b4 = 0.0f;
		this->field_0x7b0 = 0.0f;
		if (0.15f < this->timeInAir) {
			edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
			edF32Matrix4MulF32Vector4Hard(&this->fighterAnatomyZones.field_0x0, &this->field_0x760, &(this->fighterAnatomyZones).field_0x10);
			SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		else {
			this->field_0x7e0 = 0x20;
		}
	}
	else {
		edF32Vector4ScaleHard(this->field_0x7dc, &eStack144, &pCVar1->aCollisionContact[1].location);

		uVar3 = (this->pCollisionData)->aCollisionContact[1].materialFlags & 0xf;
		if (uVar3 == 0) {
			uVar3 = CScene::_pinstance->defaultMaterialIndex;
		}

		if (uVar3 == 5) {
			ComputeFrictionForceWithSpeedMax(12.0f, &eStack160, 1);
		}
		else {
			ComputeFrictionForce(90.0f, &eStack160, 0);
		}

		peVar4 = this->dynamicExt.aImpulseVelocities + 1;
		edF32Vector4AddHard(peVar4, peVar4, &eStack160);
		fVar5 = edF32Vector4GetDistHard(peVar4);
		this->dynamicExt.aImpulseVelocityMagnitudes[1] = fVar5;

		ComputeSlidingForce(&eStack176, 0);

		peVar4 = this->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(peVar4, peVar4, &eStack176);
		fVar5 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
		this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar5;
		this->timeInAir = 0.0f;

		edF32Vector4CrossProductHard(&this->field_0x7a0, &local_40.rowZ, &eStack144);
		puVar6 = edF32Vector4DotProductHard(&local_40.rowZ, &eStack144);
		if (1.0f < puVar6) {
			puVar7 = 1.0f;
		}
		else {
			puVar7 = -1.0f;;
			if (-1.0 <= puVar6) {
				puVar7 = puVar6;
			}
		}

		fVar5 = acosf(puVar7);
		fVar5 = edF32Between_2Pi(fVar5);
		this->field_0x7b4 = fVar5;
		this->field_0x7b4 = (this->field_0x7b4 - this->field_0x7b0) / (GetTimer()->cutsceneDeltaTime * 2.0f);
	}

	return;
}

float CActorFighter::_pStagger_check[8] = {
	2.3561945f,
	1.5707964f,
	0.7853982f,
	0.0f,
	3.1415927f,
	3.926991f,
	4.712389f,
	5.4977875f,
};

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitStaggerCheck()
{
	float fVar1;
	float f0;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	if ((this->fightFlags & 0x20) == 0) {
		if (this->field_0x7e4 < 8) {
			edF32Matrix4RotateYHard(_pStagger_check[this->field_0x7e4], &eStack64, &gF32Matrix4Unit);
			edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &this->field_0x7f0);
			edF32Vector4AddHard(&local_60, &this->currentLocation, &(this->fighterAnatomyZones).field_0x10);
			local_60.w = 1.0f;
			f0 = ((this->pCollisionData)->pObbPrim->field_0x90).y * 2.0f * 1.1f;
			CCollisionRay CStack128 = CCollisionRay(f0, &local_60, &local_50);
			fVar1 = CStack128.Intersect(3, (CActor*)this, (CActor*)0x0, 0x7fffffff, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			if (fVar1 == 1e+30f) {
				edF32Vector4ScaleHard(f0, &local_50, &local_50);
				edF32Vector4AddHard(&local_60, &local_60, &local_50);
				local_50.y = -1.0f;
				local_50.x = 0.0f;
				local_50.z = 0.0f;
				local_50.w = 0.0f;
				CStack128.ChangeOrigin(&local_60);
				CStack128.ChangeLeadVector(&local_50);
				fVar1 = CStack128.Intersect(3, (CActor*)this, (CActor*)0x0, 0x7fffffff, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);

				if (fVar1 == 1e+30) {
					this->field_0x7e4 = this->field_0x7e4 + 1;
				}
				else {
					this->fightFlags = this->fightFlags | 0x20;
				}
			}
			else {
				this->field_0x7e4 = this->field_0x7e4 + 1;
			}
		}
		else {
			this->fightFlags = this->fightFlags | 0x20;
			this->field_0x7e4 = 0;
		}
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if ((this->fightFlags & 0x20) != 0) {
		SetState(0x5b, -1);
	}
	return;
}

edF32VECTOR4 edF32VECTOR4_00425760 = { 0.0f, -1.0f, 0.0f, 0.0f };

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CActorFighter::_StateFighterHitStaggerFallInit(float param_1)
{
	edColPRIM_OBJECT* peVar1;
	StateConfig* pSVar2;
	int iVar3;
	float fVar4;
	float puVar5;
	float puVar6;
	float fVar7;
	float fVar8;
	float t;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	_ray_info_out _Stack16;

	local_50 = edF32VECTOR4_00425760;
	peVar1 = this->pCollisionData->pObbPrim;
	t = (peVar1->field_0x90).y;
	fVar8 = (peVar1->field_0x90).z;
	this->field_0x7dc = param_1;
	SetVectorFromAngleY(this->rotationEuler.y, &eStack64);
	edF32Vector4ScaleHard(this->field_0x7dc, &eStack64, &eStack64);
	edF32Vector4ScaleHard(-t, &local_30, &eStack64);
	edF32Vector4AddHard(&local_30, &local_30, &this->currentLocation);
	edF32Vector4AddHard(&local_30, &local_30, &(this->fighterAnatomyZones).field_0x10);
	local_30.w = 1.0f;
	CCollisionRay CStack112 = CCollisionRay(t * 2.0f, &local_30, &local_50);
	CStack112.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000008, &eStack32, &_Stack16);
	pSVar2 = GetStateCfg(this->actorState);
	fVar4 = _GetFighterAnimationLength(pSVar2->animId);
	edF32Matrix4FromEulerSoft(&this->field_0x760, &this->rotationEuler.xyz, "XYZ");
	edF32Vector4CrossProductHard(&this->field_0x7a0, &eStack64, &eStack32);
	edF32Vector4SafeNormalize0Hard(&this->field_0x7a0, &this->field_0x7a0);
	puVar5 = edF32Vector4DotProductHard(&eStack64, &eStack32);
	if (1.0f < puVar5) {
		puVar6 = 1.0f;
	}
	else {
		puVar6 = -1.0f;
		if (-1.0f <= puVar5) {
			puVar6 = puVar5;
		}
	}

	fVar7 = acosf((float)puVar6);
	this->field_0x7b0 = fVar7;
	this->field_0x7b4 = fVar7 / fVar4;
	edF32Vector4ScaleHard(fVar8, &eStack64, &eStack32);
	edF32Vector4CrossProductHard(&local_50, &eStack32, &this->field_0x7a0);
	edF32Vector4NormalizeHard(&local_50, &local_50);
	edF32Vector4ScaleHard(t, &local_50, &local_50);
	edF32Vector4AddHard(&local_30, &local_30, &local_50);
	edF32Vector4SubHard(&this->field_0x740, &local_30, &this->currentLocation);
	edF32Vector4ScaleHard(1.0f / fVar4, &this->field_0x740, &this->field_0x740);
	fVar8 = edF32Vector4NormalizeHard(&this->field_0x740, &this->field_0x740);
	this->dynamic.speed = fVar8;
	this->dynamic.rotationQuat = this->field_0x740;

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CBehaviourFighterProjected::Manage()
{
	bool bVar1;
	Timer* pTVar2;
	CLifeInterface* pLifeInterface;
	int iVar3;
	CBehaviourFighter* pCVar4;
	//CActorFighterVTable* pCVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR3 local_f0;
	edF32MATRIX4 eStack224;
	edF32VECTOR3 local_a0;
	edF32MATRIX4 eStack144;
	edF32VECTOR3 local_50;
	edF32MATRIX4 eStack64;
	CAnimation* pAnimation;
	edAnmLayer* pAnmLayer;
	CActorFighter* pFighter;

	this->pOwner->UpdateFightCommand();

	pFighter = this->pOwner;
	switch (pFighter->actorState) {
	case 0x54:
		pFighter->ManageDyn(4.0f, 0xb, (CActorsTable*)0x0);
		if (pFighter->field_0x7d8 <= pFighter->timeInAir) {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		break;
	case FIGHTER_PROJECTED_HIT_FLY:
		pFighter->_StateFighterHitFly();
		break;
	case FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE:
		pFighter->_StateFighterHitFlyToSlide();
		break;
	case FIGHTER_PROJECTED_HIT_SLIDE:
		pFighter->_StateFighterHitSlide();
		break;
	case 0x58:
		pCVar4 = static_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
		pFighter->ManageDyn(4.0f, 0x233, (CActorsTable*)0x0);

		bVar1 = pFighter->IsFightRelated(pCVar4->behaviourId);
		if ((bVar1 == false) || ((pFighter->field_0x444 & 2) == 0)) {
			iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x77, (int)pFighter->field_0x7dc);
			fVar6 = 0.6f;
		}
		else {
			iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x79, (int)pFighter->field_0x7dc);
			fVar6 = 0.1f;
		}

		if (fVar6 < pFighter->timeInAir) {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_WAKING_UP, iVar3);
		}
		break;
	case FIGHTER_PROJECTED_HIT_WAKING_UP:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pTVar2 = GetTimer();
		fVar7 = pFighter->field_0x7b0 + pFighter->field_0x7b4 * pTVar2->cutsceneDeltaTime;
		pFighter->field_0x7b0 = fVar7;

		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack64, &pFighter->field_0x7a0);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &pFighter->field_0x760, &eStack64);
		edF32Matrix4ToEulerSoft(&eStack64, &local_50, "XYZ");

		pFighter->rotationEuler.xyz = local_50;

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pCVar4 = static_cast<CBehaviourFighter*>(pFighter->GetBehaviour(pFighter->curBehaviourId));
			pFighter->SetBehaviour(pCVar4->behaviourId, -1, -1);
		}
		break;
	case FIGHTER_PROJECTED_HIT_STAGGER_CHECK:
		pFighter->_StateFighterHitStaggerCheck();
		break;
	case 0x5b:
		fVar7 = edF32Between_2Pi(pFighter->rotationEuler.y + pFighter->field_0x7b4 * GetTimer()->cutsceneDeltaTime);
		pFighter->rotationEuler.y = fVar7;
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x5c, -1);
		}
		break;
	case 0x5c:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x5d, -1);
		}
		break;
	case 0x5d:
		fVar7 = pFighter->field_0x7b4 * pFighter->timeInAir;
		if (fVar7 <= pFighter->field_0x7b0) {
			pAnimation = pFighter->pAnimationController;

			if (pAnimation->IsCurrentLayerAnimEndReached(0)) goto LAB_00305528;
		}
		else {
		LAB_00305528:
			fVar7 = pFighter->field_0x7b0;
		}

		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack144, &pFighter->field_0x7a0);
		edF32Matrix4MulF32Matrix4Hard(&eStack144, &pFighter->field_0x760, &eStack144);
		edF32Matrix4ToEulerSoft(&eStack144, &local_a0, "XYZ");

		pFighter->rotationEuler.xyz = local_a0;
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x5e, -1);
		}
		break;
	case 0x5e:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			fVar7 = pFighter->GetLifeInterface()->GetValue();
			if (fVar7 <= 0.0f) {
				iVar3 = 0x7f;
				if ((pFighter->fightFlags & 0x10) == 0) {
					iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				}
				else {
				}
				pFighter->SetState(0x62, iVar3);
			}
			else {
				iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				pFighter->SetState(0x58, iVar3);
			}
		}
		break;
	case 0x5f:
		fVar7 = pFighter->field_0x7b4 * pFighter->timeInAir;
		if (fVar7 <= pFighter->field_0x7b0) {
			pAnimation = pFighter->pAnimationController;
			if (pAnimation->IsCurrentLayerAnimEndReached(0)) goto LAB_00305798;
		}
		else {
		LAB_00305798:
			fVar7 = pFighter->field_0x7b0;
		}

		edF32Matrix4FromAngAxisSoft(-fVar7, &eStack224, &pFighter->field_0x7a0);
		edF32Matrix4MulF32Matrix4Hard(&eStack224, &pFighter->field_0x760, &eStack224);
		edF32Matrix4ToEulerSoft(&eStack224, &local_f0, "XYZ");
		pFighter->rotationEuler.xyz = local_f0;

		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->SetState(0x60, -1);
		}
		break;
	case 0x60:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			fVar7 = pFighter->GetLifeInterface()->GetValue();
			if (fVar7 <= 0.0f) {
				iVar3 = 0x7f;
				if ((pFighter->fightFlags & 0x10) == 0) {
					iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				}
				else {
				}
				pFighter->SetState(0x62, iVar3);
			}
			else {
				iVar3 = pFighter->_SV_ANM_GetTwoSidedAnim(0x75, (int)pFighter->field_0x7dc);
				pFighter->SetState(0x58, iVar3);
			}
		}
		break;
	case 0x62:
		pFighter->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		pAnimation = pFighter->pAnimationController;

		if (pAnimation->IsCurrentLayerAnimEndReached(0)) {
			pFighter->ProcessDeath();
		}
		break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}

	_ManageExit();

	return;

}

void CBehaviourFighterProjected::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorFighter* pFighter;
	CCollision* pCol;
	bool bVar3;
	int iVar4;
	StateConfig* pSVar5;
	uint uVar6;
	float fVar7;

	this->pOwner = reinterpret_cast<CActorFighter*>(pOwner);

	pFighter = this->pOwner;

	pFighter->fightFlags = pFighter->fightFlags | 0x8000;
	pFighter->fightFlags = pFighter->fightFlags & 0xfffeffff;

	this->behaviourId = this->pOwner->prevBehaviourId;
	if (this->pOwner->IsFightRelated(this->behaviourId) != false) {
		this->behaviourId = this->pOwner->GetFightBehaviour();
	}

	this->pOwner->SetLookingAtOff();
	this->pOwner->pAnimationController->RegisterBone(0x53538f8b);

	pCol = this->pOwner->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 | 0x10800;

	this->pOwner->scalarDynA.Reset();
	this->pOwner->scalarDynB.Reset();
	this->pOwner->scalarDynJump.Reset();

	_ComputeDynamics();

	pFighter = this->pOwner;
	fVar7 = (pFighter->fighterAnatomyZones).field_0x0.y + pFighter->currentLocation.y;
	pFighter->field_0x7d0 = fVar7;
	this->pOwner->field_0x7c8 = fVar7;

	pFighter = this->pOwner;
	iVar4 = pFighter->prevActorState;
	if (iVar4 == -1) {
		uVar6 = 0;
	}
	else {
		pSVar5 = pFighter->GetStateCfg(iVar4);
		uVar6 = pSVar5->flags_0x4 & 1;
	}

	if (uVar6 == 0) {
		this->pOwner->field_0x738 = this->pOwner->field_0x738 & 0xfffffffb;
	}
	else {
		this->pOwner->field_0x738 = this->pOwner->field_0x738 | 4;
	}

	if (newState == -1) {
		pFighter = this->pOwner;
		if ((pFighter->hitFlags & 2U) == 0) {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		else {
			pFighter->SetState(FIGHTER_PROJECTED_HIT_FLY, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->GetLifeInterfaceOther()->SetPriority(2);

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CBehaviourFighterProjected::End(int newBehaviourId)
{
	CActorFighter* pCVar2;

	this->pOwner->GetLifeInterfaceOther()->SetPriority(0);
	this->pOwner->pCollisionData->flags_0x0 = this->pOwner->pCollisionData->flags_0x0 & 0xfffeffff;
	this->pOwner->pCollisionData->flags_0x0 = this->pOwner->pCollisionData->flags_0x0 | 0x30;
	this->pOwner->field_0x7b0 = 0.0f;
	this->pOwner->field_0x7b4 = 0.0f;
	pCVar2 = this->pOwner;
	pCVar2->fightFlags = pCVar2->fightFlags & 0xffff7fff;
	pCVar2->fightFlags = pCVar2->fightFlags | 0x10000;

	this->pOwner->pAnimationController->UnRegisterBone(0x53538f8b);

	this->pOwner->fightFlags = this->pOwner->fightFlags | 0x2000;

	return;
}

// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CBehaviourFighterProjected::InitState(int newState)
{
	CActorFighter* pFighter;
	CCollision* pCVar2;
	CPlayerInput* pCVar3;
	Timer* pTVar4;
	int iVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack16;
	CAnimation* pAnimation;

	switch (newState) {
	case 0x54:
		this->pOwner->LifeDecrease(2.0f);
		break;
	case FIGHTER_PROJECTED_HIT_FLY:
	case FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE:
		pFighter = this->pOwner;

		pFighter->dynamicExt.field_0x6c = edF32Vector4SafeNormalize0Hard(&pFighter->field_0x740, &pFighter->field_0x740);
		pFighter->dynamicExt.normalizedTranslation = pFighter->field_0x740;

		if ((pFighter->typeID == ACTOR_HERO_PRIVATE) && (pCVar3 = pFighter->GetInputManager(1, 0), pCVar3 != (CPlayerInput*)0x0)) {
			fVar7 = edFIntervalUnitDstLERP(pFighter->dynamicExt.field_0x6c, 2.0f, 12.0f);
			fVar6 = edFIntervalUnitSrcLERP(fVar7, 0.2f, 0.1f);
			CPlayerInput::FUN_001b66f0(fVar7, 0.0f, fVar6, 0.0f, &pCVar3->field_0x40, 0);
		}

		pCVar2 = pFighter->pCollisionData;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffffcf;
		break;
	case FIGHTER_PROJECTED_HIT_SLIDE:
		pFighter = this->pOwner;
		pCVar2 = pFighter->pCollisionData;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x10;
		pFighter->ClearAllSumForceExt();
		pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
		pFighter->dynamicExt.field_0x6c = 0.0f;

		edF32Vector4ScaleHard(1.0f / GetTimer()->cutsceneDeltaTime, &eStack16, &pFighter->field_0x740);

		v0 = pFighter->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(v0, v0, &eStack16);
		fVar6 = edF32Vector4GetDistHard(pFighter->dynamicExt.aImpulseVelocities);
		pFighter->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;

		pFighter->field_0x7b0 = 0.0f;
		pFighter->field_0x7b4 = 0.0f;
		pFighter->field_0x7e0 = 0;
		pCVar2 = pFighter->pCollisionData;

		IMPLEMENTATION_GUARD_FX(
			CActorFighter::PlayOrientedFx
			((CActor*)pFighter, (undefined4*)&pCVar2->aCollisionContact[1].field_0x10,
				(edF32VECTOR4*)(pCVar2->aCollisionContact + 1), pFighter->field_0x53c, (int*)0x0);)
			break;
	case 0x58:
		pFighter = this->pOwner;
		pFighter->dynamic.speed = 0.0f;
		pFighter->ClearAllSumForceExt();
		pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
		pFighter->dynamicExt.field_0x6c = 0.0f;
		break;
	case FIGHTER_PROJECTED_HIT_WAKING_UP:
		this->pOwner->_StateFighterHitWakingUpInit();
		break;
	case FIGHTER_PROJECTED_HIT_STAGGER_CHECK:
		pFighter = this->pOwner;
		pFighter->fightFlags = pFighter->fightFlags & 0xffffffdf;
		pFighter->field_0x7e4 = 0;
		pFighter->field_0x7dc = pFighter->field_0x800;
		SetVectorFromAngleY(pFighter->rotationEuler.y, &pFighter->field_0x7f0);
		edF32Vector4ScaleHard(-pFighter->field_0x7dc, &pFighter->field_0x7f0, &pFighter->field_0x7f0);
		pFighter->dynamic.speed = 0.0f;
		pFighter->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		break;
	case 0x5b:
		pFighter = this->pOwner;
		pFighter->field_0x7b4 = CActorFighter::_pStagger_check[pFighter->field_0x7e4] / pFighter->_GetFighterAnimationLength(pFighter->currentAnimType);
		break;
	case 0x5d:
		pFighter = this->pOwner;
		pFighter->_StateFighterHitStaggerFallInit(this->pOwner->field_0x7dc);
		break;
	case 0x5e:
		this->pOwner->dynamic.speed = 0.0f;
		break;
	case 0x5f:
		pFighter->_StateFighterHitStaggerFallInit(-1.0f);
		break;
	case 0x60:
		this->pOwner->dynamic.speed = 0.0f;
		break;
	case 0x62:
		pFighter = this->pOwner;
		pFighter->dynamic.speed = 0.0;
		pFighter->ClearAllSumForceExt();
		pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
		pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
		pFighter->dynamicExt.field_0x6c = 0.0f;
		break;
	case 0x64:
		this->pOwner->_InterpretCollisions(1);
		break;
	case 0x5c:
		// Nothing
		break;
	default:
		IMPLEMENTATION_GUARD();
		break;
	}
	return;
}

void CBehaviourFighterProjected::TermState(int oldState, int newState)
{
	CActorFighter* pFighter;
	CCollision* pCol;
	float fVar3;

	if (oldState == 99) {
		pFighter = this->pOwner;
		pCol = pFighter->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 | 0x81000;
		pFighter->flags = pFighter->flags & 0xffffff5f;

		pFighter->EvaluateDisplayState();
	}
	else {
		if (oldState != 0x61) {
			if ((oldState == FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE) || (oldState == FIGHTER_PROJECTED_HIT_FLY)) {
				pFighter = this->pOwner;
				if ((newState == FIGHTER_PROJECTED_HIT_SLIDE) || (newState == 0x54)) {
					pFighter->Func_0x194(pFighter->field_0x7c8 - pFighter->field_0x7d0);
					fVar3 = (pFighter->fighterAnatomyZones).field_0x0.y + pFighter->currentLocation.y;
					pFighter->field_0x7d0 = fVar3;
					pFighter->field_0x7c8 = fVar3;
				}
			}
			else {
				if (oldState == 0x54) {
					this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
				}
			}
		}
	}

	return;
}

int CBehaviourFighterProjected::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorFighter* pFighter;
	CActor* pCVar2;
	StateConfig* pSVar3;
	int result;
	uint uVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	if (msg == 0x27) {
		result = 0;
	}
	else {
		if (msg == MESSAGE_IN_WIND_AREA) {
			NotifyWindParam* pNotifyWindParam = reinterpret_cast<NotifyWindParam*>(pMsgParam);
			edF32Vector4ScaleHard(pNotifyWindParam->field_0x10, &eStack16, &pNotifyWindParam->field_0x0);
			pFighter = this->pOwner;
			v0 = pFighter->dynamicExt.aImpulseVelocities + 2;
			edF32Vector4AddHard(v0, v0, &eStack16);
			pFighter->dynamicExt.aImpulseVelocityMagnitudes[2] = edF32Vector4GetDistHard(v0);
			result = 1;
		}
		else {
			if (msg == MESSAGE_KICKED) {
				pFighter = this->pOwner;
				uVar5 = pFighter->GetStateFlags(pFighter->actorState);

				if ((uVar5 & 0x2000000) == 0) {
					_msg_hit_param* pMsgHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);

					int projType = pMsgHitParam->projectileType;

					if (projType == 10) {
						this->pOwner->_UpdateDynForExplosion(pMsgHitParam);
						_ManageHit(false);
						result = 1;
					}
					else {
						if (projType == 8) {
							pFighter = this->pOwner;
							pFighter->field_0x690 = pMsgHitParam->field_0x40;
							pFighter->hitDamage = pMsgHitParam->damage;
							edF32Vector4SafeNormalize0Hard(&pFighter->field_0x6a0, &pMsgHitParam->field_0x20);

							pCVar2 = pFighter->field_0x634;
							if (pFighter->actorsExcludeTable.IsInList(pCVar2) == false) {
								pFighter->actorsExcludeTable.Add(1.0f, pCVar2);
							}

							pFighter->hitFlags = pMsgHitParam->flags;
							pFighter->field_0x474 = pMsgHitParam->field_0x10;
							pFighter->field_0x6b0 = pMsgHitParam->field_0x30;
							pFighter->field_0x684 = pMsgHitParam->field_0x50;
							pFighter->field_0x686 = pMsgHitParam->field_0x52;

							if ((pFighter->field_0x684 & 1U) != 0) {
								pFighter->field_0x7a0 = pMsgHitParam->field_0x60;
								pFighter->field_0x7b4 = pMsgHitParam->field_0x70;
							}

							_ManageHit((pMsgHitParam->flags & 0x80) == 0);

							result = 1;
						}
						else {
							if (projType == 7) {
								pFighter = this->pOwner;
								pFighter->field_0x690 = pMsgHitParam->field_0x40;
								pFighter->hitDamage = pMsgHitParam->damage;
								edF32Vector4SafeNormalize0Hard(&pFighter->field_0x6a0, &pMsgHitParam->field_0x20);

								pCVar2 = pFighter->field_0x634;
								if (pFighter->actorsExcludeTable.IsInList(pCVar2) == false) {
									pFighter->actorsExcludeTable.Add(-1.0f, pCVar2);
								}

								pFighter->hitFlags = pMsgHitParam->flags;
								pFighter->field_0x474 = pMsgHitParam->field_0x10;
								pFighter->field_0x6b0 = pMsgHitParam->field_0x30;
								pFighter->field_0x684 = pMsgHitParam->field_0x50;
								pFighter->field_0x686 = pMsgHitParam->field_0x52;

								if ((pFighter->field_0x684 & 1U) != 0) {
									pFighter->field_0x7a0 = pMsgHitParam->field_0x60;
									pFighter->field_0x7b4 = pMsgHitParam->field_0x70;
								}

								_ManageHit((pMsgHitParam->flags & 0x80) == 0);

								result = 1;
							}
							else {
								result = 0;
							}
						}
					}
				}
				else {
					result = 0;
				}
			}
			else {
				result = 0;
			}
		}
	}

	return result;
}

bool CBehaviourFighterProjected::Execute(s_fighter_action* param_2, s_fighter_action_param* param_3)
{
	return false;
}

void CBehaviourFighterProjected::_ManageHit(bool bPlayImpact)
{
	this->pOwner->LifeDecrease(this->pOwner->hitDamage);
	this->pOwner->hitDamage = 0.0f;

	if ((this->pOwner->hitFlags & 1U) == 0) {
		(this->pOwner->field_0x6a0).y = 2.0f;
		edF32Vector4NormalizeHard(&this->pOwner->field_0x6a0, &this->pOwner->field_0x6a0);
		this->pOwner->field_0x6b0 = 5.0f;
		this->pOwner->field_0x686 = 1;
		this->pOwner->field_0x684 = 0xe;

		_ComputeDynamics();

		this->pOwner->SetState(-1, -1);
		this->pOwner->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
	}
	else {
		_ComputeDynamics();

		this->pOwner->SetState(-1, -1);

		if (((this->pOwner->hitFlags & 2U) == 0) || ((this->pOwner->hitFlags & 4U) != 0)) {
			this->pOwner->SetState(FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE, -1);
		}
		else {
			this->pOwner->SetState(FIGHTER_PROJECTED_HIT_FLY, -1);
		}
	}

	if (bPlayImpact == true) {
		IMPLEMENTATION_GUARD_FX(
			pCVar1 = this->pOwner;
		CActorFighter::PlayImpactFx
		(pCVar1, (long)(int)&pCVar1->field_0x690, &pCVar1->field_0x6a0, (ulong)((pCVar1->hitFlags & 1U) != 0), '\0');)
	}
	return;
}


// Should be in: D:/Projects/b-witch/ActorFighterProjected.cpp
void CBehaviourFighterProjected::_ComputeDynamics()
{
	ushort uVar1;
	CActorFighter* pFighter;
	ulong uVar3;
	edF32VECTOR4* v0;
	float fVar4;
	float fVar5;
	float fVar6;
	float unaff_f20;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	uVar1 = this->pOwner->field_0x684;
	this->pOwner->dynamic.speed = 0.0f;
	pFighter = this->pOwner;
	pFighter->dynamicExt.normalizedTranslation.x = 0.0f;
	pFighter->dynamicExt.normalizedTranslation.y = 0.0f;
	pFighter->dynamicExt.normalizedTranslation.z = 0.0f;
	pFighter->dynamicExt.normalizedTranslation.w = 0.0f;
	pFighter->dynamicExt.field_0x6c = 0.0f;

	edF32Vector4SafeNormalize0Hard(&this->pOwner->field_0x740, &this->pOwner->field_0x6a0);

	v0 = &this->pOwner->field_0x740;
	edF32Vector4ScaleHard(this->pOwner->field_0x6b0, v0, v0);
	edF32Matrix4FromEulerSoft(&this->pOwner->field_0x760, &this->pOwner->rotationEuler.xyz, "XYZ");

	pFighter = this->pOwner;
	edF32Matrix4MulF32Vector4Hard(&pFighter->fighterAnatomyZones.field_0x0, &pFighter->field_0x760, &(pFighter->fighterAnatomyZones).field_0x10);

	if ((uVar1 & 1) == 0) {
		pFighter = this->pOwner;
		local_10 = pFighter->field_0x740;

		if ((uVar1 & 2) != 0) {
			if ((uVar1 & 8) == 0) {
				pFighter = this->pOwner;
				uVar3 = pFighter->_SV_HIT_GetHitZoneFromImpact(&pFighter->fighterAnatomyZones, &pFighter->field_0x690, &pFighter->currentLocation);
			}
			else {
				uVar3 = (ulong)this->pOwner->field_0x686;
			}

			if (uVar3 == 1) {
				pFighter = this->pOwner;
				local_30.x = (pFighter->fighterAnatomyZones).field_0x10.x;
				local_30.y = (pFighter->fighterAnatomyZones).field_0x10.y;
				local_30.z = (pFighter->fighterAnatomyZones).field_0x10.z;
				local_30.w = (pFighter->fighterAnatomyZones).field_0x10.w;
				unaff_f20 = ((this->pOwner->pCollisionData)->pObbPrim->field_0x90).z;
			}
			else {
				if (uVar3 == 2) {
					edF32Vector4ScaleHard(this->pOwner->field_0x428, &local_30, &(this->pOwner->fighterAnatomyZones).field_0x10);
					local_30.w = 1.0f;
					unaff_f20 = ((this->pOwner->pCollisionData)->pObbPrim->field_0x90).z / 2.0f;
				}
				else {
					if (uVar3 == 0) {
						edF32Vector4ScaleHard(this->pOwner->field_0x42c, &local_30, &(this->pOwner->fighterAnatomyZones).field_0x10);
						local_30.w = 1.0f;
						unaff_f20 = ((this->pOwner->pCollisionData)->pObbPrim->field_0x90).z / 2.0f;
					}
				}
			}

			edF32Matrix4MulF32Vector4Hard(&local_30, &this->pOwner->pMeshTransform->base.transformA, &local_30);

			if ((uVar1 & 4) == 0) {
				pFighter = this->pOwner;
				local_40.xyz = (pFighter->fighterAnatomyZones).field_0x0.xyz;
				local_40.w = 0.0f;
				edF32Vector4NormalizeHard(&local_40, &local_40);
				edF32Vector4SubHard(&eStack80, &this->pOwner->field_0x634->currentLocation, &local_30);
				fVar4 = edF32Vector4DotProductHard(&local_40, &eStack80);
				edF32Vector4ScaleHard(fVar4, &local_40, &local_40);
				edF32Vector4SubHard(&eStack80, &eStack80, &local_40);
				edF32Vector4SafeNormalize0Hard(&eStack80, &eStack80);
				edF32Vector4ScaleHard(unaff_f20, &eStack80, &eStack80);
				edF32Vector4AddHard(&this->pOwner->field_0x690, &local_30, &eStack80);
			}
			else {
				pFighter = this->pOwner;
				pFighter->field_0x690 = local_30;
			}
		}

		edF32Matrix4MulF32Vector4Hard(&eStack32, &this->pOwner->pMeshTransform->base.transformA, &(this->pOwner->fighterAnatomyZones).field_0x10);
		pFighter = this->pOwner;
		pFighter->_SV_VOLDYN_VolumeSpeedFromPointSpeed(&pFighter->field_0x740, &pFighter->field_0x7a0, &pFighter->field_0x7b4, &eStack32, &pFighter->field_0x690, &local_10);
	}

	this->pOwner->field_0x7b0 = 0.0f;

	pFighter = this->pOwner;
	pFighter->field_0x750 = pFighter->field_0x740;

	return;
}
