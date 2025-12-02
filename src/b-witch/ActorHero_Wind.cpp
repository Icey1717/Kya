#include "ActorHero_Private.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "TimeController.h"
#include "InputManager.h"
#include "EventManager.h"
#include "WayPoint.h"

void CActorHeroPrivate::ComputeBlendingWeightsInHitGlide()
{
	CCollision* pCVar1;
	ed_3d_hierarchy_node* v1;
	float fVar2;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;
	v1 = this->pMeshTransform;

	local_20.x = 0.0f;
	local_20.y = 0.0f;
	local_20.z = 0.0f;
	local_20.w = 0.0f;

	local_30.x = this->currentLocation.x;
	local_30.z = this->currentLocation.z;
	local_30.w = this->currentLocation.w;
	local_30.y = this->currentLocation.y + (pCVar1->pObbPrim->field_0xb0).y;

	if ((pCVar1->flags_0x4 & 2) != 0) {
		edF32Vector4SubHard(&eStack16, &pCVar1->aCollisionContact[1].field_0x10, &local_30);
		edF32Vector4AddHard(&local_20, &local_20, &eStack16);
	}

	if ((pCVar1->flags_0x4 & 1) != 0) {
		edF32Vector4SubHard(&eStack16, &pCVar1->aCollisionContact[0].field_0x10, &local_30);
		edF32Vector4AddHard(&local_20, &local_20, &eStack16);
	}

	if ((pCVar1->flags_0x4 & 4) != 0) {
		edF32Vector4SubHard(&eStack16, &pCVar1->aCollisionContact[2].field_0x10, &local_30);
		edF32Vector4AddHard(&local_20, &local_20, &eStack16);
	}

	edF32Vector4SafeNormalize1Hard(&local_20, &local_20);

	this->field_0x1200 = edF32Vector4DotProductHard(&local_20, &v1->base.transformA.rowX);
	this->field_0x1204 = edF32Vector4DotProductHard(&local_20, &v1->base.transformA.rowY);
	this->field_0x1208 = edF32Vector4DotProductHard(&local_20, &v1->base.transformA.rowZ);

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::ResetWindDefaultSettings()
{
	this->windBoostStrength = 0.0f;
	this->windRotationStrength = 0.0f;
	this->field_0x11fc = 0.0f;
	this->field_0x1200 = 0.0f;
	this->field_0x1208 = 0.0f;
	this->field_0x1204 = 0.0f;
	this->field_0x13d8 = 0.0f;
	this->field_0x11ec = 0.0f;
	this->field_0x11e0 = 12.0f;
	this->field_0x11e4 = 12.0f;
	this->field_0x11e8 = 20.0f;
	this->field_0x13d4 = 6.0f;
	this->windWallHorizontalSpeed = 1.25f;
	this->windWallVerticalSpeed = 1.25f;
	this->field_0x13e4 = 3.5f;
	this->field_0x1410 = 1.0f;
	this->field_0x1414 = 2.0f;
	this->field_0x1418 = 5.0f;
	this->field_0x141c = 10.0f;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
int CActorHeroPrivate::ChooseStateLanding(float speed)
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* peVar2;
	bool bVar3;
	StateConfig* pAVar4;
	uint uVar5;
	Timer* pTVar6;
	undefined4 uVar7;
	edF32VECTOR4* peVar8;
	int landingState;
	CPlayerInput* lVar9;
	edF32VECTOR4* peVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	landingState = this->actorState;

	fVar12 = 0.0f;

	if ((GetStateFlags(this->actorState) & 0x100) != 0) {
		return STATE_HERO_STAND;
	}

	this->landSpeed = speed;

	if ((this->field_0x1420 == 0) || (this->field_0x1020 == 0)) {
	LAB_0014c070:
		this->field_0x1020 = 1;

		bVar3 = CanEnterToboggan();
		if (bVar3 == false) {
			landingState = STATE_HERO_SLIDE_A;

			if ((this->dynamic.flags & 2) == 0) {
				if (0.0f < fVar12) {
					pTVar6 = GetTimer();
					fVar12 = edFIntervalLERP(pTVar6->scaledTotalTime - this->field_0x1548, this->field_0x1410, this->field_0x1414, 1.0f, 0.2f);
					this->field_0x118c = fVar12;
					landingState = 0x81;
				}
				else {
					pCVar1 = this->pPlayerInput;
					if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar12 = 0.0f;
					}
					else {
						fVar12 = pCVar1->aButtons[5].clickValue;
					}

					if (fVar12 == 0.0f) {
						landingState = STATE_HERO_JUMP_3_3_STAND;

						if (2.0f < this->dynamic.horizontalLinearAcceleration) {
							landingState = STATE_HERO_JUMP_3_3_RUN;
						}
					}
					else {
						if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar12 = 0.0f;
						}
						else {
							fVar12 = pCVar1->aAnalogSticks[PAD_STICK_LEFT].magnitude;
						}

						landingState = 0x89;

						if (fVar12 <= 0.3f) {
							landingState = 0x83;
						}
					}
				}
			}
		}
		else {
			pTVar6 = GetTimer();
			fVar12 = edFIntervalUnitDstLERP(pTVar6->scaledTotalTime - this->time_0x1538, 0.3f, 1.5f);
			this->field_0x10f4 = fVar12;
			pTVar6 = GetTimer();
			fVar12 = edFIntervalLERP(pTVar6->scaledTotalTime - this->time_0x1538, 0.3f, 1.5f, 0.5f, 1.5f);
			this->field_0x1108 = fVar12;
			landingState = STATE_HERO_TOBOGGAN;
		}
	}
	else {
		pTVar6 = GetTimer();
		fVar12 = edFIntervalLERP(pTVar6->scaledTotalTime - this->field_0x1548, this->field_0x1410, this->field_0x1414, this->field_0x1418, this->field_0x141c);

		if (this->field_0x1424 == 0) {
			CLifeInterface* pLifeInterface = GetLifeInterface();
			fVar11 = pLifeInterface->GetValue();
			if (0.0f < fVar11 - fVar12) {
				lVar9 = GetInputManager(1, 0);
				if (lVar9 != 0) {
					CPlayerInput::FUN_001b66f0(1.0f, 0.0f, 0.1f, 0.0f, &lVar9->field_0x40, 0);
				}
				LifeDecrease(fVar12);
				SetInvincible(2.0f, 1);
				goto LAB_0014c070;
			}
		}

		if (this->field_0x1424 == 0) {
			LifeDecrease(fVar12);
		}
		else {
			this->field_0x2e4 = fVar12;
		}

		bVar3 = CanEnterToboggan();
		if (bVar3 == false) {
			local_10 = gF32Vector4Zero;
			local_20 = gF32Vector4Zero;

			local_10.x = this->rotationEuler.x + 1.570796f;
			local_10.y = this->rotationEuler.y;
			local_10.z = this->rotationEuler.z;

			const static edF32VECTOR4 edF32VECTOR4_0040e6e0 = { 0.0f, -2.0f, 0.0f, 0.0f };

			local_20 = edF32VECTOR4_0040e6e0;

			edF32Vector4AddHard(&local_20, &local_20, &this->dynamic.rotationQuat);

			edF32Vector4NormalizeHard(&local_20, &local_20);

			_StateFighterHitFall(speed, &local_10, &local_20, 0);

			pTVar6 = GetTimer();
			landingState = -1;
			this->field_0x1558 = pTVar6->scaledTotalTime + 1.5f;
		}
		else {
			pTVar6 = GetTimer();
			landingState = 0x9c;
			this->field_0x1558 = pTVar6->scaledTotalTime + 1.5f;
		}
	}

	return landingState;
}


// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroWindSlideInit()
{
	this->field_0x13d8 = 0.0f;
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroWindSlide(int nextState)
{
	CCollision* pCollision;
	CPlayerInput* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	bool bVar5;
	undefined4 uVar6;
	int iVar7;
	StateConfig* pSVar8;
	Timer* pTVar9;
	int* piVar11;
	int iVar12;
	uint uVar13;
	edF32VECTOR4* peVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	edF32VECTOR4 aeStack208[2];
	edF32VECTOR4 local_b0[2];
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	int local_18;
	int local_14;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	pCollision = this->pCollisionData;

	IncreaseEffort(0.5f);

	local_30 = this->dynamicExt.aImpulseVelocities[2];
	fVar17 = this->dynamicExt.aImpulseVelocityMagnitudes[2];

	UpdateOrientationFromWind(&local_30, &local_40);

	pCVar2 = this->pPlayerInput;
	if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		local_c = 0.0f;
		local_8 = 0.0f;
		local_4 = 0.0f;
	}
	else {
		pCVar2->GetPadRelativeToNormal2D(&this->dynamic.velocityDirectionEuler, &local_4, &local_8, &local_c);
	}

	SV_UpdatePercent(local_4, 0.9f, &this->field_0x13d8);

	pCVar2 = this->pPlayerInput;
	if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		local_60 = gF32Vector4Zero;
	}
	else {
		edF32Vector4ScaleHard(pCVar2->aAnalogSticks[PAD_STICK_LEFT].magnitude, &local_60, &pCVar2->lAnalogStick);
	}

	edF32Vector4ScaleHard(this->field_0x13d4, &local_60, &local_60);
	edProjectVectorOnPlane(0.0f, &local_60, &local_60, &local_40, 0);

	fVar15 = edF32Vector4SafeNormalize1Hard(&local_60, &local_60);
	if (0.0f < local_60.x * local_40.z - local_40.x * local_60.z) {
		fVar15 = -fVar15;
	}

	edF32Vector4CrossProductHard(&local_70, &local_40, &g_xVector);
	local_70.y = 0.0f;
	edF32Vector4SafeNormalize1Hard(&local_70, &local_70);

	local_10 = this->dynamic.speed;
	if (0.0f < this->dynamic.rotationQuat.x * local_40.z -
		local_40.x * this->dynamic.rotationQuat.z) {
		local_10 = -local_10;
	}

	SV_UpdateValue(fVar15, 20.0f, &local_10);

	if (0.0f <= local_10) {
		this->dynamic.speed = local_10;
		this->dynamic.rotationQuat = local_70;
	}
	else {
		this->dynamic.speed = -local_10;
		edF32Vector4ScaleHard(-1.0f, &local_70, &local_70);
		this->dynamic.rotationQuat = local_70;
	}

	if ((pCollision->flags_0x4 & 2) != 0) {
		if (GetWindState() == (CActorWindState*)0x0) {
			fVar15 = 0.0f;
		}
		else {
			fVar15 = GetWindState()->field_0xc;
		}

		if (0.0f < fVar15) {
			if (GetWindState() == (CActorWindState*)0x0) {
				fVar15 = 0.0f;
			}
			else {
				fVar15 = GetWindState()->field_0xc;
			}

			ComputeFrictionForceWithSpeedMax(fVar15, &eStack80, 1);
			peVar14 = this->dynamicExt.aImpulseVelocities + 1;
			edF32Vector4AddHard(peVar14, peVar14, &eStack80);
			fVar15 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities + 1);
			this->dynamicExt.aImpulseVelocityMagnitudes[1] = fVar15;
		}
	}

	ManageDyn(4.0, 0x33, (CActorsTable*)0x0);

	if ((GetStateFlags(this->actorState) & 1) == 0) {
		if (nextState != -1) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(nextState, 0xffffffff);
				return;
			}
		}

		if ((pCollision->flags_0x4 & 2) == 0) {
			if (this->field_0x1184 < this->timeInAir) {
				SetState(0xf5, 0xffffffff);
				return;
			}
		}
		else {
			this->timeInAir = 0.0f;
		}

		pCVar2 = this->pPlayerInput;
		if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar13 = 0;
		}
		else {
			uVar13 = pCVar2->pressedBitfield & PAD_BITMASK_CROSS;
		}

		if (uVar13 == 0) {
			bVar5 = false;
			if (GetWindState() != (CActorWindState*)0x0) {
				iVar7 = GetWindState()->field_0x0;
				bVar5 = true;
				if (iVar7 != GetWindState()->field_0x4) {
					bVar5 = false;
					if ((GetWindState()->field_0x4 != 0) && (bVar5 = true, 0.17398384f <= fabs(local_30.y))) {
						bVar5 = false;
					}
				}
			}

			if (bVar5) {
				bVar5 = false;
				if (GetWindState() != (CActorWindState*)0x0) {
					iVar7 = GetWindState()->field_0x0;
					bVar5 = true;
					if (iVar7 != GetWindState()->field_0x4) {
						bVar5 = false;
						if ((GetWindState()->field_0x4 != 0) && (bVar5 = true, 0.17398384f <= fabs(local_30.y))) {
							bVar5 = false;
						}
					}
				}

				if ((!bVar5) || (bVar5 = true, fVar17 <= 2.0)) {
					bVar5 = false;
				}

				if (!bVar5) {
					bVar5 = CanEnterToboggan();
					if (bVar5 != false) {
						SetState(0xe8, 0xffffffff);
						return;
					}

					if ((this->dynamic.flags & 2) != 0) {
						SetState(0xe4, 0xffffffff);
						return;
					}

					edF32Vector4ScaleHard(10.0f, &eStack128, &this->dynamic.horizontalVelocityDirectionEuler);
					peVar14 = this->dynamicExt.aImpulseVelocities;
					edF32Vector4AddHard(peVar14, peVar14, &eStack128);
					fVar17 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
					this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar17;
					SetState(0xe1, 0xffffffff);
					return;
				}
			}
			else {
				if (0.001 < fVar17) {
					SetState(0xf2, 0xffffffff);
					return;
				}
			}

			if (((pCollision->flags_0x4 & 1) != 0) && (DetectStickableWalls(&local_40, &local_14, &local_18, &local_90), local_14 != 0)) {
				if (local_18 == 0) {
					this->dynamicExt.normalizedTranslation.x = 0.0f;
					this->dynamicExt.normalizedTranslation.y = 0.0f;
					this->dynamicExt.normalizedTranslation.z = 0.0f;
					this->dynamicExt.normalizedTranslation.w = 0.0f;
					this->dynamicExt.field_0x6c = 0.0f;
				}
				else {
					edF32Vector4ScaleHard(-1.0f, &this->field_0x13f0, &local_90);
					this->rotationQuat = this->field_0x13f0;

					this->field_0x13f0 = local_90;

					uVar6 = STATE_HERO_WIND_WALL_MOVE_A;
					if (this->field_0xa84 <= 0.1f) {
						uVar6 = STATE_HERO_WIND_WALL_MOVE_B;
					}

					SetState(uVar6, 0xffffffff);
				}
			}
		}
		else {
			this->field_0xf00 = this->currentLocation;
			local_b0[0].y = 700.0f;
			local_b0[0].x = 0.0f;
			local_b0[0].z = 0.0f;
			local_b0[0].w = 0.0f;
			pTVar9 = GetTimer();
			edF32Vector4ScaleHard(0.02f / pTVar9->cutsceneDeltaTime, aeStack208, local_b0);
			peVar14 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(peVar14, peVar14, aeStack208);
			fVar17 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar17;
			SetState(0xf5, 0xffffffff);
		}
	}
	else {
		if (2.0f < this->timeInAir) {
			IMPLEMENTATION_GUARD(
				(*(code*)(this->pVTable)->field_0x16c)(this);)
		}
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroFlyInit()
{
	int* piVar1;
	int iVar2;
	Timer* pTVar3;

	this->field_0x11ec = 0.0f;
	this->windBoostStrength = 0.0f;
	this->windRotationStrength = 0.0f;
	ConvertSpeedSumForceExtToSpeedPlayer2D();
	pTVar3 = GetTimer();
	this->field_0x1548 = pTVar3->scaledTotalTime;
	this->field_0x1420 = 0;
	this->field_0x1424 = 0;
	this->field_0xcb4.Init(0.0f, 800.0f);
	this->field_0x1428 = 1;

	IMPLEMENTATION_GUARD_LOG(
		CFxHandle::SV_FX_Start((CFxHandle*)&this->field_0x13c0);
	piVar1 = (int*)this->field_0x13c4;
	if (((piVar1 != (int*)0x0) && (iVar2 = *(int*)&this->field_0x13c0, iVar2 != 0)) && (iVar2 == piVar1[6])) {
		(**(code**)(*piVar1 + 0x10))(0, 0);
	})

		return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroFlyTerm()
{
	CFxHandle* pCVar1;
	int iVar2;
	bool bVar3;
	Timer* pTVar4;
	CCamera* pCVar5;
	CCameraManager* pCameraManager;

	IMPLEMENTATION_GUARD_LOG(
		pCVar1 = this->field_0x13c4;
	if (((pCVar1 == (CFxHandle*)0x0) || (iVar2 = this->field_0x13c0, iVar2 == 0)) ||
		(bVar3 = true, iVar2 != pCVar1[2].id)) {
		bVar3 = false;
	}
	if (bVar3) {
		if (((pCVar1 != (CFxHandle*)0x0) && (iVar2 = this->field_0x13c0, iVar2 != 0)) &&
			(iVar2 == pCVar1[2].id)) {
			(**(code**)(pCVar1->id + 0xc))();
		}
		this->field_0x13c4 = (CFxHandle*)0x0;
		this->field_0x13c0 = 0;
	}
	this->field_0x13cc = 0;)

		pTVar4 = GetTimer();
	this->field_0x1548 = pTVar4->scaledTotalTime;
	this->field_0x1420 = 0;
	this->field_0x1424 = 0;
	RestoreCollisionSphere(0.1f);
	RestoreVerticalOrientation();
	pCameraManager = CCameraManager::_gThis;
	pCVar5 = CCameraManager::_gThis->GetDefGameCamera((ECameraType)9);
	pCameraManager->PopCamera(pCVar5);
	pCVar5 = pCameraManager->GetDefGameCamera((ECameraType)8);
	pCameraManager->PopCamera(pCVar5);
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroGlideInit()
{
	return;
}

void CActorHeroPrivate::StateHeroGlide(int param_2, int nextState)
{
	byte bVar1;
	CCollision* pCVar2;
	CAnimation* pCVar3;
	ed_zone_3d* pZone;
	edAnmLayer* peVar4;
	bool bVar5;
	bool bVar6;
	CEventManager* pCVar7;
	bool bVar8;
	Timer* pTVar9;
	uint uVar10;
	int iVar11;
	StateConfig* pAVar12;
	CCamera* pCVar13;
	ECameraType EVar14;
	CActorWindState* uVar15;
	CActorWindState* iVar16;
	CActorWindState* pCVar17;
	StateConfig* pAVar18;
	CPlayerInput* pCVar19;
	int iVar20;
	CActorWindState* piVar21;
	uint uVar22;
	ulong uVar23;
	long lVar24;
	edF32VECTOR4* peVar25;
	float fVar26;
	float fVar27;
	float fVar28;
	float fVar29;
	float fVar30;
	float fVar31;
	edF32VECTOR4 eStack416;
	edF32VECTOR4 local_190;
	edF32VECTOR4 local_180;
	edF32VECTOR4 local_170;
	edF32VECTOR4 local_160;
	edF32VECTOR4 local_150;
	edF32VECTOR4 eStack320;
	CActorsTable local_130;
	edF32VECTOR4 local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;
	CCameraManager* pCameraManager;

	iVar20 = 0;
	pCVar2 = this->pCollisionData;
	pCVar3 = this->pAnimationController;
	bVar5 = false;
	local_130.nbEntries = 0;
	uVar22 = 0;

	fVar26 = edFIntervalUnitSrcLERP(this->field_0x11ec, 0.5f, 1.0f);
	IncreaseEffort(fVar26);

	local_20 = this->dynamicExt.aImpulseVelocities[2];

	fVar26 = this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y;
	fVar31 = this->dynamicExt.aImpulseVelocityMagnitudes[2];
	bVar6 = 0.001f < fVar31;
	if (this->field_0x1428 == 0) {
		pTVar9 = GetTimer();
		this->field_0x1548 = pTVar9->scaledTotalTime;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		ClearAllSumForceExt();
	}
	else {
		bVar8 = UpdateOrientationFromWind(&local_20, (edF32VECTOR4*)0x0);
		iVar20 = (int)bVar8;

		pTVar9 = GetTimer();
		fVar27 = powf(0.9f, pTVar9->cutsceneDeltaTime * 50.0f);
		if (1.0f < fVar27) {
			fVar27 = 1.0f;
		}
		else {
			if (fVar27 < 0.0f) {
				fVar27 = 0.0f;
			}
		}

		bVar1 = pCVar2->flags_0x4;
		if (((bVar1 & 2) == 0) && ((bVar1 & 4) == 0)) {
			uVar10 = TestState_IsInHit(0xffffffff);
			if ((uVar10 == 0) || (bVar6)) {
				pCVar19 = this->pPlayerInput;
				if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar28 = 0.0f;
				}
				else {
					fVar28 = pCVar19->aButtons[4].clickValue;
				}

				if ((int)fVar28 < 0) {
					fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
					fVar28 = fVar28 + fVar28;
				}
				else {
					fVar28 = (float)(int)fVar28;
				}

				this->field_0x11ec = this->field_0x11ec * fVar27 + (1.0f - fVar27) * fVar28;
			}
			else {
				this->field_0x11ec = 0.0f;
				this->windBoostStrength = 0.0f;
			}
		}
		else {
			pCVar19 = this->pPlayerInput;
			if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar28 = 0.0f;
			}
			else {
				fVar28 = pCVar19->aButtons[4].clickValue;
			}

			if ((int)fVar28 < 0) {
				fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
				fVar28 = fVar28 + fVar28;
			}
			else {
				fVar28 = (float)(int)fVar28;
			}

			this->field_0x11ec = fVar28;
		}

		pCVar19 = this->pPlayerInput;
		if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar28 = 0.0f;
		}
		else {
			fVar28 = pCVar19->aButtons[4].clickValue;
		}

		if ((int)fVar28 < 0) {
			fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
			fVar28 = fVar28 + fVar28;
		}
		else {
			fVar28 = (float)(int)fVar28;
		}

		this->windBoostStrength = this->windBoostStrength * fVar27 + (1.0f - fVar27) * fVar28;

		ChangeCollisionSphereForGlide(0.1f, this->windBoostStrength);

		if ((1.0f < this->field_0xa88) || (bVar6)) {
			pTVar9 = GetTimer();
			this->field_0x1548 = pTVar9->scaledTotalTime;
			this->field_0x1420 = 0;
			this->field_0x1424 = 0;
		}
		else {
			pTVar9 = GetTimer();
			if (this->field_0x1410 < pTVar9->scaledTotalTime - this->field_0x1548) {
				uVar22 = 0x40000;
				this->field_0x1420 = 1;
			}
			else {
				this->field_0x11f0 = this->currentLocation.y;
			}

			pTVar9 = GetTimer();
			if (this->field_0x1414 < pTVar9->scaledTotalTime - this->field_0x1548) {
				this->field_0x1424 = 1;
				this->animKey_0x157c = this->currentLocation.y + 20.0f;

				pCVar7 = CScene::ptable.g_EventManager_006f5080;
				int curZoneIndex = 0;
				if (2.24f <= CScene::_pinstance->field_0x1c) {
					while (true) {
						S_ZONE_STREAM_REF* pZoneStreamRef = this->field_0xe48;
						if (pZoneStreamRef == (S_ZONE_STREAM_REF*)0x0) {
							iVar11 = 0;
						}
						else {
							iVar11 = pZoneStreamRef->entryCount;
						}

						if (iVar11 <= curZoneIndex) goto LAB_00149ea0;

						pZone = pZoneStreamRef->aEntries[curZoneIndex].Get();

						if ((pZone != (ed_zone_3d*)0x0) && (uVar10 = edEventComputeZoneAgainstVertex(pCVar7->activeChunkId, pZone, &this->currentLocation, 0), (uVar10 & 1) != 0)) break;

						curZoneIndex = curZoneIndex + 1;
					}

					bVar8 = true;
				}
				else {
				LAB_00149ea0:
					bVar8 = false;
				}

				if (!bVar8) {
					if (this->distanceToGround < 10.3f) {
						this->field_0x11f0 = this->currentLocation.y;
					}
					else {
						if (100.0f < this->field_0x11f0 - this->currentLocation.y) {
							bVar5 = true;
						}
					}
				}
			}
			else {
				if (this->distanceToGround < 1.0f) {
					CLifeInterface* pLifeInterface = GetLifeInterface();
					fVar27 = pLifeInterface->GetValue();
					bVar8 = fVar27 - this->field_0x2e4 <= 0.0f;
					if (!bVar8) {
						bVar8 = (GetStateFlags(this->actorState) & 1) != 0;
					}

					if (!bVar8) {
						PlayAnim(0x103);
						goto LAB_0014a028;
					}
				}

				pAVar12 = GetStateCfg(this->actorState);
				PlayAnim(pAVar12->animId);
			}
		}
	}
LAB_0014a028:
	if (bVar6) {
		SV_UpdateValue(0.0f, 1.2f, &this->field_0x13cc);
		IMPLEMENTATION_GUARD_AUDIO(
			SV_FX_Sound_SetVolume(this->field_0x13cc, (CFxHandle*)&this->field_0x13c0);)
	}
	else {
		IMPLEMENTATION_GUARD_AUDIO(
			SV_UpdateValue
			(edFCosinus
				[(int)(fabs((this->timeInAir * 2.5 - 1.570796) * 1303.797) + 0.5)
				& 0x1fff] * 0.1 + 1.0, (float)&DAT_3f4ccccd, this, &this->field_0x13cc);
		SV_FX_Sound_SetVolume(this->field_0x13cc, (CFxHandle*)&this->field_0x13c0);
			)

			fVar27 = fabs(this->field_0xa88);
		if (fVar27 < 19.0f) {
			fVar28 = edFIntervalLERP(fVar27, 0.0f, 19.0f, 0.9f, 1.1f);
			this->field_0x13d0 = fVar28;
		}
		else {
			fVar28 = edFIntervalLERP(fVar27, 19.0f, 27.0f, 1.1f, 1.5f);
			this->field_0x13d0 = fVar28;
		}

		IMPLEMENTATION_GUARD_AUDIO(
			fVar28 = edFCosinus
			[(int)(fabs((this->timeInAir * 4.0 - 1.570796) * 1303.797) + 0.5) &
			0x1fff];)

			fVar28 = cosf(this->timeInAir * 4.0f - 1.570796f);

		if (fVar27 < 5.0) {
			fVar27 = edFIntervalLERP(fVar27, 0.0, 5.0, 0.2, 0.1);
			fVar28 = fVar28 * fVar27;
		}
		else {
			fVar27 = edFIntervalLERP(fVar27, 5.0, 27.0, 0.1, 0.02);
			fVar28 = fVar28 * fVar27;
		}

		this->field_0x13d0 = this->field_0x13d0 + fVar28;
		IMPLEMENTATION_GUARD_AUDIO(
			piVar21 = (int*)this->field_0x13c4;
		if ((piVar21 != (int*)0x0) && (*(int*)&this->field_0x13c0 != 0)) {
			(**(code**)(*piVar21 + 0x40))();
		})
	}

	pCameraManager = CCameraManager::_gThis;
	pTVar9 = GetTimer();
	if ((pTVar9->scaledTotalTime - this->field_0x1548 <= this->field_0x1410) || (this->distanceToGround != this->field_0xf0)) {
		pCVar13 = pCameraManager->GetDefGameCamera((ECameraType)9);
		pCameraManager->PushCamera(pCVar13, 0);
		pCVar13 = pCameraManager->GetDefGameCamera((ECameraType)8);
		pCameraManager->PushCamera(pCVar13, 0);
	}

	EVar14 = pCameraManager->pActiveCamera->GetMode();
	if (EVar14 == 8) {
		pCVar13 = pCameraManager->pActiveCamera;
		this->field_0xcb4.UpdateLerp(this->rotationEuler.z);
		pCVar13->SetAngleGamma(this->field_0xcb4.currentAlpha);
	}

	if (this->field_0x1428 == 0) {
		fVar27 = edFIntervalUnitDstLERP(this->scalarDynA.field_0x4, this->scalarDynA.duration, 0.0f);
		this->field_0x11fc = fVar27;

		pTVar9 = GetTimer();
		this->scalarDynJump.Integrate(pTVar9->cutsceneDeltaTime);
		pTVar9 = GetTimer();
		this->scalarDynA.Integrate(pTVar9->cutsceneDeltaTime);
		fVar27 = this->scalarDynA.GetInstantSpeed();
		edF32Vector4ScaleHard(fVar27, &local_180, &this->field_0x1400);
		local_180.y = this->scalarDynJump.field_0x20;
		fVar27 = edF32Vector4NormalizeHard(&local_180, &local_180);
		this->dynamic.rotationQuat = local_180;
		this->dynamic.speed = fVar27;

		if (0.001f < this->dynamic.linearAcceleration) {
			local_190.x = this->dynamic.velocityDirectionEuler.x;
			local_190.z = this->dynamic.velocityDirectionEuler.z;
			local_190.w = this->dynamic.velocityDirectionEuler.w;
			local_190.y = this->dynamic.velocityDirectionEuler.y * this->field_0x11fc;

			edF32Vector4SafeNormalize1Hard(&local_190, &local_190);

			this->rotationQuat = local_190;
		}

		ManageDyn(4.0f, 0x4000d, (CActorsTable*)0x0);
		uVar23 = this->scalarDynJump.OnLastValidSample();
		if (uVar23 != 0) {
			SetState(STATE_HERO_GLIDE_1, 0xffffffff);
			return;
		}
	}
	else {
		pCVar19 = this->pPlayerInput;
		fVar27 = 0.0f;
		if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar28 = 0.0f;
		}
		else {
			fVar28 = pCVar19->aAnalogSticks[PAD_STICK_LEFT].magnitude;
		}

		if (0.3f < fVar28) {
			local_150.x = -this->rotationQuat.z;
			local_150.y = 0.0f;
			local_150.z = this->rotationQuat.x;
			local_150.w = 0.0f;
			pCVar19 = this->pPlayerInput;
			if ((pCVar19 == (CPlayerInput*)0x0) || (peVar25 = &pCVar19->lAnalogStick, this->field_0x18dc != 0)) {
				peVar25 = &gF32Vector4Zero;
			}

			fVar28 = edF32Vector4DotProductHard(peVar25, &local_150);
			if (0.70710677f < fVar28) {
				fVar27 = 0.5235988f;
			}
			else {
				if (fVar28 < -0.70710677f) {
					fVar27 = -0.5235988f;
				}
			}
		}

		local_10 = this->rotationEuler.z;
		SV_UpdateValue(fVar27, 1.0472f, &local_10);
		this->rotationEuler.z = local_10;
		this->windRotationStrength = local_10 / 0.5235988;
		if (bVar6) {
			uVar15 = GetWindState();
			if (uVar15 == (CActorWindState*)0x0) {
				fVar27 = 0.0f;
			}
			else {
				iVar16 = GetWindState();
				fVar27 = iVar16->field_0x1c;
			}

			pCVar17 = GetWindState();
			if (pCVar17 == (CActorWindState*)0x0) {
				fVar28 = 0.0f;
			}
			else {
				pCVar17 = GetWindState();
				fVar28 = pCVar17->field_0x20;
			}

			edF32Vector4ScaleHard(this->field_0x11ec * (CDynamicExt::gForceGravity.y * fVar28 + local_20.y * fVar27), &eStack320, &g_xVector);
			peVar25 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(peVar25, peVar25, &eStack320);
			fVar27 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar27;
		}

		pCVar19 = this->pPlayerInput;
		if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			local_c = 0.0f;
			local_8 = 0.0f;
			local_4 = 0.0f;
		}
		else {
			pCVar19->GetPadRelativeToNormal2D(&this->dynamic.horizontalVelocityDirectionEuler, &local_4, &local_8, &local_c);
		}

		if (bVar6) {
			uVar15 = GetWindState();

			int iVar162;

			if (uVar15 == (CActorWindState*)0x0) {
				iVar162 = 0;
			}
			else {
				iVar16 = GetWindState();
				iVar162 = iVar16->field_0x28;
			}

			if (iVar162 < 1) {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar27 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar27 = iVar16->field_0x10;
				}
			}
			else {
				fVar27 = 1.0f;
				uVar22 = 0x40000;
			}
		}
		else {
			if (this->field_0x1424 == 0) {
				fVar27 = 4.0f;
			}
			else {
				fVar27 = edFIntervalUnitSrcLERP(this->field_0x11ec, 4.0f, 2.0f);
			}
		}

		if (bVar6) {
			if (0.0f <= local_8) {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar28 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar28 = iVar16->field_0xc;
				}

				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar29 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar29 = iVar16->field_0x14;
				}

				fVar29 = edFIntervalUnitSrcLERP(local_8, fVar28, fVar29);
			}
			else {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar28 = 0.0;
				}
				else {
					iVar16 = GetWindState();
					fVar28 = iVar16->field_0xc;
				}

				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar29 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar29 = iVar16->field_0x18;
				}

				fVar29 = edFIntervalUnitSrcLERP(-local_8, fVar28, fVar29);
			}

			uVar15 = GetWindState();
			if (uVar15 == (CActorWindState*)0x0) {
				fVar28 = 0.0f;
			}
			else {
				iVar16 = GetWindState();
				fVar28 = iVar16->field_0x1c;
			}

			if (fVar28 < -1.0f) {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar28 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar28 = iVar16->field_0x24;
				}
				fVar28 = fVar28 * 0.42f;
			}
			else {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar30 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar30 = iVar16->field_0x24;
				}

				fVar28 = edFIntervalUnitSrcLERP(this->field_0x11ec, 0.42f, 0.24f);
				fVar28 = fVar28 * fVar30;
			}

			SV_UpdateValue(fVar29, 8.0f, &this->field_0x11e4);
		}
		else {
			fVar28 = edFIntervalUnitSrcLERP(this->field_0x11ec, 0.42f, 0.24f);
			SV_UpdateValue(this->field_0x11e0, 3.0f, &this->field_0x11e4);
		}

		if (0.0 < this->field_0x11e4) {
			uVar15 = GetWindState();
			int iVar162;
			if (uVar15 == (CActorWindState*)0x0) {
				iVar162 = 0;
			}
			else {
				iVar16 = GetWindState();
				iVar162 = iVar16->field_0x4;
			}

			if (0 < iVar162) {
				uVar15 = GetWindState();
				CWayPoint* pWaypoint;
				if (uVar15 == (CActorWindState*)0x0) {
					pWaypoint = (CWayPoint*)0x0;
				}
				else {
					iVar16 = GetWindState();
					pWaypoint = iVar16->pWayPoint;
				}

				if (pWaypoint == (CWayPoint*)0x0) {
					IMPLEMENTATION_GUARD(
						CActorAutonomous::ComputeFrictionForce2DWithSpeedMax
						(*(float*)&this->field_0x11e4, (CActorAutonomous*)this, &eStack320, 1);
					peVar25 = this->dynamicExt.aImpulseVelocities + 1;
					edF32Vector4AddHard(peVar25, peVar25, &eStack320);
					fVar29 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities + 1);
					this->dynamicExt.aImpulseVelocityMagnitudes[1] = fVar29;)
				}
			}
		}

		MoveInFreeFall(0.0f, this->field_0x11e4, fVar28, this->field_0x11e8, this->airRotationRate, iVar20);

		pCVar17 = GetWindState();
		if (pCVar17 == (CActorWindState*)0x0) {
			iVar20 = 0;
		}
		else {
			pCVar17 = GetWindState();
			iVar20 = pCVar17->field_0x4;
		}

		if (0 < iVar20) {
			local_160.y = 0.0f;
			local_160.x = local_20.x;
			local_160.z = local_20.z;
			local_160.w = local_20.w;

			fVar28 = edF32Vector4NormalizeHard(&local_160, &local_160);
			if (2.0f < fVar28) {
				edF32Vector4ScaleHard(this->dynamic.speed, &local_170, &this->dynamic.rotationQuat);
				edProjectVectorOnPlane(0.0f, &local_170, &local_170, &local_160, 0);
				local_170.y = 0.0f;
				fVar28 = edF32Vector4SafeNormalize1Hard(&local_170, &local_170);
				this->dynamic.speed = fVar28;
				this->dynamic.rotationQuat = local_170;
			}
		}

		uVar10 = uVar22 | 0xf00010d;
		if (bVar6) {
			uVar10 = uVar22 | 0xf10010d;
		}

		if (this->field_0x1424 == 0) {
			ManageDyn(fVar27, uVar10, (CActorsTable*)0x0);
		}
		else {
			ManageDyn(fVar27, uVar10, &local_130);
		}
	}

	if ((GetStateFlags(this->actorState) & 1) == 0) {
		if (bVar5) {
			if ((GetStateFlags(this->actorState) & 1) == 0) {
				SetState(0xa1, 0xffffffff);
				return;
			}
		}

		bVar5 = (pCVar2->flags_0x4 & 2) != 0;
		if ((this->field_0x1424 != 0) && (bVar8 = false, bVar5)) {
			for (iVar20 = 0; iVar20 < local_130.nbEntries; iVar20 = iVar20 + 1) {
				IMPLEMENTATION_GUARD(
					lVar24 = BreakActor(0, 5.0, 10.0, local_130.aEntries[iVar20], 1, 0, 0);
				if (lVar24 == 1) {
					bVar5 = false;
					fVar27 = this->dynamicExt.normalizedTranslation.y *
						this->dynamicExt.field_0x6c;
					this->dynamicExt.field_0x6c =
						SQRT(this->dynamicExt.field_0x6c *
							this->dynamicExt.field_0x6c - fVar27 * fVar27);
					this->dynamicExt.normalizedTranslation.y = 0.0;
				}
				else {
					if (lVar24 == 0) {
						bVar5 = false;
						bVar8 = true;
					}
				})
			}

			if (bVar8) {
				pTVar9 = Timer::GetTimer();
				if (this->field_0x155c <= pTVar9->scaledTotalTime) {
					fVar27 = edFIntervalUnitSrcLERP(this->field_0x11ec, 5.0f, 10.0f);
					LifeDecrease(fVar27);
					CLifeInterface* pLifeInterface = GetLifeInterface();
					fVar27 = pLifeInterface->GetValue();
					if (fVar27 == 0.0f) {
						SetState(0x99, 0xffffffff);
						return;
					}

					SetInvincible(2.0f, 1);
				}

				edF32Vector4ScaleHard(this->field_0xa80, &eStack416, &this->dynamic.velocityDirectionEuler);
				edF32Vector4ScaleHard(0.25f, &eStack416, &eStack416);
				fVar27 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &eStack416);
				this->dynamicExt.field_0x6c = fVar27;
				this->windBoostStrength = -1.0f;
				pCVar19 = GetInputManager(1, 0);
				if (pCVar19 != (CPlayerInput*)0x0) {
					CPlayerInput::FUN_001b66f0(0.6f, 0.0f, 0.2f, 0.0f, &pCVar19->field_0x40, 0);
				}
			}
		}

		if ((!bVar5) || (bVar6)) {
			uVar15 = GetWindState();
			if (uVar15 == (CActorWindState*)0x0) {
				bVar5 = false;
			}
			else {
				piVar21 = GetWindState();
				iVar20 = piVar21->field_0x0;
				iVar16 = GetWindState();
				if (iVar20 == iVar16->field_0x4) {
					bVar5 = true;
				}
				else {
					iVar16 = GetWindState();
					if (iVar16->field_0x4 == 0) {
						bVar5 = false;
					}
					else {
						bVar5 = true;
						if (0.17398384f <= fabs(local_20.y)) {
							bVar5 = false;
						}
					}
				}
			}

			if ((!bVar5) || (bVar5 = true, fVar31 <= 2.0f)) {
				bVar5 = false;
			}

			if (bVar5) {
				this->field_0x1020 = 1;
				SetState(0xf5, 0xffffffff);
			}
			else {
				uVar22 = GetStateHeroFlags(this->prevActorState);
				uVar22 = TestState_IsGripped(uVar22);
				if ((bVar6) || (uVar22 = CanGrip(uVar22, &this->rotationQuat), uVar22 == 0)) {
					if (nextState != -1) {
						if (pCVar3->IsCurrentLayerAnimEndReached(0)) {
							SetState(nextState, 0xffffffff);
							return;
						}
					}

					if (param_2 != -1) {
						if ((param_2 == 0) || (bVar6)) {
							if ((param_2 == 0) && (bVar6)) {
								SetState(STATE_HERO_GLIDE_2, 0xffffffff);
							}
						}
						else {
							SetState(0xf0, -1);
						}
					}
				}
				else {
					this->field_0x1020 = 1;
					SetGripState();
				}
			}
		}
		else {
			Landing();
			iVar20 = ChooseStateLanding(fabs(fVar26));
			if (iVar20 != -1) {
				SetState(iVar20, -1);
			}
		}
	}
	else {
		if (2.0f < this->timeInAir) {
			IMPLEMENTATION_GUARD(
				(*(code*)(this->pVTable)->field_0x16c)(this);)
		}
	}
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroWindCanonInit()
{
	CActorWindState* pCVar1;
	int iVar2;
	CWayPoint* pCVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	this->scalarDynA.Reset();
	this->scalarDynJump.Reset();

	if (GetWindState() == (CActorWindState*)0x0) {
		pCVar3 = (CWayPoint*)0x0;
	}
	else {
		pCVar3 = GetWindState()->pWayPoint;
	}

	local_10.xyz = pCVar3->location;
	local_10.w = 1.0f;
	edF32Vector4SubHard(&local_10, &local_10, &this->currentLocation);
	edF32Vector4NormalizeHard(&eStack32, &local_10);

	if (GetWindState() == (CActorWindState*)0x0) {
		fVar7 = 0.0f;
	}
	else {
		fVar7 = GetWindState()->field_0x30;
	}

	edF32Vector4ScaleHard(fVar7, &eStack32, &eStack32);

	fVar7 = local_10.y;
	fVar8 = fabs(eStack32.y);
	local_10.y = 0.0f;

	fVar4 = edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
	this->rotationQuat = local_10;
	this->field_0x1400 = local_10;

	eStack32.y = 0.0f;
	fVar5 = edF32Vector4GetDistHard(&eStack32);
	fVar9 = sqrtf(fVar5 * fVar5 + fVar8 * fVar8);

	if (GetWindState() == (CActorWindState*)0x0) {
		fVar6 = 0.0f;
	}
	else {
		fVar6 = GetWindState()->field_0x30;
	}

	if (0.1 < fabs(fVar9 - fVar6)) {
		edDebugPrintf("Big error : check the values...\n");
	}
	if (GetWindState() == (CActorWindState*)0x0) {
		fVar9 = 0.0f;
	}
	else {
		fVar9 = GetWindState()->field_0x34;
	}

	this->scalarDynA.BuildFromSpeedDist(fVar5, fVar9, fVar4);

	fVar4 = 1.0f;
	if (0.0f < fVar7) {
		fVar4 = fVar7;
	}

	this->scalarDynJump.BuildFromSpeedDistTime(fVar8, 0.0f, fVar4, this->scalarDynA.duration);
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;
	this->field_0x1428 = 0;
	this->field_0x11fc = 1.0f;
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroWindCanonTerm()
{
	RestoreVerticalOrientation();
	ConvertSpeedPlayerToSpeedSumForceExt2D();
	this->field_0x1428 = 1;
	this->field_0x11fc = 0.0f;

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroFallBounce_2_2Init()
{
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(this->field_0x118c, 0);

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroFallBounce_2_2Term()
{
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	return;
}