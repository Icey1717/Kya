#include "ActorPattern.h"
#include "ActorPatternCmd.h"
#include "ActorHero.h"
#include "ActorShadows.h"
#include "MemoryStream.h"
#include "MathOps.h"

CActorPattern::~CActorPattern()
{
	this->addOnGenerator.Term();

	return;
}

void CActorPattern::Create(ByteCode* pByteCode)
{
	uint uVar1;
	float fVar2;

	CActorAutonomous::Create(pByteCode);

	this->field_0x350 = pByteCode->GetF32();
	this->field_0x354 = pByteCode->GetF32();
	this->field_0x358 = pByteCode->GetF32();
	this->field_0x35c = pByteCode->GetF32();
	this->field_0x364 = pByteCode->GetF32();
	this->field_0x360 = pByteCode->GetF32();
	this->field_0x368 = pByteCode->GetF32();
	this->field_0x36c = pByteCode->GetS32();
	this->field_0x370 = pByteCode->GetS32();

	this->boneId = pByteCode->GetU32();
	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorPattern::Init()
{
	CBehaviourPatternAlone* pBehaviourAlone;

	CActorAutonomous::Init();

	ClearLocalData();
	
	this->addOnGenerator.Init(0);

	if (GetBehaviour(PATTERN_BEHAVIOUR_ALONE) != (CBehaviour*)0x0) {
		pBehaviourAlone = static_cast<CBehaviourPatternAlone*>(GetBehaviour(PATTERN_BEHAVIOUR_ALONE));
		pBehaviourAlone->patternPart.Init(pBehaviourAlone->pOwner->field_0x350, this);
	}

	return;
}

void CActorPattern::Reset()
{
	CActorAutonomous::Reset();

	ClearLocalData();

	return;
}

CBehaviour* CActorPattern::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == PATTERN_BEHAVIOUR_MULTIPLE) {
		pBehaviour = &this->behaviourMultiple;
	}
	else {
		if (behaviourType == PATTERN_BEHAVIOUR_ALONE) {
			pBehaviour = &this->behaviourAlone;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig CActorPattern::_gStateCfg_PAT[15] = {
	StateConfig(0xD,  0x4),      // [0]
	StateConfig(0x10, 0x804),    // [1]
	StateConfig(0xE,  0x804),    // [2]
	StateConfig(0xF,  0x804),    // [3]
	StateConfig(0x10, 0x844),    // [4]
	StateConfig(0x11, 0x844),    // [5]
	StateConfig(0x12, 0x1844),   // [6]
	StateConfig(0x12, 0x1844),   // [7]
	StateConfig(0x12, 0x1844),   // [8]
	StateConfig(0x12, 0x844),    // [9]
	StateConfig(0xF,  0x844),    // [10]
	StateConfig(0x10, 0x804),    // [11]
	StateConfig(0x14, 0x804),    // [12]
	StateConfig(0x15, 0x4),      // [13]
	StateConfig(0x16, 0x1),      // [14]
};


StateConfig* CActorPattern::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 6) < 15);
		pStateConfig = _gStateCfg_PAT + state + -6;
	}

	return pStateConfig;
}

void CActorPattern::ChangeManageState(int state)
{
	CActorAutonomous::ChangeManageState(state);

	if (state == 0) {
		if (field_0x460.IsValid()) {
			field_0x460.Stop();
		}

		if (field_0x468.IsValid()) {
			field_0x468.Stop();
		}
	}

	return;
}

