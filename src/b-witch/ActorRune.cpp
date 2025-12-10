#include "ActorRune.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "DlistManager.h"
#include "LevelScheduleManager.h"
#include "CollisionManager.h"
#include "ActorHero.h"
#include "CinematicManager.h"

void CActorRune::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->cinematicId = pByteCode->GetS32();
	this->triggerDistance = pByteCode->GetF32();
	this->scenVarId = pByteCode->GetS32();
	this->fxId = pByteCode->GetS32();
	this->materialIdA = pByteCode->GetS32();
	this->materialIdB = pByteCode->GetS32();

	if (this->materialIdA != -1) {
		CActor::SV_InstallMaterialId(this->materialIdA);
	}

	if (this->materialIdB != -1) {
		CActor::SV_InstallMaterialId(this->materialIdB);
	}

	return;
}

void CActorRune::Draw()
{
	bool bVar1;
	edDList_material* pDlistMaterial;
	int iVar2;
	float width;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	CActor::Draw();

	if (((this->behaviourRune).fxHandle.IsValid()) && ((this->materialIdA != -1 || (this->materialIdB != -1)))) {
		bVar1 = GameDList_BeginCurrent();

		if (bVar1 != false) {
			edDListLoadIdentity();

			if (this->materialIdA != -1) {
				pDlistMaterial = pFileManager->GetMaterialFromId(this->materialIdA, 0);
				edDListUseMaterial(pDlistMaterial);
				edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
				edDListTexCoo2f(0.0f, 0.0f);
				edDListTexCoo2f(1.0f, 1.0f);
				iVar2 = rand();
				width = ((float)iVar2 / 2.147484e+09f) * 0.24f + 0.76f;
				edDListWidthHeight2f(width, width);
				iVar2 = rand();
				edDListColor4u8(0x80, 0x80, 0x80, (byte)(int)(((float)iVar2 / 2.147484e+09f) * 64.0f + 96.0f));
				edDListVertex4f(this->currentLocation.x, this->currentLocation.y, this->currentLocation.z, 0.0f);
				edDListEnd();
			}

			if (this->materialIdB != -1) {
				pDlistMaterial = pFileManager->GetMaterialFromId(this->materialIdB, 0);
				edDListUseMaterial(pDlistMaterial);
				edDListBegin(0.0f, 0.0f, 0.0f, 0xb, 1);
				edDListTexCoo2f(0.0f, 0.0f);
				edDListTexCoo2f(1.0f, 1.0f);
				edDListWidthHeight2f(1.48f, 1.48f);
				edDListColor4u8(0x80, 0x80, 0x80, 0x80);
				edDListVertex4f(this->currentLocation.x, this->currentLocation.y, this->currentLocation.z, 0.0f);
				edDListEnd();
			}
		}

		GameDList_EndCurrent();
	}

	return;
}

CBehaviour* CActorRune::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == RUNE_BEHAVIOUR_DEFAULT) {
		pBehaviour = &this->behaviourRune;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorRune::gStateCfg_RUN[10] =
{
	StateConfig(0x0, 0x0), // RUNE_STATE_DEFAULT_WAIT_FOR_RECEPTACLE
	StateConfig(0x0, 0x0), // RUNE_STATE_DEFAULT_IDLE
	StateConfig(0x0, 0x0), // RUNE_STATE_DEFAULT_CINEMATIC
	StateConfig(0x0, 0x1), // RUNE_STATE_DEFAULT_ACTIVATE
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorRune::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = gStateCfg_RUN + state + -5;
	}

	return pStateConfig;
}

void CActorRune::ChangeManageState(int state)
{
	CBehaviourRune* pBehaviour;

	CActor::ChangeManageState(state);

	if ((this->fxId != -1) && (this->curBehaviourId == RUNE_BEHAVIOUR_DEFAULT)) {
		pBehaviour = (CBehaviourRune*)GetBehaviour(this->curBehaviourId);

		if (state == 0) {
			if (pBehaviour->fxHandle.IsValid()) {
				pBehaviour->fxHandle.Kill();
				pBehaviour->fxHandle.Reset();
			}

			pBehaviour->fxHandle.Reset();
		}
		else {
			if (!pBehaviour->fxHandle.IsValid()) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&pBehaviour->fxHandle, pBehaviour->pOwner->fxId, FX_MATERIAL_SELECTOR_NONE);
			}

			if (pBehaviour->fxHandle.IsValid()) {
				pBehaviour->fxHandle.SetPosition(&pBehaviour->pOwner->currentLocation);
				pBehaviour->fxHandle.SetRotationEuler(&gF32Vector4Zero);
				pBehaviour->fxHandle.SetScale(&pBehaviour->pOwner->scale);
				pBehaviour->fxHandle.Start();
			}
		}
	}

	return;
}

