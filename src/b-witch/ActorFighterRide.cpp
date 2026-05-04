#include "ActorFighter.h"
#include "TimeController.h"
#include "MathOps.h"

void CBehaviourFighterRidden::Init(CActor* pOwner)
{
	return;
}

void CBehaviourFighterRidden::Term()
{
	return;
}

void CBehaviourFighterRidden::Manage()
{
	this->pOwner->BehaviourFighterRidden_Manage();
	_ManageExit();

	return;
}

void CBehaviourFighterRidden::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CLifeInterface* pLife;
	int iVar2;
	uint uVar3;
	CActorFighter* pCVar4;
	float fVar5;
	float fVar6;

	this->pOwner = static_cast<CActorFighter*>(pOwner);
	this->pOwner->field_0x354 = this->pOwner->pAdversary;
	this->pOwner->pAdversary = (CActorFighter*)0x0;
	pCVar4 = this->pOwner;
	pCVar4->fightFlags = pCVar4->fightFlags | 0x8000;
	pCVar4->fightFlags = pCVar4->fightFlags & 0xfffeffff;
	this->pOwner->scalarDynA.Reset();
	this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	if (newState == -1) {
		this->pOwner->SetState(FIGHTER_RIDDEN_RUN, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->behaviourId = this->pOwner->prevBehaviourId;
	this->pOwner->fightFlags = this->pOwner->fightFlags & 0xfffff8ff;
	pLife = this->pOwner->GetLifeInterfaceOther();
	pLife->SetPriority(2);
	uVar3 = 0;
	fVar6 = 0.0;
	this->field_0xc = GetTimer()->scaledTotalTime;
	this->field_0x18 = 0;

	do {
		iVar2 = rand();
		fVar5 = fVar6 + ((float)iVar2 / 2.147484e+09f) * 0.4f + 0.0f;
		fVar6 = fVar6 + 0.5f;
		this->field_0x10[uVar3] = fVar5;
		this->field_0x10[uVar3] = this->field_0x10[uVar3] * 8.0f;
		uVar3 = uVar3 + 1;
	} while (uVar3 < 2);

	return;
}

void CBehaviourFighterRidden::End(int newBehaviourId)
{
	CActorFighter* pCVar1;
	bool bVar2;
	CBehaviourFighter* pBehaviour;
	CLifeInterface* pLife;
	s_fighter_action_param sStack16;
	s_fighter_action local_4;

	bVar2 = this->pOwner->IsFightRelated(newBehaviourId);
	if (bVar2 == false) {
		pCVar1 = this->pOwner->field_0x354;
		local_4.all = 0x40;
		pBehaviour = static_cast<CBehaviourFighter*>(pCVar1->GetBehaviour(pCVar1->curBehaviourId));
		pBehaviour->Conditional_Execute(&local_4, &sStack16);
	}

	pLife = this->pOwner->GetLifeInterfaceOther();
	pLife->SetPriority(0);
	this->pOwner->pAdversary = this->pOwner->field_0x354;
	this->pOwner->field_0x354 = (CActorFighter*)0x0;
	pCVar1 = this->pOwner;
	pCVar1->fightFlags = pCVar1->fightFlags & 0xffff7fff;
	pCVar1->fightFlags = pCVar1->fightFlags | 0x10000;
	this->pOwner->RestoreCollisionSphere(0.0f);

	return;
}

void CBehaviourFighterRidden::InitState(int newState)
{
	this->pOwner->BehaviourFighterRidden_InitState(newState);

	return;
}

void CBehaviourFighterRidden::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourFighterRidden_TermState(oldState);

	return;
}

