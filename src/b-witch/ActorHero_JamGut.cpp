#include "ActorHero_Private.h"
#include "ActorJamGut.h"
#include "InputManager.h"
#include "MathOps.h"
#include "TimeController.h"
#include "CameraGame.h"

void CBehaviourHeroRideJamGut::Manage()
{
	this->pHero->BehaviourHeroRideJamGut_Manage(this);

	return;
}

void CBehaviourHeroRideJamGut::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pHero = static_cast<CActorHeroPrivate*>(pOwner);

	CBehaviourRideJamGut::Begin(pOwner, newState, newAnimationType);

	this->aCommands[1] = 1;

	return;
}

void CBehaviourHeroRideJamGut::End(int newBehaviourId)
{
	CCameraGame* pCameraGame;
	CActorHero* pHeroRef;

	pHeroRef = this->pHero;
	if (pHeroRef == CActorHero::_gThis) {
		pCameraGame = (CCameraGame*)pHeroRef->pMainCamera;
		float fVar1 = this->field_0xa8;
		pCameraGame->cameraConfig.field_0x58.x = fVar1;
		pCameraGame->cameraConfig.field_0xbc = fVar1;

		pCameraGame->fov = this->pHero->field_0x15cc;
		pCameraGame->SetDistance(this->pHero->field_0x15d0);
	}

	pHeroRef = this->pHero;
	pHeroRef->flags = pHeroRef->flags | 0x80000;

	CBehaviourRideJamGut::End(newBehaviourId);

	return;
}

void CBehaviourHeroRideJamGut::InitState(int newState)
{
	this->pHero->BehaviourHeroRideJamGut_InitState(newState, this);

	return;
}

void CBehaviourHeroRideJamGut::TermState(int oldState, int newState)
{
	this->pHero->BehaviourHeroRideJamGut_TermState(oldState, newState);

	return;
}

int CBehaviourHeroRideJamGut::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorAutonomous* pCVar1;
	CActorJamGut* pCVar2;
	uint uVar3;
	int result;
	edF32VECTOR4 eStack16;

	if (msg == 10) {
		IMPLEMENTATION_GUARD(
			(*(this->pVTable)->SetInvincible)((float)(int)pMsgParam, (CBehaviourRideJamGut*)this, 1);)
			result = 1;
	}
	else {
		if (msg == 0x49) {
			edF32VECTOR4* pPosition = reinterpret_cast<edF32VECTOR4*>(pMsgParam);

			uVar3 = this->pHero->TestState_00132b90(0xffffffff);
			if (uVar3 == 0) {
				pCVar2 = this->field_0x8c;

				pCVar2->InterpretMessage(pSender, (ACTOR_MESSAGE)0x49, pMsgParam);
				pCVar2 = this->field_0x8c;
				edF32Vector4ScaleHard(pCVar2->dynamic.linearAcceleration * GetTimer()->cutsceneDeltaTime, &eStack16, &pCVar2->dynamic.velocityDirectionEuler);
				edF32Vector4AddHard(pPosition, pPosition, &eStack16);
			}
			else {
				pCVar1 = this->field_0x88;
				pCVar1->InterpretMessage(pSender, (ACTOR_MESSAGE)0x49, pMsgParam);
			}

			result = 1;
			pPosition->y = pPosition->y + this->field_0xac;
		}
		else {
			GetPositionMsgParams* pGetPosMsgParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
				if ((msg == 7) && (pGetPosMsgParams->field_0x0 == 5)) {
					if (this->field_0x7c == 0) {
						pGetPosMsgParams->vectorFieldB.x = 0.0f;
						pGetPosMsgParams->vectorFieldB.y = 2.3f;
						pGetPosMsgParams->vectorFieldB.z = 0.0f;
						pGetPosMsgParams->vectorFieldB.w = 0.0f;
					}
					else {
						pGetPosMsgParams->vectorFieldB.x = 0.0f;
						pGetPosMsgParams->vectorFieldB.y = 2.3f;
						pGetPosMsgParams->vectorFieldB.z = 0.0f;
						pGetPosMsgParams->vectorFieldB.w = 0.0f;
					}

					result = 1;
				}
				else {
					result = CBehaviourRideJamGut::InterpretMessage(pSender, msg, pMsgParam);
				}
		}
	}

	return result;
}

