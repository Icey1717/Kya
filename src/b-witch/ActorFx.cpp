#include "ActorFx.h"
#include "MemoryStream.h"
#include "MiscFunctions.h"
#include "TimeController.h"
#include "PathManager.h"

void CActorFx::Create(ByteCode* pByteCode)
{
	float fVar1;

	CActor::Create(pByteCode);

	fVar1 = pByteCode->GetF32();
	this->field_0x19c = fVar1;
	if ((fVar1 < 0.0f) || (1.0f < fVar1)) {
		this->field_0x19c = 0.0f;
	}

	fVar1 = pByteCode->GetF32();
	this->field_0x1a0 = fVar1;
	if (fVar1 < 0.0f) {
		this->field_0x1a0 = 0.0f;
	}

	return;
}

CBehaviour* CActorFx::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;
	
	if (behaviourType == FX_BEHAVIOUR_UNKNOWN) {
		pBehaviour = new CBhvUnknown;
	}
	else if (behaviourType == FX_BEHAVIOUR_PATH) {
		pBehaviour = &this->bhvPath;
	}
	else if (behaviourType == FX_BEHAVIOUR_SINGLE) {
		pBehaviour = &this->bhvSingle;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

void CActorFx::ChangeManageState(int state)
{
	int iVar1;
	bool bVar3;

	iVar1 = this->curBehaviourId;
	if (iVar1 == 3) {
		if (state == 0) {
			if (this->bhvPath.fxHandle.IsValid()) {
				this->bhvPath.fxHandle.Kill();
				this->bhvPath.fxHandle.Reset();
			}
		}
	}
	else {
		if ((iVar1 == 2) && (state == 0)) {
			if (this->bhvSingle.fxHandle.IsValid()) {
				this->bhvSingle.fxHandle.Kill();
				this->bhvSingle.fxHandle.Reset();
			}
		}
	}

	CActor::ChangeManageState(state);

	return;
}

void CActorFx::CBhvSingle::Create(ByteCode* pByteCode)
{
	int iVar1;

	this->field_0xc = pByteCode->GetS32();
	this->field_0x8 = 0;
	(this->fxHandle).Reset();

	return;
}

void CActorFx::CBhvSingle::Init(CActor* pOwner)
{
	float fVar1;
	CActorFx* pFx;

	this->pOwner = static_cast<CActorFx*>(pOwner);
	pFx = this->pOwner;
	fVar1 = GetRandomTime(pFx->field_0x1a0, pFx->field_0x19c);
	pFx->field_0x1a4 = fVar1;

	return;
}

void CActorFx::CBhvSingle::Term()
{
	if (this->fxHandle.IsValid()) {
		this->fxHandle.Kill();
		this->fxHandle.Reset();
	}

	return;
}

void CActorFx::CBhvSingle::Manage()
{
	int iVar2;
	bool bVar3;
	float fVar5;
	edF32VECTOR4 local_10;
	CActorFx* pFx;

	if (this->field_0xc != -1) {
		pFx = this->pOwner;
		if (pFx->pTiedActor != (CActor*)0x0) {
			local_10 = pFx->baseLocation;
			this->pOwner->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		}

		if (this->fxHandle.IsValid()) {
			this->fxHandle.SetPosition(&pFx->currentLocation);
			this->fxHandle.SetRotationEuler(&pFx->rotationEuler);
			this->fxHandle.SetScale(&pFx->scale);
		}
		else {
			if (this->field_0x8 == 0) {
				this->fxHandle.Kill();
				this->fxHandle.Reset();

				pFx = this->pOwner;
				pFx->flags = pFx->flags & 0xffffff7f;
				pFx->flags = pFx->flags | 0x20;
				pFx->EvaluateDisplayState();
				pFx = this->pOwner;
				pFx->flags = pFx->flags & 0xfffffffd;
				pFx->flags = pFx->flags | 1;
				this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
			}
			else {
				if (0.0f < this->pOwner->field_0x1a4) {
					this->pOwner->field_0x1a4 = this->pOwner->field_0x1a4 - GetTimer()->cutsceneDeltaTime;
				}
				else {
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->fxHandle, this->field_0xc, FX_MATERIAL_SELECTOR_NONE);

					if (this->fxHandle.IsValid()) {
						this->fxHandle.SpatializeOnActor(0xe, this->pOwner, 0);

						this->fxHandle.SetPosition(&pFx->currentLocation);
						this->fxHandle.SetRotationEuler(&pFx->rotationEuler);
						this->fxHandle.SetScale(&pFx->scale);

						this->fxHandle.Start();
					}

					pFx = this->pOwner;
					fVar5 = GetRandomTime(pFx->field_0x1a0, pFx->field_0x19c);
					pFx->field_0x1a4 = fVar5;
				}
			}
		}
	}

	return;
}

