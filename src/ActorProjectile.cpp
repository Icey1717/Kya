#include "ActorProjectile.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "LightManager.h"
#include "ActorManager.h"
#include "ActorFactory.h"
#include "CollisionRay.h"
#include "ActorHero.h"

CPatternPart::CPatternPart()
{

}

void CPatternPart::Create(ByteCode* pByteCode)
{
	uint uVar2;

	this->field_0x4 = pByteCode->GetF32();
	this->field_0x0 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x8 = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->materialId = pByteCode->GetS32();
	this->field_0x1c = pByteCode->GetF32();
	this->field_0x20 = pByteCode->GetF32();

	if (CScene::_pinstance->field_0x1c < 2.21f) {
		this->field_0x24 = 1.0f;
	}
	else {
		this->field_0x24 = pByteCode->GetF32();
	}

	if (this->field_0x24 <= 0.0f) {
		this->field_0x24 = 1.0f;
	}

	this->field_0x28 = pByteCode->GetU32();
	this->field_0x48 = -1.0f;

	this->nbPointPatterns = (int)this->field_0x1c;
	if (0.0f < this->field_0x1c - (float)(int)this->field_0x1c) {
		this->nbPointPatterns = this->nbPointPatterns + 1;
	}

	this->nbPointPatterns = (int)(float)((int)(float)this->nbPointPatterns * (uint)(0.0f < (float)this->nbPointPatterns)); // clamp

	uVar2 = this->nbPointPatterns;
	if (0 < (int)uVar2) {
		this->aPointPatterns = new CPointPattern[uVar2];
	}

	return;
}

void CPatternPart::Init(float param_1, CActor* pOwner)
{
	this->pOwner = pOwner;

	CActor::SV_InstallMaterialId(this->materialId);

	if (0.0f < this->field_0x14) {
		this->field_0x48 = this->field_0x14;
	}
	else {
		if (0.0f < param_1) {
			this->field_0x48 = (float)this->nbPointPatterns / param_1;
		}
		else {
			if ((this->field_0x48 <= 0.0f) && (0 < this->nbPointPatterns)) {
				this->field_0x48 = 1.0f;
			}
		}
	}

	Reset();

	return;
}

void CPatternPart::Reset()
{
	CPointPattern* pPointPattern;
	int iVar6;

	iVar6 = 0;
	if (0 < this->nbPointPatterns) {
		do {
			pPointPattern = this->aPointPatterns + iVar6;
			pPointPattern->field_0x20 = 0;
			pPointPattern->field_0x24 = 0;
			pPointPattern->field_0x10 = g_xVector;
			pPointPattern->field_0x25 = 0;
			iVar6 = iVar6 + 1;
		} while (iVar6 < this->nbPointPatterns);
	}

	this->field_0x40 = 0;
	this->field_0x50 = 0;
	this->field_0x4c = 0;
	this->field_0x30 = g_xVector;
	this->field_0x70 = this->field_0x48;
	this->field_0x78 = -8.0f;
	this->field_0x74 = 0;
	this->field_0x5c = -1;

	this->field_0x60 = 0.0f;
	this->field_0x64 = 0.0f;
	this->field_0x68 = 0.0f;
	this->field_0x6c = 1.0f;
	return;
}

void CPatternPart::FUN_003a7cc0(float param_2)
{
	if (this->field_0x14 < 0.0f) {
		if ((param_2 <= 0.0f) || ((float)this->nbPointPatterns <= 0.0f)) {
			if (this->field_0x48 <= 0.0f) {
				this->field_0x48 = 1.0f;
			}
		}
		else {
			this->field_0x48 = (float)this->nbPointPatterns / param_2;
		}
	}

	return;
}

CActorProjectile::CActorProjectile()
{
	//this->field_0x574 = 0;
	//this->field_0x578 = 0;
	//this->field_0x590 = 0;

	return;
}

void CActorProjectile::Create(ByteCode* pByteCode)
{
	ProjectileSubObj* pPVar1;
	CCollision* pCVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);

	pPVar1 = (ProjectileSubObj*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pPVar1 + 1);
	this->aProjectileSubObjs = pPVar1;

	pPVar1 = this->aProjectileSubObjs;
	fVar3 = pPVar1->field_0x44;
	if (fVar3 < pPVar1->field_0x40) {
		pPVar1->field_0x44 = pPVar1->field_0x40;
		this->aProjectileSubObjs->field_0x40 = fVar3;
	}
	pPVar1 = this->aProjectileSubObjs;
	if ((pPVar1->timeToExplode != 0.0f) && (pPVar1->timeToExplode < pPVar1->field_0x40 + pPVar1->field_0x44)) {
		pPVar1->field_0x40 = 0.0f;
		this->aProjectileSubObjs->field_0x44 = 0.0f;
	}

	SV_InstallMaterialId(this->aProjectileSubObjs->materialId);
	this->patternPart.Create(pByteCode);

	pCVar2 = this->pCollisionData;
	if (pCVar2 != (CCollision*)0x0) {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x10001;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffffdf;
	}

	return;
}

void CActorProjectile::Init()
{
	this->patternPart.Init(-0.5f, this);
	this->patternPart.Reset();

	CActorAutonomous::Init();

	this->lightAmbient = gF32Vector4Zero;
	this->lightDirections = gF32Matrix4Unit;
	this->lightColors = gF32Matrix4Unit;

	(this->lightConfig).pLightAmbient = &this->lightAmbient;
	(this->lightConfig).pLightDirections = &this->lightDirections;
	(this->lightConfig).pLightColorMatrix = &this->lightColors;

	this->field_0x410 = 0;

	ClearLocalData();

	return;
}

void CActorProjectile::Draw()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorProjectile::ComputeLighting()
{
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, &this->lightConfig);

	if (this->field_0x410 == 1) {
		(this->lightAmbient).x = 255.0f;
		(this->lightAmbient).y = 255.0f;
		(this->lightAmbient).z = 255.0f;
	}
}

void CActorProjectile::Reset()
{
	int* piVar1;
	float fVar2;
	bool bVar3;
	float fVar4;
	float fVar5;
	int iVar6;
	edF32MATRIX4* peVar7;
	edF32MATRIX4* peVar8;

	CActorAutonomous::Reset();
	this->patternPart.Reset();

	StopAllFx();
	
	this->lightAmbient = gF32Vector4Zero;
	this->lightDirections = gF32Matrix4Unit;
	this->lightColors = gF32Matrix4Unit;

	(this->lightConfig).pLightAmbient = &this->lightAmbient;
	(this->lightConfig).pLightDirections = &this->lightDirections;
	(this->lightConfig).pLightColorMatrix = &this->lightColors;

	this->field_0x410 = 0;

	ClearLocalData();
	return;
}

void CActorProjectile::CheckpointReset()
{
	ClearLocalData();
	Reset();

	return;
}

CBehaviour* CActorProjectile::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	switch (behaviourType) {
	case PROJECTILE_BEHAVIOUR_STAND:
		pBehaviour = &behaviourProjectileStand;
		break;
	case PROJECTILE_BEHAVIOUR_STRAIGHT:
		pBehaviour = new CBehaviourProjectileStraight;
		break;
	case PROJECTILE_BEHAVIOUR_PORTABLE:
		pBehaviour = new CBehaviourProjectilePortable;
		break;
	case PROJECTILE_BEHAVIOUR_INVENTORY:
		pBehaviour = new CBehaviourInventory;
		break;
	case PROJECTILE_BEHAVIOUR_LAVA_BALL:
		pBehaviour = new CBehaviourProjectileLavaBall;
		break;
	case PROJECTILE_BEHAVIOUR_EXCUSE:
		pBehaviour = new CBehaviourProjectileExcuse;
		break;
	case PROJECTILE_BEHAVIOUR_INACTIVE:
		pBehaviour = new CBehaviourInactive;
		break;
	case 0xd:
		pBehaviour = new CBehaviourProjectileNew;
		break;
	default:
		assert(behaviourType < 3);
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		break;
	}

	return pBehaviour;
}

StateConfig CActorProjectile::_gStateCfg_PRJ[27] = {
	StateConfig(0x0, 0x0),
	StateConfig(0x13, 0x9800),
	StateConfig(0x13, 0xB800),
	StateConfig(0x13, 0x9800),
	StateConfig(0x13, 0x9800),
	StateConfig(0x13, 0x9800),
	StateConfig(0x0, 0xA902),
	StateConfig(0x11, 0x22),
	StateConfig(0x11, 0x22),
	StateConfig(0x0, 0xE900),
	StateConfig(0x9, 0x8900),
	StateConfig(0x9, 0xA900),
	StateConfig(0x0, 0x0),
	StateConfig(0xC, 0x900),
	StateConfig(0xD, 0x900),
	StateConfig(0xE, 0x0),
	StateConfig(0xF, 0x0),
	StateConfig(0x0, 0xE900),
	StateConfig(0x6, 0x0),
	StateConfig(0x10, 0x0),
	StateConfig(0x9, 0x6000),
	StateConfig(0x10, 0x0),
	StateConfig(0x10, 0x0),
	StateConfig(0xFFFFFFFF, 0x0),
	StateConfig(0xFFFFFFFF, 0x0),
	StateConfig(0xFFFFFFFF, 0x0),
	StateConfig(0x19, 0x902),
};