void CBehaviourHeroRideJamGut::ManageInput()
{
	bool bVar1;
	edF32VECTOR4* pStickDir;
	uint uVar2;
	float fVar3;
	CActorHero* pHero;
	CPlayerInput* pPlayerInput;

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pPlayerInput->aAnalogSticks[0].magnitude;
	}

	if ((pPlayerInput == (CPlayerInput*)0x0) ||
		(pStickDir = &pPlayerInput->lAnalogStick, pHero->field_0x18dc != 0)) {
		pStickDir = &gF32Vector4Zero;
	}

	if (pStickDir == (edF32VECTOR4*)0x0) {
		this->inputAnalogDir.x = 0.0f;
		this->inputAnalogDir.y = 0.0f;
		this->inputAnalogDir.z = 0.0f;
		this->inputAnalogDir.w = 0.0f;
		this->inputMagnitude = 0.0f;
	}
	else {
		this->inputAnalogDir = *pStickDir;
		this->inputMagnitude = fVar3;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pPlayerInput->aAnalogSticks[0].magnitude;
	}

	if (0.3f < fVar3) {
		if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
			fVar3 = 0.0f;
		}
		else {
			fVar3 = pPlayerInput->aAnalogSticks[0].magnitude;
		}

		if (fVar3 < 1.0f) {
			this->field_0x38 = 3;
		}
		else {
			this->field_0x38 = 4;
		}
	}
	else {
		this->field_0x38 = 2;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		uVar2 = 0;
	}
	else {
		uVar2 = pPlayerInput->pressedBitfield & 0x10;
	}
	if (uVar2 != 0) {
		this->aCommands[5] = 1;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pPlayerInput->aButtons[4].clickValue;
	}
	if (fVar3 != 0.0f) {
		this->aCommands[6] = 1;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pPlayerInput->aButtons[9].clickValue;
	}
	if (fVar3 != 0.0f) {
		this->aCommands[8] = 1;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		bVar1 = false;
	}
	else {
		bVar1 = pPlayerInput->aButtons[6].clickValue != 0.0f;
		if (bVar1) {
			bVar1 = (pPlayerInput->pressedBitfield & 0x800) != 0;
		}
	}

	if ((bVar1) && (pHero->bCanUseCheats == 1)) {
		pHero->field_0x1554 = 0.0f;
		pHero->field_0x1550 = 0.0f;
		this->aCommands[10] = 1;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pPlayerInput->aButtons[7].clickValue;
	}
	if (fVar3 != 0.0f) {
		this->aCommands[0xb] = 1;
	}

	pHero = this->pHero;
	pPlayerInput = pHero->pPlayerInput;
	if ((pPlayerInput == (CPlayerInput*)0x0) || (pHero->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pPlayerInput->aButtons[4].clickValue;
	}
	if (fVar3 != 0.0f) {
		this->aCommands[0xc] = 1;
	}

	return;
}

void CBehaviourHeroRideJamGut::SetState(int newState, int param_3)
{
	CCameraManager* pCameraManager;
	CActorJamGut* pJamGut;

	pJamGut = this->field_0x8c;

	if ((pJamGut->GetStateFlags(pJamGut->actorState) & 1) == 0) {
		if ((this->field_0x7c == 0) || (param_3 != 0)) {
			this->field_0x88->SetState(newState, -1);
		}
	}
	else {
		if (this->field_0x7c != 0) {
			pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
			pCameraManager->PopCamera(this->pHero->pIntViewCamera_0x15bc);
			this->aCommands[1] = 1;
			this->field_0x7c = 0;
		}

		this->field_0x88->SetState(newState, -1);
	}

	return;
}

void CBehaviourHeroRideJamGut::SetSpeedAnim(float speed)
{
	if (this->field_0x7c == 0) {
		CBehaviourRideJamGut::SetSpeedAnim(speed);
	}

	return;
}

void CBehaviourHeroRideJamGut::SetInvincible(float duration, int bAdd)
{
	this->pHero->SetInvincible(duration, bAdd);
}

