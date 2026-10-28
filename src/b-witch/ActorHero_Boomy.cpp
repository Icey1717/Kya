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
	SetBoomyState(HERO_BOOMY_STATE_SNIPE_PREPARE);
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
		SetBoomyState(HERO_BOOMY_STATE_SNIPE_AIM);
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
		SetBoomyState(HERO_BOOMY_STATE_IDLE);
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
		SetBoomyState(HERO_BOOMY_STATE_IDLE);
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
			SetBoomyState(HERO_BOOMY_STATE_SNIPE_THROW);
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
		SetBoomyState(HERO_BOOMY_STATE_IDLE);
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

	SetBoomyState(HERO_BOOMY_STATE_SNIPE_AIM);

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

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
bool CActorHeroPrivate::AbleTo_AttackByBoomyBlow()
{
	CActorBoomy* pCVar1;
	int iVar2;
	bool bVar3;
	uint uVar4;
	StateConfig* pSVar5;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar3 = true;

		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar3 = false;
		}

		if ((((bVar3) && (uVar4 = TestState_AllowAttack(0xffffffff), uVar4 != 0)) &&
			(bVar3 = EvolutionBoomyCanLaunch(), bVar3 != false)) && (((1 < this->actorState - 0x11aU && (uVar4 = TestState_00132b90(0xffffffff), uVar4 == 0)) &&
				(this->aBoomyBlows[0] != (s_fighter_blow*)0x0)))) {
			uVar4 = GetStateFlags(this->actorState) & 0x100;

			if (((uVar4 != 0) && (this->field_0x1a48 == 0)) && ((this->curBehaviourId != 8 &&
				((((iVar2 = this->boomyState_0x1b70, iVar2 == HERO_BOOMY_STATE_IDLE || (iVar2 == 5)) || (iVar2 == HERO_BOOMY_STATE_RETURN_DECISION)) || (iVar2 == HERO_BOOMY_STATE_CATCH_RECOVER))))))
			{
				return true;
			}
		}
	}

	return false;
}

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
bool CActorHeroPrivate::AbleTo_AttackByBoomyLaunch()
{
	CActorBoomy* pCVar1;
	int iVar2;
	bool bVar3;
	uint uVar4;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar3 = true;
		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar3 = false;
		}
		if (((bVar3) && (uVar4 = TestState_AllowAttack(0xffffffff), uVar4 != 0)) &&
			(bVar3 = true, this->field_0x1a48 == 0)) goto LAB_00136c48;
	}
	bVar3 = false;
LAB_00136c48:
	if ((bVar3) && (bVar3 = EvolutionBoomyCanLaunch(), bVar3 != false)) {
		iVar2 = this->actorState;
		if (((1 < iVar2 - 0x11aU) && ((iVar2 < 0xdc || 0xde < iVar2 && (uVar4 = TestState_00132b90(0xffffffff), uVar4 == 0)))) &&
			((iVar2 = this->boomyState_0x1b70, iVar2 == HERO_BOOMY_STATE_IDLE || ((iVar2 == HERO_BOOMY_STATE_RETURN_DECISION || (iVar2 == HERO_BOOMY_STATE_CATCH_RECOVER)))))) {
			return true;
		}
	}

	return false;
}

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
bool CActorHeroPrivate::AbleTo_AttackByBoomySnipe()
{
	CActorBoomy* pCVar1;
	int iVar2;
	bool bVar3;
	uint uVar4;
	StateConfig* pSVar5;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar3 = true;
		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar3 = false;
		}
		if (((bVar3) && (uVar4 = TestState_AllowAttack(0xffffffff), uVar4 != 0)) &&
			(bVar3 = true, this->field_0x1a48 == 0)) goto LAB_00136b08;
	}
	bVar3 = false;
