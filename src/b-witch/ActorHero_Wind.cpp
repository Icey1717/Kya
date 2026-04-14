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
	local_30.y = this->currentLocation.y + (pCVar1->pObbPrim->position).y;

	if ((pCVar1->flags_0x4 & COLLISION_GROUND_FLAG) != 0) {
		edF32Vector4SubHard(&eStack16, &pCVar1->aCollisionContact[1].field_0x10, &local_30);
		edF32Vector4AddHard(&local_20, &local_20, &eStack16);
	}

	if ((pCVar1->flags_0x4 & COLLISION_WALL_FLAG) != 0) {
		edF32Vector4SubHard(&eStack16, &pCVar1->aCollisionContact[0].field_0x10, &local_30);
		edF32Vector4AddHard(&local_20, &local_20, &eStack16);
	}

	if ((pCVar1->flags_0x4 & COLLISION_CEILING_FLAG) != 0) {
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
	this->glideEffort = 0.0f;
	this->field_0x11e0 = 12.0f;
	this->field_0x11e4 = 12.0f;
	this->field_0x11e8 = 20.0f;
	this->field_0x13d4 = 6.0f;
	this->windWallHorizontalSpeed = 1.25f;
	this->windWallVerticalSpeed = 1.25f;
	this->field_0x13e4 = 3.5f;
	this->maximumGlideTime = 1.0f;
	this->deathCheckGlideTime = 2.0f;
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
	edF32VECTOR4* peVar8;
	int landingState;
	CPlayerInput* pInput;
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
					fVar12 = edFIntervalLERP(GetTimer()->scaledTotalTime - this->currentGlideTime, this->maximumGlideTime, this->deathCheckGlideTime, 1.0f, 0.2f);
					this->field_0x118c = fVar12;
					landingState = 0x81;
				}
				else {
					pCVar1 = this->pPlayerInput;
					if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar12 = 0.0f;
					}
					else {
						fVar12 = pCVar1->aButtons[INPUT_BUTTON_INDEX_L2].clickValue;
					}

					if (fVar12 == 0.0f) {
						landingState = STATE_HERO_JUMP_3_3_STAND;

						if (2.0f < this->dynamic.horizontalLinearSpeed) {
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
			fVar12 = edFIntervalUnitDstLERP(GetTimer()->scaledTotalTime - this->time_0x1538, 0.3f, 1.5f);
			this->field_0x10f4 = fVar12;
			fVar12 = edFIntervalLERP(GetTimer()->scaledTotalTime - this->time_0x1538, 0.3f, 1.5f, 0.5f, 1.5f);
			this->field_0x1108 = fVar12;
			landingState = STATE_HERO_TOBOGGAN;
		}
	}
	else {
		fVar12 = edFIntervalLERP(GetTimer()->scaledTotalTime - this->currentGlideTime, this->maximumGlideTime, this->deathCheckGlideTime, this->field_0x1418, this->field_0x141c);

		if (this->bCheckDynCollisions == 0) {
			CLifeInterface* pLifeInterface = GetLifeInterface();
			fVar11 = pLifeInterface->GetValue();
			if (0.0f < fVar11 - fVar12) {
				pInput = GetInputManager(1, 0);
				if (pInput != 0) {
					CPlayerInput::FUN_001b66f0(1.0f, 0.0f, 0.1f, 0.0f, &pInput->field_0x40, 0);
				}

				LifeDecrease(fVar12);
				SetInvincible(2.0f, 1);
				goto LAB_0014c070;
			}
		}

		if (this->bCheckDynCollisions == 0) {
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

			landingState = -1;
			this->field_0x1558 = GetTimer()->scaledTotalTime + 1.5f;
		}
		else {
			landingState = STATE_HERO_COL_WALL_DEAD_B;
			this->field_0x1558 = GetTimer()->scaledTotalTime + 1.5f;
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

	if ((pCollision->flags_0x4 & COLLISION_GROUND_FLAG) != 0) {

		fVar15 = GetWindNeutralAirSpeed();
		if (0.0f < fVar15) {
			fVar15 = GetWindNeutralAirSpeed();
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

		if ((pCollision->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
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
				iVar7 = GetWindState()->nbActiveWind;
				bVar5 = true;
				if (iVar7 != GetWindState()->nbWindWaypoint) {
					bVar5 = false;
					if ((GetWindState()->nbWindWaypoint != 0) && (bVar5 = true, 0.17398384f <= fabs(local_30.y))) {
						bVar5 = false;
					}
				}
			}

			if (bVar5) {
				bVar5 = false;
				if (GetWindState() != (CActorWindState*)0x0) {
					iVar7 = GetWindState()->nbActiveWind;
					bVar5 = true;
					if (iVar7 != GetWindState()->nbWindWaypoint) {
						bVar5 = false;
						if ((GetWindState()->nbWindWaypoint != 0) && (bVar5 = true, 0.17398384f <= fabs(local_30.y))) {
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

			if (((pCollision->flags_0x4 & COLLISION_WALL_FLAG) != 0) && (DetectStickableWalls(&local_40, &local_14, &local_18, &local_90), local_14 != 0)) {
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
			
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, aeStack208, local_b0);
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

	this->glideEffort = 0.0f;
	this->windBoostStrength = 0.0f;
	this->windRotationStrength = 0.0f;
	ConvertSpeedSumForceExtToSpeedPlayer2D();
	pTVar3 = GetTimer();
	this->currentGlideTime = pTVar3->scaledTotalTime;
	this->field_0x1420 = 0;
	this->bCheckDynCollisions = 0;
	this->field_0xcb4.Init(0.0f, 800.0f);
	this->bFreeGlide = 1;

	SV_FX_Start(&this->field_0x13c0);
	this->field_0x13c0.Start();

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroFlyTerm()
{
	CFxHandle* pCVar1;
	int iVar2;
	bool bVar3;
	CCamera* pCVar5;
	CCameraManager* pCameraManager;

	if (this->field_0x13c0.IsValid()) {
		this->field_0x13c0.Kill();
		this->field_0x13c0.Reset();
	}

	this->field_0x13cc = 0.0f;

	this->currentGlideTime = GetTimer()->scaledTotalTime;
	this->field_0x1420 = 0;
	this->bCheckDynCollisions = 0;
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
	CCollision* pCol;
	CAnimation* pAnim;
	ed_zone_3d* pZone;
	bool bVar5;
	bool bVar6;
	CEventManager* pEventManager;
	bool bVar8;
	uint dynamicFlags;
	int iVar11;
	CCamera* pCamera;
	ECameraType cameraType;
	CPlayerInput* pInput;
	int iVar20;
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
	CActorsTable dynCollisionsTable;
	edF32VECTOR4 local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;
	CCameraManager* pCameraManager;

	iVar20 = 0;
	pCol = this->pCollisionData;
	pAnim = this->pAnimationController;
	bool bFallBelowGroundPlane = false;
	dynCollisionsTable.nbEntries = 0;
	uVar22 = 0;

	IncreaseEffort(edFIntervalUnitSrcLERP(this->glideEffort, 0.5f, 1.0f));

	local_20 = this->dynamicExt.aImpulseVelocities[2];

	fVar26 = this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y;
	fVar31 = this->dynamicExt.aImpulseVelocityMagnitudes[2];
	bVar6 = 0.001f < fVar31;
	if (this->bFreeGlide == 0) {		
		this->currentGlideTime = GetTimer()->scaledTotalTime;
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

		
		float smoothingFactor = powf(0.9f, GetTimer()->cutsceneDeltaTime * 50.0f);
		if (1.0f < smoothingFactor) {
			smoothingFactor = 1.0f;
		}
		else {
			if (smoothingFactor < 0.0f) {
				smoothingFactor = 0.0f;
			}
		}

		bVar1 = pCol->flags_0x4;
		if (((bVar1 & COLLISION_GROUND_FLAG) == 0) && ((bVar1 & COLLISION_CEILING_FLAG) == 0)) {
			dynamicFlags = TestState_IsInHit(0xffffffff);
			if ((dynamicFlags == 0) || (bVar6)) {
				pInput = this->pPlayerInput;
				if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar28 = 0.0f;
				}
				else {
					fVar28 = pInput->aButtons[INPUT_BUTTON_INDEX_CROSS].clickValue;
				}

				if ((int)fVar28 < 0) {
					fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
					fVar28 = fVar28 + fVar28;
				}
				else {
					fVar28 = (float)(int)fVar28;
				}

				this->glideEffort = this->glideEffort * smoothingFactor + (1.0f - smoothingFactor) * fVar28;
			}
			else {
				this->glideEffort = 0.0f;
				this->windBoostStrength = 0.0f;
			}
		}
		else {
			pInput = this->pPlayerInput;
			if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar28 = 0.0f;
			}
			else {
				fVar28 = pInput->aButtons[INPUT_BUTTON_INDEX_CROSS].clickValue;
			}

			if ((int)fVar28 < 0) {
				fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
				fVar28 = fVar28 + fVar28;
			}
			else {
				fVar28 = (float)(int)fVar28;
			}

			this->glideEffort = fVar28;
		}

		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar28 = 0.0f;
		}
		else {
			fVar28 = pInput->aButtons[INPUT_BUTTON_INDEX_CROSS].clickValue;
		}

		if ((int)fVar28 < 0) {
			fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
			fVar28 = fVar28 + fVar28;
		}
		else {
			fVar28 = (float)(int)fVar28;
		}

		this->windBoostStrength = this->windBoostStrength * smoothingFactor + (1.0f - smoothingFactor) * fVar28;

		ChangeCollisionSphereForGlide(0.1f, this->windBoostStrength);

		if ((1.0f < this->field_0xa88) || (bVar6)) {			
			this->currentGlideTime = GetTimer()->scaledTotalTime;
			this->field_0x1420 = 0;
			this->bCheckDynCollisions = 0;
		}
		else {			
			if (this->maximumGlideTime < GetTimer()->scaledTotalTime - this->currentGlideTime) {
				uVar22 = 0x40000;
				this->field_0x1420 = 1;
			}
			else {
				this->lastKnowGroundY = this->currentLocation.y;
			}

			
			if (this->deathCheckGlideTime < GetTimer()->scaledTotalTime - this->currentGlideTime) {
				this->bCheckDynCollisions = 1;
				this->animKey_0x157c = this->currentLocation.y + 20.0f;

				pEventManager = CScene::ptable.g_EventManager_006f5080;
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

						if ((pZone != (ed_zone_3d*)0x0) && (dynamicFlags = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &this->currentLocation, 0), (dynamicFlags & 1) != 0)) break;

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
						this->lastKnowGroundY = this->currentLocation.y;
					}
					else {
						if (100.0f < this->lastKnowGroundY - this->currentLocation.y) {
							bFallBelowGroundPlane = true;
						}
					}
				}
			}
			else {
				if (this->distanceToGround < 1.0f) {
					bVar8 = GetLifeInterface()->GetValue() - this->field_0x2e4 <= 0.0f;
					if (!bVar8) {
						bVar8 = (GetStateFlags(this->actorState) & 1) != 0;
					}

					if (!bVar8) {
						PlayAnim(0x103);
						goto LAB_0014a028;
					}
				}

				PlayAnim(GetStateCfg(this->actorState)->animId);
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
			SV_UpdateValue(edFCosinus[(int)(fabs((this->timeInAir * 2.5 - 1.570796) * 1303.797) + 0.5)
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
			fVar28 = edFCosinus[(int)(fabs((this->timeInAir * 4.0 - 1.570796) * 1303.797) + 0.5) &
			0x1fff];)

		fVar28 = cosf(this->timeInAir * 4.0f - 1.570796f);

		if (fVar27 < 5.0) {
			fVar27 = edFIntervalLERP(fVar27, 0.0f, 5.0f, 0.2f, 0.1f);
			fVar28 = fVar28 * fVar27;
		}
		else {
			fVar27 = edFIntervalLERP(fVar27, 5.0f, 27.0f, 0.1f, 0.02f);
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
	
	if ((GetTimer()->scaledTotalTime - this->currentGlideTime <= this->maximumGlideTime) || (this->distanceToGround != this->field_0xf0)) {
		pCameraManager->PushCamera(pCameraManager->GetDefGameCamera((ECameraType)9), 0);
		pCameraManager->PushCamera(pCameraManager->GetDefGameCamera((ECameraType)8), 0);
	}

	if (pCameraManager->pActiveCamera->GetMode() == 8) {
		this->field_0xcb4.UpdateLerp(this->rotationEuler.z);
		pCameraManager->pActiveCamera->SetAngleGamma(this->field_0xcb4.currentAlpha);
	}

	if (this->bFreeGlide == 0) {
		fVar27 = edFIntervalUnitDstLERP(this->scalarDynA.field_0x4, this->scalarDynA.duration, 0.0f);
		this->field_0x11fc = fVar27;
		
		this->scalarDynJump.Integrate(GetTimer()->cutsceneDeltaTime);		
		this->scalarDynA.Integrate(GetTimer()->cutsceneDeltaTime);

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
		pInput = this->pPlayerInput;
		fVar27 = 0.0f;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar28 = 0.0f;
		}
		else {
			fVar28 = pInput->aAnalogSticks[PAD_STICK_LEFT].magnitude;
		}

		if (0.3f < fVar28) {
			local_150.x = -this->rotationQuat.z;
			local_150.y = 0.0f;
			local_150.z = this->rotationQuat.x;
			local_150.w = 0.0f;

			pInput = this->pPlayerInput;
			if ((pInput == (CPlayerInput*)0x0) || (peVar25 = &pInput->lAnalogStick, this->field_0x18dc != 0)) {
				peVar25 = &gF32Vector4Zero;
			}

			const float threshold = 0.70710677f; // dot product threshold |cos(45°)| ≈ 0.707 — stick is within 45° of roll axis
			const float maximumRollAngle = 0.5235988f; // = π / 6 rad(30°) — maximum roll angle
			fVar28 = edF32Vector4DotProductHard(peVar25, &local_150);
			if (threshold < fVar28) {
				fVar27 = maximumRollAngle;
			}
			else {
				if (fVar28 < -threshold) {
					fVar27 = -maximumRollAngle;
				}
			}
		}

		local_10 = this->rotationEuler.z;
		SV_UpdateValue(fVar27, 1.0472f, &local_10);
		this->rotationEuler.z = local_10;
		this->windRotationStrength = local_10 / 0.5235988f;
		if (bVar6) {
			const float verticalLiftFactor = GetWindVerticalLiftFactor();
			const float gravityScale = GetWindGravityScale();

			edF32Vector4ScaleHard(this->glideEffort * (CDynamicExt::gForceGravity.y * gravityScale + local_20.y * verticalLiftFactor), &eStack320, &g_xVector);
			peVar25 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(peVar25, peVar25, &eStack320);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
		}

		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			local_c = 0.0f;
			local_8 = 0.0f;
			local_4 = 0.0f;
		}
		else {
			pInput->GetPadRelativeToNormal2D(&this->dynamic.horizontalVelocityDirectionEuler, &local_4, &local_8, &local_c);
		}

		if (bVar6) {
			int iVar162 = GetWindNbWindBoost();

			if (iVar162 < 1) {
				fVar27 = GetWindFriction();
			}
			else {
				fVar27 = 1.0f;
				uVar22 = 0x40000;
			}
		}
		else {
			if (this->bCheckDynCollisions == 0) {
				fVar27 = 4.0f;
			}
			else {
				fVar27 = edFIntervalUnitSrcLERP(this->glideEffort, 4.0f, 2.0f);
			}
		}

		if (bVar6) {
			if (0.0f <= local_8) {
				fVar28 = GetWindNeutralAirSpeed();
				fVar29 = GetWindMaxForwardSpeed();

				fVar29 = edFIntervalUnitSrcLERP(local_8, fVar28, fVar29);
			}
			else {
				fVar28 = GetWindNeutralAirSpeed();
				fVar29 = GetWindMaxBackwardSpeed();

				fVar29 = edFIntervalUnitSrcLERP(-local_8, fVar28, fVar29);
			}

			fVar28 = GetWindVerticalLiftFactor();

			if (fVar28 < -1.0f) {
				fVar28 = GetWindDragMultiplier();
				fVar28 = fVar28 * 0.42f;
			}
			else {
				fVar30 = GetWindDragMultiplier();
				fVar28 = edFIntervalUnitSrcLERP(this->glideEffort, 0.42f, 0.24f);
				fVar28 = fVar28 * fVar30;
			}

			SV_UpdateValue(fVar29, 8.0f, &this->field_0x11e4);
		}
		else {
			fVar28 = edFIntervalUnitSrcLERP(this->glideEffort, 0.42f, 0.24f);
			SV_UpdateValue(this->field_0x11e0, 3.0f, &this->field_0x11e4);
		}

		if (0.0f < this->field_0x11e4) {
			if (0 < GetWindNbWindWaypoint()) {
				if (GetWindWayPoint() == (CWayPoint*)0x0) {
					ComputeFrictionForce2DWithSpeedMax(this->field_0x11e4, &eStack320, true);
					peVar25 = this->dynamicExt.aImpulseVelocities + 1;
					edF32Vector4AddHard(peVar25, peVar25, &eStack320);
					fVar29 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities + 1);
					this->dynamicExt.aImpulseVelocityMagnitudes[1] = fVar29;
				}
			}
		}

		MoveInFreeFall(0.0f, this->field_0x11e4, fVar28, this->field_0x11e8, this->airRotationRate, iVar20);

		if (0 < GetWindNbWindWaypoint()) {
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

		dynamicFlags = uVar22 | 0xf00010d;
		if (bVar6) {
			dynamicFlags = uVar22 | 0xf10010d; // extra bit 0x100000: wind-stream impulse active
		}

		if (this->bCheckDynCollisions == 0) {
			ManageDyn(fVar27, dynamicFlags, (CActorsTable*)0x0);
		}
		else {
			ManageDyn(fVar27, dynamicFlags, &dynCollisionsTable);
		}
	}

	if ((GetStateFlags(this->actorState) & 1) == 0) {
		if (bFallBelowGroundPlane) {
			if ((GetStateFlags(this->actorState) & 1) == 0) {
				SetState(STATE_HERO_FALL_DEATH, 0xffffffff);

				return;
			}
		}

		bVar5 = (pCol->flags_0x4 & COLLISION_GROUND_FLAG) != 0;
		if ((this->bCheckDynCollisions != 0) && (bVar8 = false, bVar5)) {
			for (iVar20 = 0; iVar20 < dynCollisionsTable.nbEntries; iVar20 = iVar20 + 1) {
				const int breakResult = BreakActor(0, 5.0f, 10.0f, dynCollisionsTable.aEntries[iVar20], 1, 0, 0);
				if (breakResult == 1) {
					bVar5 = false;
					fVar27 = this->dynamicExt.normalizedTranslation.y * this->dynamicExt.field_0x6c;
					this->dynamicExt.field_0x6c = sqrtf(this->dynamicExt.field_0x6c * this->dynamicExt.field_0x6c - fVar27 * fVar27);
					this->dynamicExt.normalizedTranslation.y = 0.0f;
				}
				else {
					if (breakResult == 0) {
						bVar5 = false;
						bVar8 = true;
					}
				}
			}

			if (bVar8) {
				if (this->field_0x155c <= GetTimer()->scaledTotalTime) {
					fVar27 = edFIntervalUnitSrcLERP(this->glideEffort, 5.0f, 10.0f);
					LifeDecrease(fVar27);
					CLifeInterface* pLifeInterface = GetLifeInterface();
					fVar27 = pLifeInterface->GetValue();
					if (fVar27 == 0.0f) {
						SetState(STATE_HERO_COL_WALL_DEAD_C, 0xffffffff);

						return;
					}

					SetInvincible(2.0f, 1);
				}

				edF32Vector4ScaleHard(this->field_0xa80, &eStack416, &this->dynamic.velocityDirectionEuler);
				edF32Vector4ScaleHard(0.25f, &eStack416, &eStack416);
				fVar27 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &eStack416);
				this->dynamicExt.field_0x6c = fVar27;
				this->windBoostStrength = -1.0f;
				pInput = GetInputManager(1, 0);
				if (pInput != (CPlayerInput*)0x0) {
					CPlayerInput::FUN_001b66f0(0.6f, 0.0f, 0.2f, 0.0f, &pInput->field_0x40, 0);
				}
			}
		}

		// Determine if hero should enter a wind-canon stream:
		// requires active waypoints, impulse speed > 2.0, and wind direction nearly horizontal (|Y| < ~10°)
		if ((!bVar5) || (bVar6)) {
			if (GetWindState() == (CActorWindState*)0x0) {
				bVar5 = false;
			}
			else {
				iVar20 = GetWindState()->nbActiveWind;
				if (iVar20 == GetWindState()->nbWindWaypoint) {
					bVar5 = true;
				}
				else {
					if (GetWindState()->nbWindWaypoint == 0) {
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
				SetState(STATE_HERO_WIND_FLY, 0xffffffff);
			}
			else {
				uVar22 = GetStateHeroFlags(this->prevActorState);
				uVar22 = TestState_IsGripped(uVar22);
				if ((bVar6) || (uVar22 = CanGrip(uVar22, &this->rotationQuat), uVar22 == 0)) {
					if (nextState != -1) {
						if (pAnim->IsCurrentLayerAnimEndReached(0)) {
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
							SetState(STATE_HERO_GLIDE_1, -1);
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

			const int landingState = ChooseStateLanding(fabs(fVar26));
			if (landingState != -1) {
				SetState(landingState, -1);
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

	fVar7 = GetWindCannonLaunchSpeed();
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

	fVar6 = GetWindCannonLaunchSpeed();
	if (0.1 < fabs(fVar9 - fVar6)) {
		edDebugPrintf("Big error : check the values...\n");
	}

	fVar9 = GetWindCannonArcSpeed();
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
	this->bFreeGlide = 0;
	this->field_0x11fc = 1.0f;

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Wind.cpp
void CActorHeroPrivate::StateHeroWindCanonTerm()
{
	RestoreVerticalOrientation();
	ConvertSpeedPlayerToSpeedSumForceExt2D();
	this->bFreeGlide = 1;
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

void CActorHeroPrivate::StateHeroCheatFlyInit()
{
	CCameraManager* pCameraManager;
	edF32VECTOR4 newPosition;
	CPlayerInput* pInput;

	pInput = this->pPlayerInput;
	if ((pInput != (CPlayerInput*)0x0) && (this->field_0x18dc == 0)) {
		pInput->pressedBitfield = pInput->pressedBitfield & 0xfffff7ff;
	}

	this->field_0x1020 = 0;
	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	pCameraManager->AlertCamera(2, (void*)1);
	this->flags = this->flags | 0x80;
	this->flags = this->flags & 0xffffffdf;

	EvaluateDisplayState();

	this->effort = 0.0f;
	newPosition.x = this->currentLocation.x;
	newPosition.z = this->currentLocation.z;
	newPosition.w = this->currentLocation.w;
	newPosition.y = this->animKey_0x157c;

	UpdatePosition(&newPosition, true);

	return;
}

void CActorHeroPrivate::StateHeroCheatFly()
{
	CCollision* pCol;
	CPlayerInput* pInput;
	bool bVar3;
	int iVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 newPosition;

	pCol = this->pCollisionData;

	newPosition.x = this->currentLocation.x;
	newPosition.z = this->currentLocation.z;
	newPosition.w = this->currentLocation.w;

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar6 = 0.0f;
	}
	else {
		fVar6 = pInput->aButtons[INPUT_BUTTON_INDEX_TRIANGLE].clickValue;
	}

	if ((int)fVar6 < 0) {
		fVar6 = (float)((uint)fVar6 >> 1 | (uint)fVar6 & 1);
		fVar6 = fVar6 + fVar6;
	}
	else {
		fVar6 = (float)(int)fVar6;
	}

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar7 = 0.0f;
	}
	else {
		fVar7 = pInput->aButtons[INPUT_BUTTON_INDEX_CROSS].clickValue;
	}

	if ((int)fVar7 < 0) {
		fVar7 = (float)((uint)fVar7 >> 1 | (uint)fVar7 & 1);
		fVar7 = fVar7 + fVar7;
	}
	else {
		fVar7 = (float)(int)fVar7;
	}

	newPosition.y = (this->currentLocation.y + fVar6 * 0.5f) - fVar7 * 0.5f;
	UpdatePosition(&newPosition, true);

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar6 = 0.0f;
	}
	else {
		fVar6 = pInput->aAnalogSticks[PAD_STICK_LEFT].magnitude;
	}

	if (fVar6 < 0.3f) {
		this->dynamic.speed = 0.0f;
		this->timeInAir = 0.0f;
	}
	else {
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar6 = 0.0f;
		}
		else {
			fVar6 = pInput->aAnalogSticks[PAD_STICK_LEFT].magnitude;
		}

		fVar6 = edFIntervalUnitSrcLERP(fVar6, 1.0f, 5.0f);
		fVar7 = edFIntervalLERP(this->timeInAir, 0.0f, 2.0f, 1.0f, 2.0f);
		BuildHorizontalSpeedVector(fVar7 * 25.0f, this->field_0x1058 * fVar6, 2.0f, this->field_0x1040, this->field_0x1074);
	}
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0x40001, (CActorsTable*)0x0);

	if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
	LAB_0014b9f8:
		this->time_0x1538 = GetTimer()->scaledTotalTime;
	}
	else {
		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar6 = 0.0f;
		}
		else {
			fVar6 = pInput->aButtons[INPUT_BUTTON_INDEX_CROSS].clickValue;
		}

		if (fVar6 == 0.0f) goto LAB_0014b9f8;

		if (0.25f < GetTimer()->scaledTotalTime - this->time_0x1538) {
			newPosition.x = this->currentLocation.x;
			newPosition.z = this->currentLocation.z;
			newPosition.w = this->currentLocation.w;
			newPosition.y = this->currentLocation.y - 1.8f;
			UpdatePosition(&newPosition, true);
		}
	}

	if ((pCol->flags_0x4 & COLLISION_WALL_FLAG) != 0) {
		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar6 = 0.0f;
		}
		else {
			fVar6 = pInput->aAnalogSticks[PAD_STICK_LEFT].magnitude;
		}

		if (0.3f <= fVar6) {
			if (0.25f < GetTimer()->scaledTotalTime - this->time_0x153c) {
				newPosition = this->currentLocation;
				edF32Vector4ScaleHard(1.0f, &eStack32, &this->rotationQuat);
				edF32Vector4AddHard(&newPosition, &newPosition, &eStack32);
				UpdatePosition(&newPosition, true);
			}

			goto LAB_0014bb00;
		}
	}

	this->time_0x153c = GetTimer()->scaledTotalTime;

LAB_0014bb00:
	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		bVar3 = false;
	}
	else {
		bVar3 = pInput->aButtons[INPUT_BUTTON_INDEX_R2].clickValue != 0.0f;
		if (bVar3) {
			bVar3 = (pInput->pressedBitfield & PAD_BITMASK_R3) != 0;
		}
	}

	if (bVar3) {
		pInput = this->pPlayerInput;
		if ((pInput != (CPlayerInput*)0x0) && (this->field_0x18dc == 0)) {
			pInput->pressedBitfield = pInput->pressedBitfield & 0xfffff7ff;
		}

		this->dynamic.speed = 0.0f;
		SetState(ChooseStateFall(0), 0xffffffff);
	}

	return;
}