void CBehaviourHeroRideJamGut::InitMount(CActorJamGut* pJamGut, uint boneId, int param_4, uint flags)
{
	CBehaviourRideJamGut::InitMount(pJamGut, boneId, param_4, flags);

	CActorHero* pCVar2 = this->pHero;

	if (pCVar2 == CActorHero::_gThis) {
		CCameraGame* pCVar3 = (CCameraGame*)pCVar2->pMainCamera;
		this->field_0xa8 = (pCVar3->cameraConfig).field_0x58.x;
		(pCVar3->cameraConfig).field_0x58.x = 6.0f;
		(pCVar3->cameraConfig).field_0xbc = 6.0f;
	}

	this->field_0xac = 2.3f;
	this->field_0xb0 = 0;
	this->field_0xa4 = 0;
}


bool CBehaviourHeroRideJamGut::FUN_00369260()
{
	CActorJamGut* pCVar1;
	bool bVar2;
	int iVar3;

	iVar3 = this->field_0x8c->FUN_00376710();
	if (iVar3 == 0) {
		bVar2 = false;
	}
	else {
		pCVar1 = this->field_0x8c;
		iVar3 = pCVar1->actorState;
		bVar2 = true;
		if (((((iVar3 != 7) && (bVar2 = true, iVar3 != 8)) && (bVar2 = true, iVar3 != 9)) &&
			((bVar2 = true, iVar3 != 0xd && (bVar2 = true, iVar3 != 10)))) &&
			((bVar2 = true, iVar3 != 0xe &&
				((iVar3 != 0xf || (bVar2 = true, pCVar1->prevActorState != 0x10)))))) {
			bVar2 = false;
		}
	}

	return bVar2;
}