StateConfig* CActorProjectile::GetStateCfg(int state)
{
	StateConfig* pStatConfig;

	if (state < 6) {
		pStatConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state - 6) < 27);
		pStatConfig = _gStateCfg_PRJ + state + -6;
	}

	return pStatConfig;
}

void CActorProjectile::ChangeManageState(int state)
{
	int iVar1;
	int* piVar2;
	CBehaviourProjectileStand* pBehaviourStand;
	CBehaviourProjectilePortable* pBehaviourPortable;
	CActorProjectile* pProjectile;

	CActorAutonomous::ChangeManageState(state);

	iVar1 = this->curBehaviourId;
	if (iVar1 == PROJECTILE_BEHAVIOUR_PORTABLE) {
		pBehaviourPortable = reinterpret_cast<CBehaviourProjectilePortable*>(GetBehaviour(this->curBehaviourId));
		if (state == 0) {
			pProjectile = pBehaviourPortable->pOwner;

			StopAllFx();

			pProjectile = pBehaviourPortable->pOwner;
			pProjectile->flags = pProjectile->flags & 0xffffff7f;
			pProjectile->flags = pProjectile->flags | 0x20;
			pProjectile->EvaluateDisplayState();
			iVar1 = pProjectile->curBehaviourId;
			if (iVar1 == PROJECTILE_BEHAVIOUR_LAVA_BALL) {
				pProjectile->SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
			}
			else {
				if (iVar1 == PROJECTILE_BEHAVIOUR_STRAIGHT) {
					pProjectile->ClearLocalData();
					pProjectile->SetBehaviour(-1, -1, -1);
					pProjectile->SetBehaviour(PROJECTILE_BEHAVIOUR_STRAIGHT, -1, -1);
				}
				else {
					pProjectile->PreReset();
					pProjectile->Reset();
				}
			}
		}
	}
	else {
		if (((iVar1 == 3) &&
			(pBehaviourStand = reinterpret_cast<CBehaviourProjectileStand*>(GetBehaviour(this->curBehaviourId)), state == 0)) && ((pBehaviourStand->field_0x4 == 3 || (pBehaviourStand->field_0x4 == 9)))) {
			pProjectile = pBehaviourStand->pOwner;

			StopAllFx();

			pProjectile = pBehaviourStand->pOwner;
			pProjectile->flags = pProjectile->flags & 0xffffff7f;
			pProjectile->flags = pProjectile->flags | 0x20;
			pProjectile->EvaluateDisplayState();
			iVar1 = pProjectile->curBehaviourId;
			if (iVar1 == PROJECTILE_BEHAVIOUR_LAVA_BALL) {
				pProjectile->SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
			}
			else {
				if (iVar1 == PROJECTILE_BEHAVIOUR_STRAIGHT) {
					pProjectile->ClearLocalData();
					pProjectile->SetBehaviour(-1, -1, -1);
					pProjectile->SetBehaviour(PROJECTILE_BEHAVIOUR_STRAIGHT, -1, -1);
				}
				else {
					pProjectile->PreReset();
					pProjectile->Reset();
				}
			}
		}
	}

	return;
}

bool CActorProjectile::IsLockable()
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	uint uVar5;
	StateConfig* pSVar6;

	if ((this->aProjectileSubObjs->flags & 0x100) != 0) {
		if ((GetStateFlags(this->actorState) & 0x8000) != 0) {
			if ((GetStateFlags(this->actorState) & 0x100) == 0) {
				if ((GetStateFlags(this->actorState) & 0x1000) == 0) {
					return false;
				}

				if ((this->aProjectileSubObjs->flags & 0x20000) != 0) {
					return false;
				}
			}

			fVar2 = edF32Vector4DotProductHard_I(&CActorHero::_gThis->currentLocation, &this->currentLocation);
			if ((11.56f < fVar2) || ((this->aProjectileSubObjs->flags & 0x2000) != 0)) {
				return true;
			}
		}
	}

	return false;
}

void CActorProjectile::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edANM_HDR* peVar1;
	int* piVar2;
	float fVar3;

	char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
	AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

	if (newAnim == 0x13) {
		fVar3 = this->field_0x3f0;
		if (0.0f <= fVar3) {
			pValue->field_0xc = fVar3;
			pValue->field_0x10 = 1.0f - pValue->field_0xc;
			pValue->field_0x14 = 0.0f;
		}
		else {
			pValue->field_0x14 = -fVar3;
			pValue->field_0x10 = 1.0f - pValue->field_0x14;
			pValue->field_0xc = 0.0f;
		}
	}
	else {
		CActor::AnimEvaluate(layerId, pAnimator, newAnim);
	}

	return;
}

int CActorProjectile::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	uint uVar4;
	StateConfig* pSVar5;
	CLifeInterface* pCVar6;
	Timer* pTVar7;
	int iVar8;
	float fVar9;
	undefined4 unaff_f20;
	undefined4 uVar10;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if (msg == MESSAGE_SOCCER_START) {
		MessageSoccerParamsDetailed* pSoccerMsgParam = reinterpret_cast<MessageSoccerParamsDetailed*>(pMsgParam);

		if ((GetStateFlags(this->actorState) & 0x800) != 0) {
			if (((GetStateFlags(this->actorState) & 0x2000) != 0) && ((this->aProjectileSubObjs->flags & 0x10) != 0)) {
				if (pSoccerMsgParam->field_0x0 == 0) {
					this->rotationEuler.y = pSoccerMsgParam->rotation;
					SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
					this->dynamic.rotationQuat = this->rotationQuat;
					this->dynamic.speed = pSoccerMsgParam->speed;

					if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
						SetState(PROJECTILE_STATE_FLYING, -1);
					}
					else {
						if (this->currentAnimType == 10) {
							SetState(PROJECTILE_STATE_AUT_ROLL_ON_GROUND, 0);
						}
						else {
							SetState(PROJECTILE_STATE_AUT_ROLL_ON_GROUND, -1);
						}
					}
				}
				else {
					this->pSoccerActor = static_cast<CActorMovable*>(pSender);
					SetState(PROJECTILE_STATE_SOCCER, -1);
				}

				return 1;
			}
		}

		if (((((GetStateFlags(this->actorState) & 0x800) != 0) && ((this->aProjectileSubObjs->flags & 0x10) != 0)) &&
			(this->actorState == PROJECTILE_STATE_AUT_KICKED)) && (pSoccerMsgParam->field_0x0 == 0)) {
			this->field_0x404 = this->field_0x404 + GetTimer()->cutsceneDeltaTime;

			return 1;
		}

		return 0;
	}

	if (msg != 0x22) {
		if (msg == 3) {
			if ((GetStateFlags(this->actorState) & 0x800) != 0) {
				SetState(PROJECTILE_STATE_DIE, -1);
				return 1;
			}
		}
		else {
			if (msg == 2) {
				_msg_hit_param* pHitMsgParam = (_msg_hit_param*)pMsgParam;

				if ((GetStateFlags(this->actorState) & 0x800) != 0) {
					if (pHitMsgParam->projectileType == HIT_TYPE_KICK) {
						if (((this->aProjectileSubObjs->flags & 0x10) != 0) && (this->actorState == PROJECTILE_STATE_AUT_KICKED)) {
							local_10.x = this->currentLocation.x;
							local_10.z = this->currentLocation.z;
							local_10.w = this->currentLocation.w;
							local_10.y = pHitMsgParam->field_0x40.y;
							UpdatePosition(&local_10, true);
							edF32Vector4ScaleHard(pHitMsgParam->field_0x30, &eStack32, &pHitMsgParam->field_0x20);
							Project(&eStack32, false, (CActor*)0x0);
						}

						return 1;
					}

					if (pHitMsgParam->projectileType == 4) {
						if (((GetStateFlags(this->actorState) & 0x1000) != 0) && ((this->aProjectileSubObjs->flags & 0x20000) != 0)) {
							return 0;
						}
					}

					if ((this->aProjectileSubObjs->flags & 2) != 0) {
						/* WARNING: Load size is inaccurate */
						uVar10 = 1.0f;
						if ((pHitMsgParam->projectileType != 4) && (uVar10 = 1.0f/*unaff_f20*/, pSender->typeID != PROJECTILE)) {
							uVar10 = pHitMsgParam->damage;
						}

						LifeDecrease(uVar10);
					}

					pCVar6 = GetLifeInterface();
					fVar9 = pCVar6->GetValue();

					bVar1 = fVar9 <= 0.0f;
					if (((this->aProjectileSubObjs->flags & 0x80) != 0) && (pSender->typeID == PROJECTILE)) {
						bVar1 = true;
					}

					if (bVar1) {
						SetState(PROJECTILE_STATE_DIE, -1);
						return 1;
					}
				}
			}
			else {
				if (msg == 1) {
					if ((GetStateFlags(this->actorState) & 0x800) != 0) {
						return 1;
					}
				}
				else {
					if (msg == MESSAGE_GET_ACTION) {
						if (((GetStateFlags(this->actorState) & 0x2000) == 0) || ((this->aProjectileSubObjs->flags & 0x10) == 0)) {
							return 0;
						}

						if ((GetStateFlags(this->actorState) & 0x4000) == 0) {
							fVar9 = pSender->SV_GetCosAngle2D(&this->currentLocation);
							if (fVar9 <= 0.64265704f) {
								return 0;
							}

							fVar9 = (pSender->currentLocation).x - this->currentLocation.x;
							fVar2 = (pSender->currentLocation).y - this->currentLocation.y;
							fVar3 = (pSender->currentLocation).z - this->currentLocation.z;
							if (1.0f <= fVar9 * fVar9 + fVar2 * fVar2 + fVar3 * fVar3) {
								return 0;
							}
						}

						return 2;
					}
				}
			}
		}

		iVar8 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
		return iVar8;
	}

	if (pSender->pAnimationController != (CAnimation*)0x0) {
		if (((GetStateFlags(this->actorState) & 0x2000) != 0) && ((this->aProjectileSubObjs->flags & 0x10) != 0)) {
			this->pKickedByActor = static_cast<CActorMovable*>(pSender);
			SetState(PROJECTILE_STATE_AUT_KICKED, -1);
			return 1;
		}
	}

	return 0;
}

