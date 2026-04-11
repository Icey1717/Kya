#include "CameraBoomy.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include "ActorJamGut.h"
#include "ActorHero.h"
#include "ActorHero_Private.h"
#include "ActorBoomy.h"
#include "CollisionRay.h"
#include "InputManager.h"
#include "TimeController.h"

CCameraAimBase::CCameraAimBase()
	: CCameraExt()
{
	this->field_0xd0 = gF32Vector4Zero;
	this->field_0xf0 = 0;

	return;
}

void CCameraAimBase::Init()
{
	CCameraExt::Init();

	this->flags_0xc = this->flags_0xc | 0x20004;
	this->field_0x8 = -100;

	return;
}

CCameraAim::CCameraAim()
	: CCameraAimBase()
{
	this->field_0x100 = 0.1186824f;
	this->field_0x104 = 0.06283185f;

	this->switchMode = SWITCH_MODE_F;
	this->field_0x98 = 0.2666667f;
	this->field_0x94 = SWITCH_MODE_F;
	this->field_0x9c = 0.15f;

	return;
}

ECameraType CCameraAim::GetMode()
{
	return CT_SilverBoomy;
}

void CCameraAim::Init()
{
	CCameraAimBase::Init();

	this->fov = 0.364f;

	SetDistance(1.8f);

	this->field_0xe0.Init(0.0f, 6.981317f);
	this->field_0xe8.Init(0.0f, 7.853981f);

	this->field_0x178 = 0;
	this->field_0x17c = 0.12f;

	return;
}

edF32VECTOR4 edF32VECTOR4_0042adc0 = {
	-0.234804f, 1.459372f, -0.306366f, 0.0f
};