void CActorHeroPrivate::BehaviourHeroRideJamGut_InitState(int newState, CBehaviourHeroRideJamGut* pBehaviour)
{
	CCameraManager* pCameraManager;
	int iVar3;
	uint uVar4;
	float puVar5;
	float puVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	CAnimation* pAnim;

	if (this->actorState == 0x12e) {
		if ((GetStateFlags(this->prevActorState) & 1) == 0) {
			pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
			pCameraManager->PushCamera(this->pDeathCamera, 0);
		}
	}

	if (newState == STATE_HERO_BOOMY_SNIPE_AFTER_LAUNCH) {
		StateHeroBoomySnipeAfterLaunch_Init();
	}
	else {
		if (newState == STATE_HERO_BOOMY_SNIPE_BACK_2_STAND) {
			StateHeroBoomySnipeBack2Stand_Init();
		}
		else {
			if (newState == STATE_HERO_BOOMY_SNIPE_LAUNCH) {
				StateHeroBoomySnipeLaunch_Init();
			}
			else {
				if (newState == STATE_HERO_BOOMY_SNIPE_STAND) {
					StateHeroBoomySnipeStand_Init();
				}
				else {
					if (newState == STATE_HERO_BOOMY_SNIPE_PREPARE) {
						StateHeroBoomySnipePrepare_Init();
					}
					else {
						if (newState == STATE_HERO_LOOK_INTERNAL) {
							StateHeroInternalView_Init(0);
						}
						else {
							if (newState == 0x133) {
								puVar5 = SV_GetCosAngle2D(&pBehaviour->field_0x8c->rotationQuat);
								if (1.0f < puVar5) {
									puVar6 = 1.0f;
								}
								else {
									puVar6 = -1.0f;
									if (-1.0f <= puVar5) {
										puVar6 = puVar5;
									}
								}
								fVar7 = acosf(puVar6);
								pAnim = this->pAnimationController;
								iVar3 = this->currentAnimType;
								fVar9 = GetTimer()->cutsceneDeltaTime;
								iVar3 = GetIdMacroAnim(iVar3);
								if (iVar3 < 0) {
									fVar8 = fVar9 + 0.0f;
								}
								else {
									fVar8 = pAnim->GetAnimLength(iVar3, 2);
									fVar8 = fVar8 + fVar9;
								}

								pBehaviour->field_0xa4 = fVar7 / fVar8;
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CActorHeroPrivate::BehaviourHeroRideJamGut_TermState(int oldState, int newState)
{
	int iVar1;
	CCameraManager* pCameraManager;
	uint uVar2;

	if (oldState == STATE_HERO_BOOMY_SNIPE_AFTER_LAUNCH) {
		StateHeroBoomySnipeAfterLaunch_Term(newState == STATE_HERO_BOOMY_SNIPE_BACK_2_STAND);
	}
	else {
		if (oldState == STATE_HERO_BOOMY_SNIPE_BACK_2_STAND) {
			StateHeroBoomySnipeBack2Stand_Term(newState == STATE_HERO_BOOMY_SNIPE_STAND);
		}
		else {
			if (oldState == STATE_HERO_BOOMY_SNIPE_LAUNCH) {
				StateHeroBoomySnipeLaunch_Term(newState);
			}
			else {
				if (oldState == STATE_HERO_BOOMY_SNIPE_STAND) {
					StateHeroBoomySnipeStand_Term(newState == STATE_HERO_BOOMY_SNIPE_LAUNCH);
				}
				else {
					if (oldState == STATE_HERO_BOOMY_SNIPE_PREPARE) {
						StateHeroBoomySnipePrepare_Term(newState == STATE_HERO_BOOMY_SNIPE_STAND);
					}
					else {
						if (oldState == 0x11a) {
							StateHeroInternalView_Term(newState);
						}
					}
				}
			}
		}
	}

	if (this->actorState == 0x12e) {
		if ((GetStateFlags(newState) & 1) == 0) {
			pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
			pCameraManager->PopCamera(this->pDeathCamera);
		}
	}

	return;
}

void CActorHeroPrivate::FUN_00348df0()
{
	float fVar2;
	float fVar3;

	fVar2 = powf(0.9f, GetTimer()->cutsceneDeltaTime * 50.0f);
	fVar3 = 1.0f;
	if ((fVar2 <= 1.0f) && (fVar3 = fVar2, fVar2 < 0.0f)) {
		fVar3 = 0.0f;
	}
	fVar2 = 1.0f - fVar3;
	this->field_0xa80 =	fVar2 * this->dynamic.linearAcceleration + fVar3 * this->field_0xa80;
	this->field_0xa84 = fVar2 * this->dynamic.horizontalLinearAcceleration + fVar3 * this->field_0xa84;
	this->field_0xa88 = fVar3 * this->field_0xa88 + fVar2 * this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y;

	return;
}

void CActorHeroPrivate::FUN_003690a0(CBehaviourHeroRideJamGut* pBehaviour)
{
	CActorJamGut* pJamGut;
	edF32VECTOR4* peVar2;
	CCameraManager* pCameraManager;
	int* pCVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;

	this->rotationEuler.y = pBehaviour->field_0x8c->rotationEuler.y;
	RestoreVerticalOrientation();
	pCVar3 = pBehaviour->aCommands;
	iVar5 = 0;
	do {
		iVar4 = iVar5;
		pCVar3[0] = 0;
		pCVar3[1] = 0;
		iVar5 = iVar4 + 6;
		pCVar3[2] = 0;
		pCVar3[3] = 0;
		pCVar3[4] = 0;
		pCVar3[5] = 0;
		pCVar3 = pCVar3 + 6;
	} while (iVar5 < 7);
	pBehaviour->aCommands[iVar4 + 6] = 0;
	pBehaviour->field_0x38 = 0;
	pBehaviour->inputAnalogDir.x = 0.0f;
	pBehaviour->inputAnalogDir.y = 0.0f;
	pBehaviour->inputAnalogDir.z = 0.0f;
	pBehaviour->inputAnalogDir.w = 0.0f;
	pBehaviour->inputMagnitude = 0.0f;

	iVar5 = this->actorState;
	if ((iVar5 == 0x122) || (iVar5 == 0x120)) {
		pJamGut = pBehaviour->field_0x8c;
		fVar6 = 1.0f;

		fVar7 = pJamGut->dynamic.horizontalLinearAcceleration;
		if (1.0f <= fVar7) {
			fVar6 = fVar7;
		}
		if (iVar5 == 0x122) {
			fVar7 = pJamGut->GetRunSpeed();
			fVar6 = edFIntervalUnitDstLERP(fVar6, 0.0f, fVar7);
			pBehaviour->field_0x38 = 4;
		}
		else {
			fVar7 = pJamGut->GetWalkSpeed();
			fVar6 = edFIntervalUnitDstLERP(fVar6, 0.0f, fVar7);
			pBehaviour->field_0x38 = 3;
		}

		pJamGut = pBehaviour->field_0x8c;
		peVar2 = &pJamGut->dynamic.horizontalVelocityDirectionEuler;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			pBehaviour->inputAnalogDir.x = 0.0f;
			pBehaviour->inputAnalogDir.y = 0.0f;
			pBehaviour->inputAnalogDir.z = 0.0f;
			pBehaviour->inputAnalogDir.w = 0.0f;
			pBehaviour->inputMagnitude = 0.0f;
		}
		else {
			pBehaviour->inputAnalogDir.x = peVar2->x;
			pBehaviour->inputAnalogDir.y = pJamGut->dynamic.horizontalVelocityDirectionEuler.y;
			pBehaviour->inputAnalogDir.z = pJamGut->dynamic.horizontalVelocityDirectionEuler.z;
			pBehaviour->inputAnalogDir.w = pJamGut->dynamic.horizontalVelocityDirectionEuler.w;
			pBehaviour->inputMagnitude = fVar6;
		}
	}

	SetState(0x130, 0xffffffff);

	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	pCameraManager->PushCamera(this->pIntViewCamera_0x15bc, 0);
	pBehaviour->field_0x7c = 1;

	return;
}



void CActorHeroPrivate::BehaviourHeroRideJamGut_Manage(CBehaviourHeroRideJamGut* pBehaviour)
{
	CActorJamGut* pJamGut;
	CCameraGame* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	CPlayerInput* pCVar5;
	bool bVar6;
	uint uVar7;
	edF32MATRIX4* m1;
	int iVar8;
	CCameraManager* pCVar9;
	edF32VECTOR4* pNewOrientation;
	float fVar10;
	float puVar11;
	float puVar12;
	float target;
	edF32VECTOR4 eStack208;
	edF32VECTOR3 local_c0;
	edF32MATRIX4 auStack176;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 cachedLocation;
	float local_4;

	pJamGut = pBehaviour->field_0x8c;
	ClearAllSumForceExt();
	cachedLocation = this->currentLocation;

	if (this == CActorHero::_gThis) {
		if (pBehaviour->field_0x7c == 0) {
			if (pJamGut->field_0x404 * 1.4f < pJamGut->field_0x444) {
				target = 1.0f;
				fVar10 = 0.2f;
			}
			else {
				target = this->field_0x15cc;
				fVar10 = 0.4f;
			}
		}
		else {
			fVar10 = 1.0f;
			target = this->field_0x15cc;
		}

		local_4 = this->pMainCamera->fov;
		if (local_4 != target) {
			pCVar2 = (CCameraGame*)CActorHero::_gThis->pMainCamera;
			SV_UpdateValue(target, fVar10, &local_4);
			this->pMainCamera->fov = local_4;
			pCVar2->cameraConfig.field_0xbc = (this->field_0x15cc / this->pMainCamera->fov) * this->field_0x15d0;
		}
	}

	if (pBehaviour->field_0x7c == 0) {
		m1 = pJamGut->pAnimationController->GetCurBoneMatrix(pBehaviour->boneId);
		edF32Matrix4MulF32Matrix4Hard(&auStack176, m1, &pJamGut->pMeshTransform->base.transformA);
		edF32Vector4NormalizeHard(&auStack176.rowX, &auStack176.rowX);
		edF32Vector4NormalizeHard(&auStack176.rowY, &auStack176.rowY);
		edF32Vector4NormalizeHard(&auStack176.rowZ, &auStack176.rowZ);
		this->rotationQuat = auStack176.rowZ;
		edF32Matrix4ToEulerSoft(&auStack176, &local_c0, "XYZ");
		this->rotationEuler.xyz = local_c0;
		UpdatePosition(&auStack176, 1);
	}
	else {
		edF32Matrix4BuildFromVectorUnitSoft
		(&eStack96, &this->rotationQuat);
		uVar7 = TestState_00132b90(0xffffffff);
		if (uVar7 == 0) {
			pJamGut->SV_GetBoneWorldPosition(pBehaviour->boneId, &local_70);
			eStack96.rowT = local_70;
		}
		else {
			pJamGut->SV_GetBoneWorldPosition(pBehaviour->boneId, &local_70);
			eStack96.rowT.x = local_70.x;
			eStack96.rowT.z = local_70.z;
			eStack96.rowT.w = local_70.w;
			eStack96.rowT.y = this->currentLocation.y;
			eStack96.rowT.y = eStack96.rowT.y + (local_70.y - eStack96.rowT.y) * 0.15f;
		}

		UpdatePosition(&eStack96, 1);
	}

	pJamGut->SV_GetBoneDefaultWorldPosition(pBehaviour->boneId, &eStack208);

	fVar10 = pBehaviour->field_0xac * 0.3f + (this->currentLocation.y - eStack208.y) * 0.5f * 0.7f;
	pBehaviour->field_0xac = fVar10;
	if (fVar10 < -0.1f) {
		pBehaviour->field_0xac = -0.1f;
	}

	iVar8 = this->actorState;
	if ((iVar8 < 0x11e) || (0x12f < iVar8)) {
		iVar8 = this->actorState;
		if (iVar8 == STATE_HERO_BOOMY_SNIPE_AFTER_LAUNCH) {
			StateHeroBoomySnipeAfterLaunch(0x131, 0);
		}
		else {
			if (iVar8 == STATE_HERO_BOOMY_SNIPE_BACK_2_STAND) {
				StateHeroBoomySnipeBack2Stand(0);
			}
			else {
				if (iVar8 == STATE_HERO_BOOMY_SNIPE_LAUNCH) {
					StateHeroBoomySnipeLaunch(0);
				}
				else {
					if (iVar8 == STATE_HERO_BOOMY_SNIPE_STAND) {
						StateHeroBoomySnipeStand(0x131, 0);
					}
					else {
						if (iVar8 == STATE_HERO_BOOMY_SNIPE_PREPARE) {
							StateHeroBoomySnipePrepare(0x131, 0);
						}
						else {
							if (iVar8 == 0x11a) {
								StateHeroInternalView(0x131, 0, 0);
							}
							else {
								if ((iVar8 == 0x132) || (iVar8 == 0x131)) {
									IMPLEMENTATION_GUARD(
									CActor::FUN_00119cf0((CActor*)this, (CActor*)pBehaviour->field_0x8c);
									pBehaviour->aCommands[1] = 0;
									pCVar5 = this->pPlayerInput;
									if ((pCVar5 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
										fVar10 = 0.0;
									}
									else {
										fVar10 = pCVar5->aAnalogSticks[0].magnitude;
									}
									if (0.3 < fVar10) {
										pCVar5 = this->pPlayerInput;
										if ((pCVar5 == (CPlayerInput*)0x0) ||
											(pNewOrientation = &pCVar5->lAnalogStick, this->field_0x18dc != 0)) {
											pNewOrientation = &gF32Vector4Zero;
										}
										CActor::SV_UpdateOrientation2D
										(this->field_0x1040, (CActor*)this, pNewOrientation, 0);
									}
									puVar11 = edF32Vector4DotProductHard
									(&this->rotationQuat,
										&((pBehaviour->field_0x8c)->base).base.base.
										rotationQuat);
									if (1.0 < puVar11) {
										puVar12 = 1.0;
									}
									else {
										puVar12 = (float)&DAT_bf800000;
										if (-1.0 <= puVar11) {
											puVar12 = puVar11;
										}
									}
									fVar10 = acosf(puVar12);
									pJamGut = pBehaviour->field_0x8c;
									if (this->rotationQuat.x *
										pJamGut->base.rotationQuat.z -
										pJamGut->base.rotationQuat.x *
										this->rotationQuat.z < 0.0) {
										fVar10 = fVar10 * -1.0;
									}
									fVar10 = ABS(fVar10);
									if (1.570796 < fVar10) {
										fVar10 = 3.141593 - fVar10;
									}
									fVar10 = edFIntervalUnitDstLERP(fVar10, 0.0, 1.570796);
									pBehaviour->field_0xb0 = fVar10;)
								}
								else {
									if (iVar8 == 0x133) {
										IMPLEMENTATION_GUARD(
										CActor::SV_UpdateOrientation2D
										((float)pBehaviour->field_0xa4, (CActor*)this,
											&((pBehaviour->field_0x8c)->base).base.base.rotationQuat, 0);
										pCVar3 = this->pAnimationController;
										peVar4 = (pCVar3->anmBinMetaAnimator).base.aAnimData;
										if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
											bVar6 = false;
											if (peVar4->animPlayState != 0) {
												bVar6 = (peVar4->field_0xcc & 2) != 0;
											}
										}
										else {
											bVar6 = false;
										}
										if (bVar6) {
											pCVar9 = (CCameraManager*)CScene::GetManager(MO_Camera);
											CCameraManager::PopCamera(pCVar9, this->pIntViewCamera_0x15bc);
											pBehaviour->aCommands[1] = 1;
											pBehaviour->field_0x7c = 0;
										})
									}
									else {
										if (iVar8 == 0x130) {
											IMPLEMENTATION_GUARD(
											pCVar3 = this->pAnimationController;
											peVar4 = (pCVar3->anmBinMetaAnimator).base.aAnimData;
											if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
												bVar6 = false;
												if (peVar4->animPlayState != 0) {
													bVar6 = (peVar4->field_0xcc & 2) != 0;
												}
											}
											else {
												bVar6 = false;
											}
											if (bVar6) {
												iVar8 = this->prevActorState;
												if ((iVar8 == 0x122) || (iVar8 == 0x120)) {
													(*(this->pVTable)->SetState)
														((CActor*)this, 0x132, 0xffffffff);
												}
												else {
													(*(this->pVTable)->SetState)
														((CActor*)this, 0x131, 0xffffffff);
												}
												pBehaviour->aCommands[1] = 1;
											})
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		FUN_00119cf0(pBehaviour->field_0x8c);
		for (iVar8 = 0; iVar8 < 0xd; iVar8 = iVar8 + 1) {
			pBehaviour->aCommands[iVar8] = 0;
		}
		pBehaviour->field_0x38 = 0;
		pBehaviour->inputAnalogDir.x = 0.0f;
		pBehaviour->inputAnalogDir.y = 0.0f;
		pBehaviour->inputAnalogDir.z = 0.0f;
		pBehaviour->inputAnalogDir.w = 0.0f;
		pBehaviour->inputMagnitude = 0.0f;

		pBehaviour->ManageInput();

		if (this->actorState == 0x128) {
			bVar6 = FUN_0014cb60(&this->currentLocation);
			if (bVar6 == false) {
				if (this->distanceToGround < 10.3f) {
					this->field_0x11f0 = this->currentLocation.y;
				}
				else {
					if (100.0f < this->field_0x11f0 - this->currentLocation.y) {
						pBehaviour->aCommands[9] = 1;
					}
				}
			}
		}
		else {
			this->field_0x11f0 = this->currentLocation.y;
		}
	}

	ComputeRealMoving(&cachedLocation);
	FUN_00348df0();

	if (pBehaviour->field_0x78 == 0) {
		if (pBehaviour->field_0x7c == 0) {
			if (pBehaviour->FUN_00369260() != false) {
				pCVar5 = this->pPlayerInput;
				if ((pCVar5 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar10 = 0.0f;
				}
				else {
					fVar10 = pCVar5->aButtons[5].clickValue;
				}
				if (fVar10 != 0.0f) {
					FUN_003690a0(pBehaviour);
				}
			}
		}
		else {
			if (this->actorState != 0x130) {
				uVar7 = TestState_AllowInternalView(0xffffffff);
				if (uVar7 != 0) {
					pCVar5 = this->pPlayerInput;
					if ((pCVar5 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar10 = 0.0;
					}
					else {
						fVar10 = pCVar5->aButtons[10].clickValue;
					}
					if (fVar10 != 0.0f) {
						SetBehaviour(HERO_BEHAVIOUR_RIDE_JAMGUT, 0x11a, 0xffffffff);
						return;
					}
				}

				pCVar5 = this->pPlayerInput;
				if ((pCVar5 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar10 = 0.0f;
				}
				else {
					fVar10 = pCVar5->aButtons[5].clickValue;
				}

				if ((fVar10 == 0.0F) || (iVar8 = pBehaviour->field_0x8c->FUN_00376710(), iVar8 == 0)) {
					SetState(0x133, 0xffffffff);
				}
				else {
					pCVar5 = this->pPlayerInput;
					if ((pCVar5 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						uVar7 = 0;
					}
					else {
						uVar7 = pCVar5->pressedBitfield & 0x10;
					}

					if ((uVar7 != 0) && (iVar8 = pBehaviour->field_0x8c->FUN_00376710(), iVar8 != 0)) {
						pCVar9 = (CCameraManager*)CScene::GetManager(MO_Camera);
						pCVar9->PopCamera(this->pIntViewCamera_0x15bc);
						SetBehaviour(pBehaviour->behaviourId, pBehaviour->mountFlags, 0xffffffff);
					}
				}
			}
		}
	}

	return;
}