int CActorPattern::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	edF32VECTOR4* pfVar1;
	CLifeInterface* pCVar2;
	CBehaviourPatternAlone* pCVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if (msg == 0x54) {
		_msg_params_0x54* pCastParam = static_cast<_msg_params_0x54*>(pMsgParam);
		this->field_0x37c = static_cast<CActorPatternCmd*>(pSender);
		iVar4 = pCastParam->field_0x0;
		if (iVar4 == 3) {
			this->dynamic.speed = 10.0f;
			pfVar1 = pCastParam->field_0xc;
			this->field_0x440 = *pfVar1;
			SetBehaviour(PATTERN_BEHAVIOUR_MULTIPLE, 0xf, -1);
		}
		else {
			if (iVar4 == 2) {
				SetBehaviour(PATTERN_BEHAVIOUR_MULTIPLE, 0xe, -1);
				pfVar1 = pCastParam->field_0xc;
				this->field_0x440 = *pfVar1;
			}
			else {
				if (iVar4 == 1) {
					SetState(0x11, -1);

					return 1;
				}

				if (iVar4 == 0) {
					if (pCastParam->field_0x4 == 1) {
						this->field_0x430 = 1;

						iVar4 = this->actorState;
						if ((iVar4 == 6) || (iVar4 == 7)) {
							SetBehaviour(PATTERN_BEHAVIOUR_ALONE, 8, -1);
						}
						else {
							if (this->curBehaviourId == PATTERN_BEHAVIOUR_ALONE) {
								pCVar3 = static_cast<CBehaviourPatternAlone*>(GetBehaviour(this->curBehaviourId));
								CActorHero* pCVar1 = this->field_0x37c->field_0x280;
								if ((pCVar1 != (CActorHero*)0x0) &&
									(fVar7 = pCVar1->currentLocation.x - this->currentLocation.x
										, fVar8 = pCVar1->currentLocation.y - this->currentLocation.y,
										fVar5 = pCVar1->currentLocation.z - this->currentLocation.z,
										fVar6 = FUN_00120250(pCVar3->field_0x90),
										sqrtf(fVar7 * fVar7 + fVar8 * fVar8 + fVar5 * fVar5) <= fVar6)) {
									SetState(PATTERN_STATE_ALONE_FIRE_FIRST, -1);

									return 1;
								}
							}

							SetState(PATTERN_STATE_ALONE_CHASE, -1);
						}
					}
					else {
						fVar5 = pCastParam->field_0x8;
						if (fVar5 < 2.147484e+09f) {
							this->field_0x430 = (byte)static_cast<int>(fVar5);
						}
						else {
							this->field_0x430 = (byte)static_cast<int>(fVar5 - 2.147484e+09f);
						}

						SetBehaviour(PATTERN_BEHAVIOUR_MULTIPLE, 10, -1);

						pfVar1 = pCastParam->field_0xc;
						this->field_0x440 = *pfVar1;
					}

					return 1;
				}
			}
		}
	}
	else {
		if (msg != 2) {
			iVar4 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
			return iVar4;
		}

		if (this->bInvincible != false) {
			_msg_hit_param* pHitParam = static_cast<_msg_hit_param*>(pMsgParam);
			LifeDecrease(pHitParam->damage);

			fVar5 = GetLifeInterface()->GetValue();
			if (fVar5 <= 0.0f) {
				SetState(0x14, -1);
			}
			else {
				PlayAnim(0x13);
			}

			return 1;
		}
	}

	return 0;
}

void CBehaviourPattern::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourPattern::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorPattern*>(pOwner);

	return;
}

int CBehaviourPattern::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourPattern::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourPatternAlone::Create(ByteCode* pByteCode)
{
	this->field_0x90 = pByteCode->GetF32();
	this->field_0x94 = pByteCode->GetF32();
	this->field_0x98 = pByteCode->GetF32();

	this->patternPart.Create(pByteCode);

	return;
}

void CBehaviourPatternAlone::Init()
{
	return;
}

void CBehaviourPatternAlone::Manage()
{
	this->pOwner->BehaviourPatternAlone_Manage(this);
}

void CBehaviourPatternAlone::Draw()
{
	return;
}