void CActorFx::CBhvSingle::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	int iVar2;
	bool bVar3;
	CActorFx* pFx;

	this->pOwner->SetState(0, -1);

	(this->fxHandle).id = 0;
	(this->fxHandle).pFx = (CNewFx*)0x0;
	if ((this->pOwner->actorFieldS & 1) == 0) {
		bVar3 = true;
		if (this->field_0x8 != 1) {
			if (this->field_0x8 == 0) {
				if (this->fxHandle.IsValid()) {
					this->fxHandle.Kill();
					this->fxHandle.Reset();
				}
			}
			else {
				if (this->fxHandle.IsValid()) {
					this->fxHandle.Stop();
				}
			}

			this->field_0x8 = 1;
			pFx = this->pOwner;
			pFx->flags = pFx->flags & 0xffffff5f;
			pFx->EvaluateDisplayState();
			this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
			this->pOwner->flags = this->pOwner->flags | 0x400;
		}
	}

	return;
}

int CActorFx::CBhvSingle::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	byte bVar1;
	int iVar3;
	bool bVar4;
	CActorFx* pFx;

	if (msg == 0xe) {
		bVar1 = this->field_0x8;
		if (bVar1 == 0) {
			bVar4 = true;
			if (this->fxHandle.IsValid()) {
				this->fxHandle.Kill();
				this->fxHandle.Reset();
			}

			this->field_0x8 = 1;
			pFx = this->pOwner;
			pFx->flags = pFx->flags & 0xffffff5f;
			pFx->EvaluateDisplayState();
			this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
			this->pOwner->flags = this->pOwner->flags | 0x400;
		}
		else {
			if (bVar1 != 0) {
				if (bVar1 == 0) {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Kill();
						this->fxHandle.Reset();
					}
				}
				else {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Stop();
					}
				}
				this->field_0x8 = 0;
			}
		}
	}
	else {
		if (msg == 0x10) {
			if (this->field_0x8 != 0) {
				if (this->field_0x8 == 0) {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Kill();
						this->fxHandle.Reset();
					}
				}
				else {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Stop();
					}
				}
				this->field_0x8 = 0;
			}
		}
		else {
			if (msg == 0xf) {
				bVar4 = true;
				if (this->field_0x8 != 1) {
					if (this->field_0x8 == 0) {
						if (this->fxHandle.IsValid()) {
							this->fxHandle.Kill();
							this->fxHandle.Reset();
						}
					}
					else {
						if (this->fxHandle.IsValid()) {
							this->fxHandle.Stop();
						}
					}

					this->field_0x8 = 1;
					pFx = this->pOwner;
					pFx->flags = pFx->flags & 0xffffff5f;
					pFx->EvaluateDisplayState();
					this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
					this->pOwner->flags = this->pOwner->flags | 0x400;
				}
			}
		}
	}

	return 0;
}