int CActorProjectile::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	IMPLEMENTATION_GUARD();
}

void CActorProjectile::ClearLocalData()
{
	StateConfig* pSVar1;
	int iVar2;
	ProjectileSubObj* pPVar3;
	float fVar4;
	CAnimation* pAnimation;

	this->field_0x3f4 = 0;
	this->pFiringActor = (CActor*)0x0;
	this->field_0x40c = (CActor*)0x0;
	this->pKickedByActor = (CActorMovable*)0x0;
	this->pSoccerActor = (CActorMovable*)0x0;
	this->field_0x404 = 0.0f;

	this->aProjectileSubObjs->flags = this->aProjectileSubObjs->flags & 0xff7fffff;

	this->vectorDyn.Reset();

	this->field_0x570 = 0.0f;
	this->field_0x3f0 = 0.0f;
	this->timeToExplode = this->aProjectileSubObjs->timeToExplode;

	if ((this->aProjectileSubObjs->animDuration == 0.0f) && (pAnimation = this->pAnimationController, pAnimation != (CAnimation*)0x0)) {
		pSVar1 = GetStateCfg(PROJECTILE_STATE_DIE);
		iVar2 = GetIdMacroAnim(pSVar1->animId);
		if (iVar2 < 0) {
			fVar4 = 0.0f;
			pPVar3 = this->aProjectileSubObjs;
		}
		else {
			fVar4 = pAnimation->GetAnimLength(iVar2, 0);
			pPVar3 = this->aProjectileSubObjs;
		}

		pPVar3->animDuration = fVar4;
	}

	this->field_0x540 = 0;
	this->field_0x5a8 = 0;

	return;
}

void CActorProjectile::BehaviourProjectile_InitState(int newState)
{
	int* piVar1;
	uint uVar2;
	bool bVar3;
	StateConfig* pStateConfig;
	int iVar4;
	ProjectileSubObj* pSubObj;
	float fVar5;
	CAnimation* pAnimation;
	CCollision* pCol;

	if (newState == 0x12) {
		pCol = this->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefff;
		pCol = this->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7ffff;
	}
	else {
		if (newState == PROJECTILE_STATE_DIE) {
			StopAllFx();

			this->aProjectileSubObjs->flags = this->aProjectileSubObjs->flags & 0xff7fffff;
			pAnimation = this->pAnimationController;
			if ((pAnimation != (CAnimation*)0x0) && (0.0f < this->aProjectileSubObjs->animDuration)) {
				pStateConfig = GetStateCfg(PROJECTILE_STATE_DIE);
				iVar4 = GetIdMacroAnim(pStateConfig->animId);
				if (iVar4 < 0) {
					fVar5 = 0.0f;
					pSubObj = this->aProjectileSubObjs;
				}
				else {
					fVar5 = pAnimation->GetAnimLength(iVar4, 0);
					pSubObj = this->aProjectileSubObjs;
				}

				this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(pSubObj->animDuration / fVar5, 0);
			}

			this->field_0x570 = this->aProjectileSubObjs->animDuration;
		}
		else {
			if (newState == PROJECTILE_STATE_LIVING) {
				if ((this->aProjectileSubObjs->flags & 0x8000) != 0) {
					IMPLEMENTATION_GUARD(
					piVar1 = (int*)this->field_0x358;
					if (((piVar1 == (int*)0x0) || (this->field_0x354 == 0)) || (bVar3 = true, this->field_0x354 != piVar1[6])) {
						bVar3 = false;
					}
					if (((bVar3) && (piVar1 != (int*)0x0)) && ((this->field_0x354 != 0 && (this->field_0x354 == piVar1[6])))) {
						(**(code**)(*piVar1 + 0x24))(&DAT_bf800000);
					})
				}

				if ((this->aProjectileSubObjs->flags & 0x200000) != 0) {
					IMPLEMENTATION_GUARD(
					if (((this->field_0x360 == 0) || (this->field_0x35c == 0)) ||
						(this->field_0x35c != *(int*)(this->field_0x360 + 0x18))) {
						bVar3 = false;
					}
					else {
						bVar3 = true;
					}
					if ((!bVar3) && (uVar2 = this->aProjectileSubObjs->field_0x38, uVar2 != 0xffffffff)) {
						CParticlesManager::GetDynamicFx
						(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x35c, uVar2, 0xffffffffffffffff);
						piVar1 = (int*)this->field_0x360;
						if ((piVar1 == (int*)0x0) || ((this->field_0x35c == 0 || (bVar3 = true, this->field_0x35c != piVar1[6]))))
						{
							bVar3 = false;
						}
						if (bVar3) {
							if (this->pAnimationController == (CAnimation*)0x0) {
								if (((piVar1 != (int*)0x0) && (this->field_0x35c != 0)) && (this->field_0x35c == piVar1[6])) {
									(**(code**)(*piVar1 + 0x38))(piVar1, 0xe, this, 0);
								}
							}
							else {
								if (((piVar1 != (int*)0x0) && (this->field_0x35c != 0)) && (this->field_0x35c == piVar1[6])) {
									(**(code**)(*piVar1 + 0x38))(piVar1, 0xe, this, 0x686365d2);
								}
							}
							piVar1 = (int*)this->field_0x360;
							if (((piVar1 != (int*)0x0) && (this->field_0x35c != 0)) && (this->field_0x35c == piVar1[6])) {
								(**(code**)(*piVar1 + 0x10))(0, 0);
							}
						}
					})
				}

				if (0.0f < this->aProjectileSubObjs->timeToExplode) {
					SV_AUT_WarnActors(this->aProjectileSubObjs->warnRadius, 0.0f, 0);
				}
			}
			else {
				if (newState == 6) {
					pCol = this->pCollisionData;
					pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefff;
					pCol = this->pCollisionData;
					pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7ffff;
					this->flags = this->flags & 0xfffffffd;
					this->flags = this->flags | 1;
					this->flags = this->flags & 0xffffff7f;
					this->flags = this->flags | 0x20;
					EvaluateDisplayState();
				}
			}
		}
	}

	return;
}

int INT_004488f4 = 2;

bool Projectile_CriterionExploder(CActor* pActor, void* pParams)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	bool bVar5;

	CActorProjectile* pProjectile = reinterpret_cast<CActorProjectile*>(pParams);

	iVar4 = INT_004488f4;
	bVar5 = false;
	do {
		if (bVar5) {
			return false;
		}
		if ((pActor != pParams) && (pActor != pProjectile->pFiringActor)) {
			bVar5 = pActor->IsKindOfObject(iVar4);
			if ((bVar5 != false) &&
				(fVar1 = (pActor->currentLocation).x - pProjectile->currentLocation.x,
					fVar2 = (pActor->currentLocation).y - pProjectile->currentLocation.y,
					fVar3 = (pActor->currentLocation).z - pProjectile->currentLocation.z,
					sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) < pProjectile->aProjectileSubObjs->explosionRange)) {
				return true;
			}
		}

		bVar5 = true;
	} while (true);
}