int CBehaviourFighterRidden::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorFighter* pCVar1;
	int result;
	float fVar2;
	float fVar3;
	float fVar4;

	if (msg == MESSAGE_IN_WIND_AREA) {
		result = 1;
		this->pOwner->fightFlags = this->pOwner->fightFlags | 0x200;
	}
	else {
		if (msg == MESSAGE_KICKED) {
			_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
			if (pHitParam->projectileType == 7) {
				if (static_cast<CActorFighter*>(pSender) == this->pOwner->field_0x354) {
					this->pOwner->hitFlags = pHitParam->flags;
					this->pOwner->hitDamage = pHitParam->damage;
					pCVar1 = this->pOwner;
					pCVar1->field_0x690 = pHitParam->field_0x40;
					pCVar1 = this->pOwner;
					pCVar1->field_0x6a0 = pHitParam->field_0x20;
					this->pOwner->field_0x6b0 = pHitParam->field_0x30;
					this->pOwner->field_0x684 = pHitParam->field_0x50;
					this->pOwner->field_0x686 = pHitParam->field_0x52;
					pCVar1 = this->pOwner;
					if ((pCVar1->field_0x684 & 1U) != 0) {
						pCVar1->field_0x7a0 = pHitParam->field_0x60;
						this->pOwner->field_0x7b4 = pHitParam->field_0x70;
					}

					_ManageHit(1);
				}

				result = 1;
			}
			else {
				result = 1;
			}
		}
		else {
			result = 0;
		}
	}

	return result;
}

bool CBehaviourFighterRidden::Execute(s_fighter_action* pAction, s_fighter_action_param* pParam)
{
	CActorFighter* pFighter;
	StateConfig* pSVar2;
	int iVar3;
	uint uVar4;
	float fVar5;
	float puVar6;
	float puVar7;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	s_fighter_action local_4;

	local_4 = *pAction;

	pFighter = this->pOwner;
	if ((pFighter->GetStateFlags(pFighter->actorState) & 0x200000) == 0) {
		local_4.all = local_4.all & 0xfffffff0;
	}

	if ((local_4.all & 0xf) == 0xc) {
		pFighter = this->pOwner;
		if ((pFighter->flags & 0x1000) == 0) {
			fVar5 = GetAngleYFromVector(&pFighter->rotationQuat);
		}
		else {
			fVar5 = pFighter->rotationEuler.y;
		}

		SetVectorFromAngleY(fVar5, &local_20);
		edF32Vector4SafeNormalize0Hard(&local_30, pParam->field_0x0);

		iVar3 = -1;
		if (0.0f < local_20.z * local_30.x - local_20.x * local_30.z) {
			iVar3 = 1;
		}

		if (static_cast<float>(iVar3) != this->pOwner->field_0x500) {
			this->pOwner->field_0x500 = static_cast<float>(iVar3);
			this->pOwner->scalarDynA.BuildFromSpeedTime(1.570796f, 6.283185f, 0.75f);
		}

		puVar6 = edF32Vector4DotProductHard(&local_30, &local_20);
		if (1.0f < puVar6) {
			puVar7 = 1.0f;
		}
		else {
			puVar7 = -1.0f;
			if (-1.0f <= puVar6) {
				puVar7 = puVar6;
			}
		}

		fVar5 = acosf(static_cast<float>(puVar7));
		this->pOwner->field_0x7b0 = fVar5;
	}
	else {
		if ((local_4.all & 0xf) == 0) {
			this->pOwner->scalarDynA.Reset();
			this->pOwner->field_0x500 = 0.0f;
		}
	}

	return local_4.all != 0x0;
}

void CBehaviourFighterRidden::_ManageHit(bool bPlayImpact)
{
	bool bVar1;
	CActorFighter* pFighter;

	this->pOwner->SetLookingAtOff();

	pFighter = this->pOwner;
	if ((pFighter->hitFlags & 1) == 0) {
		pFighter->LifeDecrease(pFighter->hitDamage);
		this->field_0x18 = this->field_0x18 + 1;
		this->pOwner->SetState(0x25, -1);
	}
	else {
		bVar1 = this->pOwner->actorsExcludeTable.IsInList(pFighter->field_0x354);
		if (bVar1 == false) {
			this->pOwner->actorsExcludeTable.Add(-1.0f, this->pOwner->field_0x354);
		}

		this->pOwner->SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, -1, -1);
	}

	return;
}