void CActorFx::CBhvPath::Create(ByteCode* pByteCode)
{
	CPathManager* pCVar1;
	int iVar2;
	CPathFollow* pCVar3;

	this->field_0x14 = pByteCode->GetS32();
	pCVar1 = CScene::ptable.g_PathManager_004516a0;
	iVar2 = pByteCode->GetS32();
	pCVar3 = (CPathFollow*)0x0;
	if (iVar2 != -1) {
		pCVar3 = pCVar1->aPathFollow + iVar2;
	}

	this->pPathFollow = pCVar3;
	this->field_0xc = 0;
	this->fxHandle.Reset();

	return;
}

void CActorFx::CBhvPath::Init(CActor* pOwner)
{
	float fVar1;
	CActorFx* pFx;

	this->pOwner = static_cast<CActorFx*>(pOwner);
	pFx = this->pOwner;
	fVar1 = GetRandomTime(pFx->field_0x1a0, pFx->field_0x19c);
	pFx->field_0x1a4 = fVar1;

	return;
}

void CActorFx::CBhvPath::Term()
{
	if (this->fxHandle.IsValid()) {
		this->fxHandle.Kill();
		this->fxHandle.Reset();
	}

	return;
}

void CActorFx::CBhvPath::Manage()
{
	int iVar1;
	bool bVar2;
	uint uVar5;
	float fVar6;
	edF32VECTOR4 local_10;
	CActorFx* pFx;
	CNewFx* pCVar4;

	if (this->field_0x14 != -1) {
		pFx = this->pOwner;
		if (pFx->pTiedActor != (CActor*)0x0) {
			local_10 = pFx->baseLocation;
			this->pOwner->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		}

		if (this->fxHandle.IsValid()) {
			this->fxHandle.SetPosition(&pFx->currentLocation);
			this->fxHandle.SetRotationEuler(&pFx->rotationEuler);
			this->fxHandle.SetScale(&pFx->scale);

			pCVar4 = (this->fxHandle).pFx;
			uVar5 = (this->fxHandle).id & 0x7fffffff;
			if (((pCVar4 == (CNewFx*)0x0) || (uVar5 == 0)) || (uVar5 != pCVar4->id)) {
				pCVar4 = (CNewFx*)0x0;
			}

			if (pCVar4 != (CNewFx*)0x0) {
				pCVar4->Manage();
			}
		}
		else {
			if (this->field_0xc == 0) {
				this->fxHandle.Kill();
				this->fxHandle.Reset();

				pFx = this->pOwner;
				pFx->flags = pFx->flags & 0xffffff7f;
				pFx->flags = pFx->flags | 0x20;
				pFx->EvaluateDisplayState();
				pFx = this->pOwner;
				pFx->flags = pFx->flags & 0xfffffffd;
				pFx->flags = pFx->flags | 1;
				this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
			}
			else {
				if (0.0f < this->pOwner->field_0x1a4) {
					this->pOwner->field_0x1a4 = this->pOwner->field_0x1a4 - GetTimer()->cutsceneDeltaTime;
				}
				else {
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->fxHandle, this->field_0x14, FX_MATERIAL_SELECTOR_NONE);
	
					if (this->fxHandle.IsValid()) {
						this->fxHandle.SpatializeOnActor(0xe, this->pOwner, 0);

						this->fxHandle.SetPosition(&pFx->currentLocation);
						this->fxHandle.SetRotationEuler(&pFx->rotationEuler);
						this->fxHandle.SetScale(&pFx->scale);

						this->fxHandle.Hide();

						pCVar4 = (this->fxHandle).pFx;
						if (((pCVar4 != (CNewFx*)0x0) && (iVar1 = (this->fxHandle).id, iVar1 != 0)) && (iVar1 == pCVar4->id)) {
							pCVar4->flags = pCVar4->flags & 0xfffffffd;
						}

						pCVar4 = (this->fxHandle).pFx;
						if (((pCVar4 != (CNewFx*)0x0) && (iVar1 = (this->fxHandle).id, iVar1 != 0)) && (iVar1 == pCVar4->id)) {
							pCVar4->flags = pCVar4->flags & 0xfffffffe;
						}

						this->fxHandle.Start();
					}

					pFx = this->pOwner;
					fVar6 = GetRandomTime(pFx->field_0x1a0, pFx->field_0x19c);
					pFx->field_0x1a4 = fVar6;
				}
			}
		}
	}

	return;
}