void CBehaviourPatternAlone::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorPattern* pPattern;
	float fVar2;

	CBehaviourPattern::Begin(pOwner, newState, newAnimationType);

	this->patternPart.Reset();

	if (newState == -1) {
		pPattern = this->pOwner;
		if (((pPattern->pCollisionData)->flags_0x4 & 2) == 0) {
			fVar2 = pPattern->distanceToGround;
			if (pPattern->field_0xf0 < fVar2) {
				pPattern->SetState(7, -1);
			}
			else {
				if (0.5f < fVar2) {
					pPattern->SetState(PATTERN_STATE_ALONE_COMEBACK_ALT, -1);
				}
				else {
					if (fVar2 <= 0.5f) {
						pPattern->SetState(PATTERN_STATE_ALONE_COMEBACK, -1);
					}
				}
			}
		}
		else {
			pPattern->SetState(6, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourPatternAlone::InitState(int newState)
{
	this->pOwner->BehaviourPatternAlone_InitState(newState);
}

void CBehaviourPatternAlone::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourPatternAlone::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourPatternMultiple::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourPatternMultiple::Init()
{
	return;
}

void CBehaviourPatternMultiple::Manage()
{
	this->pOwner->BehaviourPatternMultiple_Manage();
}

void CBehaviourPatternMultiple::Draw()
{
	return;
}

void CBehaviourPatternMultiple::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourPattern::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(0x11, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourPatternMultiple::InitState(int newState)
{
	this->pOwner->BehaviourPatternMultiple_InitState(newState);
}

void CBehaviourPatternMultiple::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourPatternMultiple::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void FUN_003a6a50(void)
{
	// Unknown stub function.
	return;
}

void CActorPattern::BehaviourPatternAlone_InitState(int newState)
{
	edF32VECTOR4* v2;
	float fVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 eStack48;
	_msg_params_0x54 local_20;
	CAnimation* pAnim;
	CCollision* pCol;

	if (newState == PATTERN_STATE_ALONE_FIRE_SECOND) {
		pAnim = this->pAnimationController;
		if (pAnim != (CAnimation*)0x0) {
			pAnim->RegisterBone(this->boneId);
		}
	}
	else {
		if (newState == PATTERN_STATE_ALONE_FIRE_FIRST) {
			this->field_0x450 = this->rotationQuat;
			this->field_0x450.y = 0.0f;

			FUN_003a6a50();

			pAnim = this->pAnimationController;
			if (pAnim != (CAnimation*)0x0) {
				pAnim->RegisterBone(this->boneId);
			}
		}
		else {
			if (newState == 0x14) {
				if (this->curBehaviourId == 4) {
					local_20.field_0x0 = 5;
					this->DoMessage(this->field_0x37c, (ACTOR_MESSAGE)0x54, &local_20);
				}

				pCol = this->pCollisionData;
				v2 = this->GetBottomPosition();
				edF32Vector4AddHard(&eStack48, &pCol->highestVertex, v2);
				edF32Vector4ScaleHard(0.5f, &eStack48, &eStack48);
				this->addOnGenerator.Generate(&eStack48);
			}
		}
	}

	return;
}

void CActorPattern::BehaviourPatternAlone_Manage(CBehaviourPatternAlone* pBehaviour)
{
	float fVar1;
	edAnmLayer* peVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	int layerIndex;
	StateConfig* pSVar6;
	uint uVar7;
	int iVar8;
	CBehaviour* pCVar9;
	float fVar10;
	CAnimation* pAnim;

	ComputeInvincibility();

	pBehaviour->patternPart.UpdatePatternPartLife();
	pBehaviour->patternPart.Draw();

	pAnim = this->pAnimationController;
	layerIndex = pAnim->PhysicalLayerFromLayerId(8);
	bVar5 = this->pAnimationController->IsLayerActive(8);
	if (bVar5 != false) {
		if ((GetStateFlags(this->actorState) & 0x800) == 0) {
			pAnim->anmBinMetaAnimator.SetAnimOnLayer(-1, layerIndex, 0xffffffff);
		}
		else {
			pAnim->anmBinMetaAnimator.SetLayerBlendingOp(layerIndex, 0);
			(pAnim->anmBinMetaAnimator).aAnimData[layerIndex].field_0x4 = 0.5f;
			iVar8 = GetIdMacroAnim(0xc);
			pAnim->anmBinMetaAnimator.SetAnimOnLayer(iVar8, layerIndex, 0xffffffff);
		}
	}

	ManageFx();

	switch (this->actorState) {
	case 6:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		break;
	case 8:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(9, -1);
		}
		break;
	case 9:
		FUN_003a1ad0();
		break;
	case PATTERN_STATE_ALONE_CHASE:
		StatePatternChase(pBehaviour);
		break;
	case 0xb:
		SV_MOV_DecreaseSpeedIntensity(this->field_0x35c);
		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->dynamic.speed = 0.0f;
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;

			SetState(PATTERN_STATE_ALONE_FIRE_FIRST, -1);
		}
		break;
	case PATTERN_STATE_ALONE_FIRE_FIRST:
		StatePatternFireFirst(pBehaviour);
		break;
	case PATTERN_STATE_ALONE_FIRE_SECOND:
		StatePatternFireSecond(pBehaviour);
		break;
	case 0x10:
		SV_MOV_DecreaseSpeedIntensity(this->field_0x35c);

		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

		if ((pBehaviour->field_0x94 < this->timeInAir) ||
			(this->dynamic.linearAcceleration < 0.5f)) {
			this->dynamic.speed = 0.0f;
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;
			if (this->curBehaviourId == PATTERN_BEHAVIOUR_ALONE) {
				CBehaviourPatternAlone* pAlone = (CBehaviourPatternAlone*)GetBehaviour(this->curBehaviourId);
				CActor* pHero = this->field_0x37c->field_0x280;
				if ((fVar1 != 0.0f) &&
					(fVar3 = pHero->currentLocation.x - this->currentLocation.x,
						fVar4 = pHero->currentLocation.y - this->currentLocation.y,
						fVar1 = pHero->currentLocation.z - this->currentLocation.z,
						fVar10 = FUN_00120250(pAlone->field_0x90),
						sqrtf(fVar3 * fVar3 + fVar4 * fVar4 + fVar1 * fVar1) <= fVar10)) {
					SetState(PATTERN_STATE_ALONE_FIRE_FIRST, -1);
					break;
				}
			}

			SetState(PATTERN_STATE_ALONE_CHASE, -1);
		}
		break;
	case 0x11:
		FUN_003a1370();
		break;
	case PATTERN_STATE_ALONE_COMEBACK_ALT:
		StatePatternComeBackAlt();
		break;
	case PATTERN_STATE_ALONE_COMEBACK:
		StatePatternComeBack();
		break;
	case 0x14:
		if ((this->pAnimationController->IsCurrentLayerAnimEndReached(0)) && ((pBehaviour->patternPart).field_0x50 < 1)) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;

			EvaluateDisplayState();

			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
		}
	}

	if (this->currentAnimType == 0x13) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(this->actorState, -1);
		}
	}

	return;
}

