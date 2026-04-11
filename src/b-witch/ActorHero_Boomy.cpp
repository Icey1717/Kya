#include "ActorHero_Private.h"
#include "MathOps.h"
#include "CameraGame.h"
#include "TimeController.h"
#include "InputManager.h"
#include "ActorBoomy.h"



void CActorHeroPrivate::StateHeroBoomySnipePrepare_Init()
{
	CCamera* pCamera;
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	if ((this->field_0x1b64 == 0) && (pCamera = this->field_0xc94, (pCameraManager->cameraStack).pActiveCamera != pCamera)) {
		pCameraManager->PushCamera(pCamera, 1);
		this->field_0x1b64 = 1;
	}

	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(3.0f, 0);
	SetBoomyFunc(6);
	this->field_0x1b78 = 3;

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipePrepare(int nextState, int param_3)
{
	CPlayerInput* pInput;
	bool bVar3;
	CCameraManager* pCameraManager;
	int iVar5;
	float fVar6;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	this->dynamic.speed = 0.0f;
	if (param_3 == 0) {
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	}

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		this->pAnimationController->AddDisabledBone(this->animKey_0x1584);
		SetBoomyFunc(7);
		SetState(STATE_HERO_BOOMY_SNIPE_STAND, 0xffffffff);
	}
	else {
		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar6 = 0.0f;
		}
		else {
			fVar6 = pInput->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue;
		}

		if ((fVar6 == 0.0f) && ((pCameraManager->flags & 0x4000000) != 0)) {
			SetState(nextState, 0xffffffff);
		}
		else {
			if ((((this->pCollisionData)->flags_0x4 & 2) == 0) &&
				(param_3 != 0)) {
				if (this->field_0x1184 < this->timeInAir) {
					iVar5 = ChooseStateFall(0);
					SetState(iVar5, 0xffffffff);
				}
			}
			else {
				this->timeInAir = 0.0f;
			}
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipePrepare_Term(bool param_2)
{
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	pCameraManager->AlertCamera(0, 0);

	if (param_2 == false) {
		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		this->field_0x1b64 = 0;
		this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
		SetBoomyFunc(0);
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeStand_Init()
{
	CCamera* pCamera;
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	if ((this->field_0x1b64 == 0) && (pCamera = this->field_0xc94, (pCameraManager->cameraStack).pActiveCamera != pCamera)) {
		pCameraManager->PushCamera(pCamera, 1);
		this->field_0x1b64 = 1;
	}

	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(3.0f, 0);

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeStand(int nextState, int param_3)
{
	CPlayerInput* pInput;
	bool bVar3;
	CCameraManager* pCameraManager;
	int iVar4;
	float fVar5;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	this->dynamic.speed = 0.0f;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
	}

	if (param_3 == 0) {
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	}

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar5 = 0.0f;
	}
	else {
		fVar5 = pInput->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue;
	}

	if (fVar5 == 0.0f) {
		if ((pCameraManager->flags & 0x4000000) == 0) {
			SetState(STATE_HERO_BOOMY_SNIPE_LAUNCH, 0xffffffff);
		}
		else {
			SetState(nextState, 0xffffffff);
		}
	}
	else {
		if ((((this->pCollisionData)->flags_0x4 & 2) == 0) &&
			(param_3 != 0)) {
			if (this->field_0x1184 < this->timeInAir) {
				iVar4 = ChooseStateFall(0);
				SetState(iVar4, 0xffffffff);
			}
		}
		else {
			this->timeInAir = 0.0f;
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeStand_Term(bool param_2)
{
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	if (param_2 == false) {
		pCameraManager->AlertCamera(0, 0);
		this->field_0x1b64 = 0;
		this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
		SetBoomyFunc(0);
	}
	else {
		pCameraManager->AlertCamera(0, (void*)1);
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeLaunch_Init()
{
	CCamera* pCamera;
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	if ((this->field_0x1b64 == 0) && (pCamera = this->field_0xc94, (pCameraManager->cameraStack).pActiveCamera != pCamera)) {
		pCameraManager->PushCamera(pCamera, 1);
		this->field_0x1b64 = 1;
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeLaunch(int param_2)
{
	bool bVar3;
	int iVar5;
	CActorBoomy* pBoomy;

	this->dynamic.speed = 0.0f;

	if (param_2 == 0) {
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	}

	if (0.08f <= GetTimer()->scaledTotalTime - this->time_0x1538) {
		if (this->field_0x1b68 == 0) {
			pBoomy = this->pActorBoomy;
			pBoomy->aBoomyTypeInfo[0].flags = pBoomy->aBoomyTypeInfo[0].flags | 1;
			this->pActorBoomy->field_0x1dc = 0.01f;
			DoMessage(this->pActorBoomy, (ACTOR_MESSAGE)4, (MSG_PARAM)1);
			this->field_0x1b78 = 3;
			SetBoomyFunc(8);
			this->field_0x1b68 = 1;
		}

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(STATE_HERO_BOOMY_SNIPE_AFTER_LAUNCH, 0xffffffff);
			return;
		}
	}

	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) && (param_2 != 0)) {
		if (this->field_0x1184 < this->timeInAir) {
			iVar5 = ChooseStateFall(0);
			SetState(iVar5, 0xffffffff);
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeLaunch_Term(int prevState)
{
	this->field_0x1b68 = 0;

	if ((prevState != STATE_HERO_BOOMY_SNIPE_AFTER_LAUNCH) && (prevState != STATE_HERO_BOOMY_SNIPE_BACK_2_STAND)) {
		this->field_0x1b64 = 0;
		SetBoomyFunc(0);
		this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeBack2Stand_Init()
{
	CCamera* pCamera;
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	if ((this->field_0x1b64 == 0) && (pCamera = this->field_0xc94, (pCameraManager->cameraStack).pActiveCamera != pCamera)) {
		pCameraManager->PushCamera(pCamera, 1);
		this->field_0x1b64 = 1;
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeBack2Stand(int param_2)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	int iVar4;

	this->dynamic.speed = 0.0f;
	SetVectorFromAngles(&this->rotationQuat, (edF32VECTOR3*)&this->rotationEuler);
	if (param_2 == 0) {
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	}

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(0xd3, 0xffffffff);
	}
	else {
		if ((((this->pCollisionData)->flags_0x4 & 2) == 0) && (param_2 != 0)) {
			if (this->field_0x1184 < this->timeInAir) {
				iVar4 = ChooseStateFall(0);
				SetState(iVar4, 0xffffffff);
			}
		}
		else {
			this->timeInAir = 0.0f;
		}
	}
	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeBack2Stand_Term(bool param_2)
{
	CCameraManager* pCameraManager;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	pCameraManager->AlertCamera(0, 0);

	this->pAnimationController->AddDisabledBone(this->animKey_0x1584);
	if (param_2 == false) {
		this->field_0x1b64 = 0;
	}

	SetBoomyFunc(7);

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeAfterLaunch_Init()
{
	this->field_0x1b68 = 0;

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeAfterLaunch(int nextState, int param_3)
{
	CCameraManager* pCameraManager;
	int iVar1;
	float fVar2;
	CPlayerInput* pInput;

	pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar2 = 0.0f;
	}
	else {
		fVar2 = pInput->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue;
	}

	if (fVar2 != 0.0f) {
		pCameraManager->AlertCamera(0, 0);
		this->dynamic.speed = 0.0f;
		SetVectorFromAngles(&this->rotationQuat, (edF32VECTOR3*)&this->rotationEuler);
	}

	if (param_3 == 0) {
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	}

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar2 = 0.0f;
	}
	else {
		fVar2 = pInput->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue;
	}

	if (fVar2 == 0.0f) {
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar2 = 0.0f;
		}
		else {
			fVar2 = pInput->aAnalogSticks[0].magnitude;
		}

		if ((0.0f < fVar2) || (this->field_0x1b64 == 0)) {
			this->field_0x1b64 = 0;
			SetState(nextState, 0xffffffff);
			return;
		}
	}
	else {
		iVar1 = this->boomyState_0x1b70;
		if ((iVar1 == 2) || (iVar1 == 3)) {
			this->field_0x1b64 = 1;
			SetState(STATE_HERO_BOOMY_SNIPE_BACK_2_STAND, 0xffffffff);
			return;
		}
	}
	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) && (param_3 != 0)) {
		if (this->field_0x1184 < this->timeInAir) {
			this->field_0x1b64 = 0;
			iVar1 = ChooseStateFall(0);
			SetState(iVar1, 0xffffffff);
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	return;
}

void CActorHeroPrivate::StateHeroBoomySnipeAfterLaunch_Term(bool param_2)
{
	if (param_2 == false) {
		this->field_0x1b64 = 0;
	}

	return;
}