void CActorFx::CBhvPath::Draw()
{}

void CActorFx::CBhvPath::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	int iVar2;
	bool bVar3;
	CActorFx* pFx;

	this->pOwner->SetState(0, -1);

	this->fxHandle.Reset();

	if ((this->pOwner->actorFieldS & 1) == 0) {
		bVar3 = true;
		if (this->field_0xc != 1) {
			if (this->field_0xc == 0) {
				if (this->fxHandle.IsValid()) {
					this->fxHandle.Kill();
					this->fxHandle.Reset();
				}
			}
			else {
				if (this->fxHandle.IsValid()) {
					this->fxHandle.Stop();
				}
			}

			this->field_0xc = 1;
			pFx = this->pOwner;
			pFx->flags = pFx->flags & 0xffffff5f;
			pFx->EvaluateDisplayState();
			this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
			this->pOwner->flags = this->pOwner->flags | 0x400;
		}
	}

	return;
}

int CActorFx::CBhvPath::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	byte bVar1;
	CNewFx* pCVar2;
	int iVar3;
	CActorFx* pCVar4;
	bool bVar5;

	if (msg == 0xe) {
		bVar1 = this->field_0xc;
		if (bVar1 == 0) {
			bVar5 = true;
			if (this->fxHandle.IsValid()) {
				this->fxHandle.Kill();
				this->fxHandle.Reset();
			}

			this->field_0xc = 1;
			pCVar4 = this->pOwner;
			pCVar4->flags = pCVar4->flags & 0xffffff5f;
			pCVar4->EvaluateDisplayState();
			this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
			this->pOwner->flags = this->pOwner->flags | 0x400;
		}
		else {
			if (bVar1 != 0) {
				if (bVar1 == 0) {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Kill();
						this->fxHandle.Reset();
					}
				}
				else {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Stop();
					}
				}
				this->field_0xc = 0;
			}
		}
	}
	else {
		if (msg == 0x10) {
			if (this->field_0xc != 0) {
				if (this->field_0xc == 0) {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Kill();
						this->fxHandle.Reset();
					}
				}
				else {
					if (this->fxHandle.IsValid()) {
						this->fxHandle.Stop();
					}
				}
				this->field_0xc = 0;
			}
		}
		else {
			if (msg == 0xf) {
				bVar5 = true;
				if (this->field_0xc != 1) {
					if (this->field_0xc == 0) {
						if (this->fxHandle.IsValid()) {
							this->fxHandle.Kill();
							this->fxHandle.Reset();
						}
					}
					else {
						if (this->fxHandle.IsValid()) {
							this->fxHandle.Stop();
						}
					}

					this->field_0xc = 1;
					pCVar4 = this->pOwner;
					pCVar4->flags = pCVar4->flags & 0xffffff5f;
					pCVar4->EvaluateDisplayState();
					this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
					this->pOwner->flags = this->pOwner->flags | 0x400;
				}
			}
		}
	}

	return 0;
}

void CActorFx::CBhvUnknown::Create(ByteCode* pByteCode)
{
	int* piVar1;

	piVar1 = reinterpret_cast<int*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	this->field_0x4 = piVar1;

	return;
}

void CActorFx::CBhvUnknown::Init(CActor * pOwner)
{
	this->pOwner = pOwner;

	return;
}

void CActorFx::CBhvUnknown::Manage()
{
	return;
}

int CActorFx::CBhvUnknown::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}