void CActorPattern::BehaviourPatternMultiple_InitState(int newState)
{
	edF32VECTOR4* v2;
	edF32VECTOR4 eStack48;
	_msg_params_0x54 local_20;
	CCollision* pCol;

	if (newState == 0xe) {
		this->dynamic.speed = 2.0f;
	}
	else {
		if (newState == 0x14) {
			if (this->curBehaviourId == 4) {
				local_20.field_0x0 = 5;
				DoMessage(this->field_0x37c, (ACTOR_MESSAGE)0x54, &local_20);
			}

			pCol = this->pCollisionData;
			v2 = this->GetBottomPosition();
			edF32Vector4AddHard(&eStack48, &pCol->highestVertex, v2);
			edF32Vector4ScaleHard(0.5f, &eStack48, &eStack48);
			this->addOnGenerator.Generate(&eStack48);
		}
	}

	return;
}

void CActorPattern::BehaviourPatternMultiple_Manage()
{
	IMPLEMENTATION_GUARD();
}

void CActorPattern::ClearLocalData()
{
	this->lightConfig.lightAmbient = gF32Vector4Zero;
	this->lightConfig.lightDirections = gF32Matrix4Unit;
	this->lightConfig.lightColorMatrix = gF32Matrix4Unit;

	(this->lightConfig).config.pLightAmbient = &this->lightConfig.lightAmbient;
	(this->lightConfig).config.pLightDirections = &this->lightConfig.lightDirections;
	(this->lightConfig).config.pLightColorMatrix = &this->lightConfig.lightColorMatrix;

	this->field_0x430 = 1;
	this->field_0xf0 = this->field_0x368;

	return;
}

void CActorPattern::ComputeInvincibility()
{
	int iVar1;

	if (this->currentAnimType == 0x13) {
		this->bInvincible = false;
	}
	else {
		switch (this->actorState) {
		case 6:
			this->bInvincible = false;
			break;
		case 8:
			this->bInvincible = false;
			break;
		case 9:
			this->bInvincible = false;
			break;
		case PATTERN_STATE_ALONE_CHASE:
			this->bInvincible = false;
			break;
		case 0xb:
			this->bInvincible = true;
			break;
		case 0xf:
		case 0x10:
			this->bInvincible = true;
			break;
		case 0x11:
			this->bInvincible = false;
			break;
		case PATTERN_STATE_ALONE_COMEBACK_ALT:
			this->bInvincible = false;
			break;
		case PATTERN_STATE_ALONE_COMEBACK:
			this->bInvincible = false;
			break;
		case 0x14:
			this->bInvincible = false;
		}

		iVar1 = this->curBehaviourId;
		if (iVar1 == PATTERN_BEHAVIOUR_MULTIPLE) {
			if (this->actorState == 0xe) {
				this->bInvincible = true;
			}
		}
		else {
			if (iVar1 == PATTERN_BEHAVIOUR_ALONE) {
				iVar1 = this->actorState;
				if (iVar1 == PATTERN_STATE_ALONE_FIRE_SECOND) {
					this->bInvincible = true;
				}
				else {
					if (iVar1 == PATTERN_STATE_ALONE_FIRE_FIRST) {
						this->bInvincible = true;
					}
				}
			}
		}
	}

	return;
}