void CActorRune::BehaviourDefault_Manage()
{
	CNewFx* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	int iVar6;
	CCinematic* pCinematic;
	CCollision* pCol;

	iVar6 = this->actorState;
	if (iVar6 == RUNE_STATE_DEFAULT_ACTIVATE) {
		CLevelScheduler::ScenVar_Set(this->scenVarId, 2);

		DoMessage(CActorHero::_gThis, MESSAGE_RECEPTACLE_CHANGED, (MSG_PARAM)this->scenVarId);

		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;
		this->flags = this->flags & 0xfffffbff;

		pCol = this->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7efff;
	}
	else {
		if (iVar6 == RUNE_STATE_DEFAULT_CINEMATIC) {
			pCinematic = g_CinematicManager_0048efc->GetCinematic(this->cinematicId);
			if (pCinematic == (CCinematic*)0x0) {
				SetState(RUNE_STATE_DEFAULT_ACTIVATE, -1);
			}
			else {
				if (pCinematic->state == CS_Stopped) {
					SetState(RUNE_STATE_DEFAULT_ACTIVATE, -1);
				}
			}
		}
		else {
			if (iVar6 == RUNE_STATE_DEFAULT_IDLE) {
				pCol = this->pCollisionData;
				pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7efff;
				fVar2 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
				fVar3 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
				fVar4 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
				if (sqrtf(fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4) < this->triggerDistance) {
					pCinematic = g_CinematicManager_0048efc->GetCinematic(this->cinematicId);
					this->flags = this->flags & 0xffffff7f;
					this->flags = this->flags | 0x20;
					EvaluateDisplayState();

					if (this->fxId != -1) {
						if (this->behaviourRune.fxHandle.IsValid()) {
							this->behaviourRune.fxHandle.Stop();
						}
					}

					if (pCinematic != (CCinematic*)0x0) {
						pCinematic->TryTriggerCutscene(this, 0);
					}

					SetState(RUNE_STATE_DEFAULT_CINEMATIC, -1);
				}
			}
			else {
				if (iVar6 == RUNE_STATE_DEFAULT_WAIT_FOR_RECEPTACLE) {
					pCol = this->pCollisionData;
					pCol->flags_0x0 = pCol->flags_0x0 | 0x81000;

					iVar6 = CLevelScheduler::ScenVar_Get(0xf);
					if (iVar6 != 0) {
						SetState(RUNE_STATE_DEFAULT_IDLE, -1);
					}
				}
			}
		}
	}

	return;
}

void CBehaviourRune::Manage()
{
	this->pOwner->BehaviourDefault_Manage();

	if ((this->pOwner->actorState == RUNE_STATE_DEFAULT_ACTIVATE) && (this->pOwner->fxId != -1)) {
		if (this->fxHandle.IsValid()) {
			this->fxHandle.Stop();
		}
	}

	return;
}

void CBehaviourRune::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	bool bVar1;
	int iVar2;
	CCollision* pCol;
	CActorRune* pRune;

	this->pOwner = static_cast<CActorRune*>(pOwner);

	if (newState == -1) {
		iVar2 = CLevelScheduler::ScenVar_Get(this->pOwner->scenVarId);
		bVar1 = iVar2 == 2;

		if (!bVar1) {
			iVar2 = CLevelScheduler::ScenVar_Get(0x39);
			bVar1 = iVar2 != 0;
		}

		if (bVar1) {
			pRune = this->pOwner;
			pRune->flags = pRune->flags & 0xffffff7f;
			pRune->flags = pRune->flags | 0x20;
			pRune->EvaluateDisplayState();
			pRune->flags = pRune->flags & 0xfffffffd;
			pRune->flags = pRune->flags | 1;
			pRune->flags = pRune->flags & 0xfffffbff;

			pCol = pRune->pCollisionData;
			pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7efff;
		}
		else {
			iVar2 = CLevelScheduler::ScenVar_Get(SCN_ABILITY_RECEPTACLE);

			if (iVar2 == 0) {
				this->pOwner->SetState(RUNE_STATE_DEFAULT_WAIT_FOR_RECEPTACLE, -1);
			}
			else {
				this->pOwner->SetState(RUNE_STATE_DEFAULT_IDLE, -1);
			}
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}