bool CCameraAim::Manage()
{
	bool bVar1;
	CActor* pCVar2;
	CPlayerInput* pInput;
	CActor* pCVar4;
	CBehaviour* pCVar5;
	CActorBoomy* pBoomy;
	edF32MATRIX4* outputMatrix;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	undefined4 local_8;
	undefined4 local_4;

	pBoomy = (CActorBoomy*)0x0;
	pInput = GetTarget()->GetInputManager(0, 0);
	if ((GetTarget() == (CActor*)0x0) || (pInput == (CPlayerInput*)0x0)) {
		bVar1 = false;
	}
	else {
		bVar1 = CCamera::Manage();

		if (bVar1 == false) {
			if (GetTarget()->typeID == 6) {
				CActorHero* pHero = static_cast<CActorHero*>(GetTarget());
				pBoomy = pHero->pActorBoomy;
			}

			bVar1 = false;
			if (pBoomy != (CActorBoomy*)0x0) {
				if ((this->flags_0xc & 0x100000) == 0) {
					fVar7 = edF32Between_2Pi(this->field_0x100 + (GetTarget()->rotationEuler).x);
					SetAngleAlpha(fVar7);
					fVar7 = edF32Between_2Pi(this->field_0x104 + GetTarget()->rotationEuler.y);
					SetAngleBeta(fVar7);
				}

				ComputeTargetPosition(&local_30);
				edF32Matrix4MulF32Vector4Hard(&local_20, &GetTarget()->pMeshTransform->base.transformA, &edF32VECTOR4_0042adc0);
				edF32Vector4AddHard(&local_30, &local_30, &local_20);
				fVar7 = this->lookAt.y;
				if (0.01f <= fabsf(local_30.y - fVar7)) {
					fVar7 = local_30.y;
				}
				local_30.y = fVar7;
				this->lookAt.x = local_30.x;
				this->lookAt.y = local_30.y;
				this->lookAt.z = local_30.z;
				this->lookAt.w = local_30.w;
				fVar7 = GetAngleAlpha();
				edF32Matrix4RotateXHard(fVar7, &this->transformationMatrix, &gF32Matrix4Unit);
				outputMatrix = &this->transformationMatrix;
				fVar7 = GetAngleBeta();
				edF32Matrix4RotateYHard(fVar7, outputMatrix, outputMatrix);
				edF32Matrix4MulF32Vector4Hard(&this->field_0xd0, outputMatrix, &gF32Vector4UnitZ);
				fVar7 = GetDistance();
				edF32Vector4ScaleHard(fVar7, &local_20, &this->field_0xd0);
				edF32Vector4SubHard(&local_20, &this->lookAt, &local_20);
				this->transformationMatrix.rowT = local_20;
				if ((this->flags_0xc & 0x100000) == 0) {
					(this->vision).location = this->lookAt;
					(this->vision).rotationQuat = this->field_0xd0;
					pCVar2 = this->vision.SV_GetBestActorInFrontOf(0);
					this->field_0x174 = pCVar2;
					edF32Vector4ScaleHard(-1.0f, &local_20, &this->field_0xd0);
					CCollisionRay CStack96 = CCollisionRay(2.4f, &this->lookAt, &local_20);
					fVar7 = CStack96.Intersect(3, GetTarget(), (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
					this->field_0xf0 = fVar7;
					if (fVar7 == 1e+30f) {
						this->field_0xf0 = 2.4f;
					}
					else {
						if (fVar7 < 0.8f) {
							this->field_0xf0 = 0.8f;
						}
					}

					SetDistance(this->field_0xf0 - 0.6f);

					if ((CCamera::_gpcam_man->flags & 0x4000000) == 0) {
						CStack96.ChangeLeadVector(&this->field_0xd0);
						CStack96.ChangeMaxDistance(this->field_0x170);
						fVar7 = CStack96.Intersect(3, GetTarget(), (CActor*)0x0, 0x40000020, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
						this->field_0xf0 = fVar7;
						if (fVar7 == 1e+30f) {
							this->field_0xf0 = this->field_0x170;
						}

						pCVar2 = this->field_0x174;
						if (pCVar2 != this->field_0x178) {
							if (pCVar2 != (CActor*)0x0) {
								GetTarget()->DoMessage(pCVar2, (ACTOR_MESSAGE)0x4a, 0);
							}

							pCVar2 = this->field_0x178;
							if (pCVar2 != (CActor*)0x0) {
								GetTarget()->DoMessage(pCVar2, (ACTOR_MESSAGE)0x4a, (MSG_PARAM)1);
							}
						}

						fVar7 = pInput->aAnalogSticks[0].y;
						fVar8 = pInput->aAnalogSticks[0].x;
						if ((((this->field_0x174 == 0) && (this->field_0x178 != 0)) && (fVar8 != 0.0f)) && (fVar7 != 0.0f)) {
							this->field_0xe0.currentAlpha = 0.0f;
							this->field_0xe8.currentAlpha = 0.0f;
						}

						if (((pInput == (CPlayerInput*)0x0) || (this->field_0x174 == 0)) || ((fVar7 != 0.0f || (fVar8 != 0.0f)))) {
							edF32Vector4ScaleHard(this->field_0xf0, &local_20, &this->field_0xd0);
							edF32Vector4AddHard(&local_20, &local_20, &this->lookAt);
							pBoomy->SetTarget(&local_20);
						}
						else {
							edF32Vector4ScaleHard(this->field_0xf0, &eStack64, &this->field_0xd0);
							edF32Vector4AddHard(&eStack64, &eStack64, &this->lookAt);
							GetTarget()->SV_GetActorTargetPos(this->field_0x174, &eStack64);
							edF32Vector4SubHard(&local_20, &eStack64, &this->lookAt);
							fVar7 = edF32Vector4NormalizeHard(&local_20, &local_20);
							this->field_0xf0 = fVar7;
							fVar7 = GetAngleXFromVector(&local_20);
							this->field_0x100 = this->field_0x100 + this->field_0x17c * (fVar7 - this->field_0x100);
							fVar8 = (GetTarget()->rotationEuler).y;
							fVar7 = GetAngleYFromVector(&local_20);
							fVar7 = edF32Between_0_2Pi(fVar7 - this->field_0x104);
							fVar7 = edF32GetAnglesDelta(fVar8, fVar7);
							fVar7 = edF32Between_0_2Pi(this->field_0x17c * fVar7 + GetTarget()->rotationEuler.y);
							GetTarget()->rotationEuler.y = fVar7;
							SetVectorFromAngles(&GetTarget()->rotationQuat, &GetTarget()->rotationEuler.xyz);
							edF32Vector4ScaleHard(this->field_0xf0, &local_20, &this->field_0xd0);
							edF32Vector4AddHard(&local_20, &local_20, &this->lookAt);
							pBoomy->SetTarget(this->field_0x174, &local_20);
						}

						this->field_0x178 = this->field_0x174;

						Manage_Pad();
					}
				}

				if (((this->pOtherTarget == (CActor*)0x0) && (GetTarget()->typeID == 6)) && (GetTarget()->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
					CBehaviourHeroRideJamGut* pRideBehaivour = static_cast<CBehaviourHeroRideJamGut*>(GetTarget()->GetBehaviour(GetTarget()->curBehaviourId));
					CActorJamGut* pJamGut = pRideBehaivour->field_0x8c;
					pCVar2 = pJamGut->pTiedActor;
					if (pCVar2 == (CActor*)0x0) {
						pCVar2 = pJamGut->GetCollidingActor();
					}
					SetOtherTarget(pCVar2);

					edF32Vector4ScaleHard(pJamGut->dynamic.linearAcceleration * GetTimer()->cutsceneDeltaTime, &local_70, &pJamGut->dynamic.velocityDirectionEuler);
					edF32Vector4AddHard(&local_70, &local_70, &this->transformationMatrix.rowT);
					fVar7 = this->transformationMatrix.db;
					if (0.01f <= fabsf(local_70.y - fVar7)) {
						fVar7 = local_70.y;
					}

					local_70.y = fVar7;
					this->transformationMatrix.rowT = local_70;
				}

				FUN_00199b80();
				bVar1 = true;
			}
		}
		else {
			bVar1 = false;
		}
	}

	return bVar1;
}

void CCameraAim::Draw()
{
	return;
}

bool CCameraAim::AlertCamera(int param_2, void* pParams, CCamera* param_4)
{
	CCamera* pCVar1;
	ECameraType EVar3;
	int iVar4;
	float fVar5;

	if (param_2 == 2) {
		(GetTarget()->rotationEuler).x = 0.0f;
		pCVar1 = CCamera::_gpcam_man->pActiveCamera;

		EVar3 = pCVar1->GetMode();
		if ((EVar3 == CT_IntView) || (EVar3 = pCVar1->GetMode(), EVar3 == CT_SilverBoomy)) {
			fVar5 = pCVar1->GetAngleAlpha();
			this->field_0x100 = fVar5;
			this->field_0x104 = 0.04f;
		}
		else {
			this->field_0x100 = 0.1186824f;
			this->field_0x104 = 0.06283185f;
		}

		this->field_0x174 = (CActor*)0x0;
		this->field_0x178 = (CActor*)0x0;

		if (GetTarget()->typeID == ACTOR_HERO_PRIVATE) {
			CActorHero* pCVar2 = (CActorHero*)GetTarget();
			this->field_0x170 = pCVar2->pActorBoomy->GetVision()->visionRange;
		}
		else {
			this->field_0x170 = 20.0f;
		}

		this->vision.pOwner = GetTarget();
		this->vision.Init(8.0f, this->field_0x170, 3.3f, 0.5f);
		this->vision.flags = 0x2000001b;
	}
	else {
		if (param_2 == 3) {
			this->flags_0xc = this->flags_0xc & 0xffefffff;
		}
		else {
			CActor* pCVar2;
			if (((param_2 == 1) && (pCVar2 = this->field_0x178, pCVar2 != (CActor*)0x0)) && (pCVar2 == this->field_0x174)) {
				GetTarget()->DoMessage(pCVar2, (ACTOR_MESSAGE)0x4a, (MSG_PARAM)1);
			}
		}
	}

	CCamera::AlertCamera(param_2, pParams, param_4);

	return false;
}

void CCameraAim::Manage_Pad()
{
	CActor* pActor;
	CPlayerInput* pInput;
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	fVar5 = 0.0f;
	fVar2 = 0.0f;
	fVar3 = 0.0f;

	pActor = GetTarget();
	pInput = pActor->GetInputManager(0, 1);
	if (pInput != (CPlayerInput*)0x0) {
		if ((pInput->aButtons[0x11].clickValue != 0.0f) && (-0.3839724f < this->field_0x104)) {
			this->field_0x104 = this->field_0x104 - CCamera::_gpcam_man->time_0x4 * 0.3926991f;
		}
		if ((pInput->aButtons[0x10].clickValue != 0.0f) && (this->field_0x104 < 0.3839724f)) {
			this->field_0x104 = this->field_0x104 + CCamera::_gpcam_man->time_0x4 * 0.3926991f;
		}

		fVar4 = pInput->aAnalogSticks[1].y;
		if ((-0.4f <= fVar4) || (fVar1 = GetDistance(), 10.0f <= fVar1)) {
			if ((0.4f < fVar4) && (fVar1 = GetDistance(), 0.5f < fVar1)) {
				fVar5 = edFIntervalLERP(fVar4, 0.4f, 0.9f, 0.0f, -4.0f);
			}
		}
		else {
			fVar5 = edFIntervalLERP(fVar4, -0.4f, -0.9f, 0.0f, 4.0f);
		}

		this->distance = this->distance + fVar5 * CCamera::_gpcam_man->time_0x4;
		fVar5 = pInput->aAnalogSticks[0].y;
		if ((-0.4f <= fVar5) || (0.3490658f < this->field_0x100)) {
			if ((0.4f < fVar5) && (-0.7853982f <= this->field_0x100)) {
				fVar2 = edFIntervalLERP(fVar5, 0.4f, 0.9f, 0.0f, -2.094395f);
			}
		}
		else {
			fVar2 = edFIntervalLERP(fVar5, -0.4f, -0.9f, 0.0f, 2.094395f);
		}

		this->field_0xe0.Update(fVar2);
		fVar5 = this->field_0x100 + this->field_0xe0.currentAlpha * CCamera::_gpcam_man->time_0x4;
		this->field_0x100 = fVar5;
		if (fVar5 < -0.7853982f) {
			this->field_0x100 = -0.7853982f;
			this->field_0xe0.currentAlpha = 0.0f;
		}

		if (0.3490658f < this->field_0x100) {
			this->field_0x100 = 0.3490658f;
			this->field_0xe0.currentAlpha = 0.0f;
		}

		fVar5 = pInput->aAnalogSticks[0].x;
		if (fVar5 < -0.4f) {
			fVar3 = edFIntervalLERP(fVar5, -0.4f, -0.9f, 0.0f, -2.617994f);
		}
		else {
			if (0.4f < fVar5) {
				fVar3 = edFIntervalLERP(fVar5, 0.4f, 0.9f, 0.0f, 2.617994f);
			}
		}

		this->field_0xe8.Update(fVar3);
		fVar5 = this->field_0xe8.currentAlpha;
		fVar2 = CCamera::_gpcam_man->time_0x4;
		pActor = GetTarget();
		fVar5 = edF32Between_0_2Pi(pActor->rotationEuler.y + -fVar5 * fVar2);
		pActor->rotationEuler.y = fVar5;
		pActor = GetTarget();
		SetVectorFromAngles(&pActor->rotationQuat, (edF32VECTOR3*)&pActor->rotationEuler);
		fVar5 = edFIntervalLERP(pInput->aButtons[9].analogValue, 0.4f, 1.0f, this->field_0x17c, 0.6f);
		this->field_0x17c = fVar5;
	}

	return;
}