void CActorPattern::ManageFx()
{
	int iVar1;
	CNewFx* pCVar2;
	bool bVar3;
	StateConfig* pSVar4;
	uint uVar5;
	edF32VECTOR4 local_10;
	CFxManager* pFxManager;

	if ((GetStateFlags(this->actorState) & 0x1000) == 0) {
		if (this->field_0x460.IsValid()) {
			this->field_0x460.Stop();
		}

		if (this->field_0x468.IsValid()) {
			this->field_0x468.Stop();
		}
	}
	else {
		SV_GetBoneWorldPosition(this->boneId, &local_10);

		pFxManager = CScene::ptable.g_EffectsManager_004516b8;
	
		if (!this->field_0x460.IsValid()) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x460, this->field_0x36c, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x460.Start();
			this->field_0x460.SpatializeOnActor(6, this, this->boneId);
		}

		if ((!this->field_0x468.IsValid()) && (this->distanceToGround < this->field_0xf0)) {
			pFxManager->GetDynamicFx(&this->field_0x468, this->field_0x370, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x468.Start();
		}

		if (this->field_0x468.IsValid()) {
			local_10.y = this->currentLocation.y - this->distanceToGround;
			if (this->distanceToGround < this->field_0xf0) {
				this->field_0x468.SetPosition(&local_10);				
			}
			else {
				this->field_0x468.Stop();
			}
		}
	}

	return;
}

void CActorPattern::StatePatternComeBack()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	undefined8 uVar4;
	float fVar5;
	edF32VECTOR4 local_70;
	CActorMovLevitateParamOut movLevitateParamOut;
	CActorMovLevitateParamIn movLevitateParamIn;

	movLevitateParamIn.pRotation = (edF32VECTOR4*)0x0;
	movLevitateParamOut.flags = 0;
	movLevitateParamIn.rotSpeed = this->field_0x358;
	movLevitateParamIn.acceleration = this->field_0x354;
	movLevitateParamIn.speed = 1.0f;
	movLevitateParamIn.field_0x1c = 1.0f;
	movLevitateParamIn.field_0x20 = 0.0f;
	movLevitateParamIn.flags = 0x456;
	local_70 = this->currentLocation;

	SV_MOV_MoveInLevitation(&movLevitateParamOut, &movLevitateParamIn, &local_70);

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		fVar5 = this->distanceToGround;

		if (this->field_0xf0 < fVar5) {
			SetState(7, -1);
		}
		else {
			if (0.5f < fVar5) {
				SetState(PATTERN_STATE_ALONE_COMEBACK_ALT, -1);
			}
			else {
				if (fVar5 <= 0.5f) {
					SetState(PATTERN_STATE_ALONE_COMEBACK, -1);
				}
			}
		}
	}
	else {
		SetState(6, -1);
	}

	if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
		SetState(6, -1);
	}

	return;
}

void CActorPattern::FUN_003a1ad0()
{
	float fVar1;
	float fVar2;
	CBehaviourPatternAlone* pBehaviourAlone;
	float fVar4;
	float fVar5;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	edF32VECTOR4 local_10;
	CActorHero* pHero;

	local_10 = this->currentLocation;

	fVar5 = this->distanceToGround;
	if (fVar5 < this->field_0xf0) {
		local_10.y = local_10.y + (this->field_0x360 - fVar5);
	}

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = this->field_0x358 / 2.0f;
	movParamsIn.flags = 0x406;
	movParamsIn.acceleration = this->field_0x354 / 2.0f;
	movParamsIn.speed = 5.0f;

	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &local_10);

	pHero = this->field_0x37c->field_0x280;
	if (pHero != (CActorHero*)0x0) {
		CActor::SV_UpdateOrientationToPosition2D(2.0f, &pHero->currentLocation);
	}

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	fVar5 = this->distanceToGround;
	if ((this->field_0xf0 < fVar5) || (this->field_0x360 < fVar5)) {
		if (this->curBehaviourId == PATTERN_BEHAVIOUR_ALONE) {
			pBehaviourAlone =(CBehaviourPatternAlone*)GetBehaviour(this->curBehaviourId);
			pHero = this->field_0x37c->field_0x280;
			if ((pHero != (CActorHero*)0x0) &&
				(fVar5 = pHero->currentLocation.x - this->currentLocation.x, fVar1 = pHero->currentLocation.y - this->currentLocation.y,
					fVar2 = pHero->currentLocation.z - this->currentLocation.z,
					fVar4 = FUN_00120250(pBehaviourAlone->field_0x90),
					sqrtf(fVar5 * fVar5 + fVar1 * fVar1 + fVar2 * fVar2) <= fVar4)) {
				SetState(PATTERN_STATE_ALONE_FIRE_FIRST, -1);

				return;
			}
		}

		SetState(PATTERN_STATE_ALONE_CHASE, -1);
	}

	return;
}