void CBehaviourFighterRidden::_ManageExit()
{
	int iVar1;
	bool bVar2;
	bool bVar3;
	StateConfig* pSVar4;
	uint uVar6;
	CBehaviourFighterRidden* pCVar7;
	float fVar8;
	CActorFighter* pFighter;

	pFighter = this->pOwner;
	iVar1 = pFighter->actorState;
	if (iVar1 == -1) {
		uVar6 = 0;
	}
	else {
		pSVar4 = pFighter->GetStateCfg(iVar1);
		uVar6 = pSVar4->flags_0x4;
	}
	if ((uVar6 & 0x104000) == 0x104000) {
		bVar3 = this->pOwner->ColWithLava();
		if (((bVar3 == false) && (bVar3 = this->pOwner->ColWithCactus(), bVar3 == false)) && (bVar3 = this->pOwner->ColWithAToboggan(), bVar3 == false)) {
			fVar8 = GetTimer()->scaledTotalTime - this->field_0xc;
			bVar3 = false;
			if (8.0f <= fVar8) {
				bVar3 = true;
			}
			else {
				uVar6 = 0;
				while ((uVar6 < 2 && (!bVar3))) {
					bVar2 = false;
					if (pCVar7->field_0x10[uVar6] <= fVar8) {
						bVar2 = fVar8 <= pCVar7->field_0x10[uVar6] + GetTimer()->cutsceneDeltaTime;
					}

					if (bVar2) {
						bVar3 = true;
					}

					uVar6 = uVar6 + 1;
				}
			}

			if (bVar3) {
				this->pOwner->SetState(0x27, -1);
			}
		}
		else {
			pFighter = this->pOwner->field_0x354;
			pFighter->SetState(0x22, -1);
			this->pOwner->SetState(0x26, -1);
		}
	}

	return;
}

void CActorFighter::BehaviourFighterRidden_InitState(int newState)
{
	if (newState == FIGHTER_RIDDEN_EJECT) {
		_StateFighterRiddenEjectInit();
	}
	else {
		if ((newState == 0x27) || (newState == 0x26)) {
			this->dynamic.speed = 0.0f;
		}
		else {
			if (newState == 0x25) {
				this->dynamic.speed = 0.0f;
			}
			else {
				if (newState == FIGHTER_RIDDEN_RUN) {
					this->dynamic.speed = this->field_0x504;
					this->dynamic.rotationQuat = this->rotationQuat;
				}
			}
		}
	}

	return;
}

void CActorFighter::BehaviourFighterRidden_Manage()
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	CLifeInterface* pCVar5;
	CBehaviourFighterRidden* pCVar6;
	float fVar7;

	iVar1 = this->actorState;
	if (iVar1 == FIGHTER_RIDDEN_EJECT) {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pCVar6 = static_cast<CBehaviourFighterRidden*>(this->GetBehaviour(this->curBehaviourId));
			this->SetBehaviour(pCVar6->behaviourId, -1, -1);
		}
	}
	else {
		if (iVar1 == 0x27) {
			ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(FIGHTER_RIDDEN_EJECT, -1);
			}
		}
		else {
			if (iVar1 == 0x26) {
				ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					this->field_0x354->SetState(0x23, -1);
					this->field_0x684 = 1;
					SetBehaviour(FIGHTER_BEHAVIOUR_PROJECTED, 0x5f, -1);
				}
			}
			else {
				if (iVar1 == 0x25) {
					ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

					if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
						pCVar5 = this->GetLifeInterface();
						fVar7 = pCVar5->GetValue();

						if ((fVar7 <= 0.0f) || (pCVar6 = static_cast<CBehaviourFighterRidden*>(this->GetBehaviour(this->curBehaviourId)), 2 < pCVar6->field_0x18)) {
							this->field_0x354->SetState(0x22, -1);
							this->SetState(0x26, -1);
						}
						else {
							this->field_0x354->SetState(0x20, -1);
							SetState(FIGHTER_RIDDEN_RUN, -1);
						}
					}
				}
				else {
					if (iVar1 == FIGHTER_RIDDEN_RUN) {
						_StateFighterRiddenRun();
					}
				}
			}
		}
	}

	return;
}

