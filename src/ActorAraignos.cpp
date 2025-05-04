#include "ActorAraignos.h"
#include "MemoryStream.h"
#include "PathManager.h"
#include "CollisionManager.h"
#include "TimeController.h"
#include "MathOps.h"

void CActorAraignos::Create(ByteCode* pByteCode)
{
	CAudioManager* this_00;
	CPathManager* pCVar1;
	int iVar2;
	uint uVar3;
	CSoundSample* pCVar4;
	CActorSound* pCVar5;
	CPathFollow* pPathFollow;
	float fVar6;

	pCVar1 = CScene::ptable.g_PathManager_004516a0;
	CActorMovable::Create(pByteCode);

	this->field_0x1d0 = 0.0f;

	iVar2 = pByteCode->GetS32();
	if (iVar2 == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = pCVar1->aPathFollow + iVar2;
	}

	this->field_0x1f8 = pByteCode->GetF32();
	this->field_0x1fc = pByteCode->GetF32();
	this->field_0x200 = pByteCode->GetF32();
	
	this->field_0x240 = CScene::ptable.g_AudioManager_00451698->GetSound(pByteCode->GetU32());
	this->field_0x238 = CScene::ptable.g_AudioManager_00451698->GetSound(pByteCode->GetU32());
	this->field_0x23c = CScene::ptable.g_AudioManager_00451698->GetSound(pByteCode->GetU32());

	this->pathFollowAbsolute.Create(this->field_0x1f8 * 1.5f, pPathFollow, 0);

	if (((this->field_0x240 == 0) && (this->field_0x238 == 0)) && (this->field_0x23c == 0)) {
		this->pActorSound = (CActorSound*)0x0;
	}
	else {
		this->pActorSound = CreateActorSound(2);
	}

	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorAraignos::Init()
{
	CActor::Init();

	this->field_0x1d0 = 0.0f;
	this->field_0x1d4 = (CActor*)0x0;

	this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 & 0xfffffffd;
	this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 | 0x9000;

	this->addOnGenerator.Init(0);

	return;
}

void CActorAraignos::Term()
{
	CActor::Term();
	this->addOnGenerator.Term();

	return;
}

void CActorAraignos::Reset()
{
	CActor::Reset();

	this->field_0x1d0 = 0.0f;
	this->field_0x1d4 = (CActor*)0x0;

	this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 & 0xfffffffd;
	this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 | 0x9000;

	return;
}

CBehaviour* CActorAraignos::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == ARAIGNOS_BEHAVIOUR_DEFAULT) {
		pBehaviour = &this->behaviourDefault;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorAraignos::_gStateCfg_ARA[2] =
{
	StateConfig(0x6, 0x4),
	StateConfig(0x7, 0x1),
};

StateConfig* CActorAraignos::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 2);
		pStateConfig = _gStateCfg_ARA + state + -5;
	}

	return pStateConfig;
}

void CActorAraignos::ChangeManageState(int state)
{
	StateConfig* pSVar1;
	int iVar2;
	uint uVar3;

	CActorMovable::ChangeManageState(state);

	iVar2 = this->actorState;
	if (iVar2 != -1) {
		if (iVar2 == -1) {
			uVar3 = 0;
		}
		else {
			pSVar1 = GetStateCfg(iVar2);
			uVar3 = pSVar1->flags_0x4;
		}

		if ((uVar3 & 1) == 0) {
			if (state == 0) {
				IMPLEMENTATION_GUARD_AUDIO(
				if ((this->pActorSound != (CActorSound*)0x0) &&
					(iVar2 = CActorSound::IsInstanceAlive(this->pActorSound, 1), iVar2 != 0)) {
					CActorSound::FUN_0032c600(this->pActorSound, 1);
				})
			}
			else {
				IMPLEMENTATION_GUARD_AUDIO(
				if (this->pActorSound != (CActorSound*)0x0) {
					CActorSound::SoundStart
					(this->pActorSound, (CActor*)this, 1, (CSound*)this->field_0x240, 0, 0,
						(SOUND_SPATIALIZATION_PARAM*)0x0);
					CActorSound::SetFrequency(this->field_0x1f8, this->pActorSound, 1);
				})

				this->field_0x1d0 = 0.0f;
			}
		}
	}

	return;
}