void CActorPattern::StatePatternChase(CBehaviourPatternAlone* pBehaviour)
{
	float fVar1;
	CActorMovLevitateParamOut movLevitateParamOut;
	CActorMovLevitateParamIn movLevitateParamIn;

	if (this->field_0x37c->field_0x280 == (CActorHero*)0x0) {
		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);
		SetState(0x11, -1);
	}
	else {
		movLevitateParamIn.pRotation = (edF32VECTOR4*)0x0;
		movLevitateParamOut.flags = 0;
		movLevitateParamIn.rotSpeed = this->field_0x358;
		movLevitateParamIn.acceleration = this->field_0x354;
		movLevitateParamIn.speed = this->field_0x350;
		movLevitateParamIn.flags = 0x456;
		movLevitateParamIn.field_0x20 = this->field_0x360;
		movLevitateParamIn.field_0x1c = this->field_0x350 / 2.0f;
		SV_MOV_MoveInLevitation(&movLevitateParamOut, &movLevitateParamIn, &this->field_0x37c->field_0x280->currentLocation);
		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);
		fVar1 = FUN_00120250(pBehaviour->field_0x90);
		if (movLevitateParamOut.moveVelocity <= fVar1) {
			SetState(0xb, -1);
		}
	}

	return;
}

void CActorPattern::StatePatternFireFirst(CBehaviourPatternAlone* pBehaviour)
{
	float fVar1;
	CActorMovLevitateParamOut movLevitateParamOut;
	CActorMovLevitateParamIn movLevitateParamIn;
	edF32VECTOR4 eStack16;

	if (this->field_0x37c->field_0x280 == (CActorHero*)0x0) {
		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);
		SetState(0x11, -1);
	}
	else {
		ComputeFireFirstPoints(&eStack16, pBehaviour);

		movLevitateParamIn.pRotation = (edF32VECTOR4*)0x0;
		movLevitateParamOut.flags = 0;
		movLevitateParamIn.rotSpeed = this->field_0x358;
		movLevitateParamIn.acceleration = this->field_0x354;
		movLevitateParamIn.speed = this->field_0x350;
		movLevitateParamIn.flags = 0x452;
		movLevitateParamIn.field_0x20 = this->field_0x360;
		movLevitateParamIn.field_0x1c = this->field_0x350 / 2.0f;
		SV_MOV_MoveInLevitation(&movLevitateParamOut, &movLevitateParamIn, &eStack16);

		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);
		SV_GetBoneWorldPosition(this->boneId, &eStack16);
		eStack16.y = this->currentLocation.y - this->distanceToGround;
		if (movLevitateParamOut.field_0x20 != false) {
			pBehaviour->patternPart.FireContinue(this->field_0x37c->field_0x280, &this->pShadow->field_0x20, &eStack16);
		}

		fVar1 = FUN_00120250(0.3f);
		if (movLevitateParamOut.moveVelocity <= fVar1) {
			ComputeFireSecondPoints(&this->field_0x440, pBehaviour);
			SetState(PATTERN_STATE_ALONE_FIRE_SECOND, -1);
		}
	}

	return;
}

void CActorPattern::StatePatternFireSecond(CBehaviourPatternAlone* pBehaviour)
{
	float fVar1;
	CActorMovLevitateParamOut movLevitateParamOut;
	CActorMovLevitateParamIn movLevitateParamIn;
	edF32VECTOR4 eStack16;

	if (this->field_0x37c->field_0x280 == (CActorHero*)0x0) {
		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);
		SetState(0x11, -1);
	}
	else {
		movLevitateParamIn.pRotation = (edF32VECTOR4*)0x0;
		movLevitateParamOut.flags = 0;
		movLevitateParamIn.rotSpeed = this->field_0x358;
		movLevitateParamIn.acceleration = this->field_0x354;
		movLevitateParamIn.speed = this->field_0x350;
		movLevitateParamIn.flags = 0x452;
		movLevitateParamIn.field_0x20 = this->field_0x360;
		movLevitateParamIn.field_0x1c = this->field_0x350 / 2.0f;
		SV_MOV_MoveInLevitation(&movLevitateParamOut, &movLevitateParamIn, &this->field_0x440);
		ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);
		SV_GetBoneWorldPosition(this->boneId, &eStack16);
		eStack16.y = this->currentLocation.y - this->distanceToGround;
		if (movLevitateParamOut.field_0x20 != false) {
			pBehaviour->patternPart.FireContinue(this->field_0x37c->field_0x280, &this->pShadow->field_0x20, &eStack16);
		}

		fVar1 = FUN_00120250(0.3f);
		if ((movLevitateParamOut.moveVelocity <= fVar1) ||
			(((this->pCollisionData)->flags_0x4 & 7) != 0)) {
			SetState(0x10, -1);
		}
	}

	return;
}