void CActorFighter::BehaviourFighterRidden_TermState(int oldState)
{
	CCollision* pCVar1;

	if (oldState == FIGHTER_RIDDEN_EJECT) {
		pCVar1 = this->pCollisionData;
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	}

	return;
}

edF32VECTOR4 vFront$12305 = { 0.0f, 2.0f, -1.0f, 0.0f };
edF32VECTOR4 vSide$12306 = { 1.0f, 1.0f, -1.0f, 0.0f };
edF32VECTOR4 vFrontRot$12307 = { -1.0f, 0.0f, 0.0f, 0.0f };
edF32VECTOR4 vSideRot$12308 = { 0.0f, 1.0f, 0.0f, 0.0f };

void CActorFighter::_StateFighterRiddenRun()
{
	CActor* pActor;
	CCollision* pCVar1;
	float uVar2;
	float uVar3;
	float uVar4;
	float end;
	float alpha;
	float end_00;
	float alpha_00;
	bool bVar5;
	float alpha_01;
	float end_01;
	float alpha_02;
	int iVar7;
	float fVar8;
	float fVar9;
	float puVar9;
	float puVar12;
	float fVar10;
	float puVar11;
	float puVar10;
	edF32VECTOR4 local_260;
	edF32VECTOR4 local_250;
	edF32VECTOR4 local_240;
	edF32MATRIX4 eStack560;
	edF32VECTOR4 local_1f0;
	_msg_hit_param local_1e0;
	edF32VECTOR4 local_160;
	edF32MATRIX4 eStack336;
	CActorsTable local_110;
	_msg_hit_param* local_4;

	local_110.nbEntries = 0;
	fVar9 = this->scalarDynA.Integrate(GetTimer()->cutsceneDeltaTime);
	fVar8 = fabs(this->field_0x7b0); // Note: This is uninitialized in original code.
	if (fVar9 <= fVar8) {
		fVar8 = fVar9;
	}
	local_160.x = 0.0f;
	local_160.y = this->field_0x500;
	local_160.z = 0.0f;
	local_160.w = 0.0f;
	edF32Matrix4FromAngAxisSoft(-fVar8, &eStack336, &local_160);
	edF32Matrix4MulF32Vector4Hard(&local_160, &eStack336, &this->rotationQuat);
	this->rotationQuat = local_160;
	this->dynamic.rotationQuat = local_160;

	ManageDyn(4.0f, 0x1002023b, &local_110);

	if (((this->pCollisionData)->flags_0x4 & 2) != 0) {
		this->timeInAir = 0.0f;
	}
	iVar7 = 0;
	if (local_110.nbEntries != 0) {
		for (; iVar7 < local_110.nbEntries; iVar7 = iVar7 + 1) {
			pActor = local_110.aEntries[iVar7];
			bVar5 = this->actorsExcludeTable.IsInList(pActor);
			if (bVar5 == false) {
				bVar5 = pActor->IsKindOfObject(8);
				alpha_00 = vSideRot$12308.z;
				end_00 = vFrontRot$12307.z;
				alpha = vSide$12306.z;
				end = vFront$12305.z;
				fVar9 = vFront$12305.y;
				fVar8 = vFront$12305.x;

				if (bVar5 == false) {
					edF32Vector4SubHard(&local_1f0, &pActor->currentLocation, &this->currentLocation);
					local_1f0.y = 0.0f;
					fVar8 = edF32Vector4NormalizeHard(&local_1f0, &local_1f0);
					if (fVar8 < 0.001f) {
						local_1f0 = this->rotationQuat;
					}

					edF32Vector4ScaleHard(1.0f, &local_1f0, &local_1f0);

					local_1f0.y = 0.5f;
					local_1e0.field_0x20.x = local_1f0.x;
					local_1e0.field_0x20.y = 0.5f;
					local_1e0.field_0x20.z = local_1f0.z;
					local_1e0.field_0x20.w = local_1f0.w;
					local_1e0.field_0x30 = 10.0f;
				}
				else {
					alpha_02 = vSide$12306.x;
					uVar2 = vSide$12306.y;
					end_01 = vFrontRot$12307.x;
					uVar3 = vFrontRot$12307.y;
					alpha_01 = vSideRot$12308.x;
					uVar4 = vSideRot$12308.y;

					local_240.x = this->rotationQuat.z;
					local_240.y = 0.0f;
					local_240.z = -this->rotationQuat.x;
					local_240.w = 0.0f;
					edF32Vector4SubHard(&local_1f0, &pActor->currentLocation, &this->currentLocation);
					local_1f0.y = 0.0f;
					edF32Vector4SafeNormalize1Hard(&local_1f0, &local_1f0);
					puVar9 = edF32Vector4DotProductHard(&local_1f0, &local_240);
					if (1.0f < puVar9) {
						puVar12 = 1.0f;
					}
					else {
						puVar12 = -1.0f;
						if (-1.0f <= puVar9) {
							puVar12 = puVar9;
						}
					}
					fVar10 = acosf(puVar12);
					fVar10 = (1.570796f - fVar10) / 1.570796f;
					if (0.0f <= fVar10) {
						puVar11 = 1.0f;
					}
					else {
						puVar11 = -1.0f;
					}
					local_250.x = edFIntervalUnitSrcLERP(fabsf(fVar10), fVar8, alpha_02);
					local_250.x = puVar11 * local_250.x;
					local_250.y = edFIntervalUnitSrcLERP(fabsf(fVar10), fVar9, uVar2);
					local_250.z = edFIntervalUnitSrcLERP(fabsf(fVar10), end, alpha);
					local_250.w = 0.0f;
					edF32Vector4NormalizeHard(&local_250, &local_250);
					local_260.x = edFIntervalUnitSrcLERP(fabsf(fVar10), end_01, alpha_01);
					if (0.0f <= fVar10) {
						puVar10 = 1.0f;
					}
					else {
						puVar10 = -1.0f;
					}
					local_260.y = edFIntervalUnitSrcLERP(fabsf(fVar10), uVar3, uVar4);
					local_260.y = puVar10 * local_260.y;
					local_260.z = edFIntervalUnitSrcLERP(fabsf(fVar10), end_00, alpha_00);
					local_260.w = 0.0f;
					edF32Vector4NormalizeHard(&local_260, &local_260);
					local_1e0.field_0x30 = edFIntervalUnitSrcLERP(fabsf(fVar10), 12.0f, 8.0f);
					local_1e0.field_0x70 = edFIntervalUnitSrcLERP(fabsf(fVar10), 6.0f, 12.0f);
					fVar8 = GetAngleYFromVector(&this->rotationQuat);
					edF32Matrix4RotateYHard(fVar8, &eStack560, &gF32Matrix4Unit);
					edF32Matrix4MulF32Vector4Hard(&local_1e0.field_0x20, &eStack560, &local_250);
					edF32Matrix4MulF32Vector4Hard(&local_1e0.field_0x60, &eStack560, &local_260);
				}

				local_1e0.projectileType = 7;
				local_1e0.flags = 1;
				local_1e0.field_0x50 = 1;
				pCVar1 = pActor->pCollisionData;
				local_4 = &local_1e0;
				local_1e0.field_0x40.x = (pCVar1->field_0x90).x;
				local_1e0.field_0x40.y = (pCVar1->field_0x90).y;
				local_1e0.field_0x40.z = (pCVar1->field_0x90).z;
				local_1e0.field_0x40.w = (pCVar1->field_0x90).w;
				local_1e0.field_0x10 = 0.3f;
				local_1e0.damage = 5.0f;
				DoMessage(pActor, MESSAGE_KICKED, local_4);
				this->actorsExcludeTable.Add(1.0f, pActor);
			}
		}
	}

	return;
}