int CActorAraignos::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	StateConfig* pSVar1;
	uint uVar2;
	int iVar3;

	uVar2 = GetStateFlags(this->actorState);

	if ((uVar2 & 1) == 0) {
		if (msg == 0xe) {
			if ((this->flags & 0x2000001) == 0) {
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
				this->flags = this->flags & 0xffffff7f;
				this->flags = this->flags | 0x20;
				EvaluateDisplayState();
				IMPLEMENTATION_GUARD_AUDIO(
				if ((this->pActorSound != (CActorSound*)0x0) &&
					(iVar3 = CActorSound::IsInstanceAlive(this->pActorSound, 1), iVar3 != 0)) {
					CActorSound::FUN_0032c600(this->pActorSound, 1);
				})
			}
			else {
				this->flags = this->flags & 0xfffffffc;
				this->flags = this->flags & 0xffffff5f;
				EvaluateDisplayState();
				IMPLEMENTATION_GUARD_AUDIO(
				if (this->pActorSound != (CActorSound*)0x0) {
					CActorSound::SoundStart
					(this->pActorSound, (CActor*)this, 1, (CSound*)this->field_0x240, 0, 0,
						(SOUND_SPATIALIZATION_PARAM*)0x0);
					CActorSound::SetFrequency(this->field_0x1f8, this->pActorSound, 1);
				})

				this->field_0x1d0 = 0.0f;
			}
			iVar3 = 1;
		}
		else {
			if (msg == 0xf) {
				this->flags = this->flags & 0xfffffffc;
				this->flags = this->flags & 0xffffff5f;
				EvaluateDisplayState();
				IMPLEMENTATION_GUARD_AUDIO(
				if (this->pActorSound != (CActorSound*)0x0) {
					CActorSound::SoundStart
					(this->pActorSound, (CActor*)this, 1, (CSound*)this->field_0x240, 0, 0,
						(SOUND_SPATIALIZATION_PARAM*)0x0);
					CActorSound::SetFrequency(this->field_0x1f8, this->pActorSound, 1);
				})
				this->field_0x1d0 = 0.0f;
				iVar3 = 1;
			}
			else {
				if (msg == 0x10) {
					this->flags = this->flags & 0xfffffffd;
					this->flags = this->flags | 1;
					this->flags = this->flags & 0xffffff7f;
					this->flags = this->flags | 0x20;
					EvaluateDisplayState();
					IMPLEMENTATION_GUARD_AUDIO(
					if ((this->pActorSound != (CActorSound*)0x0) &&
						(iVar3 = CActorSound::IsInstanceAlive(this->pActorSound, 1), iVar3 != 0)) {
						CActorSound::FUN_0032c600(this->pActorSound, 1);
					})
					iVar3 = 1;
				}
				else {
					if (msg == 0xd) {
						IMPLEMENTATION_GUARD(
						if (*(int*)((int)pMsgParam + 4) == 0) {
							this->flags = this->flags & 0xfffffffd;
							this->flags = this->flags | 1;
							this->flags = this->flags & 0xffffff7f;
							this->flags = this->flags | 0x20;
							EvaluateDisplayState();
							if ((this->pActorSound != (CActorSound*)0x0) &&
								(iVar3 = CActorSound::IsInstanceAlive(this->pActorSound, 1), iVar3 != 0)) {
								CActorSound::FUN_0032c600(this->pActorSound, 1);
							}
						}
						else {
							this->flags = this->flags & 0xfffffffc;
							this->flags = this->flags & 0xffffff5f;
							EvaluateDisplayState();
							if (this->pActorSound != (CActorSound*)0x0) {
								CActorSound::SoundStart
								(this->pActorSound, (CActor*)this, 1, (CSound*)this->field_0x240, 0, 0,
									(SOUND_SPATIALIZATION_PARAM*)0x0);
								CActorSound::SetFrequency(this->field_0x1f8, this->pActorSound, 1);
							}
							this->field_0x1d0 = 0.0f;
						})
						iVar3 = 1;
					}
					else {
						if ((msg == 2) || (msg == 3)) {
							SetState(ARAIGNOS_STATE_DIE, -1);
							iVar3 = 1;
						}
						else {
							if (msg != 1) {
								if (msg != 0x1c) {
									iVar3 = CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
									return iVar3;
								}

								if (pSender->typeID == 6) {
									this->field_0x1d4 = pSender;
									return 1;
								}
							}

							iVar3 = 1;
						}
					}
				}
			}
		}
	}
	else {
		iVar3 = 0;
	}

	return iVar3;
}