void CActorPattern::FUN_003a1370()
{
	float fVar1;
	float fVar3;
	float fVar4;
	CActorMovLevitateParamOut movLevitateParamOut;
	CActorMovLevitateParamIn movLevitateParamIn;
	edF32VECTOR4 local_10;

	local_10 = this->baseLocation;
	fVar4 = this->distanceToGround;
	if (fVar4 < this->field_0xf0) {
		local_10.y = local_10.y + (this->field_0x360 - fVar4);
	}

	movLevitateParamIn.pRotation = (edF32VECTOR4*)0x0;
	movLevitateParamOut.flags = 0;
	movLevitateParamIn.rotSpeed = this->field_0x358;
	movLevitateParamIn.acceleration = this->field_0x354;
	movLevitateParamIn.speed = this->field_0x350;
	movLevitateParamIn.flags = 0x452;
	movLevitateParamIn.field_0x20 = this->field_0x360;
	movLevitateParamIn.field_0x1c = this->field_0x350 / 2.0f;
	SV_MOV_MoveInLevitation(&movLevitateParamOut, &movLevitateParamIn, &local_10);

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	fVar4 = this->baseLocation.x - this->currentLocation.x;
	fVar1 = this->baseLocation.z - this->currentLocation.z;
	fVar3 = FUN_00120250(0.5f);

	if (sqrtf(fVar4 * fVar4 + 0.0f + fVar1 * fVar1) < fVar3) {
		this->dynamic.speed = 0.0f;
		if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
			fVar4 = this->distanceToGround;
			if (this->field_0xf0 < fVar4) {
				SetState(7, -1);
			}
			else {
				if (0.5f < fVar4) {
					SetState(PATTERN_STATE_ALONE_COMEBACK_ALT, -1);
				}
				else {
					if (fVar4 <= 0.5f) {
						SetState(PATTERN_STATE_ALONE_COMEBACK, -1);
					}
				}
			}
		}
		else {
			SetState(6, -1);
		}
	}

	return;
}

void CActorPattern::StatePatternComeBackAlt()
{
	undefined8 uVar1;
	float fVar2;
	edF32VECTOR4 local_70;
	CActorMovLevitateParamOut movLevitateParamOut;
	CActorMovLevitateParamIn movLevitateParamIn;

	movLevitateParamIn.pRotation = (edF32VECTOR4*)0x0;
	movLevitateParamOut.flags = 0;
	movLevitateParamIn.rotSpeed = this->field_0x358;
	movLevitateParamIn.acceleration = this->field_0x354;
	movLevitateParamIn.speed = this->field_0x350;
	movLevitateParamIn.field_0x20 = 0.0f;
	movLevitateParamIn.flags = 0x456;
	movLevitateParamIn.field_0x1c = this->field_0x350 / 2.0f;

	local_70 = this->currentLocation;
	local_70.y = local_70.y - this->distanceToGround;
	CActorMovable::SV_MOV_MoveInLevitation(&movLevitateParamOut, &movLevitateParamIn, &local_70);

	ManageDyn(4.0f, 0x40000, (CActorsTable*)0x0);

	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		fVar2 = this->distanceToGround;
		if (this->field_0xf0 < fVar2) {
			SetState(7, -1);
		}
		else {
			if (0.5f < fVar2) {
				SetState(PATTERN_STATE_ALONE_COMEBACK_ALT, -1);
			}
			else {
				if (fVar2 <= 0.5f) {
					SetState(PATTERN_STATE_ALONE_COMEBACK, -1);
				}
			}
		}
	}
	else {
		SetState(6, -1);
	}

	return;
}