void CActorFighter::_Ride_GetPossibleExit()
{
	byte bVar1;
	int iVar2;
	CActorFighter* pCVar3;
	CCollision* pCVar4;
	StateConfig* pSVar5;
	CBehaviourFighterRidden* pCVar6;
	uint uVar6;
	edF32VECTOR4* v0;
	_msg_hit_param local_110;
	_msg_hit_param local_90;

	if ((GetStateFlags(this->actorState) & 0x100000) != 0) {
		pCVar3 = this->field_0x354;
		if (((pCVar3->fightFlags & 0x200) != 0) || ((bVar1 = (pCVar3->pCollisionData)->flags_0x4, (bVar1 & 2) == 0 && (0.3f < pCVar3->timeInAir)))) {
			local_90.projectileType = 7;
			local_90.flags = 1;
			local_90.field_0x50 = 1;
			local_90.field_0x60.x = this->rotationQuat.z;
			local_90.field_0x60.y = 0.0f;
			local_90.field_0x60.z = -this->rotationQuat.x;
			local_90.field_0x60.w = 0.0f;
			local_90.field_0x20.x = this->rotationQuat.x;
			local_90.field_0x20.y = -1.0f;
			local_90.field_0x20.z = this->rotationQuat.z;
			local_90.field_0x20.w = 0.0f;
			edF32Vector4NormalizeHard(&local_90.field_0x20, &local_90.field_0x20);
			local_90.field_0x30 = this->field_0x504;
			local_90.field_0x70 = 2.0f;
			local_90.damage = 0.0f;
			local_90.field_0x10 = 0.0f;
			DoMessage(this->field_0x354, MESSAGE_KICKED, &local_90);
			pCVar6 = static_cast<CBehaviourFighterRidden*>(GetBehaviour(this->curBehaviourId));
			SetBehaviour(pCVar6->behaviourId, -1, -1);
		}
		else {
			if ((bVar1 & 1) != 0) {
				local_110.projectileType = 7;
				v0 = &local_110.field_0x20;
				local_110.flags = 1;
				local_110.field_0x50 = 1;
				local_110.field_0x30 = this->field_0x504;
				local_110.field_0x60 = gF32Vector4UnitY;
				local_110.field_0x70 = 0.0f;
				local_110.damage = 0.0f;
				local_110.field_0x10 = 0.0f;

				pCVar4 = pCVar3->pCollisionData;
				local_110.field_0x20.x = pCVar4->aCollisionContact[0].location.x;
				local_110.field_0x20.z = pCVar4->aCollisionContact[0].location.z;
				local_110.field_0x20.w = pCVar4->aCollisionContact[0].location.w;
				local_110.field_0x20.y = pCVar4->aCollisionContact[0].location.y + 0.1f;

				edF32Vector4NormalizeHard(v0, v0);
				DoMessage(this->field_0x354, MESSAGE_KICKED, &local_110);
				pCVar4 = pCVar3->pCollisionData;
				local_110.field_0x20.x = pCVar4->aCollisionContact[0].location.x;
				local_110.field_0x20.z = pCVar4->aCollisionContact[0].location.z;
				local_110.field_0x20.w = pCVar4->aCollisionContact[0].location.w;
				local_110.field_0x20.y = pCVar4->aCollisionContact[0].location.y + 0.4f;
				edF32Vector4NormalizeHard(v0, v0);
				DoMessage(this, MESSAGE_KICKED, &local_110);
			}
		}
	}

	return;
}