void CActorAraignos::BehaviourDefault_InitState(int newState)
{
	CCollision* pCVar1;
	edF32VECTOR4 local_10;

	if (newState == 5) {
		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(this->field_0x1f8, 0);
		this->pAnimationController->anmBinMetaAnimator.SetLayerAnimTime(this->field_0x1fc, 0, 0);

		IMPLEMENTATION_GUARD_AUDIO(
		if (this->pActorSound != (CActorSound*)0x0) {
			CActorSound::SoundStart
			(this->pActorSound, (CActor*)this, 1, (CSound*)this->field_0x240, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
			CActorSound::SetFrequency(this->field_0x1f8, this->pActorSound, 1);
		})
	}
	else {
		if (newState == 6) {
			pCVar1 = this->pCollisionData;
			pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffefff;
			this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

			IMPLEMENTATION_GUARD_AUDIO(
			if (this->pActorSound != (CActorSound*)0x0) {
				CActorSound::SoundStart
				(this->pActorSound, (CActor*)this, 0, (CSound*)this->field_0x23c, 0, 0, (SOUND_SPATIALIZATION_PARAM*)0x0)
					;
				CActorSound::SetFrequency(1.0, this->pActorSound, 0);
			})

			local_10.x = this->currentLocation.x;
			local_10.z = this->currentLocation.z;
			local_10.w = this->currentLocation.w;
			local_10.y = this->currentLocation.y + 0.5f;

			this->addOnGenerator.Generate(&local_10);
		}
	}

	return;
}

void CActorAraignos::BehaviourDefault_Manage()
{
	edAnmLayer* peVar1;
	bool bVar2;
	int iVar4;
	float fVar5;
	_msg_hit_param local_f0;
	edF32VECTOR4 eStack96;
	edF32MATRIX4 eStack80;
	undefined4* local_4;
	CAnimation* pAnimation;

	if ((this->pathFollowAbsolute).pActor3C_0x0 == (CPathFollow*)0x0) {
		local_f0.field_0x80 = this->baseLocation;
		SV_UpdatePosition_Rel(&local_f0.field_0x80, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}
	else {
		fVar5 = this->field_0x1d0 - this->field_0x1fc;
		if (fVar5 < 0.0f) {
			fVar5 = 0.0f;
		}

		this->pathFollowAbsolute.ComputeMatrix(fVar5, &eStack80, &eStack96, (S_PATHREADER_POS_INFO*)0x0);

		F32MatrixBuildFromF32VectorAndF32Matrix(&eStack80, &eStack96, &eStack80);
		SV_UpdateMatrix_Rel(&eStack80, 1, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	iVar4 = this->actorState;
	if (iVar4 == 6) {
		pAnimation = this->pAnimationController;
		peVar1 = (pAnimation->anmBinMetaAnimator).aAnimData;

		bVar2 = false;
		if ((peVar1->currentAnimDesc).animType == pAnimation->currentAnimType_0x30) {
			if (peVar1->animPlayState == 0) {
				bVar2 = false;
			}
			else {
				bVar2 = (peVar1->field_0xcc & 2) != 0;
			}
		}

		if ((bVar2) && (bVar2 = pAnimation->FUN_0017f730(), bVar2 == false)) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;

			EvaluateDisplayState();

			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;

			IMPLEMENTATION_GUARD_AUDIO(
			if (this->pActorSound != (CActorSound*)0x0) {
				iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, 0);
				if (iVar4 != 0) {
					CActorSound::SoundStop(this->pActorSound, 0);
				}
				iVar4 = CActorSound::IsInstanceAlive(this->pActorSound, 1);
				if (iVar4 != 0) {
					CActorSound::SoundStop(this->pActorSound, 1);
				}
			})
		}
	}
	else {
		if (iVar4 == 5) {
			this->field_0x1d0 = this->field_0x1d0 + Timer::GetTimer()->cutsceneDeltaTime;
			if (this->field_0x1d4 != (CActor*)0x0) {
				local_f0.damage = this->field_0x200;
				local_f0.flags = 1;
				local_f0.projectileType = 0;

				DoMessage(this->field_0x1d4, MESSAGE_KICKED, &local_f0);

				fVar5 = this->field_0x1f8;

				IMPLEMENTATION_GUARD_AUDIO(
				if (this->pActorSound != (CActorSound*)0x0) {
					CActorSound::SoundStart
					(this->pActorSound, (CActor*)this, 0, (CSound*)this->field_0x238, 0, 0,
						(SOUND_SPATIALIZATION_PARAM*)0x0);
					CActorSound::SetFrequency(fVar5, this->pActorSound, 0);
				})

				this->field_0x1d4 = (CActor*)0x0;
			}
		}
	}

	return;
}

void CActorAraignos::CBhvDefault::Create(ByteCode* pByteCode)
{
	return;
}

void CActorAraignos::CBhvDefault::Manage()
{
	this->pOwner->BehaviourDefault_Manage();

	return;
}

void CActorAraignos::CBhvDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorAraignos*>(pOwner);

	this->pOwner->SetState(ARAIGNOS_STATE_DEFAULT, -1);

	return;
}

void CActorAraignos::CBhvDefault::End(int newBehaviourId)
{
	return;
}

void CActorAraignos::CBhvDefault::InitState(int newState)
{
	this->pOwner->BehaviourDefault_InitState(newState);

	return;
}

void CActorAraignos::CBhvDefault::TermState(int oldState, int newState)
{
	if (oldState == 6) {
		this->pOwner->field_0x1d4 = 0;
	}

	return;
}