LAB_00136b08:
	if (((bVar3) && (bVar3 = EvolutionBoomyCanSnipe(), bVar3 != false)) && (iVar2 = this->actorState, iVar2 - 0x11aU < 2)) {
		if ((((GetStateFlags(this->actorState) & 0x100) != 0) && (uVar4 = TestState_IsOnAToboggan(0xffffffff), uVar4 == 0)) &&
			((iVar2 = this->boomyState_0x1b70, iVar2 == 0 || ((iVar2 == 2 || (iVar2 == 3)))))) {
			return true;
		}
	}

	return false;
}

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
bool CActorHeroPrivate::AbleTo_AttackByBoomyControl()
{
	CActorBoomy* pCVar1;
	int iVar2;
	CActor* pCVar3;
	bool bVar4;
	uint uVar5;
	StateConfig* pSVar6;
	ulong uVar7;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar4 = true;
		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar4 = false;
		}
		if (((bVar4) && (uVar5 = TestState_AllowAttack(0xffffffff), uVar5 != 0)) &&
			(bVar4 = true, this->field_0x1a48 == 0)) goto LAB_00136998;
	}
	bVar4 = false;
LAB_00136998:
	if ((((bVar4) && (bVar4 = EvolutionBoomyCanControl(), bVar4 != false)) && ((uVar7 = TestState_AllowInternalView(0xffffffff), uVar7 != 0 &&
		((uVar5 = TestState_AllowMagic(0xffffffff), uVar5 != 0 && (uVar5 = TestState_00132b90(0xffffffff), uVar5 == 0)))))) &&
		(iVar2 = this->actorState, 1 < iVar2 - 0x11aU)) {
		if ((((GetStateFlags(this->actorState) & 0x100) != 0) && (this->field_0x1574 == 0)) && (this->boomyState_0x1b70 == HERO_BOOMY_STATE_IDLE)) {
			pCVar3 = this->pTiedActor;

			if ((pCVar3 != (CActor*)0x0) && ((pCVar3->actorFieldS & 0x200) == 0)) {
				return false;
			}

			return true;
		}
	}

	return false;
}


// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
void CActorHeroPrivate::ResetBoomyDefaultSettings()
{
	uint* puVar1;
	ulong uVar2;
	CScene::GetManager(MO_Camera);
	this->boomyOverlaySpeed = 8.0f;
	this->currentBoomyLayerOverlayWeight = 0.0f;
	this->targetBoomyLayerOverlayWeight = 0.0f;
	this->bBoomyLayerOverlayActive = 0;
	this->boomyTargetRayDist = -1.0f;
	this->field_0x1b64 = 0;
	this->field_0x1b68 = 0;
	this->field_0x1b6c = 0;
	SetBoomyState(HERO_BOOMY_STATE_IDLE);
	this->field_0x1b78 = 0;
	this->field_0x1b74 = 0.0f;
	this->field_0x1000 = -1.0f;
	this->field_0x1004 = 10.0f;
	this->field_0x1008 = 5.0f;

	DisableLayer(8);
	SetMagicMode(0);

	if (EvolutionBoomyCanLaunch() == 0) {
		this->pAnimationController->AddDisabledBone(this->animKey_0x1584);
	}
	else {
		this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
	}

	this->boomyBlowStage = 0;

	this->aBoomyBlows[0] = FindBlowByName("BOOMY_1");
	this->aBoomyBlows[1] = FindBlowByName("BOOMY_2");
	this->aBoomyBlows[2] = FindBlowByName("BOOMY_3");

	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
void CActorHeroPrivate::ManageBoomyStateInit()
{
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
void CActorHeroPrivate::ManageBoomyState()
{
	// PS2: 0x00134e60-0x001360d7; storm states: 0x00135828-0x0013604c.
	CPlayerInput* pCVar1;
	CActorBoomy* pBoomy;
	CActorBoomy* pCVar3;
	CAnimation* this_00;
	s_fighter_blow* psVar4;
	s_fighter_blow_sub_obj* psVar5;
	uint uVar6;
	CCamera* pCVar9;
	bool bVar10;
	CCameraManager* pCameraViewManager;
	CActor* pCVar12;
	CVision* pCVar13;
	int iVar14;
	uint uVar17;
	float fVar18;
	float fVar20;
	edF32VECTOR4 eStack512;
	edF32VECTOR4 local_1f0;
	edF32VECTOR4 local_1e0 = {};
	edF32VECTOR4 local_1d0;
	edF32VECTOR4 eStack448;
	edF32MATRIX4 eStack432;
	_msg_hit_param local_170 = {};
	edF32VECTOR4 local_f0;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 local_d0;
	s_fighter_collision_desc local_c0;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	_msg_params_get_position local_60;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;

	pCameraViewManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	this->boomyTargetRayDist = -1.0f;

	if (this->boomyState_0x1b70 != HERO_BOOMY_STATE_IDLE) {
		this->bBoomyLayerOverlayActive = 1;
	}

	if (this->bBoomyLayerOverlayActive != 0) {
		SetLayerProperty(this->currentBoomyLayerOverlayWeight, 8);

		if (this->currentBoomyLayerOverlayWeight < this->targetBoomyLayerOverlayWeight) {
			this->currentBoomyLayerOverlayWeight = this->currentBoomyLayerOverlayWeight + this->boomyOverlaySpeed * GetTimer()->cutsceneDeltaTime;

			fVar20 = this->targetBoomyLayerOverlayWeight;
			if (fVar20 < this->currentBoomyLayerOverlayWeight) {
				this->currentBoomyLayerOverlayWeight = fVar20;
			}
		}
		else {
			this->currentBoomyLayerOverlayWeight = this->currentBoomyLayerOverlayWeight - this->boomyOverlaySpeed * GetTimer()->cutsceneDeltaTime;
			fVar20 = this->targetBoomyLayerOverlayWeight;
			if (this->currentBoomyLayerOverlayWeight < fVar20) {
				this->currentBoomyLayerOverlayWeight = fVar20;
			}
		}

		if ((this->currentBoomyLayerOverlayWeight == 0.0f) && (this->bBoomyLayerOverlayActive = 0, this->field_0x1000 == -1.0f)) {
			DisableLayer(8);
		}
	}

	switch (this->boomyState_0x1b70) {
	case HERO_BOOMY_STATE_IDLE:
		this->field_0x1b74 = 0.0f;
		break;
	case HERO_BOOMY_STATE_THROW:
		this->field_0x1b78 = 2;
		fVar20 = this->field_0x1b74 + GetTimer()->cutsceneDeltaTime;
		this->field_0x1b74 = fVar20;

		if (0.02f <= fVar20) {
			pBoomy = this->pActorBoomy;
			iVar14 = pBoomy->actorState;
			bVar10 = true;
			if ((iVar14 != 5) && (iVar14 != 9)) {
				bVar10 = false;
			}

			if (bVar10) {
				pBoomy->UpdateFromOwner(4, &this->rotationQuat);

				pCVar12 = this->pActorBoomy->GetBestActorInVision();
				pBoomy = this->pActorBoomy;
				pBoomy->aBoomyTypeInfo[0].flags = pBoomy->aBoomyTypeInfo[0].flags & 0xfffffffe;

				if (pCVar12 == (CActor*)0x0) {
					pCVar3 = this->pActorBoomy;
					pCVar13 = pCVar3->GetVision();
					edF32Vector4ScaleHard(pCVar13->visionRange, &eStack48, &this->rotationQuat);
					SV_GetBoneWorldPosition(this->field_0x157c, &eStack32);
					edF32Vector4AddHard(&eStack32, &eStack32, &eStack48);
					this->pActorBoomy->UpdateFromOwner(3, &this->rotationQuat);
					this->pActorBoomy->SetTarget(&eStack32);
					(this->pActorBoomy)->field_0x1dc = 0.1f;
				}
				else {
					this->pActorBoomy->UpdateFromOwner(3, &this->rotationQuat);

					local_60.field_0x0 = 0;
					pBoomy = this->pActorBoomy;
					local_60.vectorFieldA = pBoomy->currentLocation;
					iVar14 = DoMessage(pCVar12, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_60);
					if (iVar14 == 0) {
						local_70 = pCVar12->currentLocation;
					}
					else {
						edF32Vector4AddHard(&local_70, &local_60.vectorFieldB, &pCVar12->currentLocation);
					}

					this->pActorBoomy->SetTarget(pCVar12, &local_70);
					(this->pActorBoomy)->field_0x1dc = 0.01f;
				}

				pBoomy = this->pActorBoomy;
				pBoomy->aBoomyTypeInfo[0].flags = pBoomy->aBoomyTypeInfo[0].flags | 1;
				DoMessage(this->pActorBoomy, (ACTOR_MESSAGE)4, 0);
				this->pAnimationController->AddDisabledBone(this->animKey_0x1584);
			}
		}

		bVar10 = IsLayerAnimFinished(8);
		if ((bVar10 != false) || (bVar10 = IsLayerAnimEndReached(8), bVar10 != false)) {
			if (this->actorState - 0x73U < 3) {
				SetState(0x73, -1);
			}

			SetBoomyState(HERO_BOOMY_STATE_IDLE);
		}
		break;
	case HERO_BOOMY_STATE_RETURN_DECISION:
		if (((this->field_0x1b6c == 0) && (bVar10 = IsLayerAnimFinished(8), bVar10 == false)) && (bVar10 = IsLayerAnimEndReached(8), bVar10 == false)) break;

		this->field_0x1b6c = 0;
		pBoomy = this->pActorBoomy;
		if (pBoomy == (CActorBoomy*)0x0) {
		LAB_001352d8:
			bVar10 = false;
		}
		else {
			iVar14 = pBoomy->actorState;
			bVar10 = true;
			if ((iVar14 != 5) && (iVar14 != 9)) {
				bVar10 = false;
			}

			if (((!bVar10) || (uVar17 = TestState_AllowAttack(0xffffffff), uVar17 == 0)) || (bVar10 = true, this->field_0x1a48 != 0)) goto LAB_001352d8;
		}

		if ((bVar10) && (bVar10 = EvolutionBoomyCanLaunch(), bVar10 != false)) {
			iVar14 = this->actorState;
			if (((iVar14 - 0x11aU < 2) || ((0xdb < iVar14 && iVar14 < 0xdf || (TestState_00132b90(0xffffffff) != 0)))) ||
				((iVar14 = this->boomyState_0x1b70, iVar14 != HERO_BOOMY_STATE_IDLE && ((iVar14 != HERO_BOOMY_STATE_RETURN_DECISION && (iVar14 != HERO_BOOMY_STATE_CATCH_RECOVER))))))
				goto LAB_00135370;
			bVar10 = true;
		}
		else {
		LAB_00135370:
			bVar10 = false;
		}

		if (bVar10) {
			pCVar1 = this->pPlayerInput;
			if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar20 = 1000000.0f;
			}
			else {
				fVar20 = pCVar1->aButtons[INPUT_BUTTON_INDEX_SQUARE].pressedDuration;
			}

			bVar10 = true;

			if (0.2f < fVar20) goto LAB_001353d8;
		}
		else {
		LAB_001353d8:
			bVar10 = false;
		}

		if ((bVar10) &&
			(((iVar14 = this->field_0x1b78, iVar14 == 2 || (iVar14 == 0)) && (TestState_AllowAttack(0xffffffff) != 0)))) {
			SetLayerAnim(this->currentBoomyLayerOverlayWeight, 8, 0x8a);

			if (this->actorState - 0x73U < 3) {
				SetState(STATE_HERO_BOOMY, -1);
			}

			SetBoomyState(HERO_BOOMY_STATE_RETHROW);
		}
		else {
			uVar17 = FUN_00132c60(0xffffffff);
			if (uVar17 == 0) {
				this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
			}
			else {
				SetLayerAnim(this->currentBoomyLayerOverlayWeight, 8, 0x8b);
			}

			if (this->actorState - 0x73U < 3) {
				SetState(STATE_HERO_BOOMY_CATCH, -1);
			}

			SetBoomyState(HERO_BOOMY_STATE_CATCH_RECOVER);
		}
		break;
	case HERO_BOOMY_STATE_CATCH_RECOVER:
		fVar20 = this->field_0x1b74 + GetTimer()->cutsceneDeltaTime;
		this->field_0x1b74 = fVar20;
		if (0.28f <= fVar20) {
			this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
		}

		bVar10 = IsLayerAnimFinished(8);
		if ((bVar10 != false) || (bVar10 = IsLayerAnimEndReached(8), bVar10 != false)) {
			pCVar1 = this->pPlayerInput;
			if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar20 = 0.0f;
			}
			else {
				fVar20 = pCVar1->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue;
			}

			if (fVar20 == 0.0f) {
				this->field_0x1b64 = 0;
			}

			if (this->actorState - 0x73U < 3) {
				SetState(STATE_HERO_STAND, -1);
			}

			SetBoomyState(HERO_BOOMY_STATE_IDLE);
		}
		break;
	case HERO_BOOMY_STATE_RETHROW:
		fVar20 = this->field_0x1b74 + GetTimer()->cutsceneDeltaTime;
		this->field_0x1b74 = fVar20;

		if (fVar20 < 0.02f) break;

		pBoomy = this->pActorBoomy;
		if (pBoomy == (CActorBoomy*)0x0) {
		LAB_00135678:
			bVar10 = false;
		}
		else {
			iVar14 = pBoomy->actorState;
			bVar10 = true;
			if ((iVar14 != 5) && (iVar14 != 9)) {
				bVar10 = false;
			}

			if (((!bVar10) || (TestState_AllowAttack(0xffffffff) == 0)) || (bVar10 = true, this->field_0x1a48 != 0)) goto LAB_00135678;
		}

		if (((bVar10) && (bVar10 = EvolutionBoomyCanLaunch(), bVar10 != false)) && (1 < this->actorState - 0x11aU)) {
			this->pActorBoomy->UpdateFromOwner(4, &this->rotationQuat);
			pCVar12 = this->pActorBoomy->GetBestActorInVision();
			pBoomy = this->pActorBoomy;
			pBoomy->aBoomyTypeInfo[0].flags = pBoomy->aBoomyTypeInfo[0].flags & 0xfffffffe;
			if (pCVar12 == (CActor*)0x0) {
				pCVar3 = this->pActorBoomy;
				pCVar13 = pCVar3->GetVision();
				edF32Vector4ScaleHard(pCVar13->visionRange, &eStack48, &this->rotationQuat);
				SV_GetBoneWorldPosition(this->field_0x157c, &eStack32);
				edF32Vector4AddHard(&eStack32, &eStack32, &eStack48);
				this->pActorBoomy->UpdateFromOwner(3, &this->rotationQuat);
				this->pActorBoomy->SetTarget(&eStack32);
				(this->pActorBoomy)->field_0x1dc = 0.1f;
			}
			else {
				this->pActorBoomy->UpdateFromOwner(3, &this->rotationQuat);
				this->pActorBoomy->SetTarget(pCVar12, &pCVar12->currentLocation);
				(this->pActorBoomy)->field_0x1dc = 0.01f;
			}

			DoMessage(this->pActorBoomy, (ACTOR_MESSAGE)4, 0);
		}

		bVar10 = IsLayerAnimFinished(8);
		if ((bVar10 != false) || (bVar10 = IsLayerAnimEndReached(8), bVar10 != false)) {
			if (this->actorState - 0x73U < 3) {
				SetState(STATE_HERO_STAND, -1);
			}

			SetBoomyState(HERO_BOOMY_STATE_IDLE);
			this->pAnimationController->AddDisabledBone(this->animKey_0x1584);
		}
		break;
	case 0xb:
		this->field_0x1b74 = this->field_0x1b74 + GetTimer()->cutsceneDeltaTime;
		SV_GetBoneWorldPosition(this->field_0x157c, &eStack128);
		this->pActorBoomy->UpdatePosition(&eStack128, false);
		if (1.7f <= this->field_0x1b74) {
			bVar10 = this->field_0x18c0.IsValid();
			if (!bVar10) {
				this->field_0x18b4.Stop();
				this->field_0x18c0.InitSpatialized(this->pActorBoomy, 0x656ad6d2);
				this->field_0x18cc.InitSpatialized(this->pActorBoomy, 0x656ad6d2);
			}
		}
		break;
	case 0xd:
		fVar20 = edFIntervalUnitSrcLERP(this->field_0x1b84, 0.5f, 4.0f);
		SV_GetBoneWorldPosition(this->field_0x1598, &local_d0);
		edF32Vector4SubHard(&local_f0, &local_d0, &this->currentLocation);
		local_f0.y = 0.0f;
		fVar18 = edF32Vector4NormalizeHard(&local_f0, &local_f0);
		if (fVar18 == 0.0f) {
			local_f0.x = this->rotationQuat.x;
			local_f0.y = this->rotationQuat.y;
			local_f0.z = this->rotationQuat.z;
			local_f0.w = this->rotationQuat.w;
		}
		edF32Vector4ScaleHard(fVar20, &local_e0, &local_f0);
		edF32Vector4AddHard(&local_e0, &local_e0, &this->currentLocation);
		local_d0.x = local_e0.x;
		local_d0.z = local_e0.z;
		this_00 = this->pAnimationController;
		iVar14 = GetIdMacroAnim(this->currentAnimType);
		if (iVar14 < 0) {
			fVar20 = 0.0f;
		}
		else {
			fVar20 = this_00->GetAnimLength(iVar14, 1);
		}
		fVar18 = this->field_0x1b74;
		if (fVar18 <= fVar20 * 0.1f) {
			fVar20 = edFIntervalLERP(fVar18, 0.0f, fVar20 * 0.1f, 0.0f, 1.0f);
		}
		else {
			if (fVar20 * 0.9f <= fVar18) {
				fVar20 = edFIntervalLERP(fVar18, fVar20 * 0.9f, fVar20, 1.0f, 0.0f);
			}
			else {
				fVar20 = 1.0f;
			}
		}
		SV_GetBoneWorldPosition(this->field_0x157c, &local_e0);
		edF32Vector4ScaleHard(fVar20, &local_d0, &local_d0);
		edF32Vector4ScaleHard(1.0f - fVar20, &local_e0, &local_e0);
		edF32Vector4AddHard(&local_e0, &local_e0, &local_d0);
		local_e0.w = 1.0f;
		this->pActorBoomy->UpdatePosition(&local_e0, false);
		this->field_0x1b74 = this->field_0x1b74 + GetTimer()->cutsceneDeltaTime;
		SV_GetBoneWorldPosition(0x53538f8b, &local_e0);
		bVar10 = _SV_HIT_FightCollisionCheckIntersect(&local_c0, &local_e0, &this->pActorBoomy->currentLocation, 1);
		if ((bVar10 != false) &&
			(bVar10 = this->actorsExcludeTable.IsInList(local_c0.pActor),
				bVar10 == false)) {
			local_170.projectileType = 7;
			bVar10 = 0.25f <= this->field_0x1b84;
			if (bVar10) {
				local_170.flags = 0x35;
				local_170.damage = this->field_0x1b84 * 50.0f;
				local_170.field_0x10 = this->pBlow->field_0x10;
				local_170.field_0x30 = this->field_0x1b84 * 14.0f;
				local_170.field_0x70 = this->field_0x1b84 * 8.0f;
			}
			else {
				local_170.flags = 0x30;
				local_170.damage = this->pBlow->damage;
				local_170.field_0x10 = this->pBlow->field_0x10;
				local_170.field_0x30 = this->pBlow->field_0x18;
				local_170.field_0x70 = 0.0f;
			}
			local_170.field_0x50 = (ushort)bVar10;
			local_170.field_0x40 = local_c0.field_0x10;
			local_170.field_0x20.x = local_f0.z;
			local_170.field_0x20.y = 1.0f;
			local_170.field_0x20.z = -local_f0.x;
			local_170.field_0x20.w = 0.0f;
			local_170.field_0x60.x = -local_f0.x;
			local_170.field_0x60.y = 0.0f;
			local_170.field_0x60.z = -local_f0.z;
			local_170.field_0x60.w = 0.0f;
			DoMessage(local_c0.pActor, (ACTOR_MESSAGE)2, &local_170);
			this->actorsExcludeTable.Add(-1.0f, local_c0.pActor);
			this->fightFlags = this->fightFlags | FIGHT_FLAG_ATTACK_CONNECTED;
		}
		psVar4 = this->pBlow;
		if ((psVar4->nbSubObjs != 0) && (this->field_0x1b84 != 0.0f)) {
			psVar5 = psVar4->field_0x48;
			psVar5->boneRefA.pActor->SV_GetBoneWorldPosition(psVar5->boneRefA.boneId, &eStack448);
			psVar5->boneRefB.pActor->SV_GetBoneWorldPosition(psVar5->boneRefB.boneId, &local_1d0);
			edF32Vector4SubHard(&local_f0, &eStack448, &local_1d0);
			fVar20 = edF32Vector4NormalizeHard(&local_f0, &local_f0);
			uVar17 = EncodeFloat(fVar20 + 0.5f);
			if (uVar17 == 0) {
				uVar17 = 1;
			}
			uVar6 = this->field_0x18b0;
			if (uVar17 != uVar6) {
				if (uVar17 < uVar6) {
					this->field_0x18b0 = uVar6 - 1;
					this->field_0x1880[this->field_0x18b0].Kill();
					this->field_0x1880[this->field_0x18b0].Reset();
				}
				else {
					this->field_0x1880[uVar6].InitPositionRotation((edF32VECTOR4*)0x0, (edF32VECTOR4*)0x0);
					this->field_0x18b0 = this->field_0x18b0 + 1;
				}
			}
			local_d0.x = -local_f0.y;
			local_d0.y = local_f0.x;
			local_d0.z = 0.0f;
			local_d0.w = 0.0f;
			edF32Vector4SafeNormalize0Hard(&local_d0, &local_d0);
			if (1.0f < local_f0.z) {
				fVar18 = 1.0f;
			}
			else {
				fVar18 = -1.0f;
				if (-1.0f <= local_f0.z) {
					fVar18 = local_f0.z;
				}
			}
			fVar18 = acosf(fVar18);
			edF32Matrix4FromAngAxisSoft(-fVar18, &eStack432, &local_d0);
			edF32Matrix4ToEulerSoft(&eStack432, &local_1e0.xyz, "XYZ");
			edF32Vector4ScaleHard(1.0f, &eStack512, &local_f0);
			local_1f0 = local_1d0;
			uVar17 = 0;
			if (this->field_0x18b0 != 1) {
				do {
					this->field_0x1880[uVar17].SetRotationEuler(&local_1e0);
					this->field_0x1880[uVar17].SetPosition(&local_1f0);
					edF32Vector4AddHard(&local_1f0, &local_1f0, &eStack512);
					fVar20 = fVar20 - 1.0f;
					uVar17 = uVar17 + 1;
				} while (uVar17 < this->field_0x18b0 - 1U);
			}
			this->field_0x1880[uVar17].SetRotationEuler(&local_1e0);
			this->field_0x1880[uVar17].SetPosition(&local_1f0);
			local_d0.x = 1.0f;
			local_d0.y = 1.0f;
			local_d0.z = fVar20;
			local_d0.w = 1.0f;
			this->field_0x1880[uVar17].SetScale(&local_d0);
		}
	}

	if ((this->field_0x1b64 != 0) && (pCVar9 = this->field_0xc94, (pCameraViewManager->cameraStack).pActiveCamera != pCVar9)) {
		pCameraViewManager->PushCamera(pCVar9, 1);
	}

	if ((this->field_0x1b64 == 0) && (pCVar9 = this->field_0xc94, (pCameraViewManager->cameraStack).pActiveCamera == pCVar9)) {
		pCameraViewManager->PopCamera(pCVar9);
	}

	return;
}

void CActorHeroPrivate::ManageBoomyStateTerm()
{
	return;
}

// Should be in: D:/Projects/b-witch/ActorHero_Boomy.cpp
void CActorHeroPrivate::SetBoomyHairOn()
{
	this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);
}