void CActorPattern::ComputeFireFirstPoints(edF32VECTOR4* param_2, CBehaviourPatternAlone* pBehaviour)
{
	CActorHero* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	this->field_0x37c->field_0x280->SV_GetGroundPosition(&eStack32);

	pCVar1 = this->field_0x37c->field_0x280;

	local_30.x = pCVar1->dynamic.horizontalVelocityDirectionEuler.x;
	local_30.z = pCVar1->dynamic.horizontalVelocityDirectionEuler.z;
	local_30.w = pCVar1->dynamic.horizontalVelocityDirectionEuler.w;
	local_30.y = 0.0f;

	edF32Vector4ScaleHard(this->field_0x37c->field_0x280->dynamic.horizontalLinearSpeed * this->field_0x37c->field_0x1f0, &local_30, &local_30);
	edF32Vector4AddHard(&eStack16, &eStack32, &local_30);
	pCVar1 = this->field_0x37c->field_0x280;
	if (pCVar1->dynamic.horizontalLinearSpeed < 0.5f) {
		SV_GetGroundPosition(&eStack80);
		edF32Vector4SubHard(&local_40, &eStack80, &eStack32);
	}
	else {
		local_40.x = pCVar1->rotationQuat.z;
		local_40.y = 0.0f;
		local_40.z = -this->field_0x37c->rotationQuat.x;
	}

	edF32Vector4NormalizeHard(&local_40, &local_40);
	edF32Vector4ScaleHard(pBehaviour->field_0x98 * 0.5f, &local_40, &local_40);
	edF32Vector4AddHard(&local_60, &eStack16, &local_40);
	edF32Vector4GetNegHard(&local_40, &local_40);
	edF32Vector4AddHard(&local_70, &eStack16, &local_40);

	fVar2 = local_60.x - this->currentLocation.x;
	fVar4 = local_60.y - this->currentLocation.y;
	fVar6 = local_60.z - this->currentLocation.z;

	fVar3 = local_70.x - this->currentLocation.x;
	fVar5 = local_70.y - this->currentLocation.y;
	fVar7 = local_70.z - this->currentLocation.z;

	if (sqrtf(fVar2 * fVar2 + fVar4 * fVar4 + fVar6 * fVar6) <
		sqrtf(fVar3 * fVar3 + fVar5 * fVar5 + fVar7 * fVar7)) {
		*param_2 = local_60;
	}
	else {
		*param_2 = local_70;
	}

	return;
}

void CActorPattern::ComputeFireSecondPoints(edF32VECTOR4* param_2, CBehaviourPatternAlone* pBehaviour)
{
	CActorHero* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	this->field_0x37c->field_0x280->SV_GetGroundPosition(&eStack32);
	pCVar1 = this->field_0x37c->field_0x280;

	local_30.x = pCVar1->rotationQuat.x;
	local_30.z = pCVar1->rotationQuat.z;
	local_30.w = pCVar1->rotationQuat.w;
	local_30.y = 0.0f;

	edF32Vector4ScaleHard(this->field_0x37c->field_0x280->dynamic.horizontalLinearSpeed * this->field_0x37c->field_0x1f0, &local_30, &local_30);
	edF32Vector4AddHard(&eStack16, &eStack32, &local_30);
	pCVar1 = this->field_0x37c->field_0x280;
	if (pCVar1->dynamic.horizontalLinearSpeed < 0.5f) {
		SV_GetGroundPosition(&eStack80);
		edF32Vector4SubHard(&local_40, &eStack80, &eStack32);
	}
	else {
		local_40.x = pCVar1->rotationQuat.z;
		local_40.y = 0.0f;
		local_40.z = -this->field_0x37c->rotationQuat.x;
	}

	edF32Vector4NormalizeHard(&local_40, &local_40);
	edF32Vector4ScaleHard(-pBehaviour->field_0x98 * 0.5f, &local_40, &local_40);
	edF32Vector4AddHard(&local_60, &eStack16, &local_40);
	edF32Vector4GetNegHard(&local_40, &local_40);
	edF32Vector4AddHard(&local_70, &eStack16, &local_40);

	fVar2 = local_60.x - this->currentLocation.x;
	fVar4 = local_60.y - this->currentLocation.y;
	fVar6 = local_60.z - this->currentLocation.z;

	fVar3 = local_70.x - this->currentLocation.x;
	fVar5 = local_70.y - this->currentLocation.y;
	fVar7 = local_70.z - this->currentLocation.z;

	if (sqrtf(fVar3 * fVar3 + fVar5 * fVar5 + fVar7 * fVar7) <
		sqrtf(fVar2 * fVar2 + fVar4 * fVar4 + fVar6 * fVar6)) {
		*param_2 = local_60;
	}
	else {
		*param_2 = local_70;
	}

	return;
}