void CActorProjectile::BehaviourProjectile_Manage(CBehaviourProjectileStand* pBehaviourStand)
{
	int iVar1;
	CCollision* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	bool bVar5;
	uint uVar6;
	StateConfig* pSVar7;
	Timer* pTVar8;
	ulong uVar9;
	float fVar10;
	CActorsTable local_120;
	edF32VECTOR4 local_10;

	if ((this->aProjectileSubObjs->flags & 0x100000) != 0) {
		uVar6 = GetStateFlags(this->actorState) & 0x800;

		IMPLEMENTATION_GUARD_LOG(
		if (uVar6 != 0) {
			CPatternPart::FireContinue(&this->patternPart, CActorHero::_gThis, &g_xVector);
		}
		CPatternPart::UpdatePatternPartLife(&this->patternPart);
		CPatternPart::Draw(&this->patternPart);)
	}

	pCVar2 = this->pCollisionData;
	uVar9 = 0x1002023b;
	pCVar2->flags_0x0 = pCVar2->flags_0x0 | 1;
	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar6 = 0;
	}
	else {
		pSVar7 = GetStateCfg(iVar1);
		uVar6 = pSVar7->flags_0x4 & 0x1000;
	}

	if (uVar6 != 0) {
		uVar9 = 0x41920;
	}

	uVar6 = this->aProjectileSubObjs->flags;
	if ((uVar6 & 0x2000) != 0) {
		uVar9 = uVar9 & 0xfffffffffffbe7ff;
	}

	if (((uVar6 & 0x200) != 0) && ((uVar6 & 0x800000) == 0)) {
		uVar9 = uVar9 & 0xffffffffffffffdf;
	}

	if ((uVar6 & 0x400) != 0) {
		uVar9 = uVar9 & 0xfffffffffffffeff;
	}

	iVar1 = this->actorState;
	if (iVar1 == PROJECTILE_STATE_AUT_ROLL_ON_GROUND) {
		StateAutRollOnGround(8.0f, 2.0f, 0.3f, PROJECTILE_STATE_LIVING, PROJECTILE_STATE_FLYING);
	}
	else {
		if (iVar1 == PROJECTILE_STATE_AUT_KICKED) {
			StateAutSoccer(0.0f,  0, 0, this->pKickedByActor);

			if (0.0f < this->field_0x404) {
				fVar10 = this->field_0x404 + GetTimer()->cutsceneDeltaTime;
				this->field_0x404 = fVar10;
				if (0.06f < fVar10) {
					this->dynamic.speed = 0.0f;
					SetState(PROJECTILE_STATE_FLYING_PROJECTED, -1);
				}
			}
		}
		else {
			if (iVar1 == PROJECTILE_STATE_SOCCER) {
				StateAutSoccer(4.0f, 1, 9, this->pSoccerActor);
			}
			else {
				if (iVar1 == 0x12) {
					return;
				}

				if (iVar1 == PROJECTILE_STATE_DYING) {
					IMPLEMENTATION_GUARD(
					CBehaviourProjectileStand::StateDying((long)(int)pBehaviourStand, uVar9);)
				}
				else {
					if (iVar1 == PROJECTILE_STATE_DIE) {
						if (pBehaviourStand->field_0xc == 2) {
							IMPLEMENTATION_GUARD(
							SetState(PROJECTILE_STATE_DYING, -1);
							CBehaviourProjectileStand::StateDying((long)(int)pBehaviourStand, uVar9);)
						}
						else {
							StateDie(uVar9, pBehaviourStand->field_0xc, pBehaviourStand->field_0x4);
						}
					}
					else {
						if (iVar1 == PROJECTILE_STATE_FLYING_DIRECTED) {
							StateFlyingDirected(uVar9, pBehaviourStand->field_0xc);
						}
						else {
							if (iVar1 == PROJECTILE_STATE_FLYING) {
								StateFlying(0.0f, uVar9, -1, pBehaviourStand->field_0xc);
							}
							else {
								if (iVar1 == PROJECTILE_STATE_FLYING_PROJECTED) {
									StateFlying(0.2f, uVar9, PROJECTILE_STATE_FLYING, pBehaviourStand->field_0xc);
								}
								else {
									if (iVar1 == PROJECTILE_STATE_LIVING) {
										StateLiving(uVar9, pBehaviourStand->field_0xc);
									}
									else {
										if (iVar1 == 0x20) {
											IMPLEMENTATION_GUARD(
											StateLiving(this, uVar9, (long)pBehaviourStand->field_0xc);

											if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
												SetState(PROJECTILE_STATE_LIVING, -1);
											})
										}
										else {
											if (iVar1 == PROJECTILE_STATE_FLYING_LAVA_BALL) {
												ManageDyn(4.0f, 0, (CActorsTable*)0x0);
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
	}

	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar6 = 0;
	}
	else {
		pSVar7 = GetStateCfg(iVar1);
		uVar6 = pSVar7->flags_0x4 & 0x800;
	}

	if ((uVar6 == 0) || (this->curBehaviourId != 3)) goto LAB_00208f00;

	bVar5 = false;
	if (0.0f < this->aProjectileSubObjs->timeToExplode) {
		pTVar8 = Timer::GetTimer();
		fVar10 = this->timeToExplode - pTVar8->cutsceneDeltaTime;
		this->timeToExplode = fVar10;
		if (fVar10 <= 0.0f) {
			bVar5 = true;
		}
	}

	if (bVar5) {
	LAB_00208ee0:
		SetState(PROJECTILE_STATE_DIE, -1);
	}
	else {
		bVar5 = false;
		if (0.0f < this->aProjectileSubObjs->explosionRange) {
			local_120.nbEntries = 0;
			local_10.xyz = this->currentLocation.xyz;
			local_10.w = this->aProjectileSubObjs->explosionRange;
			(CScene::ptable.g_ActorManager_004516a4)->cluster.GetActorsIntersectingSphereWithCriterion(&local_120, &local_10, Projectile_CriterionExploder, this);
			if (0 < local_120.nbEntries) {
				bVar5 = true;
			}
		}

		if (bVar5) goto LAB_00208ee0;
	}

	UpdateSmoke();

LAB_00208f00:
	if (this->actorState == 0x1e) {
		SetBehaviour(pBehaviourStand->field_0x4, -1, -1);
	}
	return;
}

void CActorProjectile::BehaviourProjectileStand_InitState(int newState, CBehaviourProjectileStand* pBehaviourStand)
{
	bool bVar1;
	int local_8;
	int* local_4;

	if (newState == PROJECTILE_STATE_DYING) {
		if (this->aProjectileSubObjs->field_0x30 != -1) {
			IMPLEMENTATION_GUARD_LOG(
			local_8 = 0;
			local_4 = (int*)0x0;
			CParticlesManager::GetDynamicFx
			(CScene::ptable.g_EffectsManager_004516b8, &local_8, this->aProjectileSubObjs->field_0x30, 0xffffffffffffffff);

			if (((local_4 == (int*)0x0) || (local_8 == 0)) || (bVar1 = true, local_8 != local_4[6])) {
				bVar1 = false;
			}

			if (bVar1) {
				if (((local_4 != (int*)0x0) && (local_8 != 0)) && (local_8 == local_4[6])) {
					(**(code**)(*local_4 + 0x3c))(local_4, 3, &this->currentLocation);
				}
				if (((local_4 != (int*)0x0) && (local_8 != 0)) && (local_8 == local_4[6])) {
					(**(code**)(*local_4 + 0x10))(0, 0);
				}
			})
		}
	}
	else {
		if (((newState == PROJECTILE_STATE_FLYING_DIRECTED) || (newState == PROJECTILE_STATE_FLYING)) || (newState == PROJECTILE_STATE_FLYING_PROJECTED)) {
			if (this->pFiringActor != (CActor*)0x0) {
				(this->pCollisionData)->actorFieldA = this->pFiringActor;
				this->pFiringActor->pCollisionData->actorFieldA = this;
			}
		}
		else {
			pBehaviourStand->pOwner->BehaviourProjectile_InitState(newState);
		}
	}

	return;
}

void CActorProjectile::BehaviourProjectileStand_TermState(int oldState, int newState, CBehaviourProjectileStand* pBehaviourStand)
{
	CActorProjectile* pProjectile;
	CCollision* pCVar1;
	CAnimation* this_01;
	int iVar2;
	int iVar3;

	if (oldState == PROJECTILE_STATE_DYING) {
		iVar3 = 0;
		iVar2 = 0;
		do {
			IMPLEMENTATION_GUARD_LOG(
			FUN_001d64b0((int)&pBehaviourStand->aFxSparks->field_0x0 + iVar2);
			iVar3 = iVar3 + 1;
			iVar2 = iVar2 + 0x390;)
		} while (iVar3 < 4);
	}
	else {
		if ((oldState == PROJECTILE_STATE_FLYING_DIRECTED) || (oldState == PROJECTILE_STATE_FLYING)) {
			(this->pCollisionData)->actorFieldA = (CActor*)0x0;

			if (this->pFiringActor != (CActor*)0x0) {
				this->pFiringActor->pCollisionData->actorFieldA = (CActor*)0x0;
			}
		}
		else {
			if (oldState == PROJECTILE_STATE_FLYING_PROJECTED) {
				this->field_0x404 = 0.0f;

				if ((newState != PROJECTILE_STATE_FLYING) && (newState != PROJECTILE_STATE_FLYING_DIRECTED)) {
					(this->pCollisionData)->actorFieldA = (CActor*)0x0;

					if (this->pFiringActor != (CActor*)0x0) {
						this->pFiringActor->pCollisionData->actorFieldA = (CActor*)0x0;
					}
				}
			}
			else {
				pProjectile = pBehaviourStand->pOwner;
				if (oldState == PROJECTILE_STATE_DIE) {
					this_01 = pProjectile->pAnimationController;
					if (this_01 != (CAnimation*)0x0) {
						this_01->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
					}

					pProjectile->field_0x5a8 = 0;
				}
				else {
					if (oldState == 6) {
						pCVar1 = pProjectile->pCollisionData;
						pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
						pCVar1 = pProjectile->pCollisionData;
						pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;
						pProjectile->timeToExplode = (((ed_g3d_hierarchy*)pProjectile->aProjectileSubObjs)->transformA).aa;
						pProjectile->flags = pProjectile->flags & 0xfffffffc;
						pProjectile->flags = pProjectile->flags & 0xffffff5f;
						pProjectile->EvaluateDisplayState();
					}
				}
			}
		}
	}

	return;
}

void CActorProjectile::AppearToDie()
{
	this->flags = this->flags & 0xffffff5f;
	EvaluateDisplayState();
	this->flags = this->flags & 0xfffffffc;
	SetState(PROJECTILE_STATE_LIVING, -1);

	this->timeToExplode = this->aProjectileSubObjs->timeToExplode;

	return;
}

void CActorProjectile::GoToSleep()
{
	int behaviourId;

	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	behaviourId = this->curBehaviourId;
	if (behaviourId == PROJECTILE_BEHAVIOUR_LAVA_BALL) {
		SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
	}
	else {
		if (behaviourId == PROJECTILE_BEHAVIOUR_STRAIGHT) {
			ClearLocalData();
			SetBehaviour(-1, -1, -1);
			SetBehaviour(PROJECTILE_BEHAVIOUR_STRAIGHT, -1, -1);
		}
		else {
			PreReset();
			Reset();
		}
	}

	return;
}

void CActorProjectile::Die()
{
	this->SetState(PROJECTILE_STATE_DIE, -1);

	return;
}

void CActorProjectile::UpdateSmoke()
{
	ProjectileSubObj* pProjectileSubObj;
	CNewFx* pFx;
	int iVar3;
	bool bVar4;

	if (0.0 < this->timeToExplode) {
		if (((this->aProjectileSubObjs->flags & 0x8000) == 0) && (this->timeToExplode <= this->aProjectileSubObjs->field_0x44)) {
			pFx = (this->field_0x354).pFx;

			if ((pFx == (CNewFx*)0x0) ||
				((iVar3 = (this->field_0x354).id, iVar3 == 0 || (bVar4 = true, iVar3 != pFx->id)))) {
				bVar4 = false;
			}

			if ((((bVar4) && (pFx != (CNewFx*)0x0)) && (iVar3 = (this->field_0x354).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
				pFx->Stop(-1.0f);
			}
		}

		pProjectileSubObj = this->aProjectileSubObjs;
		if (((pProjectileSubObj->flags & 0x200000) == 0) && (this->timeToExplode <= pProjectileSubObj->field_0x40)) {
			pFx = (this->field_0x35c).pFx;
			if ((pFx == (CNewFx*)0x0) || ((iVar3 = (this->field_0x35c).id, iVar3 == 0 || (iVar3 != pFx->id)))) {
				bVar4 = false;
			}
			else {
				bVar4 = true;
			}

			if ((!bVar4) && (pProjectileSubObj->field_0x38 != 0xffffffff)) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x35c, pProjectileSubObj->field_0x38, FX_MATERIAL_SELECTOR_NONE);
				pFx = (this->field_0x35c).pFx;
				if ((pFx == (CNewFx*)0x0) || ((iVar3 = (this->field_0x35c).id, iVar3 == 0 || (bVar4 = true, iVar3 != pFx->id)))) {
					bVar4 = false;
				}

				if (bVar4) {
					if (this->pAnimationController == (CAnimation*)0x0) {
						if (((pFx != (CNewFx*)0x0) && (iVar3 = (this->field_0x35c).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
							pFx->SpatializeOnActor(0xe, (CActor*)this, 0);
						}
					}
					else {
						if (((pFx != (CNewFx*)0x0) && (iVar3 = (this->field_0x35c).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
							pFx->SpatializeOnActor(0xe, (CActor*)this, 0x686365d2);
						}
					}

					pFx = (this->field_0x35c).pFx;
					if (((pFx != (CNewFx*)0x0) && (iVar3 = (this->field_0x35c).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
						pFx->Start(0, 0);
					}
				}
			}
		}
	}

	return;
}

void CActorProjectile::Project(edF32VECTOR4* pDestination, bool bShowFx, CActor* pFiringActor)
{
	edF32VECTOR4* v0;
	edF32VECTOR4 newRotation;

	this->pFiringActor = pFiringActor;
	this->dynamic.speed = 0.0f;
	v0 = this->dynamicExt.aImpulseVelocities;
	edF32Vector4AddHard(v0, v0, pDestination);
	this->dynamicExt.aImpulseVelocityMagnitudes[0] = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);

	SetState(PROJECTILE_STATE_FLYING_PROJECTED, -1);

	if ((this->aProjectileSubObjs->flags & 0x2000) != 0) {
		edF32Vector4NormalizeHard(&newRotation, pDestination);
		this->rotationQuat = newRotation;
	}

	if (bShowFx != false) {
		this->timeToExplode = this->aProjectileSubObjs->timeToExplode;
		ShowFx();
	}

	return;
}



void CActorProjectile::Project(float velocity, edF32VECTOR4* pDirection, bool bShowFx, CActor* pFiringActor)
{
	edF32VECTOR4 eStack16;

	edF32Vector4ScaleHard(velocity, &eStack16, pDirection);
	Project(&eStack16, bShowFx, pFiringActor);

	return;
}

void CActorProjectile::ProjectDirected(float velocity, edF32VECTOR4* pSource, edF32VECTOR4* pTarget, bool bShowFx, CActor* pFiringActor)
{
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	this->pFiringActor = pFiringActor;

	edF32Vector4SubHard(&eStack16, pTarget, pSource);
	edF32Vector4ScaleHard(2.0f, &eStack32, &CDynamicExt::gForceGravity);
	this->vectorDyn.BuildFromAccelDistAmplitude(velocity, &CDynamicExt::gForceGravity, &eStack16, 1);

	SetState(PROJECTILE_STATE_FLYING_DIRECTED, -1);

	if (bShowFx != false) {
		this->timeToExplode = this->aProjectileSubObjs->timeToExplode;
		ShowFx();
	}

	return;
}

void CActorProjectile::StateLiving(uint param_2, int param_3)
{
	CCollision* pCVar1;
	CActor* pHitActor;
	uint uVar3;
	float fVar4;
	edF32VECTOR4 local_120;
	CActorsTable local_110;

	local_110.nbEntries = 0;

	this->dynamic.speed = 0.0f;
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	if ((this->aProjectileSubObjs->flags & 0x2000) == 0) {
		uVar3 = 0x100a023b;
	}
	else {
		pCVar1 = this->pCollisionData;
		uVar3 = 0;
		pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfffffffe;
	}

	if ((this->aProjectileSubObjs->flags & 0x4000) == 0) {
		ManageDyn(4.0f, uVar3, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, uVar3, &local_110);
	}

	if (0.0f < this->aProjectileSubObjs->timeToExplode) {
		SV_AUT_WarnActors(this->aProjectileSubObjs->warnRadius, 0.0f, (CActor*)0x0);
	}

	if ((((this->pCollisionData)->flags_0x4 & 7) == 0) && ((this->aProjectileSubObjs->flags & 0x2000) == 0)) {
		if (0.2f < this->timeInAir) {
			SetState(PROJECTILE_STATE_FLYING, -1);
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	if ((this->aProjectileSubObjs->flags & 0x4000) != 0) {
		while (local_110.nbEntries != 0) {
			pHitActor = local_110.PopCurrent();
			if ((pHitActor != this->pFiringActor) && (pHitActor->typeID != PROJECTILE)) {
				local_120.xyz = this->currentLocation.xyz;
				local_120.w = 3.402823e+38f;
				HitActor(&local_120, pHitActor, param_3, 0);
				LifeDecrease(1.0f);
			}
		}

		if (GetLifeInterface()->GetValue() <= 0.0f) {
			SetState(PROJECTILE_STATE_DIE, -1);
		}
	}

	return;
}

void CActorProjectile::StateFlying(float param_1, uint dynFlags, int nextState, int param_5)
{
	CCollision* pCVar1;
	bool bVar2;
	CActor* pHitActor;
	uint uVar3;
	float fVar4;
	float value;
	edF32VECTOR4 local_160;
	edF32VECTOR4 eStack336;
	edF32VECTOR4 local_140;
	edF32VECTOR4 local_130;
	edF32VECTOR4 local_120;
	CActorsTable local_110;

	pCVar1 = this->pCollisionData;
	value = fabs(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y);
	fVar4 = edFIntervalUnitDstLERP(value, 10.0f, 0.0f);
	this->field_0x3f0 = fVar4;
	if ((this->aProjectileSubObjs->flags & 0x4800) == 0) {
		ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);
	}
	else {
		local_110.nbEntries = 0;
		ManageDyn(4.0f, dynFlags, &local_110);

		while (local_110.nbEntries != 0) {
			pHitActor = local_110.PopCurrent();
			if (pHitActor != this->pFiringActor) {
				uVar3 = this->aProjectileSubObjs->flags;
				if ((uVar3 & 0x4000) != 0) {
					local_120.xyz = this->currentLocation.xyz;
					local_120.w = 3.402823e+38f;
					if (pHitActor->typeID == PROJECTILE) {
						if ((this->aProjectileSubObjs->flags & 0x40000) != 0) {
							pHitActor->SetState(PROJECTILE_STATE_DIE, -1);
						}
					}
					else {
						HitActor(&local_120, pHitActor, param_5, 0);

						this->dynamic.speed = 0.0f;

						uVar3 = this->aProjectileSubObjs->flags;
						if ((uVar3 & 0x2000) != 0) {
							this->aProjectileSubObjs->flags = uVar3 | 0x800000;
						}
					}

					this->dynamic.speed = 0.0f;
					SetState(PROJECTILE_STATE_DIE, -1);

					return;
				}

				if ((uVar3 & 0x800) != 0) {
					this->dynamic.speed = 0.0f;
					SetState(PROJECTILE_STATE_DIE, -1);
					return;
				}
			}
		}
	}

	if ((this->aProjectileSubObjs->flags & 0x20) != 0) {
		edF32Vector4NormalizeHard(&local_130, &this->dynamic.velocityDirectionEuler);
		this->rotationQuat = local_130;
	}

	if ((pCVar1->flags_0x4 & 7) != 0) {
		uVar3 = this->aProjectileSubObjs->flags;
		if ((uVar3 & 4) != 0) {
			this->dynamic.speed = 0.0f;
			SetState(PROJECTILE_STATE_DIE, -1);

			return;
		}

		if ((uVar3 & 0x2000) != 0) {
			local_140 = gF32Vector4Zero;

			bVar2 = (pCVar1->flags_0x4 & 2) != 0;
			if (bVar2) {
				edF32Vector4AddHard(&local_140, &local_140, &pCVar1->aCollisionContact[1].field_0x10);
			}

			uVar3 = (uint)bVar2;
			if ((pCVar1->flags_0x4 & 1) != 0) {
				edF32Vector4AddHard(&local_140, &local_140, &pCVar1->aCollisionContact[0].field_0x10);
				uVar3 = uVar3 + 1;
			}

			if ((pCVar1->flags_0x4 & 4) != 0) {
				edF32Vector4AddHard(&local_140, &local_140, &pCVar1->aCollisionContact[2].field_0x10);
				uVar3 = uVar3 + 1;
			}

			if (uVar3 != 0) {
				edF32Vector4ScaleHard(1.0f / (float)uVar3, &local_140, &local_140);
			}

			edF32Vector4SubHard(&eStack336, &this->currentLocation, &local_140);
			edF32Vector4ScaleHard(0.175f, &eStack336, &eStack336);
			edF32Vector4AddHard(&local_140, &local_140, &eStack336);
			this->dynamic.speed = 0.0f;
			UpdatePosition(&local_140, true);
			local_160.x = this->rotationQuat.x;
			local_160.z = this->rotationQuat.z;
			local_160.w = this->rotationQuat.w;
			local_160.y = 0.0f;

			edF32Vector4NormalizeHard(&local_160, &local_160);

			this->rotationQuat = local_160;
			SetState(0x20, -1);

			return;
		}

		if ((pCVar1->flags_0x4 & 2) != 0) {
			if (6.0f < value) {
				fVar4 = edFIntervalUnitDstLERP(value, 0.0f, 10.0f);
				this->field_0x3f0 = -fVar4 * 0.5f;
			}
			else {
				PlayAnim(0);
			}

			if (value < 1.0f) {
				this->field_0x3f0 = 0.0f;
				this->dynamic.speed = 0.0f;

				if ((this->aProjectileSubObjs->flags & 1) != 0) {
					SetState(PROJECTILE_STATE_DIE, -1);
					return;
				}

				SetState(PROJECTILE_STATE_LIVING, -1);

				return;
			}
		}
	}

	if ((nextState != -1) && (param_1 < this->timeInAir)) {
		SetState(nextState, -1);
	}

	return;
}

void CActorProjectile::StateFlyingDirected(ulong flags, int param_3)
{
	bool bVar1;
	Timer* pTVar2;
	CActor* pCVar3;
	uint dynFlags;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_130;
	edF32VECTOR4 local_120;
	CActorsTable local_110;

	pTVar2 = GetTimer();
	this->vectorDyn.Integrate(pTVar2->cutsceneDeltaTime);

	dynFlags = 0x4392b;
	if ((this->aProjectileSubObjs->flags & 0x2000) != 0) {
		dynFlags = 0x212b;
	}

	bVar1 = this->vectorDyn.IsFinished();
	if (bVar1 == false) {
		fVar5 = (this->vectorDyn).field_0x60.y;
		fVar6 = (this->vectorDyn).field_0x60.z;
		dynFlags = dynFlags | 0x1c000;

		this->dynamicExt.field_0x60.x = (this->vectorDyn).field_0x60.x;
		this->dynamicExt.field_0x60.y = fVar5;
		this->dynamicExt.field_0x60.z = fVar6;
		this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x1c000;
	}
	else {
		this->dynamic.speed = 0.0f;
	}

	if ((this->aProjectileSubObjs->flags & 0x4800) == 0) {
		ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);
	LAB_002061f0:
		if ((this->aProjectileSubObjs->flags & 0x20) != 0) {
			edF32Vector4NormalizeHard(&local_130, &this->dynamic.velocityDirectionEuler);
			this->rotationQuat = local_130;
		}

		if (((this->pCollisionData)->flags_0x4 & 2) != 0) {
			this->dynamic.speed = 0.0f;

			if ((this->aProjectileSubObjs->flags & 4) == 0) {
				SetState(PROJECTILE_STATE_LIVING, -1);
			}
			else {
				SetState(PROJECTILE_STATE_DIE, -1);
			}
		}
	}
	else {
		local_110.nbEntries = 0;
		ManageDyn(4.0f, dynFlags, &local_110);

		bVar1 = false;
		do {
			do {
				if (local_110.nbEntries == 0) goto LAB_002061f0;
				pCVar3 = local_110.PopCurrent();
			} while (pCVar3 == this->pFiringActor);

			if ((this->aProjectileSubObjs->flags & 0x4000) != 0) {
				local_120.xyz = this->currentLocation.xyz;
				local_120.w = 3.402823e+38f;
				HitActor(&local_120, pCVar3, param_3, 0);

				if (!bVar1) {
					this->dynamic.speed = 0.0;
					dynFlags = this->aProjectileSubObjs->flags;
					bVar1 = true;

					if ((dynFlags & 0x2000) != 0) {
						this->aProjectileSubObjs->flags = dynFlags | 0x800000;
						bVar1 = true;
					}
				}
			}
		} while ((this->aProjectileSubObjs->flags & 0x800) == 0);

		this->dynamic.speed = 0.0f;
		SetState(PROJECTILE_STATE_DIE, -1);
	}

	return;
}

bool CActorProjectile_HitActorCriterion(CActor* pActor, void* pParams)

{
	return CActorFactory::gClassProperties[pActor->typeID].flags & 0x10;
}

void CActorProjectile::StateDie(uint dynFlags, int param_3, int param_4)
{
	ProjectileSubObj* pPVar1;
	bool bVar2;
	Timer* pTVar3;
	CActorsTable* pCVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float radius;
	float fVar10;
	CActorsTable local_130;
	edF32VECTOR4 local_20;
	int local_10;
	int* local_c;
	int local_8;
	int* local_4;

	ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);

	if (((this->aProjectileSubObjs->flags & 8) != 0) && ((this->field_0x3f4 & 1) == 0)) {
		if (this->aProjectileSubObjs->animDuration == 0.0f) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
		}

		if (this->field_0x540 == 0) {
			if ((this->aProjectileSubObjs->field_0x30 != -1) && (this->field_0x5a8 == 0)) {
				IMPLEMENTATION_GUARD_LOG(
				local_10 = 0;
				local_c = (int*)0x0;
				CParticlesManager::GetDynamicFx
				(CScene::ptable.g_EffectsManager_004516b8, &local_10, this->aProjectileSubObjs->field_0x30, 0xffffffffffffffff);
				if ((local_c == (int*)0x0) || ((local_10 == 0 || (bVar2 = true, local_10 != local_c[6])))) {
					bVar2 = false;
				}
				if (bVar2) {
					if (((local_c != (int*)0x0) && (local_10 != 0)) && (local_10 == local_c[6])) {
						(**(code**)(*local_c + 0x3c))();
					}
					if (((local_c != (int*)0x0) && (local_10 != 0)) && (local_10 == local_c[6])) {
						(**(code**)(*local_c + 0x10))(0, 0);
					}
					this->field_0x5a8 = 1;
				})
			}
		}
		else {
			if ((this->aProjectileSubObjs->field_0x34 != -1) && (this->field_0x5a8 == 0)) {
				IMPLEMENTATION_GUARD_LOG(
				local_8 = 0;
				local_4 = (int*)0x0;
				CParticlesManager::GetDynamicFx
				(CScene::ptable.g_EffectsManager_004516b8, &local_8, this->aProjectileSubObjs->field_0x34,
					0xffffffffffffffff);
				if ((local_4 == (int*)0x0) || ((local_8 == 0 || (bVar2 = true, local_8 != local_4[6])))) {
					bVar2 = false;
				}
				if (bVar2) {
					if (((local_4 != (int*)0x0) && (local_8 != 0)) && (local_8 == local_4[6])) {
						local_4[0x14] = *(int*)&this->field_0x560;
						local_4[0x15] = *(int*)&this->field_0x564;
						local_4[0x16] = *(int*)&this->field_0x568;
						local_4[0x17] = *(int*)&this->field_0x56c;
					}
					if (((local_4 != (int*)0x0) && (local_8 != 0)) && (local_8 == local_4[6])) {
						(**(code**)(*local_4 + 0x3c))();
					}
					if (((local_4 != (int*)0x0) && (local_8 != 0)) && (local_8 == local_4[6])) {
						(**(code**)(*local_4 + 0x10))(0, 0);
					}
					this->field_0x540 = 0;
					this->field_0x5a8 = 1;
				})
			}
		}

		if ((this->aProjectileSubObjs->flags & 0x10000) == 0) {
			if (((param_3 == 3) || (param_3 == 1)) || (param_3 == 0)) {
				this->field_0x3f4 = this->field_0x3f4 | 1;
			}

			local_20.xyz = this->currentLocation.xyz;
			local_20.w = this->aProjectileSubObjs->hitRadius;

			if ((this->actorFieldS & 8) == 0) {
				local_130.nbEntries = 0;
				(CScene::ptable.g_ActorManager_004516a4)->cluster.GetActorsIntersectingSphereWithCriterion(&local_130, &local_20,
					CActorProjectile_HitActorCriterion, (void*)0x0);

				for (int i = 0; i < local_130.nbEntries; i++) {
					HitActor(&local_20, local_130.aEntries[i], param_3, 1);
				}
			}
			else {
				HitActor(&local_20, CActorHero::_gThis, param_3, 1);
			}
		}
	}

	fVar10 = this->field_0x570;
	pTVar3 = Timer::GetTimer();
	this->field_0x570 = this->field_0x570 - pTVar3->cutsceneDeltaTime;

	pPVar1 = this->aProjectileSubObjs;
	if ((pPVar1->flags & 0x10000) == 0) {
		fVar9 = pPVar1->hitRadius;
		radius = this->aProjectileSubObjs->field_0x1c;

		if (fVar9 < radius) {
			fVar8 = pPVar1->animDuration;

			if (fVar8 != 0.0f) {
				fVar6 = this->field_0x570;
				if (fVar6 < 0.0f) {
					fVar6 = 0.0f;
				}

				fVar9 = pPVar1->hitRadius;
				fVar7 = radius - fVar9;
				radius = fVar9 + (1.0f - fVar6 / fVar8) * fVar7;
				fVar9 = fVar9 + (1.0f - fVar10 / fVar8) * fVar7;
			}

			SV_AUT_WarnActors(radius, fVar9, 0);
		}
	}

	if (this->field_0x570 <= 0.0f) {
		if (param_3 == 2) {
			this->field_0x3f4 = this->field_0x3f4 | 1;
		}

		if (param_4 == this->curBehaviourId) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();

			iVar5 = this->curBehaviourId;
			if (iVar5 == PROJECTILE_BEHAVIOUR_LAVA_BALL) {
				SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
			}
			else {
				if (iVar5 == PROJECTILE_BEHAVIOUR_STRAIGHT) {
					ClearLocalData();
					SetBehaviour(-1, -1, -1);
					SetBehaviour(4, -1, -1);
				}
				else {
					PreReset();
					Reset();
				}
			}
		}
		else {
			SetState(0x1e, -1);
		}
	}

	return;
}


void CActorProjectile::HitActor(edF32VECTOR4* pSphere, CActor* pHitActor, int explode, int param_5)
{
	CCollision* pCVar1;
	float fVar2;
	bool bVar3;
	edF32VECTOR4* peVar4;
	uint hitMsgId;
	float fVar5;
	float fVar6;
	float t;
	float puVar7;
	CCollisionRay CStack256;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 eStack192;
	float local_b0;
	float local_ac;
	float fStack168;
	float fStack164;
	_msg_hit_param auStack160;
	_ray_info_out local_10;

	if (pHitActor != this) {
		pCVar1 = pHitActor->pCollisionData;
		if ((pCVar1 == (CCollision*)0x0) || (pCVar1->pDynCol->nbTriangles != 0)) {
			fVar5 = (pHitActor->currentLocation).x - this->currentLocation.x;
			fVar6 = (pHitActor->currentLocation).y - this->currentLocation.y;
			fVar2 = (pHitActor->currentLocation).z - this->currentLocation.z;
			fVar5 = sqrtf(fVar5 * fVar5 + fVar6 * fVar6 + fVar2 * fVar2);
		}
		else {
			fVar5 = CCollision::GetObbTreeDistanceFromPosition(pCVar1->pObbTree, &this->currentLocation);
		}

		if (fVar5 < pSphere->w) {
			if ((param_5 != 0) && ((this->aProjectileSubObjs->flags & 0x400000) != 0)) {
				SV_GetActorColCenter(&eStack192);
				pHitActor->SV_GetActorColCenter(&eStack208);
				edF32Vector4SubHard(&eStack224, &eStack208, &eStack192);
				fVar6 = edF32Vector4SafeNormalize1Hard(&eStack224, &eStack224);
				CCollisionRay CStack256 = CCollisionRay(fVar6, &eStack192, &eStack224);
				fVar6 = CStack256.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)this, 0x40000048, (edF32VECTOR4*)0x0, &local_10);
				bVar3 = false;
				if ((fVar6 != 1e+30f) && (bVar3 = false, local_10.pActor_0x0 == (CActor*)0x0)) {
					bVar3 = true;
				}

				if (bVar3) {
					return;
				}
			}

			auStack160.flags = 0;
			if (explode == 3) {
				hitMsgId = 99;
			}
			else {
				if (explode == 2) {
					hitMsgId = 2;
					auStack160.projectileType = 5;
				}
				else {
					if (explode == 1) {
						auStack160.flags = 1;
						auStack160.projectileType = 0;
						hitMsgId = 2;
					}
					else {
						if (explode == 0) {
							auStack160.projectileType = 10;
							hitMsgId = 2;
						}
					}
				}
			}

			auStack160.damage = this->aProjectileSubObjs->damage;
			auStack160.field_0x30 = edFIntervalUnitSrcLERP(fVar5 / pSphere->w, this->aProjectileSubObjs->field_0x20, this->aProjectileSubObjs->field_0x24);
			local_b0 = (pHitActor->currentLocation).x;
			local_ac = (pHitActor->currentLocation).y;
			fStack168 = (pHitActor->currentLocation).z;
			fStack164 = (pHitActor->currentLocation).w;
			peVar4 = pHitActor->GetBottomPosition();
			local_ac = peVar4->y - (pHitActor->currentLocation).y / 2.0f;

			edF32Vector4SubHard(&auStack160.field_0x80, &pHitActor->currentLocation, &this->currentLocation);

			auStack160.field_0x80.y = 0.0f;
			auStack160.field_0x80.w = 0.0f;

			fVar5 = edF32Vector4SafeNormalize0Hard(&auStack160.field_0x80, &auStack160.field_0x80);
			t = fVar5 / pSphere->w;
			edF32Vector4ScaleHard(t, &auStack160.field_0x20, &auStack160.field_0x80);
			if (1.0f < t) {
				puVar7 = 1.0f;
			}
			else {
				puVar7 = -1.0f;
				if (-1.0f <= t) {
					puVar7 = t;
				}
			}

			fVar5 = acosf(puVar7);
			auStack160.field_0x20.y = cosf(fVar5);
			edF32Vector4NormalizeHard(&auStack160.field_0x20, &auStack160.field_0x20);
			DoMessage(pHitActor, (ACTOR_MESSAGE)hitMsgId, &auStack160);
		}
	}

	return;
}



float CActorProjectile::GetTimeToExplode()
{
	int iVar1;
	uint uVar3;
	float fVar4;

	fVar4 = 3.402823e+38f;
	if (((GetStateFlags(this->actorState) & 0x800) != 0) && (fVar4 = this->timeToExplode, fVar4 < 0.0f)) {
		fVar4 = 1e-20f;
	}

	return fVar4;
}

void CActorProjectile::ShowFx()
{
	int iVar3;
	bool bVar3;

	uint uVar1 = this->aProjectileSubObjs->field_0x3c;
	if (uVar1 != 0xffffffff) {
		CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x354, uVar1, FX_MATERIAL_SELECTOR_NONE);
		CNewFx* pFx = (this->field_0x354).pFx;
		if (((pFx == (CNewFx*)0x0) || (iVar3 = (this->field_0x354).id, iVar3 == 0)) || (bVar3 = true, iVar3 != pFx->id)) {
			bVar3 = false;
		}

		if (bVar3) {
			if (this->pAnimationController == (CAnimation*)0x0) {
				if (((pFx != (CNewFx*)0x0) && (iVar3 = (this->field_0x354).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
					pFx->SpatializeOnActor(0xe, this, 0);
				}
			}
			else {
				if (((pFx != (CNewFx*)0x0) && (iVar3 = (this->field_0x354).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
					pFx->SpatializeOnActor(0xe, this, 0x686365d2);
				}
			}

			pFx = (this->field_0x354).pFx;
			if (((pFx != (CNewFx*)0x0) && (iVar3 = (this->field_0x354).id, iVar3 != 0)) && (iVar3 == pFx->id)) {
				pFx->Start(0, 0);
			}
		}
	}

	return;
}

void CActorProjectile::StopAllFx()
{
	int iVar4;

	CNewFx* pCVar1 = (this->field_0x354).pFx;
	if (((pCVar1 != (CNewFx*)0x0) && (iVar4 = (this->field_0x354).id, iVar4 != 0)) && (iVar4 == pCVar1->id)) {
		pCVar1->Stop(-1.0f);
	}

	pCVar1 = (this->field_0x35c).pFx;
	if (((pCVar1 != (CNewFx*)0x0) && (iVar4 = (this->field_0x35c).id, iVar4 != 0)) && (iVar4 == pCVar1->id)) {
		pCVar1->Stop(-1.0f);
	}

	return;
}

void CBehaviourProjectile::Create(ByteCode* pByteCode)
{
	this->field_0x4 = pByteCode->GetS32();
	return;
}

void CBehaviourProjectile::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorProjectile*>(pOwner);
	return;
}

void CBehaviourProjectile::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorProjectile* pProjectile;

	if (newState == -1) {
		pProjectile = this->pOwner;
		if ((pProjectile->prevBehaviourId == PROJECTILE_BEHAVIOUR_INVENTORY) || (pProjectile->aProjectileSubObjs->timeToExplode == 0.0f)) {
			pProjectile->SetState(PROJECTILE_STATE_LIVING, -1);
		}
		else {
			pProjectile->SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourProjectile::InitState(int newState)
{
	this->pOwner->BehaviourProjectile_InitState(newState);
	return;
}

void CBehaviourProjectile::TermState(int oldState, int newState)
{
	CAnimation* pAnimation;
	CCollision* pCol;
	CActorProjectile* pProjectile;

	pProjectile = this->pOwner;

	if (oldState == PROJECTILE_STATE_DIE) {
		pAnimation = pProjectile->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		}

		pProjectile->field_0x5a8 = 0;
	}
	else {
		if (oldState == PROJECTILE_STATE_FLYING_LAVA_BALL) {
			pCol = pProjectile->pCollisionData;
			pCol->flags_0x0 = pCol->flags_0x0 | 0x1000;
			pCol = pProjectile->pCollisionData;
			pCol->flags_0x0 = pCol->flags_0x0 | 0x80000;

			pProjectile->timeToExplode = pProjectile->aProjectileSubObjs->timeToExplode;

			pProjectile->flags = pProjectile->flags & 0xfffffffc;
			pProjectile->flags = pProjectile->flags & 0xffffff5f;

			pProjectile->EvaluateDisplayState();
		}
	}

	return;
}

void CBehaviourProjectileStand::Create(ByteCode* pByteCode)
{
	int iVar3;
	CFxSparkNoAlloc<3, 12>* pAVar4;

	this->field_0x4 = pByteCode->GetS32();
	this->field_0xc = pByteCode->GetS32();
	this->materialId = pByteCode->GetS32();
	if (this->field_0xc == 2) {
		CActor::SV_InstallMaterialId(this->materialId);
		this->aFxSparks = new CFxSparkNoAlloc<3, 12>[4];
		iVar3 = 0;
		do {
			pAVar4 = this->aFxSparks + iVar3;

			pAVar4->Create(3, 0xc, pAVar4->aVectorData, pAVar4->aFloatData, this->materialId);
			pAVar4->field_0xe4 = pAVar4->aUnknown;
			pAVar4->SetParameters(0.5f, 0.025f, 10.0f, 0.125f, 2.0f, 3);
			iVar3 = iVar3 + 1;
		} while (iVar3 < 4);
	}
	else {
		this->aFxSparks = (CFxSparkNoAlloc<3, 12> *)0x0;
	}

	return;
}

void CBehaviourProjectileStand::Init(CActor* pOwner)
{
	int iVar2;

	this->pOwner = reinterpret_cast<CActorProjectile*>(pOwner);

	if (this->field_0xc == 2) {
		iVar2 = 0;
		do {
			this->aFxSparks[iVar2].Init(this->pOwner->objectId);
			this->aFxSparks[iVar2].field_0x90 = 0x80f09614;
			iVar2 = iVar2 + 1;
		} while (iVar2 < 4);
	}

	return;
}

void CBehaviourProjectileStand::Term()
{
	if (this->field_0xc == 2) {
		delete[] this->aFxSparks;
		this->aFxSparks = (CFxSparkNoAlloc<3, 12> *)0x0;
	}

	return;
}

void CBehaviourProjectileStand::Manage()
{
	this->pOwner->BehaviourProjectile_Manage(this);
}

void CBehaviourProjectileStand::Draw()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourProjectileStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorProjectile* pProjectile;

	this->pOwner->ClearLocalData();

	if (newState == -1) {
		pProjectile = this->pOwner;
		if ((pProjectile->prevBehaviourId == PROJECTILE_BEHAVIOUR_INVENTORY) || (pProjectile->aProjectileSubObjs->timeToExplode == 0.0f)) {
			pProjectile->SetState(PROJECTILE_STATE_LIVING, -1);
		}
		else {
			pProjectile->SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	pProjectile = this->pOwner;
	if ((pProjectile->prevBehaviourId == PROJECTILE_BEHAVIOUR_INACTIVE) && (newState == -1)) {
		pProjectile->SetState(PROJECTILE_STATE_LIVING, -1);
	}

	this->pOwner->field_0x404 = 0.0f;

	return;
}

void CBehaviourProjectileStand::InitState(int newState)
{
	this->pOwner->BehaviourProjectileStand_InitState(newState, this);
	return;
}

void CBehaviourProjectileStand::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourProjectileStand_TermState(oldState, newState, this);
}

void CBehaviourProjectileStraight::Create(ByteCode* pByteCode)
{
	this->field_0x4 = pByteCode->GetS32();
	this->field_0xc = pByteCode->GetS32();
	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x18 = pByteCode->GetF32();
	return;
}

void CBehaviourProjectileStraight::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorProjectile*>(pOwner);

	if ((this->pOwner->aProjectileSubObjs->flags & 0x100000) != 0) {
		this->pOwner->patternPart.FUN_003a7cc0(this->field_0x10);
	}

	return;
}

void CBehaviourProjectileStraight::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourProjectileStraight::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourProjectileStraight::InitState(int newState)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourProjectileStraight::TermState(int oldState, int newState)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourProjectileExcuse::Create(ByteCode* pByteCode)
{
	this->field_0x4 = pByteCode->GetS32();
	this->field_0xc = pByteCode->GetU32();
	this->actorRef.index = pByteCode->GetS32();

	return;
}

void CBehaviourProjectileExcuse::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorProjectile*>(pOwner);
	this->actorRef.Init();

	return;
}

void CBehaviourProjectileExcuse::Manage()
{
	IMPLEMENTATION_GUARD();

}

void CBehaviourProjectileExcuse::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();

}

void CBehaviourProjectilePortable::Create(ByteCode* pByteCode)
{
	this->field_0x4 = pByteCode->GetS32();
	this->field_0xc = pByteCode->GetU32();
	this->field_0x10 = pByteCode->GetF32();

	return;
}

void CBehaviourProjectilePortable::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorProjectile*>(pOwner);

	return;
}

void CBehaviourProjectilePortable::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourProjectilePortable::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorProjectile* pProjectile;

	if (newState == -1) {
		pProjectile = this->pOwner;
		if ((pProjectile->prevBehaviourId == PROJECTILE_BEHAVIOUR_INVENTORY) || (pProjectile->aProjectileSubObjs->timeToExplode == 0.0f)) {
			pProjectile->SetState(PROJECTILE_STATE_LIVING, -1);
		}
		else {
			pProjectile->SetState(PROJECTILE_STATE_FLYING_LAVA_BALL, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	if (newState == -1) {
		pProjectile = this->pOwner;
		if (pProjectile->actorState != PROJECTILE_STATE_LIVING) {
			pProjectile->SetState(PROJECTILE_STATE_LIVING, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourProjectilePortable::InitState(int newState)
{
	if (newState == 0x13) {
		(this->pOwner->pCollisionData)->actorFieldA = this->field_0x14;
		this->field_0x18 = 0;
	}
	else {
		this->pOwner->BehaviourProjectile_InitState(newState);
	}

	return;
}

void CBehaviourProjectilePortable::TermState(int oldState, int newState)
{
	IMPLEMENTATION_GUARD();
}
