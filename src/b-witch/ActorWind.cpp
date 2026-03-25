#include "ActorWind.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "MathOps.h"
#include "DlistManager.h"
#include "EventManager.h"
#include "ActorFactory.h"
#include "ActorAutonomous.h"
#include "WayPoint.h"
#include "CameraViewManager.h"
#include "CameraGame.h"
#include "ed3D/ed3DG2D.h"
#include "edDList/edDList.inl"

void CActorWind::Create(ByteCode* pByteCode)
{
	byte bVar1;
	int* piVar2;
	char* pcVar3;
	bool bVar4;
	CAudioManager* pGVar5;
	uint uVar6;
	int iVar7;
	uint uVar8;
	float fVar9;

	CActor::Create(pByteCode);
	this->field_0x160 = pByteCode->GetU32();
	this->maxWind = pByteCode->GetF32();
	this->field_0x168 = pByteCode->GetF32();
	this->field_0x16c = pByteCode->GetF32();
	this->field_0x174 = pByteCode->GetF32();
	this->field_0x180 = pByteCode->GetF32();
	this->field_0x184 = pByteCode->GetF32();
	this->field_0x188 = pByteCode->GetF32();
	this->field_0x18c = pByteCode->GetF32();
	this->field_0x178 = pByteCode->GetF32();
	this->field_0x17c = pByteCode->GetF32();

	this->field_0x190 = pByteCode->GetU32();

	if (this->field_0x168 <= 0.0f) {
		this->field_0x160 = this->field_0x160 | 1;
		this->field_0x168 = 0.001f;
	}

	bVar4 = true;
	if ((((this->field_0x160 & 0x20) == 0) && (bVar4 = false, (this->field_0x160 & 0x40) == 0)) &&
		(bVar4 = true, 0.17398384f <= fabs(this->rotationQuat.y))) {
		bVar4 = false;
	}

	if (bVar4) {
		this->field_0x160 = this->field_0x160 | 1;
	}
	else {
		SnapOrientation(0.01745329f, 0.01745329f, 0.01745329f);
	}

	pGVar5 = CScene::ptable.g_AudioManager_00451698;
	uVar6 = pByteCode->GetU32();
	this->activationZone.index = pByteCode->GetS32();

	this->pActorStreamRef = S_ACTOR_STREAM_REF::Create(pByteCode);

	this->materialId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(this->materialId);

	this->field_0x1c4.index = pByteCode->GetS32();
	this->field_0x1c8 = pByteCode->GetU32();

	if (this->field_0x1c8.r < 0x80) {
		this->field_0x1c8.r = this->field_0x1c8.r << 1;
	}
	else {
		this->field_0x1c8.r = 0xff;
	}

	bVar1 = this->field_0x1c8.g;
	if (bVar1 < 0x80) {
		this->field_0x1c8.g = bVar1 << 1;
	}
	else {
		this->field_0x1c8.g = 0xff;
	}

	bVar1 = this->field_0x1c8.b;
	if (bVar1 < 0x80) {
		this->field_0x1c8.b = bVar1 << 1;
	}
	else {
		this->field_0x1c8.b = 0xff;
	}

	this->field_0x1cc.index = pByteCode->GetS32();
	this->sectorObj.Create(pByteCode);

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		this->field_0x1d0.Create(pByteCode);
		this->field_0x1d8.Create(pByteCode);
		this->field_0x1e0.Create(pByteCode);
	}

	this->field_0x170 = 4.0f;
	this->field_0x194 = pByteCode->GetF32();
	this->field_0x198 = pByteCode->GetF32();
	this->field_0x19c = pByteCode->GetF32();
	this->field_0x1a0 = pByteCode->GetF32();
	this->field_0x1a4 = pByteCode->GetF32();

	this->pWayPointRef.index = pByteCode->GetS32();

	this->field_0x1ac = pByteCode->GetF32();
	this->field_0x1b0 = pByteCode->GetF32();

	if (100.0f < this->field_0x16c) {
		this->field_0x16c = 100.0f;
	}
	else {
		if (this->field_0x16c < 0.0f) {
			this->field_0x16c = 0.0f;
		}
	}

	this->field_0x1fc = this->field_0x16c + fabs(this->field_0x194);
	this->field_0x200 = this->field_0x16c - fabs(this->field_0x198);
	if (this->field_0x200 < 0.0f) {
		this->field_0x200 = 0.0f;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	this->pSoundWind = pGVar5->GetWindSound();)

	this->aFxWind = (CFxWind*)0x0;
	this->nbFxWind = 0;
	this->field_0x1e8 = (edF32MATRIX4*)0x0;
	this->aAppliedToActorFlags = (int*)0x0;

	return;
}

void CActorWind::Init()
{
	ed_zone_3d* peVar1;
	CEventManager* pCVar2;
	void* pvVar3;
	uint uVar4;
	int* pBase;
	CFxWind* pCVar5;
	int iVar6;
	Timer* pTVar7;
	S_NTF_TARGET_STREAM_REF* pSVar8;
	int iVar9;
	S_STREAM_REF<CActor>* pSVar10;
	S_ACTOR_STREAM_REF* pSVar12;
	int iVar11;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	pCVar2 = CScene::ptable.g_EventManager_006f5080;
	CActor::Init();
	pSVar10 = this->pActorStreamRef->aEntries;
	for (iVar11 = this->pActorStreamRef->entryCount; iVar11 != 0; iVar11 = iVar11 + -1) {
		pSVar10->Init();
		pSVar10 = pSVar10 + 1;
	}

	this->activationZone.Init();
	this->field_0x1c4.Init();

	if (this->field_0x1e8 != (edF32MATRIX4*)0x0) {
		peVar1 = (this->activationZone).Get();
		if (peVar1 != (ed_zone_3d*)0x0) {
			peVar1->pMatrix = STORE_POINTER(this->field_0x1e8);
		}

		peVar1 = (this->field_0x1c4).Get();
		if (peVar1 != (ed_zone_3d*)0x0) {
			peVar1->pMatrix = STORE_POINTER(this->field_0x1e8);
		}
	}

	this->sectorObj.Init(this->sectorId);

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		this->field_0x1d0.Init();
		this->field_0x1d8.Init();
		this->field_0x1e0.Init();
	}

	edF32Matrix4FromEulerSoft(&eStack64, &this->rotationEuler.xyz, "XYZ");
	eStack64.rowT = this->baseLocation;
	edF32Matrix4GetInverseOrthoHard(&eStack128, &eStack64);
	pSVar12 = this->pActorStreamRef;
	iVar11 = 0;
	if (pSVar12 != (S_ACTOR_STREAM_REF*)0x0) {
		iVar11 = pSVar12->entryCount;
	}
	if (0 < iVar11) {
		iVar11 = 0;
		if (pSVar12 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar11 = pSVar12->entryCount;
		}

		this->aAppliedToActorFlags = new int[iVar11];
	}

	this->field_0x21c = (this->field_0x1c4).Get();
	if (this->field_0x21c == (ed_zone_3d*)0x0) {
		this->field_0x21c = (this->activationZone).Get();
	}

	if ((this->field_0x160 & 0x80) != 0) {
		this->field_0x21c = (ed_zone_3d*)0x0;
	}

	if (this->field_0x21c != (ed_zone_3d*)0x0) {
		uVar4 = edEventGetChunkZoneNbInclusivePrimitives(pCVar2->activeChunkId, this->field_0x21c);
		this->nbFxWind = uVar4;
		uVar4 = this->nbFxWind;
		if (uVar4 != 0) {
			this->aFxWind = new CFxWind[uVar4];
			uVar4 = this->nbFxWind;
			pCVar5 = this->aFxWind;

			iVar11 = 0;
			if (0 < (int)uVar4) {
				do {
					pCVar5->Create(this, iVar11);
					uVar4 = this->nbFxWind;
					iVar11 = iVar11 + 1;
					pCVar5 = pCVar5 + 1;
				} while (iVar11 < (int)uVar4);
			}

			pCVar5 = this->aFxWind;
			iVar11 = 0;
			if (0 < (int)uVar4) {
				do {
					pCVar5->Init(&eStack128);
					iVar11 = iVar11 + 1;
					pCVar5 = pCVar5 + 1;
				} while (iVar11 < (int)this->nbFxWind);
			}

			ReComputeBoundingSphere(&eStack128);
		}
	}

	this->pWayPointRef.Init();
	this->field_0x1cc.Init();

	this->field_0x1f0 = 0;

	iVar9 = 0;
	while (true) {
		if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = 0;
		}
		else {
			iVar6 = this->pActorStreamRef->entryCount;
		}

		if (iVar6 <= iVar9) break;

		this->aAppliedToActorFlags[iVar9] = 0x0;
		iVar9 = iVar9 + 1;
	}

	ComputeSpeedsInterpoltation();
	this->field_0x1f8 = -1.0f;
	pTVar7 = GetTimer();
	this->time_0x214 = pTVar7->scaledTotalTime;

	ResetTiming();

	return;
}

void CActorWind::Term()
{
	CActor::Term();
	this->sectorObj.Term(this->sectorId);
}

void CActorWind::Draw()
{
	CEventManager* pEventManager;
	int iVar1;
	CFxWind* pCVar2;
	float fVar3;

	pEventManager = CScene::ptable.g_EventManager_006f5080;

	pCVar2 = this->aFxWind;
	if (pCVar2 == (CFxWind*)0x0) {
		if (this->field_0x21c != (ed_zone_3d*)0x0) {
			IMPLEMENTATION_GUARD(
			fVar3 = edFIntervalLERP(this->curWindAlpha, 0.0f, this->maxWind, 10.0f, 50.0f);
			edEventDrawZone(pEventManager, this->field_0x21c,
				(uint) * (byte*)((int)&this->field_0x1c8 + 2) |
				(uint) * (byte*)((int)&this->field_0x1c8 + 1) << 8 |
				(int)fVar3 << 0x18 | (uint) * (byte*)&this->field_0x1c8 << 0x10, 0x1fff0000);)
		}
	}
	else {
		iVar1 = 0;
		if (0 < this->nbFxWind) {
			do {
				pCVar2->Draw();
				iVar1 = iVar1 + 1;
				pCVar2 = pCVar2 + 1;
			} while (iVar1 < this->nbFxWind);
		}
	}

	fVar3 = edFIntervalLERP(this->curWindAlpha, 0.0f, this->maxWind, 0.0f, 128.0f);
	this->sectorObj.SetAlpha(fVar3);

	if (this->curWindAlpha == 0.0f) {
		this->sectorObj.SetHiddenOn();
	}
	else {
		this->sectorObj.SetHiddenOff();
	}
	return;
}

void CActorWind::Reset()
{
	int iVar1;
	Timer* pTVar2;
	S_NTF_TARGET_STREAM_REF* pSVar3;
	int iVar4;
	CFxWind* pWindFx;
	int iVar5;

	CActor::Reset();

	this->field_0x1f0 = 0;
	iVar5 = 0;
	while (true) {
		if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
			iVar1 = 0;
		}
		else {
			iVar1 = this->pActorStreamRef->entryCount;
		}

		if (iVar1 <= iVar5) break;

		this->aAppliedToActorFlags[iVar5] = 0;

		iVar5 = iVar5 + 1;
	}

	ComputeSpeedsInterpoltation();

	this->field_0x1f8 = -1.0f;
	pTVar2 = GetTimer();
	this->time_0x214 = pTVar2->scaledTotalTime;
	ResetTiming();

	pWindFx = this->aFxWind;
	iVar4 = 0;
	if (0 < this->nbFxWind) {
		do {
			pWindFx->Reset();
			iVar4 = iVar4 + 1;
			pWindFx = pWindFx + 1;
		} while (iVar4 < this->nbFxWind);
	}

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		this->field_0x1d0.Reset(this);
		this->field_0x1d8.Reset(this);
		this->field_0x1e0.Reset(this);
	}

	return;
}

void CActorWind::SectorChange(int oldSectorId, int newSectorId)
{
	CFxWind* pFxWind;
	int iVar1;

	iVar1 = 0;
	pFxWind = this->aFxWind;
	if (0 < this->nbFxWind) {
		do {
			pFxWind->SectorChange(oldSectorId, newSectorId);
			iVar1 = iVar1 + 1;
			pFxWind = pFxWind + 1;
		} while (iVar1 < this->nbFxWind);
	}
	return;
}

struct S_SAVE_CLASS_WIND
{
	int field_0x0;
};

void CActorWind::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_WIND* pSaveData = reinterpret_cast<S_SAVE_CLASS_WIND*>(pData);

	if ((this->flags & 0x2000001) == 0) {
		pSaveData->field_0x0 = 1;
	}
	else {
		pSaveData->field_0x0 = 0;
	}

	return;
}

void CActorWind::LoadContext(void* pData, uint mode, uint maxSize)
{
	int iVar1;
	int iVar4;

	S_SAVE_CLASS_WIND* pSaveData = reinterpret_cast<S_SAVE_CLASS_WIND*>(pData);

	if (mode == 1) {
		iVar1 = pSaveData->field_0x0;

		if (iVar1 == 0) {
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->sectorObj.SetHiddenOn();
		}
		else {
			this->flags = this->flags & 0xfffffffc;
			this->flags = this->flags & 0xffffff5f;
			EvaluateDisplayState();
			ResetTiming();
			this->sectorObj.SetHiddenOff();
		}

		iVar4 = 0;
		if (0 < this->nbFxWind) {
			do {
				CFxWind* pFxWind = this->aFxWind + iVar4;
				if (iVar1 == 0) {
					pFxWind->flags_0x54 = pFxWind->flags_0x54 & ~FXWIND_FLAG_ACTIVE;
				}
				else {
					pFxWind->flags_0x54 = pFxWind->flags_0x54 | FXWIND_FLAG_ACTIVE;
				}

				iVar4 = iVar4 + 1;
			} while (iVar4 < this->nbFxWind);
		}
	}

	return;
}

CBehaviour* CActorWind::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourWind;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig _gStateCfg_WND[6] =
{
	StateConfig(0x0, 0x200),
	StateConfig(0x0, 0x200),
	StateConfig(0x0, 0x100),
	StateConfig(0x0, 0x100),
	StateConfig(0x0, 0x100),
	StateConfig(0x0, 0x1),
};

StateConfig* CActorWind::GetStateCfg(int state)
{
	StateConfig* pAVar1;

	if (state < 5) {
		pAVar1 = CActor::GetStateCfg(state);
	}
	else {
		pAVar1 = _gStateCfg_WND + state + -5;
	}
	return pAVar1;
}

void CActorWind::ChangeManageState(int state)
{
	int maxEntryIndex;
	int curActorIndex;
	_wind_param_in windParamsMsg;
	_wind_param_in* pWindParamsMsg;

	CActor::ChangeManageState(state);

	if (state == 0) {
		curActorIndex = 0;
		while (true) {
			if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
				maxEntryIndex = 0;
			}
			else {
				maxEntryIndex = this->pActorStreamRef->entryCount;
			}

			if (maxEntryIndex <= curActorIndex) break;

			if (this->aAppliedToActorFlags[curActorIndex] != 0x0) {
				windParamsMsg.field_0x0 = this->field_0x16c;
				windParamsMsg.field_0x4 = this->field_0x170;
				windParamsMsg.field_0x8 = 1;

				if ((((this->field_0x160 & 0x20) == 0) && (windParamsMsg.field_0x8 = 0, (this->field_0x160 & 0x40) == 0)) &&
					(windParamsMsg.field_0x8 = 1, 0.17398384f <= fabs(this->rotationQuat.y))) {
					windParamsMsg.field_0x8 = 0;
				}

				windParamsMsg.field_0x10 = this->field_0x1fc;
				windParamsMsg.field_0x14 = this->field_0x200;
				windParamsMsg.field_0x18 = this->field_0x19c;
				windParamsMsg.field_0x1c = this->field_0x1a0;
				windParamsMsg.field_0x20 = this->field_0x1a4;
				windParamsMsg.field_0x24 = (uint)((this->field_0x160 & 0x400) != 0);
				windParamsMsg.pWayPoint = this->pWayPointRef.Get();
				if (windParamsMsg.pWayPoint == (CWayPoint*)0x0) {
					windParamsMsg.pWayPoint = (CWayPoint*)0x0;
				}

				pWindParamsMsg = &windParamsMsg;
				windParamsMsg.field_0x2c = this->field_0x1ac;
				windParamsMsg.field_0x30 = this->field_0x1b0;
				DoMessage(this->pActorStreamRef->aEntries[curActorIndex].Get(), MESSAGE_LEAVE_WIND, pWindParamsMsg);
				this->aAppliedToActorFlags[curActorIndex] = 0x0;
			}

			curActorIndex = curActorIndex + 1;
		}
	}

	return;
}

void CActorWind::ChangeVisibleState(int state)
{
	int iVar1;
	int iVar2;

	CActor::ChangeVisibleState(state);

	iVar2 = 0;
	if (0 < (int)this->nbFxWind) {
		iVar1 = 0;
		do {
			this->aFxWind[iVar1].ChangeVisibleState(state);
			iVar2 = iVar2 + 1;
		} while (iVar2 < (int)this->nbFxWind);
	}

	return;
}

void CActorWind::TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5)
{
	CActor::TieToActor(pTieActor, carryMethod, param_4, param_5);
	this->field_0x1e8 = param_5;

	return;
}

int CActorWind::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	uint uVar5;

	if (msg == 0xe) {
		uVar5 = this->flags & 0x2000001;
		if (uVar5 == 0) {
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->sectorObj.SetHiddenOn();
		}
		else {
			this->flags = this->flags & 0xfffffffc;
			this->flags = this->flags & 0xffffff5f;
			EvaluateDisplayState();
			ResetTiming();
			this->sectorObj.SetHiddenOff();
		}

		iVar4 = 0;
		if (0 < this->nbFxWind) {
			do {
				CFxWind* pFxWind = this->aFxWind + iVar4;
				if (iVar1 == 0) {
					pFxWind->flags_0x54 = pFxWind->flags_0x54 & ~FXWIND_FLAG_ACTIVE;
				}
				else {
					pFxWind->flags_0x54 = pFxWind->flags_0x54 | FXWIND_FLAG_ACTIVE;
				}

				iVar4 = iVar4 + 1;
			} while (iVar4 < this->nbFxWind);
		}

		iVar1 = 1;
	}
	else {
		if (msg == 0xf) {
			this->flags = this->flags & 0xfffffffc;
			this->flags = this->flags & 0xffffff5f;
			EvaluateDisplayState();
			ResetTiming();
			this->sectorObj.SetHiddenOff();

			iVar4 = 0;
			if (0 < this->nbFxWind) {
				do {
					CFxWind* pFxWind = this->aFxWind + iVar4;
					pFxWind->flags_0x54 = pFxWind->flags_0x54 | FXWIND_FLAG_ACTIVE;

					iVar4 = iVar4 + 1;
				} while (iVar4 < this->nbFxWind);
			}

			iVar1 = 1;
		}
		else {
			if ((msg == 0x10) || (msg == 0x5d)) {
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
				this->flags = this->flags & 0xffffff7f;
				this->flags = this->flags | 0x20;
				EvaluateDisplayState();
				this->sectorObj.SetHiddenOn();

				iVar4 = 0;
				if (0 < this->nbFxWind) {
					do {
						CFxWind* pFxWind = this->aFxWind + iVar4;
						pFxWind->flags_0x54 = pFxWind->flags_0x54 & ~FXWIND_FLAG_ACTIVE;

						iVar4 = iVar4 + 1;
					} while (iVar4 < this->nbFxWind);
				}

				iVar1 = 1;
			}
			else {
				if (msg == MESSAGE_TIED) {
					_msg_tied_params* pTiedParams = reinterpret_cast<_msg_tied_params*>(pMsgParam);
					iVar1 = pTiedParams->bTied;

					if (iVar1 == 0) {
						this->flags = this->flags & 0xfffffffd;
						this->flags = this->flags | 1;
						this->flags = this->flags & 0xffffff7f;
						this->flags = this->flags | 0x20;
						EvaluateDisplayState();

						this->sectorObj.SetHiddenOn();
					}
					else {
						this->flags = this->flags & 0xfffffffc;
						this->flags = this->flags & 0xffffff5f;
						EvaluateDisplayState();
						ResetTiming();
						this->sectorObj.SetHiddenOff();
					}

					iVar4 = 0;
					if (0 < this->nbFxWind) {
						do {
							CFxWind* pFxWind = this->aFxWind + iVar4;
							if (iVar1 == 0) {
								pFxWind->flags_0x54 = pFxWind->flags_0x54 & ~FXWIND_FLAG_ACTIVE;
							}
							else {
								pFxWind->flags_0x54 = pFxWind->flags_0x54 | FXWIND_FLAG_ACTIVE;
							}

							iVar4 = iVar4 + 1;
						} while (iVar4 < this->nbFxWind);
					}

					iVar1 = 1;
				}
				else {
					iVar1 = CActor::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}

	return iVar1;
}

void CActorWind::BehaviourWind_Manage()
{
	int iVar1;
	CEventManager* pEventManager;
	bool bVar3;
	int iVar4;
	uint uVar5;
	StateConfig* pAVar8;
	CActor* pActor;
	CFxWind* pFxWind;
	float fVar11;
	edF32VECTOR4 local_20;
	float curTime;

	if (this->pTiedActor != (CActor*)0x0) {
		local_20 = this->baseLocation;
		SV_UpdatePosition_Rel(&local_20, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	curTime = ComputeCurTime();
	if (curTime != -1.0f) {
		SetState(ComputeStateFromTime(&curTime), -1);

		switch (this->actorState) {
		case WIND_STATE_IDLE:
			SV_UpdateValue(0.0f, this->field_0x20c, &this->curWindAlpha);
			break;
		case WIND_STATE_DORMANT:
			this->curWindAlpha = 0.0f;
			break;
		case WIND_STATE_FADE_IN:
			SV_UpdateValue(this->maxWind, this->field_0x210, &this->curWindAlpha);
			break;
		case WIND_STATE_ACTIVE:
			this->curWindAlpha = this->maxWind;
			break;
		case WIND_STATE_FADE_OUT:
			SV_UpdateValue(0.0f, this->field_0x208, &this->curWindAlpha);
			break;
		case WIND_STATE_COOLDOWN:
			this->curWindAlpha = 0.0f;
		}

		pEventManager = CScene::ptable.g_EventManager_006f5080;
		if ((this->activationZone).Get() != (ed_zone_3d*)0x0) {
			fVar11 = edFIntervalLERP(this->curWindAlpha, 0.0f, this->maxWind, 0.1f, this->field_0x168);
			int curActorIndex = 0;
			while (true) {
				int entryCount;
				if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
					entryCount = 0x0;
				}
				else {
					entryCount = this->pActorStreamRef->entryCount;
				}

				if (entryCount <= curActorIndex) break;

				pActor = this->pActorStreamRef->aEntries[curActorIndex].Get();

				if (((pActor != (CActor*)0x0) && ((pActor->flags & 4) != 0)) && ((CActorFactory::gClassProperties[pActor->typeID].flags & 4) != 0)) {
					uint zoneResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, (this->activationZone).Get(), &pActor->currentLocation, 0);

					bVar3 = pActor->IsKindOfObject(2);
					if (bVar3 != false) {
						uint topZoneResult = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, (this->activationZone).Get(), pActor->GetBottomPosition(), 0);
						zoneResult = zoneResult | topZoneResult;
					}

					if ((zoneResult & 1) == 0) {
						if (((zoneResult & 2) != 0) && (aAppliedToActorFlags[curActorIndex] != 0)) {
							_wind_param_in windParamMsg;
							windParamMsg.field_0x0 = this->field_0x16c;
							windParamMsg.field_0x4 = this->field_0x170;

							windParamMsg.field_0x8 = 1;

							if (((this->field_0x160 & 0x20) == 0) && ((windParamMsg.field_0x8 = 0, (this->field_0x160 & 0x40) == 0 &&
									(windParamMsg.field_0x8 = 1, 0.17398384f <= fabs(this->rotationQuat.y))))) {
								windParamMsg.field_0x8 = 0;
							}

							windParamMsg.field_0xc = 0;
							if ((this->field_0x19c < -1.0f) && (this->field_0x1a0 < -1.0f)) {
								windParamMsg.field_0xc = 1;
							}

							windParamMsg.field_0x10 = this->field_0x1fc;
							windParamMsg.field_0x14 = this->field_0x200;
							windParamMsg.field_0x18 = this->field_0x19c;
							windParamMsg.field_0x1c = this->field_0x1a0;
							windParamMsg.field_0x20 = this->field_0x1a4;
							windParamMsg.field_0x24 = (uint)((this->field_0x160 & 0x400) != 0);

							windParamMsg.pWayPoint = this->pWayPointRef.Get();
							if (windParamMsg.pWayPoint == (CWayPoint*)0x0) {
								windParamMsg.pWayPoint = (CWayPoint*)0x0;
							}

							_wind_param_in* local_c = &windParamMsg;
							windParamMsg.field_0x2c = this->field_0x1ac;
							windParamMsg.field_0x30 = this->field_0x1b0;

							DoMessage(pActor, MESSAGE_LEAVE_WIND, local_c);
							aAppliedToActorFlags[curActorIndex] = 0;
						}
					}
					else {
						if (aAppliedToActorFlags[curActorIndex] == 0) {
							_wind_param_in windParamMsg;
							windParamMsg.field_0x0 = this->field_0x16c;
							windParamMsg.field_0x4 = this->field_0x170;

							windParamMsg.field_0x8 = 1;
							if ((((this->field_0x160 & 0x20) == 0) && (windParamMsg.field_0x8 = 0, (this->field_0x160 & 0x40) == 0)) &&
								(windParamMsg.field_0x8 = 1, 0.17398384f <= fabs(this->rotationQuat.y))) {
								windParamMsg.field_0x8 = 0;
							}

							windParamMsg.field_0xc = 0;
							if ((this->field_0x19c < -1.0f) && (this->field_0x1a0 < -1.0f)) {
								windParamMsg.field_0xc = 1;
							}

							windParamMsg.field_0x10 = this->field_0x1fc;
							windParamMsg.field_0x14 = this->field_0x200;
							windParamMsg.field_0x18 = this->field_0x19c;
							windParamMsg.field_0x1c = this->field_0x1a0;
							windParamMsg.field_0x20 = this->field_0x1a4;
							windParamMsg.field_0x24 = (uint)((this->field_0x160 & 0x400) != 0);
							windParamMsg.pWayPoint = this->pWayPointRef.Get();

							if (windParamMsg.pWayPoint == (CWayPoint*)0x0) {
								windParamMsg.pWayPoint = (CWayPoint*)0x0;
							}

							_wind_param_in* local_8 = &windParamMsg;
							windParamMsg.field_0x2c = this->field_0x1ac;
							windParamMsg.field_0x30 = this->field_0x1b0;

							DoMessage(pActor, MESSAGE_ENTER_WIND, local_8);
							aAppliedToActorFlags[curActorIndex] = 1;
						}
					}

					if (aAppliedToActorFlags[curActorIndex] != 0) {
						if (((GetStateFlags(this->actorState) & 0x100) != 0) && (this->curWindAlpha != 0.0f)) {
							NotifyActorInWindArea(fVar11, pActor);
						}
					}
				}

				curActorIndex = curActorIndex + 1;
			}
		}

		this->field_0x1f8 = this->curWindAlpha;

		pFxWind = this->aFxWind;
		for (iVar4 = 0; iVar4 < (int)this->nbFxWind; iVar4 = iVar4 + 1) {
			pFxWind->Manage(0.0f < this->curWindAlpha);
			pFxWind = pFxWind + 1;
		}

		IMPLEMENTATION_GUARD_AUDIO(
		if ((this->aFxWind != (CFxWind*)0x0) && (this->pSoundWind != (CSoundWind*)0x0)) {
			pFxWind = this->aFxWind + this->nbFxWind;
			while (bVar3 = this->aFxWind < pFxWind, pFxWind = pFxWind + -1, bVar3) {
				this->pSoundWind->Play(this, pFxWind);
			}
		})

		if (2.2f <= CScene::_pinstance->field_0x1c) {
			this->field_0x1d0.pStreamEventCamera->Manage(this);
			this->field_0x1d8.pStreamEventCamera->Manage(this);
			this->field_0x1e0.pStreamEventCamera->Manage(this);
		}
	}

	return;
}

float CActorWind::ComputeCurTime()
{
	Timer* pTVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	float fVar5;
	uint uVar6;
	float x;

	pTVar1 = GetTimer();
	x = (pTVar1->scaledTotalTime - this->time_0x214) + this->field_0x174;
	fVar5 = x / this->field_0x204;

	if (this->field_0x190 != 0) {
		if (fVar5 < 2.147484e+09f) {
			uVar6 = (uint)fVar5;
		}
		else {
			uVar6 = (int)(fVar5 - 2.147484e+09f) | 0x80000000;
		}

		if (this->field_0x190 < uVar6) {
			fmodf(x, this->field_0x204);
			this->field_0x218 = this->field_0x190;
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;

			EvaluateDisplayState();
			this->sectorObj.SetHiddenOn();

			iVar4 = 0;
			if (0 < this->nbFxWind) {
				do {
					CFxWind* pFxWind = this->aFxWind + iVar4;
					pFxWind->flags_0x54 = pFxWind->flags_0x54 & ~FXWIND_FLAG_ACTIVE;

					iVar4 = iVar4 + 1;
				} while (iVar4 < this->nbFxWind);
			}

			return -1.0f;
		}
	}

	do {
		fVar5 = this->field_0x204;
		if (x <= fVar5) {
			return x;
		}

		this->time_0x214 = this->time_0x214 + fVar5;
		x = x - fVar5;
	} while ((this->field_0x190 <= this->field_0x218) || (this->field_0x218 = this->field_0x218 + 1, this->field_0x218 != this->field_0x190));

	this->flags = this->flags & 0xfffffffd;
	this->flags = this->flags | 1;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;
	EvaluateDisplayState();
	this->sectorObj.SetHiddenOn();

	iVar4 = 0;
	if (0 < this->nbFxWind) {
		do {
			CFxWind* pFxWind = this->aFxWind + iVar4;
			pFxWind->flags_0x54 = pFxWind->flags_0x54 & ~FXWIND_FLAG_ACTIVE;

			iVar4 = iVar4 + 1;
		} while (iVar4 < this->nbFxWind);
	}

	return  -1.0f;
}

int CActorWind::ComputeStateFromTime(float* pTime)
{
	int newState = WIND_STATE_IDLE;

	if (this->field_0x180 <= *pTime) {
		*pTime = *pTime - this->field_0x180;
		newState = WIND_STATE_DORMANT;
		if (this->field_0x184 <= *pTime) {
			*pTime = *pTime - this->field_0x184;
			newState = WIND_STATE_FADE_IN;
			if (this->field_0x188 <= *pTime) {
				*pTime = *pTime - this->field_0x188;
				newState = WIND_STATE_ACTIVE;
				if (this->field_0x18c <= *pTime) {
					*pTime = *pTime - this->field_0x18c;
					newState = WIND_STATE_FADE_OUT;
					if (this->field_0x178 <= *pTime) {
						newState = WIND_STATE_COOLDOWN;
						*pTime = *pTime - this->field_0x178;
					}
				}
			}
		}
	}

	return newState;
}

void CActorWind::ComputeSpeedsInterpoltation()
{
	float fVar1;

	if (this->field_0x180 != 0.0f) {
		this->field_0x20c = 0.0f / this->field_0x180;
	}

	if (this->field_0x188 != 0.0f) {
		this->field_0x210 = fabs(this->maxWind) / this->field_0x188;
	}

	if (this->field_0x178 != 0.0f) {
		this->field_0x208 = fabs(-this->maxWind) / this->field_0x178;
	}

	fVar1 = this->field_0x17c + this->field_0x178 + this->field_0x18c + this->field_0x188 + this->field_0x180 + this->field_0x184;
	this->field_0x204 = fVar1;

	if (fVar1 == 0.0f) {
		this->field_0x18c = 1.0f;
		this->field_0x204 = 1.0f;
	}

	if (this->field_0x204 < this->field_0x174) {
		fVar1 = fmodf(this->field_0x174, this->field_0x204);
		this->field_0x174 = fVar1;
	}

	if (this->field_0x204 == 0.0f) {
		this->field_0x18c = 1.0f;
		this->field_0x204 = 1.0f;
	}
	return;
}

void CActorWind::ResetTiming()
{
	Timer* pTVar1;
	int iVar2;
	float fVar3;
	float local_4;

	if ((this->field_0x160 & 4) != 0) {
		pTVar1 = GetTimer();
		this->time_0x214 = pTVar1->scaledTotalTime;
	}

	this->curWindAlpha = 0.0f;
	this->field_0x218 = 0;

	local_4 = ComputeCurTime();
	if (local_4 != -1.0f) {
		iVar2 = ComputeStateFromTime(&local_4);
		switch (iVar2) {
		case WIND_STATE_IDLE:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x180, 0.0f, 0.0f);
			this->curWindAlpha = fVar3;
			break;
		case WIND_STATE_DORMANT:
			this->curWindAlpha = 0.0f;
			break;
		case WIND_STATE_FADE_IN:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x188, 0.0f, this->maxWind);
			this->curWindAlpha = fVar3;
			break;
		case WIND_STATE_ACTIVE:
			this->curWindAlpha = this->maxWind;
			break;
		case WIND_STATE_FADE_OUT:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x178, this->maxWind, 0.0f);
			this->curWindAlpha = fVar3;
			break;
		case WIND_STATE_COOLDOWN:
			this->curWindAlpha = 0.0f;
		}
	}

	return;
}

void CActorWind::NotifyActorInWindArea(float param_1, CActor* pNotifyActor)
{
	CWayPoint* pfVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_50;
	NotifyWindParam notifyWindParam;
	edF32VECTOR4 local_20;
	NotifyWindParam* pNotifyWindParam;

	local_20 = this->rotationQuat;

	if ((this->field_0x160 & 0x80) != 0) {
		edF32Vector4SubHard(&local_20, &pNotifyActor->currentLocation, &this->currentLocation);
		edF32Vector4NormalizeHard(&local_20, &local_20);
		edProjectVectorOnPlane(0.0f, &local_20, &local_20, &this->rotationQuat, 0);
	}

	pNotifyActor->IsKindOfObject(OBJ_TYPE_AUTONOMOUS);

	if ((this->field_0x160 & 1) == 0) {
		fVar2 = edDistPointToPlane(&this->currentLocation, &this->rotationQuat, &pNotifyActor->currentLocation);
		if (fVar2 < 0.0f) {
			fVar2 = 0.0f;
		}

		fVar3 = this->curWindAlpha;
		if (-CDynamicExt::gForceGravity.y < fVar3) {
			notifyWindParam.field_0x10 = fVar3 + ((-CDynamicExt::gForceGravity.y - fVar3) / param_1) * fVar2;
		}
		else {
			notifyWindParam.field_0x10 = edFIntervalLERP(fVar2, 0.0f, param_1, fVar3, 0.0f);
		}
	}
	else {
		notifyWindParam.field_0x10 = this->curWindAlpha;
	}

	if (notifyWindParam.field_0x10 != 0.0f) {
		pfVar1 = this->field_0x1cc.Get();
		if ((pfVar1 != (CWayPoint*)0x0) && (pNotifyActor->typeID != 6)) {
			local_50.xyz = pfVar1->location;
			local_50.w = 1.0f;
			edF32Vector4SubHard(&local_50, &local_50, &pNotifyActor->currentLocation);
			local_50.y = 0.0f;
			edF32Vector4ScaleHard(notifyWindParam.field_0x10 * 0.01f, &local_50, &local_50);
			edF32Vector4ScaleHard(notifyWindParam.field_0x10, &local_20, &local_20);
			edF32Vector4AddHard(&local_20, &local_20, &local_50);
			edF32Vector4NormalizeHard(&local_20, &local_20);
		}

		pNotifyWindParam = &notifyWindParam;
		notifyWindParam.field_0x0 = local_20;
		DoMessage(pNotifyActor, MESSAGE_IN_WIND_AREA, pNotifyWindParam);
	}

	return;
}


void ComputeDimensionsAndBase(edF32MATRIX4* m0, edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float fVar1;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.x = -1.0f;
	local_10.y = 0.0f;
	local_10.z = 0.0f;
	local_10.w = 1.0f;
	local_20.x = 0.5f;
	local_20.y = 0.0f;
	local_20.z = 0.0f;
	local_20.w = 1.0f;
	edF32Matrix4MulF32Vector4Hard(&local_10, m0, &local_10);
	edF32Matrix4MulF32Vector4Hard(&local_20, m0, &local_20);
	edF32Vector4SubHard(&local_10, &local_10, &local_20);
	fVar1 = edF32Vector4NormalizeHard(&local_10, &local_10);
	v0->x = fVar1;
	local_10.x = 0.0f;
	local_10.y = -1.0f;
	local_10.z = 0.0f;
	local_10.w = 1.0f;
	local_20.x = 0.0f;
	local_20.y = 0.5f;
	local_20.z = 0.0f;
	local_20.w = 1.0f;
	edF32Matrix4MulF32Vector4Hard(v1, m0, &local_10);
	edF32Matrix4MulF32Vector4Hard(&local_20, m0, &local_20);
	edF32Vector4SubHard(&local_10, &local_20, v1);
	fVar1 = edF32Vector4NormalizeHard(&local_10, &local_10);
	v0->y = fVar1;
	local_10.x = 0.0f;
	local_10.y = 0.0f;
	local_10.z = -1.0f;
	local_10.w = 1.0f;
	local_20.x = 0.0f;
	local_20.y = 0.0f;
	local_20.z = 0.5f;
	local_20.w = 1.0f;
	edF32Matrix4MulF32Vector4Hard(&local_10, m0, &local_10);
	edF32Matrix4MulF32Vector4Hard(&local_20, m0, &local_20);
	edF32Vector4SubHard(&local_10, &local_10, &local_20);
	fVar1 = edF32Vector4NormalizeHard(&local_10, &local_10);
	v0->z = fVar1;
	v0->w = 0.0f;

	return;
}

void CActorWind::ReComputeBoundingSphere(edF32MATRIX4* pMatrix)
{
	CFxWind* pCVar1;
	KyaUpdateObjA* pKVar2;
	bool bVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	iVar4 = 1;
	pCVar1 = this->aFxWind;
	local_20.w = (pCVar1->field_0x140).w;
	local_20.x = (pCVar1->field_0x140).x - pCVar1->field_0x160;
	local_20.y = (pCVar1->field_0x140).y - pCVar1->field_0x160;
	local_20.z = (pCVar1->field_0x140).z - pCVar1->field_0x160;

	local_30.w = (pCVar1->field_0x140).w;
	local_30.x = (pCVar1->field_0x140).x + pCVar1->field_0x160;
	local_30.y = (pCVar1->field_0x140).y + pCVar1->field_0x160;
	local_30.z = (pCVar1->field_0x140).z + pCVar1->field_0x160;
	if (1 < (int)this->nbFxWind) {
		do {
			fVar9 = pCVar1[1].field_0x140.x;
			fVar5 = pCVar1[1].field_0x160;
			fVar6 = pCVar1[1].field_0x140.y;
			fVar7 = pCVar1[1].field_0x140.z;

			fVar10 = fVar9 - fVar5;
			fVar11 = fVar6 - fVar5;
			fVar8 = fVar7 - fVar5;
			fVar9 = fVar9 + fVar5;
			fVar6 = fVar6 + fVar5;
			fVar7 = fVar7 + fVar5;

			if (local_20.x <= fVar10) {
				fVar10 = local_20.x;
			}

			local_20.x = fVar10;
			if (local_20.y <= fVar11) {
				fVar11 = local_20.y;
			}

			local_20.y = fVar11;
			if (local_20.z <= fVar8) {
				fVar8 = local_20.z;
			}

			local_20.z = fVar8;
			if (fVar9 <= local_30.x) {
				fVar9 = local_30.x;
			}

			local_30.x = fVar9;
			if (fVar6 <= local_30.y) {
				fVar6 = local_30.y;
			}

			local_30.y = fVar6;
			if (fVar7 <= local_30.z) {
				fVar7 = local_30.z;
			}

			local_30.z = fVar7;
			iVar4 = iVar4 + 1;
			pCVar1 = pCVar1 + 1;
		} while (iVar4 < (int)this->nbFxWind);
	}

	edF32Vector4SubHard(&eStack64, &local_30, &local_20);
	edF32Vector4ScaleHard(0.5f, &eStack64, &eStack64);
	fVar9 = edF32Vector4GetDistHard(&eStack64);
	edF32Vector4AddHard(&eStack16, &local_20, &eStack64);
	edF32Matrix4MulF32Vector4Hard(&eStack16, pMatrix, &eStack16);

	bVar3 = this->field_0x19c < -1.0f;
	if ((bVar3) && (bVar3 = true, -1.0f <= this->field_0x1a0)) {
		bVar3 = false;
	}

	if ((bVar3) && (pKVar2 = this->subObjA, fVar6 = pKVar2->visibilityDistance, pKVar2->cullingDistance < fVar6)) {
		pKVar2 = this->subObjA;
		pKVar2->floatFieldB = fVar6;
		pKVar2->cullingDistance = fVar6;
	}

	CActor::SetLocalBoundingSphere(fVar9 / 1.414214f, &eStack16);
	return;
}



bool CActorWind::IsAValidFxPrimitive(CFxWind* pFx, byte param_3, edF32MATRIX4* param_4, edF32MATRIX4* param_5)
{
	int iVar1;
	edF32MATRIX4* peVar2;
	ed_3d_hierarchy_node* peVar3;
	edF32MATRIX4* peVar4;
	float fVar5;
	edF32VECTOR4 local_100;
	edF32VECTOR4 local_f0;
	edF32MATRIX4 eStack224;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32MATRIX4 local_40;

	static edF32VECTOR4 edF32VECTOR4_0040e710 = { 0.9f, 1.0f, 0.9f, 1.0f };

	local_90 = edF32VECTOR4_0040e710;

	edF32Matrix4GetInverseGaussSoft(&local_40, param_4);
	local_40.dd = 1.0f;

	ComputeDimensionsAndBase(&local_40, &local_50, &eStack96);

	if ((pFx != (CFxWind*)0x0) && (param_3 == 3)) {
		edF32Vector4ScaleV4Hard(&local_50, &local_50, &local_90);
	}

	if (param_3 == 2) {
		local_50.x = local_50.x * 0.5f;
		local_50.z = local_50.z * 0.5f;
	}

	local_70 = local_40.rowT;

	edF32Vector4SubHard(&eStack128, &local_70, &eStack96);
	edF32Vector4NormalizeHard(&eStack128, &eStack128);
	fVar5 = edF32Vector4DotProductHard(&eStack128, &this->rotationQuat);
	if (pFx != (CFxWind*)0x0) {
		if (fVar5 <= -0.98f) {
			edF32Matrix4SetIdentityHard(&eStack224);
			edF32Matrix4RotateYHard(3.141593f, &eStack224, &eStack224);
			edF32Matrix4MulF32Matrix4Hard(&local_40, &local_40, &eStack224);
			local_40.rowT = local_70;
			edF32Vector4SubHard(&eStack160, &local_70, &eStack96);
			edF32Vector4AddHard(&local_70, &local_70, &eStack160);
			edF32Vector4AddHard(&local_70, &local_70, &eStack160);
			edF32Vector4ScaleHard(-1.0f, &eStack128, &eStack128);
		}

		edF32Matrix4MulF32Vector4Hard(&local_f0, param_5, &local_40.rowT);

		pFx->windMatrix = this->pMeshTransform->base.transformA;
		pFx->field_0x58 = param_3;
		
		pFx->field_0xa0 = local_40;

		pFx->field_0x130 = local_50;
		pFx->field_0x150 = local_f0;
		edF32Matrix4MulF32Vector4Hard(&local_f0, &this->pMeshTransform->base.transformA, &local_f0);
		pFx->field_0x140 = local_f0;

		// Setting y not showing up in decomp even though its a sq instruction.
		local_100 = local_50;

		if (param_3 == 2) {
			local_100.x = local_50.x * 2.0f;
			local_100.z = local_50.z * 2.0f;
		}

		fVar5 = edF32Vector4GetDistHard(&local_100);
		pFx->field_0x160 = fVar5 * 0.5f;
	}

	return true;
}

void CBehaviourWind::Manage()
{
	pOwner->BehaviourWind_Manage();
}

void CBehaviourWind::ManageFrozen()
{
	CActorWind* pActorWind;
	uint fxIndex;

	pActorWind = this->pOwner;
	fxIndex = 0;
	if (pActorWind->nbFxWind != 0) {
		do {
			IMPLEMENTATION_GUARD_WIND_FX(
			pActorWind->aFxWind[fxIndex]->ManageFrozen();)
			pActorWind = this->pOwner;
			fxIndex = fxIndex + 1;
		} while (fxIndex < pActorWind->nbFxWind);
	}
	return;
}

void CBehaviourWind::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorWind*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(WIND_STATE_IDLE, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	return;
}

void CBehaviourWind::InitState(int newState)
{
	int entryIndex;
	CActorWind* pActorWind;

	pActorWind = this->pOwner;

	if (newState == WIND_STATE_COOLDOWN) {
		if (2.2f <= CScene::_pinstance->field_0x1c) {
			pActorWind->field_0x1e0.Switch(pActorWind);
		}
	}
	else {
		if (newState == WIND_STATE_ACTIVE) {
			if (2.2f <= CScene::_pinstance->field_0x1c) {
				pActorWind->field_0x1d8.Switch(pActorWind);
			}
		}
		else {
			if ((newState == WIND_STATE_DORMANT) && (2.2f <= CScene::_pinstance->field_0x1c)) {
				pActorWind->field_0x1d0.Switch(pActorWind);
			}
		}
	}

	return;
}

CFxEmitterPool* gpWIND_PartPool = (CFxEmitterPool*)0x0;

CFxWind::CFxWind()
{
	this->field_0x390 = -1.0f;
	//*(undefined4*)&this->field_0x398 = 0;
	//*(undefined*)&this->field_0x39c = 0;
}

bool CFxWind::IsKindOfObject(ulong kind)
{
	return (kind & 0x210000) != 0;
}

bool CFxWind::InitDlistPatchable(int patchId)
{
	return this->windSolid.CreatePatchableDlist(patchId);
}

void CFxWind::Create(CActorWind* pOwner, int param_3)
{
	CActorSound* pCVar1;
	CFxEmitterPool* pWindPartPool;

	this->pOwner = pOwner;
	this->sectorId = pOwner->sectorId;
	this->field_0x164 = param_3;
	this->field_0x2c8 = 1.0f;
	this->field_0x2cc = 0.5f;
	this->field_0x2d0 = 1.0f;
	this->field_0x2d4 = 1.0f;
	this->field_0x180 = (ed_g2d_manager*)0x0;
	this->field_0x58 = 6;
	this->flags_0x54 = 0;
	this->field_0x374 = pOwner->CreateActorSound(3);

	(this->windSolid).pOwner = this;
	(this->windSolid).pAlphaData = 0;

	pWindPartPool = gpWIND_PartPool;

	if ((gpWIND_PartPool == (CFxEmitterPool*)0x0)) {
		pWindPartPool = new CFxEmitterPool;
	}

	gpWIND_PartPool = pWindPartPool;

	return;
}

CFxWind::WindAnimST CFxWind::gUseAnimST[5] =
{
	{ 1, 1.0f,  0.0f },
	{ 0, 0.0f,  0.0f },
	{ 0, 0.0f,  0.0f },
	{ 1, 0.0f, -1.0f },
	{ 0, 0.0f,  0.0f },
};

void CFxWind::Init(edF32MATRIX4* pMatrix)
{
	byte* pbVar1;
	byte* pbVar2;
	CFxEmitterPool* pCVar3;
	C3DFileManager* p3dFileManager;
	edF32MATRIX4* peVar4;
	edDList_material* pDlistMaterial;
	ed_hash_code* pMaterialHashCode;
	int iVar7;
	char* pManagerBuffer;
	ed_g2d_manager* pDuplicatedManager;
	ed_g2d_texture* peVar10;
	void* pvVar11;
	CFxWindHandle* pHandle;
	int index;
	int* pMaterialSizeIt;
	ed_hash_code** pHashCodeIt;
	int* pMaterialSizeItInner;
	ed_hash_code** pHashCodeItInner;
	int totalMaterialSize;
	WindAnimST* pWindAnimStIt;
	undefined4 uVar20;
	undefined4 uVar21;
	float fVar22;
	int aMaterialSizes[8];
	ed_hash_code* aHashCodes[7];
	e_ed_event_prim3d_type local_4;

	p3dFileManager = CScene::ptable.g_C3DFileManager_00451664;
	peVar4 = edEventGetChunkZonePrimitive((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->pOwner->field_0x21c, this->field_0x164, &local_4);
	this->field_0x5c = peVar4;
	edF32Matrix4GetInverseGaussSoft(&this->field_0x10, peVar4);
	this->pOwner->IsAValidFxPrimitive(this, local_4, peVar4, pMatrix);

	RetrieveFlags(this->pOwner);

	IMPLEMENTATION_GUARD_AUDIO(
		edCSound3DPrim::Init(&this->field_0x3a0, this->field_0x58, &this->field_0x10);)

	if (((this->flags_0x54 & FXWIND_FLAG_EMITTER_POOL_ACTIVE) == 0) || ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) != 0)) {
		pHashCodeIt = aHashCodes;
		pMaterialSizeIt = aMaterialSizes;
		totalMaterialSize = 0;
		index = 0;
		pWindAnimStIt = gUseAnimST;
		pHashCodeItInner = pHashCodeIt;
		pMaterialSizeItInner = pMaterialSizeIt;

		do {
			if (pWindAnimStIt->field_0x0 != 0) {
				pDlistMaterial = p3dFileManager->GetMaterialFromId(this->pOwner->materialId, index);
				pMaterialHashCode = ed3DG2DGetHASHFromMat(pDlistMaterial->pManager, pDlistMaterial->pMaterial);
				*pHashCodeItInner = pMaterialHashCode;
				*pMaterialSizeItInner = ed3DG2DGetNeededSizeForDuplicateMaterial(*pHashCodeItInner);
				totalMaterialSize = totalMaterialSize + *pMaterialSizeItInner;
			}

			index = index + 1;
			pWindAnimStIt = pWindAnimStIt + 1;
			pHashCodeItInner = pHashCodeItInner + 1;
			pMaterialSizeItInner = pMaterialSizeItInner + 1;
		} while (index < 5);

		this->field_0x180 = reinterpret_cast<ed_g2d_manager*>(edMemAlloc(TO_HEAP(H_MAIN), totalMaterialSize));
		memset(this->field_0x180, 0, totalMaterialSize);
		pManagerBuffer = reinterpret_cast<char*>(this->field_0x180);

		MaterialManagerCombined* pMaterialIt = this->aCombinedMaterials;
		index = 0;
		pWindAnimStIt = gUseAnimST;
		do {
			if (pWindAnimStIt->field_0x0 == 0) {
				pDlistMaterial = p3dFileManager->GetMaterialFromId(this->pOwner->materialId, index);
				pMaterialIt->material = *pDlistMaterial;
				pMaterialIt->manager = *(pMaterialIt->material).pManager;
			}
			else {
				pDuplicatedManager = ed3DG2DDuplicateMaterial(*pHashCodeIt, pManagerBuffer, &pMaterialIt->manager);
				ed3DG2DGetG2DMaterial(pDuplicatedManager, (*pHashCodeIt)->hash.number);
				edDListCreatMaterialFromHashCode(&pMaterialIt->material, (*pHashCodeIt)->hash.number, pDuplicatedManager, 0);
				peVar10 = ed3DG2DGetTextureFromMaterial((pMaterialIt->material).pMaterial, 0);
				if (peVar10->pAnimSpeedNormalExtruder != 0x0) {
					edF32VECTOR4* pData = LOAD_POINTER_CAST(edF32VECTOR4*, peVar10->pAnimSpeedNormalExtruder);
					pData->x = 0.0f;
					pData->y = 0.0f;
					pData->z = 0.0f;
					pData->w = 0.0f;
				}

				pManagerBuffer = pManagerBuffer + *pMaterialSizeIt;
			}

			index = index + 1;
			pWindAnimStIt = pWindAnimStIt + 1;
			pHashCodeIt = pHashCodeIt + 1;
			pMaterialSizeIt = pMaterialSizeIt + 1;
			pMaterialIt = pMaterialIt + 1;
		} while (index < 5);
	}

	if ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) != 0) {
		(this->windSolid).field_0x10 = 1.0f;
		(this->windSolid).field_0x14 = 1.0f;
		(this->windSolid).field_0xc = 0.0f;
		(this->windSolid).field_0x8 = 0.0f;

		(this->windSolid).outerStateA.prevAlpha = 0.0f;
		(this->windSolid).outerStateA.alpha = 0.0f;
		(this->windSolid).outerStateB.prevAlpha = 0.0f;
		(this->windSolid).outerStateB.alpha = 0.0f;
		(this->windSolid).fakePlaneState.prevAlpha = 0.0f;
		(this->windSolid).fakePlaneState.alpha = 0.0f;
		(this->windSolid).whirlStateA.prevAlpha = 0.0f;
		(this->windSolid).whirlStateA.alpha = 0.0f;
		(this->windSolid).whirlStateB.prevAlpha = 0.0f;
		(this->windSolid).whirlStateB.alpha = 0.0f;


		fVar22 = edF32Vector4GetDistHard(&((this->windSolid).pOwner)->field_0x130);
		(this->windSolid).field_0x4 = (int)(fVar22 / 20.0f + 1.0f);
		if ((int)(this->windSolid).field_0x4 < 3) {
			(this->windSolid).field_0x4 = 3;
		}

		(this->windSolid).pAlphaData = edMemAlloc(TO_HEAP(H_MAIN), (this->windSolid).field_0x4 * 0x16 + -2);
		byte* pData = reinterpret_cast<byte*>((this->windSolid).pAlphaData);
		(this->windSolid).outerStateB.pAlphaMultiplier = pData;
		(this->windSolid).outerStateA.pAlphaMultiplier = pData;
		(this->windSolid).fakePlaneState.pAlphaMultiplier = (this->windSolid).outerStateA.pAlphaMultiplier + ((this->windSolid).field_0x4 - 1) * 0xe;
		(this->windSolid).whirlStateA.pAlphaMultiplier = (this->windSolid).fakePlaneState.pAlphaMultiplier + (this->windSolid).field_0x4 * 4;
		(this->windSolid).whirlStateB.pAlphaMultiplier = (this->windSolid).whirlStateA.pAlphaMultiplier + (this->windSolid).field_0x4 * 4;

		this->outerPatchId = GameDListPatch_Register(this, ((this->windSolid).field_0x4 - 1) * 0x1c, 0);
		this->whirlPatchId = GameDListPatch_Register(this, (this->windSolid).field_0x4 * 4 + 0x18, 0);
		this->fakePlanePatchId = GameDListPatch_Register(this, (this->windSolid).field_0x4 << 2, 0);

		(this->windSolid).outerPatchId = this->outerPatchId;
		(this->windSolid).whirlPatchId = this->whirlPatchId;
		(this->windSolid).fakePlanePatchId = this->fakePlanePatchId;
	}

	pCVar3 = gpWIND_PartPool;
	index = -1;
	if (((this->pOwner->sectorId == -1) && (gpWIND_PartPool->nbHandles < 0x40)) &&
		(totalMaterialSize = 0, pHandle = gpWIND_PartPool->aWindHandles, gpWIND_PartPool->nbHandles != 0x40)) {
		do {
			if (pHandle->pFxWind == (CFxWind*)0x0) {
				index = totalMaterialSize;
			}

			totalMaterialSize = totalMaterialSize + 1;
		} while ((totalMaterialSize < 0x40) && (pHandle = pHandle + 1, index == -1));

		gpWIND_PartPool->aWindHandles[index].pFxWind = this;
		pCVar3->nbHandles = pCVar3->nbHandles + 1;
	}

	IMPLEMENTATION_GUARD_AUDIO(
	this->pOwner->pSoundWind->Init(this);)

	return;
}

void CFxWind::SectorChange(int oldSectorId, int newSectorId)
{
	int iVar1;
	CFxLightEmitter* pCurEmitter;
	CFxWindHandle* pCVar2;
	int iVar3;
	CFxWindHandle* pMyWindHandle;
	_EmiNfo* pCurEmiInfo;
	_EmiNfo* pMyEmiInfo;
	CFxWindHandle* pCurHandle;
	CFxEmitterPool* pWindPartPool;

	pWindPartPool = gpWIND_PartPool;
	if ((oldSectorId == this->pOwner->sectorId) && (iVar3 = -1, oldSectorId != -1)) {
		iVar1 = 0;
		pCurHandle = gpWIND_PartPool->aWindHandles;
		while ((iVar1 < 0x40 && (iVar3 == -1))) {
			if (pCurHandle->pFxWind == this) {
				iVar3 = iVar1;
			}

			pCurHandle = pCurHandle + 1;
			iVar1 = iVar1 + 1;
		}

		pMyWindHandle = (CFxWindHandle*)0x0;
		if (iVar3 != -1) {
			pMyWindHandle = gpWIND_PartPool->aWindHandles + iVar3;
		}

		int activeEmiIndex = -1;
		if (pMyWindHandle != (CFxWindHandle*)0x0) {
			iVar1 = 0;
			_EmiNfo* pCurInfo = gpWIND_PartPool->aHolders;
			do {
				if (this == pCurInfo->pFxWind) {
					activeEmiIndex = iVar1;
				}

				iVar1 = iVar1 + 1;
				pCurInfo = pCurInfo + 1;
			} while ((iVar1 < 4) && (activeEmiIndex == -1));

			pMyEmiInfo = (_EmiNfo*)0x0;
			if (activeEmiIndex != -1) {
				pMyEmiInfo = gpWIND_PartPool->aHolders + activeEmiIndex;
			}

			iVar3 = 0;
			pCurHandle = gpWIND_PartPool->aWindHandles;
			pCurEmitter = pMyEmiInfo->field_0x0;
			if (pMyEmiInfo != (_EmiNfo*)0x0) {
				do {
					pCurEmitter->Enable(0);
					pCurEmitter->ReleaseRays(gpWIND_PartPool->field_0xd5c[iVar3].nbRays);
					iVar3 = iVar3 + 1;
					pCurHandle = pCurHandle + 1;
					pCurEmitter = pCurEmitter + 1;
				} while (iVar3 < 3);

				pMyEmiInfo->pFxWind = (CFxWind*)0x0;
				pWindPartPool->field_0x14 = pWindPartPool->field_0x14 + -1;
			}

			iVar3 = -1;
			iVar1 = 0;
			pCurEmiInfo = pWindPartPool->aHolders;
			do {
				if (this == pCurEmiInfo->pFxWind) {
					iVar3 = iVar1;
				}
				iVar1 = iVar1 + 1;
				pCurEmiInfo = pCurEmiInfo + 1;
			} while ((iVar1 < 4) && (iVar3 == -1));

			if (iVar3 != -1) {
				iVar1 = 0;
				pCurEmitter = pWindPartPool->aHolders[iVar3].field_0x0;
				do {
					pCurEmitter->Enable(0);
					iVar1 = iVar1 + 1;
					pCurEmitter = pCurEmitter + 1;
				} while (iVar1 < 3);

				pWindPartPool->aHolders[iVar3].pFxWind = (CFxWind*)0x0;
			}

			pMyWindHandle->pFxWind = (CFxWind*)0x0;
			pMyWindHandle->field_0x4 = 1000000.0f;
			pWindPartPool->nbHandles = pWindPartPool->nbHandles + -1;
		}
	}

	pWindPartPool = gpWIND_PartPool;
	iVar3 = -1;
	if ((((newSectorId == this->pOwner->sectorId) && (newSectorId != -1)) && (gpWIND_PartPool->nbHandles < 0x40)) &&
		(iVar1 = 0, pCurHandle = gpWIND_PartPool->aWindHandles, gpWIND_PartPool->nbHandles != 0x40)) {
		do {
			if (pCurHandle->pFxWind == (CFxWind*)0x0) {
				iVar3 = iVar1;
			}

			iVar1 = iVar1 + 1;
		} while ((iVar1 < 0x40) && (pCurHandle = pCurHandle + 1, iVar3 == -1));

		gpWIND_PartPool->aWindHandles[iVar3].pFxWind = this;
		pWindPartPool->nbHandles = pWindPartPool->nbHandles + 1;
	}

	if ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) != 0) {
		(this->windSolid).whirlStateA.drawState = 2;
		(this->windSolid).outerStateA.drawState = 2;
		(this->windSolid).outerStateB.drawState = 2;
		(this->windSolid).fakePlaneState.drawState = 2;
		(this->windSolid).whirlStateB.drawState = 2;
	}

	return;
}

void CFxWind::Manage(int param_2)
{
	uint uVar1;
	bool bVar2;
	ed_g2d_texture* pTexture;
	int iVar5;
	float fVar6;
	float fVar7;

	fVar7 = 40.0f;
	fVar6 = this->pOwner->curWindAlpha;
	if (40.0f <= fVar6) {
		fVar7 = fVar6;
	}

	fVar6 = 160.0f;
	if (fVar7 <= 160.0f) {
		fVar6 = fVar7;
	}

	this->field_0x16c = sqrtf(fVar6) * 2.0f;

	uVar1 = this->pOwner->field_0x160;
	bVar2 = true;
	if ((((uVar1 & 0x20) == 0) && (bVar2 = false, (uVar1 & 0x40) == 0)) &&
		(bVar2 = true, 0.17398384f <= fabsf(this->pOwner->rotationQuat.y))) {
		bVar2 = false;
	}

	if (bVar2) {
		this->field_0x16c = this->field_0x16c * 1.5f;
	}

	this->field_0x170 = (this->field_0x16c * 2.0f) / (this->field_0x130).y;
	this->distanceToCameraScale = edFIntervalUnitDstLERP(this->pOwner->distanceToCamera, 0.0f, (this->pOwner->subObjA)->cullingDistance);

	bVar2 = false;
	if ((this->pOwner->field_0x19c < -1.0f) && (this->pOwner->field_0x1a0 < -1.0f)) {
		bVar2 = true;
	}

	if (bVar2) {
		this->distanceToCameraScale = edFIntervalUnitSrcLERP(this->distanceToCameraScale, 0.0f, 0.3f);
	}

	_Compute_Matrix(&CCameraManager::_gThis->transMatrix_0x390);

	if ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) != 0) {
		fVar7 = this->field_0x170 * GetTimer()->cutsceneDeltaTime;
		for (iVar5 = 0; iVar5 < 5; iVar5 = iVar5 + 1) {
			pTexture = ed3DG2DGetTextureFromMaterial(this->aCombinedMaterials[iVar5].material.pMaterial, 0);
			if ((pTexture->pAnimSpeedNormalExtruder != 0x0) && (gUseAnimST[iVar5].field_0x0 != 0)) {
				edF32VECTOR4* pAnimSpeedNormalExtruder = LOAD_POINTER_CAST(edF32VECTOR4*, pTexture->pAnimSpeedNormalExtruder);
				pAnimSpeedNormalExtruder->x = fVar7 * gUseAnimST[iVar5].field_0x4 * -(this->windSolid).field_0x14;
				pAnimSpeedNormalExtruder->y = fVar7 * gUseAnimST[iVar5].field_0x8 * (this->windSolid).field_0x10;
			}
		}
	}

	if (param_2 == 0) {
		this->flags_0x54 = this->flags_0x54 & ~FXWIND_FLAG_ACTIVE;
	}
	else {
		this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_ACTIVE;
	}

	gpWIND_PartPool->Manage(this);

	return;
}

void CFxWind::Draw()
{
	bool bVar1;
	edF32VECTOR4* pRowT;
	edF32VECTOR4* pRowZ;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 cameraZ;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CCameraManager* pCameraManager;

	pCameraManager = CCameraManager::_gThis;
	if ((this->flags_0x54 & FXWIND_FLAG_ACTIVE) == 0) {
		ChangeVisibleState(0);
	}
	else {
		ChangeVisibleState(1);

		bVar1 = pCameraManager->InFrustum(this->field_0x160, this->pOwner->subObjA->cullingDistance, &this->field_0x140);
		if (bVar1 != false) {
			cameraZ = (CCameraManager::_gThis->transformationMatrix).rowZ;
			const float dotYZ = edF32Vector4DotProductHard(&(this->windMatrix).rowY, &cameraZ);
			fVar9 = sqrtf(1.0f - dotYZ * dotYZ);
			fVar2 = edFIntervalUnitDstLERP(this->pOwner->curWindAlpha, 0.0f, this->pOwner->maxWind);
			this->baseAlpha = fVar2;
			this->perpendicularAlpha = fVar2 * fVar9;
			this->parallelAlpha = this->baseAlpha * (1.0f - fVar9);
			fVar6 = this->perpendicularAlpha;
			fVar7 = this->field_0x2c8;
			fVar10 = this->baseAlpha;
			fVar2 = this->parallelAlpha;
			fVar9 = this->field_0x2d4;
			fVar8 = this->field_0x2cc * fVar6;
			fVar11 = this->field_0x2d0 * fVar10;
			fVar3 = edFIntervalUnitDstLERP(this->distanceToCameraScale, 0.3f, 0.1f);
			this->field_0x2c4 = fVar10 * fVar3;

			if ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) != 0) {
				fVar3 = this->distanceToCameraScale;
				if (fVar3 < 0.3f) {
					fVar3 = edFIntervalUnitDstLERP(fVar3, 0.1f, 0.3f);
				}
				else {
					fVar3 = edFIntervalUnitDstLERP(fVar3, 1.0f, 0.3f);
				}

				fVar10 = this->distanceToCameraScale;
				fVar8 = fVar8 * fVar3;
				if (fVar10 <= 0.75f) {
					fVar10 = edFIntervalLERP(fVar10, 0.2f, 0.75f, 1.0f, 1.0f);
				}
				else {
					fVar10 = edFIntervalLERP(fVar10, 0.75f, 1.0f, 1.0f, 0.0f);
				}

				pRowZ = &(this->field_0xa0).rowZ;
				fVar11 = fVar11 * fVar10;
				edF32Vector4AddHard(&eStack16, &this->pOwner->currentLocation, pRowZ);
				eStack16.y = (pCameraManager->transformationMatrix).rowT.y;
				pRowT = &(pCameraManager->transformationMatrix).rowT;
				edF32Vector4SubHard(&eStack32, pRowT, &eStack16);
				fVar10 = edF32Vector4GetDistHard(&eStack32);
				fVar10 = edFIntervalUnitDstLERP(fVar10 / (this->field_0x130).z, 0.12f, 0.22f);
				fVar10 = fVar11 * fVar10;
				edF32Vector4SubHard(&eStack16, &this->pOwner->currentLocation, pRowZ);
				eStack16.y = (pCameraManager->transformationMatrix).rowT.y;
				edF32Vector4SubHard(&eStack32, pRowT, &eStack16);
				fVar4 = edF32Vector4GetDistHard(&eStack32);
				fVar4 = edFIntervalUnitDstLERP(fVar4 / (this->field_0x130).z, 0.12f, 0.22f);
				fVar5 = edFIntervalUnitDstLERP(this->distanceToCameraScale, 0.1f, 0.4f);

				(this->windSolid).outerStateA.prevAlpha = (this->windSolid).outerStateA.alpha;
				(this->windSolid).outerStateA.alpha = fVar11 * fVar4;

				(this->windSolid).outerStateB.prevAlpha = (this->windSolid).outerStateB.alpha;
				(this->windSolid).outerStateB.alpha = fVar10;

				(this->windSolid).fakePlaneState.prevAlpha = (this->windSolid).fakePlaneState.alpha;
				(this->windSolid).fakePlaneState.alpha = fVar7 * fVar6 * fVar3;

				(this->windSolid).whirlStateA.prevAlpha = (this->windSolid).whirlStateA.alpha;
				(this->windSolid).whirlStateA.alpha = fVar8;

				(this->windSolid).whirlStateB.prevAlpha = (this->windSolid).whirlStateB.alpha;
				(this->windSolid).whirlStateB.alpha = fVar9 * fVar2 * fVar5;

				this->windSolid.Draw(&this->windMatrix);
			}
		}
	}

	gpWIND_PartPool->Draw();

	return;
}

void CFxWind::Reset()
{
	IMPLEMENTATION_GUARD_AUDIO(
	this->pOwner->pSoundWind->Reset(this);)

	if ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) != 0) {
		(this->windSolid).field_0xc = 0.0f;
		(this->windSolid).field_0x8 = 0.0f;

		(this->windSolid).outerStateA.prevAlpha = 0.0f;
		(this->windSolid).outerStateA.alpha = 0.0f;
		(this->windSolid).outerStateB.prevAlpha = 0.0f;
		(this->windSolid).outerStateB.alpha = 0.0f;
		(this->windSolid).fakePlaneState.prevAlpha = 0.0f;
		(this->windSolid).fakePlaneState.alpha = 0.0f;
		(this->windSolid).whirlStateA.prevAlpha = 0.0f;
		(this->windSolid).whirlStateA.alpha = 0.0f;
		(this->windSolid).whirlStateB.prevAlpha = 0.0f;
		(this->windSolid).whirlStateB.alpha = 0.0f;
	}
	return;
}

void CFxWind::RetrieveFlags(CActorWind* pWind)
{
	bool bVar1;

	if ((pWind->field_0x160 & 0x100) == 0) {
		bVar1 = false;
		if ((pWind->field_0x19c < -1.0f) && (pWind->field_0x1a0 < -1.0f)) {
			bVar1 = true;
		}

		if (!bVar1) {
			this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_SOLID_GRAPHICS_ENABLED;
		}
	}

	if ((pWind->field_0x160 & 8) != 0) {
		this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_UNUSED_0x08;
	}

	if ((pWind->field_0x160 & 0x10) != 0) {
		this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_UNUSED_0x10;
	}

	if ((pWind->field_0x160 & 0x800) == 0) {
		bVar1 = false;
		if ((pWind->field_0x19c < -1.0f) && (pWind->field_0x1a0 < -1.0f)) {
			bVar1 = true;
		}

		if (!bVar1) goto LAB_00210ed8;
	}

	this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_UNUSED_0x20;

LAB_00210ed8:
	if ((pWind->field_0x160 & 0x1000) != 0) {
		this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_UNUSED_0x40;
	}

	if ((pWind->field_0x160 & 0x4000) != 0) {
		this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_UNUSED_0x80;
	}

	if ((pWind->field_0x160 & 0x8000) != 0) {
		this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_EMITTER_POOL_ACTIVE;
	}

	if ((pWind->field_0x160 & 0x2000) == 0) {
		bVar1 = false;
		if ((pWind->field_0x19c < -1.0f) && (pWind->field_0x1a0 < -1.0f)) {
			bVar1 = true;
		}

		if (!bVar1) {
			bVar1 = true;

			if ((((pWind->field_0x160 & 0x20) == 0) && (bVar1 = false, (pWind->field_0x160 & 0x40) == 0))
				&& (bVar1 = true, 0.17398384f <= fabsf(pWind->rotationQuat.y))) {
				bVar1 = false;
			}

			if (!bVar1) {
				return;
			}
		}
	}

	this->flags_0x54 = this->flags_0x54 | FXWIND_FLAG_CAMERA_ALIGNED;

	return;
}


void CFxWind::_Compute_Matrix(edF32MATRIX4* pCameraMatrix)
{
	edF32VECTOR4* peVar1;
	int iVar2;
	edF32VECTOR4* peVar3;
	edF32MATRIX4* peVar4;
	ed_3d_hierarchy_node* peVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float in_vf0x;
	float fVar9;
	float fVar10;
	float fVar11;

	this->windMatrix = this->pOwner->pMeshTransform->base.transformA;

	edF32Matrix4MulF32Vector4Hard(&(this->windMatrix).rowT, &this->windMatrix, &this->field_0x150);
	this->field_0x140 = (this->windMatrix).rowT;

	(this->windMatrix).rowY = (this->windMatrix).rowZ;

	if ((this->flags_0x54 & FXWIND_FLAG_CAMERA_ALIGNED) == 0) {
		peVar3 = &pCameraMatrix->rowZ;
		peVar1 = &pCameraMatrix->rowY;
	}
	else {
		peVar3 = &gF32Vector4UnitX;
		peVar1 = &gF32Vector4UnitZ;
	}

	if (fabsf((this->windMatrix).ba * peVar3->x + (this->windMatrix).bb * peVar3->y + (this->windMatrix).bc * peVar3->z) <= 0.999f) {
		peVar1 = peVar3;
	}

	fVar6 = peVar1->x;
	fVar7 = peVar1->y;
	fVar8 = peVar1->z;
	fVar9 = (this->windMatrix).ba;
	fVar10 = (this->windMatrix).bb;
	fVar11 = (this->windMatrix).bc;
	(this->windMatrix).aa = fVar7 * fVar11 - fVar10 * fVar8;
	(this->windMatrix).ab = fVar8 * fVar9 - fVar11 * fVar6;
	(this->windMatrix).ac = fVar6 * fVar10 - fVar9 * fVar7;
	(this->windMatrix).ad = in_vf0x;

	fVar6 = (this->windMatrix).aa;
	fVar7 = (this->windMatrix).ab;
	fVar8 = (this->windMatrix).ac;
	fVar9 = 1.0f / (sqrtf(fVar6 * fVar6 + fVar7 * fVar7 + fVar8 * fVar8) + 0.0f);
	(this->windMatrix).aa = fVar6 * fVar9;
	(this->windMatrix).ab = fVar7 * fVar9;
	(this->windMatrix).ac = fVar8 * fVar9;
	(this->windMatrix).ad = 0.0f;

	fVar6 = (this->windMatrix).aa;
	fVar7 = (this->windMatrix).ab;
	fVar8 = (this->windMatrix).ac;
	fVar9 = (this->windMatrix).ba;
	fVar10 = (this->windMatrix).bb;
	fVar11 = (this->windMatrix).bc;
	(this->windMatrix).ca = fVar7 * fVar11 - fVar10 * fVar8;
	(this->windMatrix).cb = fVar8 * fVar9 - fVar11 * fVar6;
	(this->windMatrix).cc = fVar6 * fVar10 - fVar9 * fVar7;
	(this->windMatrix).cd = 0.0f;

	fVar6 = (this->windMatrix).ca;
	fVar7 = (this->windMatrix).cb;
	fVar8 = (this->windMatrix).cc;
	fVar9 = 1.0f / (sqrtf(fVar6 * fVar6 + fVar7 * fVar7 + fVar8 * fVar8) + 0.0f);
	(this->windMatrix).ca = fVar6 * fVar9;
	(this->windMatrix).cb = fVar7 * fVar9;
	(this->windMatrix).cc = fVar8 * fVar9;
	(this->windMatrix).cd = 0.0f;

	fVar6 = (this->field_0x130).x;
	fVar7 = (this->windMatrix).ab;
	fVar8 = (this->windMatrix).ac;
	fVar9 = (this->windMatrix).ad;
	(this->field_0xa0).aa = (this->windMatrix).aa * fVar6;
	(this->field_0xa0).ab = fVar7 * fVar6;
	(this->field_0xa0).ac = fVar8 * fVar6;
	(this->field_0xa0).ad = fVar9 * fVar6;

	fVar6 = (this->field_0x130).y;
	fVar7 = (this->windMatrix).bb;
	fVar8 = (this->windMatrix).bc;
	fVar9 = (this->windMatrix).bd;
	(this->field_0xa0).ba = (this->windMatrix).ba * fVar6;
	(this->field_0xa0).bb = fVar7 * fVar6;
	(this->field_0xa0).bc = fVar8 * fVar6;
	(this->field_0xa0).bd = fVar9 * fVar6;

	fVar6 = (this->field_0x130).z;
	fVar7 = (this->windMatrix).cb;
	fVar8 = (this->windMatrix).cc;
	fVar9 = (this->windMatrix).cd;
	(this->field_0xa0).ca = (this->windMatrix).ca * fVar6;
	(this->field_0xa0).cb = fVar7 * fVar6;
	(this->field_0xa0).cc = fVar8 * fVar6;
	(this->field_0xa0).cd = fVar9 * fVar6;

	this->field_0xa0.rowT = this->windMatrix.rowT;

	edF32Matrix4GetInverseOrthoHard(&this->field_0xe0, &this->windMatrix);

	return;
}

void CFxWind::ChangeVisibleState(int state)
{
	uint patchId;
	CGlobalDListPatch* pPatch;
	int patchSectorId;
	uint flagActive;
	CGlobalDListManager* pDlistManager;

	pDlistManager = CScene::ptable.g_GlobalDListManager_004516bc;
	if ((this->flags_0x54 & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) == 0) {
		return;
	}

	if (((this->flags_0x54 & FXWIND_FLAG_ACTIVE) != 0) && (state != 0)) {
		// Outer patch
		patchId = this->outerPatchId;

		if (((CScene::ptable.g_GlobalDListManager_004516bc)->ppGlobalDlist == (GlobalDlistEntry*)0x0)
			|| ((patchSectorId = (int)patchId >> 0x10, (CScene::ptable.g_GlobalDListManager_004516bc)->activeSectorPatchId != patchSectorId &&
				(patchSectorId != 0)))) {
		LAB_002105b8:
			flagActive = 0xffffffff;
		}
		else {
			pPatch = (CScene::ptable.g_GlobalDListManager_004516bc)->ppGlobalDlist[patchSectorId].pDlistPatch;

			if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_002105b8;

			flagActive = (pPatch->aPatches[patchId & 0xffff]->flags & 1) != 0;
		}

		if (flagActive == 0) {
			CScene::ptable.g_GlobalDListManager_004516bc->SetActive(patchId, 1);

			(this->windSolid).whirlStateA.drawState = 2;
			(this->windSolid).outerStateA.drawState = 2;
			(this->windSolid).outerStateB.drawState = 2;
			(this->windSolid).fakePlaneState.drawState = 2;
			(this->windSolid).whirlStateB.drawState = 2;
		}
	}

	if (state == 0) {
		// Outer patch
		patchId = this->outerPatchId;

		if ((pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0) ||
			((patchSectorId = (int)patchId >> 0x10, pDlistManager->activeSectorPatchId != patchSectorId && (patchSectorId != 0))))
		{
		LAB_00210650:
			flagActive = 0xffffffff;
		}
		else {
			pPatch = pDlistManager->ppGlobalDlist[patchSectorId].pDlistPatch;
			if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_00210650;
			flagActive = (pPatch->aPatches[patchId & 0xffff]->flags & 1) != 0;
		}

		if (flagActive == 1) {
			pDlistManager->SetActive(patchId, 0);
		}
	}

	if (((this->flags_0x54 & FXWIND_FLAG_ACTIVE) != 0) && (state != 0)) {
		// Whirl patch
		patchId = this->whirlPatchId;

		if ((pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0) ||
			((patchSectorId = (int)patchId >> 0x10, pDlistManager->activeSectorPatchId != patchSectorId && (patchSectorId != 0))))
		{
		LAB_002106e8:
			flagActive = 0xffffffff;
		}
		else {
			pPatch = pDlistManager->ppGlobalDlist[patchSectorId].pDlistPatch;
			if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_002106e8;
			flagActive = (pPatch->aPatches[patchId & 0xffff]->flags & 1) != 0;
		}

		if (flagActive == 0) {
			pDlistManager->SetActive(patchId, 1);
		}
	}

	if (state == 0) {
		// Whirl patch
		patchId = this->whirlPatchId;

		if ((pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0) ||
			((patchSectorId = (int)patchId >> 0x10, pDlistManager->activeSectorPatchId != patchSectorId && (patchSectorId != 0))))
		{
		LAB_00210768:
			flagActive = 0xffffffff;
		}
		else {
			pPatch = pDlistManager->ppGlobalDlist[patchSectorId].pDlistPatch;
			if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_00210768;
			flagActive = (pPatch->aPatches[patchId & 0xffff]->flags & 1) != 0;
		}

		if (flagActive == 1) {
			pDlistManager->SetActive(patchId, 0);
		}
	}

	if (((this->flags_0x54 & FXWIND_FLAG_ACTIVE) != 0) && (state != 0)) {
		// Fake plane patch
		patchId = this->fakePlanePatchId;

		if ((pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0) ||
			((patchSectorId = (int)patchId >> 0x10, pDlistManager->activeSectorPatchId != patchSectorId && (patchSectorId != 0))))
		{
		LAB_00210800:
			flagActive = 0xffffffff;
		}
		else {
			pPatch = pDlistManager->ppGlobalDlist[patchSectorId].pDlistPatch;
			if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_00210800;
			flagActive = (pPatch->aPatches[patchId & 0xffff]->flags & 1) != 0;
		}

		if (flagActive == 0) {
			pDlistManager->SetActive(patchId, 1);
		}
	}

	if (state != 0) {
		return;
	}

	patchId = this->fakePlanePatchId;
	if ((pDlistManager->ppGlobalDlist != (GlobalDlistEntry*)0x0) &&
		((patchSectorId = (int)patchId >> 0x10, pDlistManager->activeSectorPatchId == patchSectorId || (patchSectorId == 0)))) {
		pPatch = pDlistManager->ppGlobalDlist[patchSectorId].pDlistPatch;
		if (pPatch != (CGlobalDListPatch*)0x0) {
			flagActive = (pPatch->aPatches[patchId & 0xffff]->flags & 1) != 0;
			goto LAB_00210888;
		}
	}

	flagActive = 0xffffffff;

LAB_00210888:
	if (flagActive == 1) {
		pDlistManager->SetActive(patchId, 0);
	}

	return;
}

edF32VECTOR4 edF32VECTOR4_0040f0f0 = { 0.0f, 0.0f, 6.2831855f, 0.0f };
edF32VECTOR4 edF32VECTOR4_0040f100 = { 0.0f, 0.0f, 0.25f, 0.25f };

CFxEmitterPool::CFxEmitterPool()
{
	void* pvVar1;
	int iVar2;
	CFxEmitterPool* pCVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	CFxLightEmitter* pLightEmitter;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	iVar4 = 0;

	local_10 = edF32VECTOR4_0040f0f0;
	local_20 = edF32VECTOR4_0040f100;

	this->field_0x0 = 1;
	this->field_0x4 = 0;
	this->field_0xc = 0;
	this->nbHandles = 0;
	this->field_0x14 = 0;
	this->field_0x8 = 0.0f;
	this->field_0x18 = 1.0f;

	for (int i = 0; i < 0x40; i++) {
		this->aWindHandles[i].pFxWind = (CFxWind*)0x0;
		this->aWindHandles[i].field_0x4 = 100000.0f;
	}

	this->field_0xd5c[0].nbRays = 0x10;
	this->field_0xd5c[1].nbRays = 0x40;
	this->field_0xd5c[2].nbRays = 200;

	iVar6 = 0;
	do {
		this->field_0xd5c[iVar6].pRayDef = new RAY_DEF[this->field_0xd5c[iVar6].nbRays];
		iVar6 = iVar6 + 1;
	} while (iVar6 < 3);

	iVar6 = 0;
	do {
		this->aHolders[iVar6].pFxWind = (CFxWind*)0x0;
		iVar5 = 0;
		do {
			pLightEmitter = this->aHolders[iVar6].field_0x0 + iVar5;
			if (iVar5 == 0) {
				pLightEmitter->Create(2.0f, 0x10, 0xf12);
			}
			else {
				if (iVar5 == 1) {
					pLightEmitter->Create(2.0f, 0x40, 0xe02);
					pLightEmitter->ChangeLengthAndWidth(0.2f, 0.04f, 0.06f, 0.04f);
				}
				else {
					pLightEmitter->Create(2.0f, 200, 0xe12);
					pLightEmitter->ChangeUvMapping(&local_20);
					pLightEmitter->ChangeLengthAndWidth(1.0f, 0.125f, 0.25f, 0.1f);
					pLightEmitter->ChangeUvSpeed(50.0f, 4, 4);
					pLightEmitter->ChangeColor(0, 0x80808080);
					pLightEmitter->ChangeColor(1, 0x80707070);
					pLightEmitter->ChangeColor(2, 0x80606060);
					pLightEmitter->ChangeColor(3, 0x80404040);
				}
			}

			pLightEmitter->SetSharedRays(this->field_0xd5c[iVar5].pRayDef);
			pLightEmitter->field_0x20 = local_10;
			pLightEmitter->Enable(0);
			iVar5 = iVar5 + 1;
		} while (iVar5 < 3);

		iVar6 = iVar6 + 1;
	} while (iVar6 < 4);

	iVar4 = 0;
	do {
		this->field_0xd20[iVar4] = 0.0f;
		this->field_0xd2c[iVar4] = 3.141593f / (float)(iVar4 + 1);
		this->aHolders[0].field_0x0[iVar4].InitRays(this->field_0xd5c[iVar4].nbRays);
		iVar4 = iVar4 + 1;
	} while (iVar4 < 3);

	(this->heatFxParam).field_0x0 = 4;
	(this->heatFxParam).field_0x4 = 4;
	(this->heatFxParam).field_0x8 = 1;
	(this->heatFxParam).field_0xc = 0x80808080;
	(this->heatFxParam).field_0x10 = 0;
	(this->heatFxParam).field_0x14 = 0.55f;
	(this->heatFxParam).field_0x18 = 0.0f;
	(this->heatFxParam).field_0x1c = 1.25f;
	(this->heatFxParam).field_0x20 = 0x10;

	return;
}

void CFxEmitterPool::Manage(CFxWind* pFxWind)
{
	CActorWind* pCVar1;
	CFxWind* pCVar2;
	bool bVar3;
	edF32VECTOR4* peVar4;
	_SP_PartNfo* pSubPartInfo;
	edF32VECTOR4* peVar6;
	int iVar7;
	edF32VECTOR4* peVar8;
	CFxLightEmitter* pCVar9;
	uint uVar10;
	ulong uVar11;
	CFxWindHandle* pCVar12;
	int iVar13;
	_EmiNfo* pCurEmiInfo;
	CFxWindHandle* pCVar14;
	int iVar15;
	CFxLightEmitter* this_00;
	CFxLightEmitter* pCVar16;
	undefined4* pCVar17;
	int* piVar18;
	int iVar19;
	int iVar20;
	float fVar21;
	RayMem* local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 auStack64;

	iVar15 = -1;
	iVar20 = 0;
	CFxWindHandle* pCurHandle = this->aWindHandles;
	while ((iVar20 < 0x40 && (iVar15 == -1))) {
		if (pCurHandle->pFxWind == pFxWind) {
			iVar15 = iVar20;
		}

		pCurHandle = pCurHandle + 1;
		iVar20 = iVar20 + 1;
	}

	pCVar12 = (CFxWindHandle*)0x0;
	if (iVar15 != -1) {
		pCVar12 = this->aWindHandles + iVar15;
	}

	if (pCVar12 == (CFxWindHandle*)0x0) {
		return;
	}

	if ((pFxWind->flags_0x54 & FXWIND_FLAG_ACTIVE) != 0) {
		pCVar1 = pFxWind->pOwner;
		bVar3 = (pCVar1->flags & 0x4400) != 0;
		if (bVar3) {
			bVar3 = pCVar1->distanceToCamera <= (pCVar1->subObjA)->cullingDistance;
		}

		if (bVar3) {
			pCVar12->field_0x4 = pFxWind->distanceToCameraScale;
			goto LAB_0020e508;
		}
	}

	pCVar12->field_0x4 = 100000.0f;
LAB_0020e508:
	if ((this->field_0x8 != GetTimer()->scaledTotalTime) || (GetTimer()->cutsceneDeltaTime == 0.0f)) {
		iVar15 = 0;
		this->field_0x8 = GetTimer()->scaledTotalTime;
		this->field_0x4 = 0;
		do {
			fVar21 = this->field_0xd20[iVar15] + this->field_0xd2c[iVar15] * GetTimer()->cutsceneDeltaTime;
			this->field_0xd20[iVar15] = fVar21;
			fVar21 = edF32Between_2Pi(fVar21);
			this->field_0xd20[iVar15] = fVar21;
			iVar15 = iVar15 + 1;
		} while (iVar15 < 3);

		_SortRegNfo(this->field_0x0);

		this->field_0x0 = 0;
		iVar15 = this->field_0xc;
		if (3 < iVar15) {
			iVar15 = 4;
		}

		iVar20 = 0;
		_EmiNfo* pCurEmiInfo = this->aHolders;
		do {
			pCVar2 = pCurEmiInfo->pFxWind;
			iVar19 = -1;

			if (pCVar2 != (CFxWind*)0x0) {
				iVar13 = 0;
				pCurHandle = this->aWindHandles;
				while ((iVar13 < iVar15 && (iVar19 == -1))) {
					if (pCurHandle->pFxWind == pCVar2) {
						iVar19 = iVar13;
					}

					pCurHandle = pCurHandle + 1;
					iVar13 = iVar13 + 1;
				}

				pCVar12 = (CFxWindHandle*)0x0;
				if (iVar19 != -1) {
					pCVar12 = this->aWindHandles + iVar19;
				}

				if ((pCVar12 == (CFxWindHandle*)0x0) || ((pCVar2->flags_0x54 & FXWIND_FLAG_ACTIVE) == 0)) {
					iVar19 = -1;
					iVar13 = 0;
					_EmiNfo* pCurEmiInfoInternal = this->aHolders;
					do {
						if (pCVar2 == pCurEmiInfoInternal->pFxWind) {
							iVar19 = iVar13;
						}
						iVar13 = iVar13 + 1;
						pCurEmiInfoInternal = pCurEmiInfoInternal + 1;
					} while ((iVar13 < 4) && (iVar19 == -1));

					pCurEmiInfoInternal = (_EmiNfo*)0x0;
					if (iVar19 != -1) {
						pCurEmiInfoInternal = this->aHolders + iVar19;
					}

					iVar19 = 0;
					RayMem* pRayMem = this->field_0xd5c;
					this_00 = pCurEmiInfoInternal->field_0x0;
					if (pCurEmiInfoInternal != (_EmiNfo*)0x0) {
						do {
							this_00->Enable(0);
							this_00->ReleaseRays(pRayMem->nbRays);
							iVar19 = iVar19 + 1;
							pRayMem = pRayMem + 1;
							this_00 = this_00 + 1;
						} while (iVar19 < 3);

						pCurEmiInfoInternal->pFxWind = (CFxWind*)0x0;
						this->field_0x14 = this->field_0x14 + -1;
					}
				}
			}

			iVar20 = iVar20 + 1;
			pCurEmiInfo = pCurEmiInfo + 1;
		} while (iVar20 < 4);

		iVar15 = 0;
		pCVar14 = this->aWindHandles;
		do {
			pCVar2 = pCVar14->pFxWind;
			iVar20 = -1;
			iVar19 = 0;
			pCurEmiInfo = this->aHolders;
			do {
				if (pCVar2 == pCurEmiInfo->pFxWind) {
					iVar20 = iVar19;
				}

				iVar19 = iVar19 + 1;
				pCurEmiInfo = pCurEmiInfo + 1;
			} while ((iVar19 < 4) && (iVar20 == -1));

			pCurEmiInfo = (_EmiNfo*)0x0;
			if (iVar20 != -1) {
				pCurEmiInfo = this->aHolders + iVar20;
			}

			if ((((pCurEmiInfo == (_EmiNfo*)0x0) && ((pCVar2->flags_0x54 & FXWIND_FLAG_ACTIVE) != 0)) && ((pCVar2->flags_0x54 & FXWIND_FLAG_EMITTER_POOL_ACTIVE) == 0)) && (pCVar14->field_0x4 < 0.3f)) {
				iVar20 = -1;
				iVar19 = 0;
				_EmiNfo* pCurEmiInfoInternal = this->aHolders;
				if (this->field_0x14 < 4) {
					do {
						if (pCurEmiInfoInternal->pFxWind == (CFxWind*)0x0) {
							iVar20 = iVar19;
						}
						iVar19 = iVar19 + 1;
					} while ((iVar19 < 4) && (pCurEmiInfoInternal = pCurEmiInfoInternal + 1, iVar20 == -1));

					_EmiNfo* pCurEmiInfoInternal = this->aHolders + iVar20;
					pCurEmiInfoInternal->pFxWind = pCVar2;
					pCurEmiInfo = pCurEmiInfoInternal;
					pCurEmiInfoInternal->field_0x2b0 = 0.0f;
					this->field_0x14 = this->field_0x14 + 1;
				}
				else {
					pCurEmiInfo = (_EmiNfo*)0x0;
				}

				if (pCurEmiInfo != (_EmiNfo*)0x0) {
					bVar3 = pCVar2->pOwner->field_0x19c < -1.0f;
					if ((bVar3) && (bVar3 = true, -1.0f <= pCVar2->pOwner->field_0x1a0)) {
						bVar3 = false;
					}

					if (bVar3) {
						pSubPartInfo = reinterpret_cast<_SP_PartNfo*>(gSP_Manager.GetFreeBuffer(sizeof(_SP_PartNfo)));
						pSubPartInfo->field_0x4[0] = 1;
						pSubPartInfo->field_0x4[1] = 1;
						pSubPartInfo->field_0x4[2] = 2;
						pSubPartInfo->field_0x0 = &gF32Vector4UnitY;
						pSubPartInfo->field_0x10 = 16.0f;
						pSubPartInfo->field_0x14 = 13.0f;
						pSubPartInfo->field_0x18 = pCurEmiInfo->pFxWind->field_0x16c * 1.5f;
						fVar21 = edFIntervalLERP(pCurEmiInfo->pFxWind->field_0x16c, 5.0f, 11.0f, 0.5f, 8.0f);
						pSubPartInfo->field_0x1c = fVar21;
						_PrepareCommonSubPart(pCurEmiInfo, pSubPartInfo);
						gSP_Manager.ReleaseBuffer(pSubPartInfo);
					}
					else {
						pSubPartInfo = reinterpret_cast<_SP_PartNfo*>(gSP_Manager.GetFreeBuffer(sizeof(_SP_PartNfo)));
						pSubPartInfo->field_0x4[0] = 1;
						pSubPartInfo->field_0x4[1] = 1;
						pSubPartInfo->field_0x4[2] = 2;
						pSubPartInfo->field_0x0 = &gF32Vector4UnitY;
						pSubPartInfo->field_0x14 = (pCurEmiInfo->pFxWind->field_0x130).x;
						pSubPartInfo->field_0x10 = (pCurEmiInfo->pFxWind->field_0x130).y;
						fVar21 = pCurEmiInfo->pFxWind->field_0x16c * 1.5f;
						pSubPartInfo->field_0x18 = fVar21;
						if (30.0f < fVar21) {
							pSubPartInfo->field_0x18 = 30.0f;
						}

						fVar21 = edFIntervalLERP(pCurEmiInfo->pFxWind->field_0x16c, 5.0f, 11.0f, 0.5f, 8.0f);
						pSubPartInfo->field_0x1c = fVar21;
						_PrepareCommonSubPart(pCurEmiInfo, pSubPartInfo);
						gSP_Manager.ReleaseBuffer(pSubPartInfo);
					}
				}
			}

			iVar15 = iVar15 + 1;
			pCVar14 = pCVar14 + 1;
		} while (iVar15 < 4);

		iVar15 = 0;
		pCurEmiInfo = this->aHolders;
		do {
			iVar20 = 0;
			pCVar16 = pCurEmiInfo->field_0x0;
			pCVar17 = pCurEmiInfo->field_0x2a4;
			do {
				pCVar2 = pCurEmiInfo->pFxWind;
				*pCVar17 = 0;
				if ((pCVar2 != (CFxWind*)0x0) && ((pCVar2->flags_0x54 & FXWIND_FLAG_EMITTER_POOL_ACTIVE) == 0)) {
					bVar3 = pCVar2->pOwner->field_0x19c < -1.0f;
					if ((bVar3) && (bVar3 = true, -1.0f <= pCVar2->pOwner->field_0x1a0)) {
						bVar3 = false;
					}

					if (bVar3) {
						local_50 = {};

						iVar19 = rand();
						local_50.x = 0.08726646f - ((float)iVar19 / 2.147484e+09f) * 0.1745329f;
						iVar19 = rand();
						local_50.y = 0.08726646f - ((float)iVar19 / 2.147484e+09f) * 0.1745329f;
						iVar19 = rand();
						local_50.z = 0.08726646f - ((float)iVar19 / 2.147484e+09f) * 0.1745329f;
						edF32Matrix4RotateXYZHard(&auStack64, &gF32Matrix4Unit, &local_50);
						pCVar16->ChangeDirection(&auStack64.rowY);
					}
				}

				iVar20 = iVar20 + 1;
				pCVar17 = pCVar17 + 1;
				pCVar16 = pCVar16 + 1;
			} while (iVar20 < 3);

			iVar15 = iVar15 + 1;
			pCurEmiInfo = pCurEmiInfo + 1;
		} while (iVar15 < 4);

		iVar20 = 0;
		iVar15 = 0;
		pCurEmiInfo = this->aHolders;
		do {
			if (pCurEmiInfo->pFxWind != (CFxWind*)0x0) {
				iVar20 = iVar20 + 1;
			}
			iVar15 = iVar15 + 1;
			pCurEmiInfo = pCurEmiInfo + 1;
		} while (iVar15 < 4);

		iVar15 = 0;
		pCurEmiInfo = this->aHolders;
		do {
			pCVar2 = pCurEmiInfo->pFxWind;
			if (pCVar2 == (CFxWind*)0x0) {
				pCurEmiInfo->field_0x2b4 = 0.0f;
			}
			else {
				fVar21 = edFIntervalLERP(pCVar2->distanceToCameraScale, 0.1f, 0.3f, 1.0f, 0.0f);
				pCurEmiInfo->field_0x2b4 = fVar21 / (float)iVar20;
			}

			iVar15 = iVar15 + 1;
			pCurEmiInfo = pCurEmiInfo + 1;
		} while (iVar15 < 4);

		iVar15 = 0;
		if (iVar20 != 0) {

			iVar19 = 0;
			local_60 = this->field_0xd5c;
			IMPLEMENTATION_GUARD_FX(
			do {
				this->field_0xd74.Init(local_60->pRayDef, local_60->nbRays);

				iVar13 = (int)this->aWindHandles + (*(int*)&this->field_0xd74 % 3) * 0xc + -0x20;
				uVar11 = SEXT48(*(int*)(iVar13 + 0xd98) << 6);
				if ((*(int*)(iVar13 + 0xd98) != 0) && (uVar11 <= (ulong)(long)*(int*)&this->field_0xd84))
				{
					edDmaLoadToFastRam_nowait(*(undefined4*)(iVar13 + 0xd90), uVar11, *(undefined4*)(iVar13 + 0xd94));
				}
				do {
					this->field_0xd74.Sync(1);

					iVar13 = (int)this->aWindHandles + (*(int*)&this->field_0xd74 % 3) * 0xc + -0x20;
					uVar11 = SEXT48(*(int*)(iVar13 + 0xd98) << 6);
					if ((*(int*)(iVar13 + 0xd98) != 0) && (uVar11 <= (ulong)(long)*(int*)&this->field_0xd84)
						) {
						edDmaLoadToFastRam_nowait
						(*(undefined4*)(iVar13 + 0xd90), uVar11, *(undefined4*)(iVar13 + 0xd94));
					}
					iVar13 = (int)this->aWindHandles + ((*(int*)&this->field_0xd74 + 1) % 3) * 0xc + -0x20;
					uVar10 = *(int*)(iVar13 + 0xd98) << 6;
					if ((uVar10 <= *(uint*)&this->field_0xd84) && (*(int*)(iVar13 + 0xd98) != 0)) {
						edDmaLoadFromFastRam_nowait
						(*(void**)(iVar13 + 0xd94), uVar10, *(void**)(iVar13 + 0xd90));
					}
					iVar13 = 0;
					pCVar14 = this;
					do {
						if (pCVar14->aHolders[0].pFxWind != (CFxWind*)0x0) {
							pCVar9 = (CFxLightEmitter*)((int)&pCVar14->aHolders[0].field_0x0[0].flags + iVar20);
							uVar10 = (uint)(pCVar14->aHolders[0].field_0x2b4 *
								(float)local_60->field_0xd5c[0].nbRays);
							iVar7 = (int)&pCVar14->aHolders[0].field_0x0[0].flags + iVar19;
							if (((pCVar9->flags & 1) != 0) &&
								(piVar18 = (int*)(iVar7 + 0x2a4), *(int*)(iVar7 + 0x2a4) < (int)uVar10)) {
								iVar7 = (int)this->aWindHandles +
									((*(int*)&this->field_0xd74 + 2) % 3) * 0xc + -0x20;
								iVar7 = CFxLightEmitter::ManageSlice
								(pCVar9, *(edF32VECTOR4**)(iVar7 + 0xd94), *(int*)(iVar7 + 0xd98),
									0, uVar10);
								*piVar18 = *piVar18 + iVar7;
							}
						}
						iVar13 = iVar13 + 1;
						pCVar14 = (CFxEmitterPool*)&pCVar14->aHolders[0].field_0x0[0].field_0xa0;
					} while (iVar13 < 4);
					FASTRAM_MNG::Sync((FASTRAM_MNG*)&this->field_0xd74, 0);
				} while (*(int*)((int)this->field_0xd5c + (*(int*)&this->field_0xd74 % 3) * 0xc + 0x3c) !=
					0);
				iVar13 = (int)this->aWindHandles + ((*(int*)&this->field_0xd74 + 2) % 3) * 0xc + -0x20;
				uVar10 = *(int*)(iVar13 + 0xd98) << 6;
				if ((uVar10 <= *(uint*)&this->field_0xd84) && (*(int*)(iVar13 + 0xd98) != 0)) {
					edDmaLoadFromFastRam_nowait(*(void**)(iVar13 + 0xd94), uVar10, *(void**)(iVar13 + 0xd90));
				}

				this->field_0xd74.Sync(0);

				iVar13 = 0;
				pCurEmiInfo = this->aHolders;
				do {
					if (pCurEmiInfo->field_0x2b0 == 1.0f) {
						iVar7 = 0;
						do {
							pCVar2 = pCurEmiInfo->pFxWind;
							if (pCVar2 != (CFxWind*)0x0) {
								pCurEmiInfo->field_0x0[iVar7].ChangeAlphaFactor(pCVar2->field_0x2c4);
							}
							iVar7 = iVar7 + 1;
						} while (iVar7 < 3);
					}
					else {
						fVar21 = pCurEmiInfo->field_0x2b0 + this->field_0x18 * GetTimer()->cutsceneDeltaTime;
						pCurEmiInfo->field_0x2b0 = fVar21;
						if (1.0f < fVar21) {
							pCurEmiInfo->field_0x2b0 = 1.0f;
						}

						iVar7 = 0;
						do {
							pCurEmiInfo->field_0x0[iVar7].ChangeAlphaFactor(pCurEmiInfo->field_0x2b0);
							iVar7 = iVar7 + 1;
						} while (iVar7 < 3);
					}

					iVar13 = iVar13 + 1;
					pCVar14 = pCVar14 + 1;
				} while (iVar13 < 4);

				gSP_Manager.ReleaseBuffer(this->field_0xd74.field_0x8);
				gSP_Manager.ReleaseBuffer(this->field_0xd74.field_0x4);

				iVar15 = iVar15 + 1;
				iVar20 = iVar20 + 0xe0;
				local_60 = local_60 + 1;
				iVar19 = iVar19 + 4;
			} while (iVar15 < 3);)
		}
	}

	return;
}

void CFxEmitterPool::_SortRegNfo(int param_2)
{
	CFxWindHandle* pSwapBuffer;
	CFxWindHandle* pHandleBuffer;
	int iVar3;
	int iVar4;
	CFxWindHandle* pOrigHandle;
	int iVar6;
	CFxWindHandle* pCurHandle;

	pSwapBuffer = (CFxWindHandle*)gSP_Manager.GetFreeBuffer(sizeof(CFxWindHandle*) * 2);
	pHandleBuffer = (CFxWindHandle*)gSP_Manager.GetFreeBuffer(sizeof(CFxWindHandle) * 0x40);

	iVar6 = 0;
	pOrigHandle = this->aWindHandles;
	pCurHandle = pHandleBuffer;
	do {
		if (pOrigHandle->pFxWind != (CFxWind*)0x0) {
			*pCurHandle = *pOrigHandle;
			pCurHandle = pCurHandle + 1;
		}

		iVar6 = iVar6 + 1;
		pOrigHandle = pOrigHandle + 1;
	} while (iVar6 < 0x40);

	if (param_2 == 0) {
		iVar6 = 0;
		pCurHandle = pHandleBuffer;
		if (0 < this->nbHandles + -1) {
			do {
				if (pCurHandle[1].field_0x4 <= pCurHandle->field_0x4) {
					*pSwapBuffer = *pCurHandle;
					*pCurHandle = pCurHandle[1];
					pCurHandle[1] = *pSwapBuffer;
				}
				iVar6 = iVar6 + 1;
				pCurHandle = pCurHandle + 1;
			} while (iVar6 < this->nbHandles + -1);
		}
	}
	else {
		iVar6 = this->nbHandles;
		iVar4 = 0;
		if (0 < iVar6 + -2) {
			do {
				iVar6 = iVar6 + -1;
				iVar3 = 0;
				pCurHandle = pHandleBuffer;
				if (0 < iVar6) {
					do {
						if (pCurHandle[1].field_0x4 <= pCurHandle->field_0x4) {
							*pSwapBuffer = *pCurHandle;
							*pCurHandle = pCurHandle[1];
							pCurHandle[1] = *pSwapBuffer;
						}

						iVar3 = iVar3 + 1;
						pCurHandle = pCurHandle + 1;
					} while (iVar3 < iVar6);
				}

				iVar4 = iVar4 + 1;
			} while (iVar4 < this->nbHandles + -2);
		}
	}

	this->field_0xc = 0;
	iVar6 = this->nbHandles;
	iVar4 = 0;
	pOrigHandle = this->aWindHandles;
	pCurHandle = pHandleBuffer;
	if (0 < iVar6) {
		do {
			*pOrigHandle = *pCurHandle;
			if (pCurHandle->field_0x4 < 0.3f) {
				this->field_0xc = this->field_0xc + 1;
			}

			iVar6 = this->nbHandles;
			iVar4 = iVar4 + 1;
			pOrigHandle = pOrigHandle + 1;
			pCurHandle = pCurHandle + 1;
		} while (iVar4 < iVar6);
	}

	if (iVar6 < 0x40) {
		pCurHandle = this->aWindHandles + iVar6 + -4;
		do {
			iVar6 = iVar6 + 1;
			pCurHandle[4].pFxWind = (CFxWind*)0x0;
			pCurHandle = pCurHandle + 1;
		} while (iVar6 < 0x40);
	}

	gSP_Manager.ReleaseBuffer(pHandleBuffer);
	gSP_Manager.ReleaseBuffer(pSwapBuffer);

	return;
}

float ComputeAccelDuration(float param_1, float param_2, float param_3)
{
	return (param_2 - param_1) / param_3;
}

void CFxEmitterPool::_PrepareCommonSubPart(_EmiNfo* pEmiInfo, _SP_PartNfo* pSubPartInfo)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int* p_Var4;
	CFxLightEmitter* pLightEmitter;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float unaff_f20;
	float fVar9;

	fVar2 = gF32Vertex4Zero.w;
	fVar1 = gF32Vertex4Zero.z;
	iVar5 = 0;
	fVar3 = gF32Vertex4Zero.x;
	fVar6 = (pEmiInfo->pFxWind->field_0x130).y;
	pLightEmitter = pEmiInfo->field_0x0;
	p_Var4 = pSubPartInfo->field_0x4;

	do {
		pLightEmitter->Enable(1);
		pLightEmitter->ChangeMaterialId(pEmiInfo->pFxWind->pOwner->materialId, *p_Var4);
		pLightEmitter->ChangeDirection(pSubPartInfo->field_0x0);
		pLightEmitter->field_0x10.x = fVar3;
		pLightEmitter->field_0x10.y = -fVar6 * 0.5f;
		pLightEmitter->field_0x10.z = fVar1;
		pLightEmitter->field_0x10.w = fVar2;
		pLightEmitter->field_0x68 = pSubPartInfo->field_0x10;
		fVar7 = GetAngleXFromVector(pSubPartInfo->field_0x0);
		fVar8 = GetAngleYFromVector(pSubPartInfo->field_0x0);
		pLightEmitter->field_0x40 = fVar7;
		pLightEmitter->field_0x44 = fVar8;
		if (iVar5 == 2) {
			unaff_f20 = 30.0f;
		}
		else {
			if (iVar5 == 1) {
				unaff_f20 = 150.0f;
			}
			else {
				if (iVar5 == 0) {
					unaff_f20 = 15.0f;
				}
			}
		}

		fVar7 = pSubPartInfo->field_0x18;
		pLightEmitter->ChangeSpeedAndAccel(fVar7, fVar7, fVar7, unaff_f20);
		fVar8 = pSubPartInfo->field_0x18;
		fVar9 = pSubPartInfo->field_0x10;
		fVar7 = ComputeAccelDistance(fVar8, fVar8, unaff_f20);
		fVar9 = fVar9 - fVar7;
		if (fVar9 < 0.0f) {
			fVar9 = 0.0f;
		}

		fVar7 = ComputeAccelDuration(fVar8, fVar8, unaff_f20);
		pLightEmitter->field_0x60 = fVar9 / fVar8 + fVar7;
		if (iVar5 == 1) {
			pLightEmitter->ChangeLengthAndWidth
			(pSubPartInfo->field_0x1c, 0.035f, 0.045f, 0.04f);
		}

		pLightEmitter->ChangeStallTime(pLightEmitter->field_0x60);
		pLightEmitter->ChangeRadius(0.1f, pSubPartInfo->field_0x14);
		iVar5 = iVar5 + 1;
		pLightEmitter = pLightEmitter + 1;
		p_Var4 = p_Var4 + 1;
	} while (iVar5 < 3);

	return;
}

void CFxEmitterPool::Draw()
{
	IMPLEMENTATION_GUARD_FX();
	return;
}

void CFxWindSolid::_Create_SemiCylinder(_wind_fx_vtx* pVtx, int param_3, int param_4, int param_5)
{
	CFxWind* pCVar1;
	uint uVar2;
	uint uVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	pCVar1 = this->pOwner;
	uVar2 = this->field_0x4;
	uVar3 = pCVar1->flags_0x54;
	fVar9 = (pCVar1->field_0x130).x;
	fVar7 = (pCVar1->field_0x130).y;
	fVar10 = 0.0f;

	for (iVar4 = 0; iVar4 < (int)this->field_0x4; iVar4 = iVar4 + 1) {
		fVar8 = 0.0f;
		fVar6 = 0.0f;

		for (iVar5 = 0; iVar5 < 7; iVar5 = iVar5 + 1) {
			pVtx->field_0x0 = cosf(fVar8);
			pVtx->field_0x4 = (float)iVar4 / (float)(this->field_0x4 - 1) + -0.5f;
			if (param_5 == 0) {
				pVtx->field_0x8 = -cosf(fVar8);
			}
			else {
				pVtx->field_0x8 = cosf(fVar8);
			}

			pVtx->field_0x10 = fVar10;
			pVtx->field_0x14 = fVar6;
			fVar12 = 1.0f;
			if ((uVar3 & 4) == 0) {
				fVar12 = fabsf(pVtx->field_0x8);
			}

			if (param_3 == 0) {
				fVar11 = 1.0f;
				if (param_4 != 0) {
					fVar11 = 0.5f - pVtx->field_0x4;
				}
			}
			else {
				if (param_4 == 0) {
					fVar11 = pVtx->field_0x4 + 0.5f;
				}
				else {
					fVar11 = 1.0f - fabsf(pVtx->field_0x4) * 2.0f;
				}
			}

			pVtx->field_0xc = fVar12 * fVar11;
			fVar8 = fVar8 + 0.5235988f;
			fVar6 = fVar6 + 0.1428571f;
			pVtx->field_0x0 = pVtx->field_0x0 * fVar9;
			pVtx->field_0x4 = pVtx->field_0x4 * fVar7;
			pVtx->field_0x8 = pVtx->field_0x8 * fVar9;
			pVtx = pVtx + 1;
		}

		fVar10 = fVar10 + 1.0f / (float)(uVar2 - 1);
	}

	return;
}

void CFxWindSolid::_Create_FakePlane(float param_1, float param_2, float param_3, float param_4, _wind_fx_vtx* pVtx, int param_7, int param_8)
{
	CFxWind* pCVar1;
	int iVar2;
	int iVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	fVar6 = 1.0f;
	if (param_7 != 0) {
		fVar6 = 0.0f;
	}
	fVar4 = 1.0f;
	if (param_8 == 0) {
		pCVar1 = this->pOwner;
	}
	else {
		fVar4 = 0.0f;
		pCVar1 = this->pOwner;
	}
	iVar3 = 0;
	fVar5 = (pCVar1->field_0x130).x;
	fVar9 = (pCVar1->field_0x130).y * 0.5f;
	if (0 < (int)this->field_0x4) {
		do {
			iVar2 = 0;
			do {
				fVar8 = fVar6;
				if ((iVar3 != 0) && (fVar8 = 1.0f, iVar3 == this->field_0x4 - 1)) {
					fVar8 = fVar4;
				}
				fVar7 = (float)iVar2;
				iVar2 = iVar2 + 1;
				pVtx->field_0x0 = -fVar5 + (fVar7 * fVar5 * 2.0f) / 2.0f;
				pVtx->field_0x4 = -fVar9 + ((float)iVar3 * fVar9 * 2.0f) / (float)(this->field_0x4 - 1);
				pVtx->field_0x8 = param_4;
				pVtx->field_0xc = fVar8;
				pVtx->field_0x10 = param_1 + (fVar7 * param_2) / 2.0f;
				pVtx->field_0x14 = ((float)iVar3 * param_3) / (float)(this->field_0x4 - 1);
				pVtx = pVtx + 1;
			} while (iVar2 < 3);
			iVar3 = iVar3 + 1;
		} while (iVar3 < (int)this->field_0x4);
	}
	return;
}


void CFxWindSolid::_Create_Whirl(_wind_fx_vtx* pVtx)
{
	int iVar1;
	int iVar2;
	int iVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	iVar3 = 0;
	fVar6 = (this->pOwner->field_0x130).x;
	fVar5 = (this->pOwner->field_0x130).y * 0.5f;
	do {
		iVar2 = 0;
		do {
			iVar1 = 0;
			do {
				pVtx->field_0x0 = -fVar6 + ((float)iVar1 * fVar6 * 2.0f) / 2.0f;
				pVtx->field_0x4 = -fVar5 + (float)iVar3 * fVar5 * 2.0f;
				pVtx->field_0x8 = -fVar6 + ((float)iVar2 * fVar6 * 2.0f) / 2.0f;
				if ((iVar2 == 1) && (iVar1 == 1)) {
					pVtx->field_0xc = 1.0f;
				}
				else {
					pVtx->field_0xc = 0.0f;
				}
				fVar4 = 0.5f;
				if (iVar1 != 1) {
					fVar4 = 0.0f;
				}
				pVtx->field_0x10 = fVar4 + 0.5f;
				fVar4 = 1.0f;
				if (iVar2 != 1) {
					fVar4 = 0.0f;
				}
				iVar1 = iVar1 + 1;
				pVtx->field_0x14 = fVar4;
				pVtx = pVtx + 1;
			} while (iVar1 < 3);
			iVar2 = iVar2 + 1;
		} while (iVar2 < 3);
		iVar3 = iVar3 + 1;
	} while (iVar3 < 2);
	return;
}

bool CFxWindSolid::CreatePatchableDlist(int patchId)
{
	byte bVar1;
	_wind_fx_vtx* pVtx;
	byte* puVar2;
	byte* pbVar3;
	byte* pbVar4;
	int iVar5;
	int iVar6;
	float* pfVar7;
	ed_3d_hierarchy_node* peVar8;
	uint uVar9;
	byte* pbVar10;
	uint uVar11;
	int iVar12;
	_wind_fx_vtx* p_Var13;
	_wind_fx_vtx* p_Var14;
	int iVar15;
	uint uVar16;
	int iVar17;
	_wind_fx_vtx* p_Var18;
	float fVar19;
	float fVar20;
	int local_70;
	int local_60;
	edF32MATRIX4 local_40;

	pVtx = (_wind_fx_vtx*)gSP_Manager.GetFreeBuffer(this->field_0x4 * 0x150);
	local_40 = this->pOwner->pOwner->pMeshTransform->base.transformA;
	
	edF32Matrix4MulF32Vector4Hard(&local_40.rowT, &local_40, &this->pOwner->field_0x150);
	this->field_0x8c = this->pOwner->pOwner->field_0x1c8;

	if (patchId == this->outerPatchId) {
		edDListLoadMatrix(&local_40);
		edDListUseMaterial(&this->pOwner->aCombinedMaterials[0].material);
		edDListBegin(0.0f, 0.0f, 0.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, (this->field_0x4 - 1) * 0x1c);
		uVar16 = this->pOwner->flags_0x54;
		_Create_SemiCylinder(pVtx, (uint)((uVar16 & 8) != 0), (uint)((uVar16 & 0x10) != 0), 0);
		this->outerStateA.index = 0;
		uVar16 = this->pOwner->flags_0x54;
		_Create_SemiCylinder(pVtx + this->field_0x4 * 7, (uint)((uVar16 & 8) != 0), (uint)((uVar16 & 0x10) != 0), 1);
		iVar5 = 0;
		this->outerStateB.index = (this->field_0x4 - 1) * 0xe;
		p_Var18 = pVtx;
		do {
			local_60 = 0;
			uVar16 = this->field_0x4;
			iVar15 = 0;
			if (0 < (int)(uVar16 - 1)) {
				local_70 = 0;
				do {
					uVar16 = 0;
					iVar17 = local_70 + 7;
					iVar12 = local_70;
					do {
						p_Var14 = p_Var18 + iVar12;
						p_Var13 = p_Var18 + iVar17;
						if (iVar5 == 0) {
							iVar6 = iVar15 + 1;
							puVar2 = this->outerStateA.pAlphaMultiplier + iVar15;
							iVar15 = iVar15 + 2;
							*puVar2 = (int)(p_Var14->field_0xc * 128.0f);
							this->outerStateA.pAlphaMultiplier[iVar6] = (int)(p_Var13->field_0xc * 128.0f);
						}

						edDListTexCoo2f(p_Var14->field_0x10, p_Var14->field_0x14);
						edDListColor4u8(this->field_0x8c.r, this->field_0x8c.g, this->field_0x8c.b, 0);

						if (uVar16 == 0) {
							fVar20 = 49152.0f;
							fVar19 = p_Var14->field_0x0;
						}
						else {
							fVar20 = 1.0f;
							fVar19 = p_Var14->field_0x0;
						}

						edDListVertex4f(fVar19, p_Var14->field_0x4, p_Var14->field_0x8, fVar20);
						edDListTexCoo2f(p_Var13->field_0x10, p_Var13->field_0x14);
						edDListColor4u8(this->field_0x8c.r, this->field_0x8c.g, this->field_0x8c.b, 0);

						if (uVar16 == 0) {
							fVar20 = 49152.0f;
							fVar19 = p_Var13->field_0x0;
						}
						else {
							fVar20 = 1.0f;
							fVar19 = p_Var13->field_0x0;
						}

						edDListVertex4f(fVar19, p_Var13->field_0x4, p_Var13->field_0x8, fVar20);
						uVar16 = uVar16 + 1;
						iVar12 = iVar12 + 1;
						iVar17 = iVar17 + 1;
					} while (uVar16 < 7);

					local_70 = local_70 + 7;
					local_60 = local_60 + 1;
					uVar16 = this->field_0x4;
				} while (local_60 < (int)(uVar16 - 1));
			}

			iVar5 = iVar5 + 1;
			p_Var18 = pVtx + uVar16 * 7;
		} while (iVar5 < 2);

		edDListEnd();
	}
	else {
		if (patchId == this->whirlPatchId) {
			pbVar3 = (byte*)gSP_Manager.GetFreeBuffer(this->field_0x4 << 2);
			pbVar4 = (byte*)gSP_Manager.GetFreeBuffer(0x18);
			uVar16 = 0;
			pbVar10 = pbVar3;
			do {
				uVar11 = 0;
				if (this->field_0x4 != 0) {
					iVar5 = uVar16 + 1;
					uVar9 = uVar16;
					do {
						bVar1 = 0x80;
						if (uVar11 != 0) {
							bVar1 = 0;
						}
						uVar11 = uVar11 + 1;
						*pbVar10 = bVar1 | (byte)uVar9;
						pbVar10[1] = bVar1 | (byte)iVar5;
						uVar9 = uVar9 + 3;
						iVar5 = iVar5 + 3;
						pbVar10 = pbVar10 + 2;
					} while (uVar11 < this->field_0x4);
				}
				uVar16 = uVar16 + 1;
			} while (uVar16 < 2);

			uVar16 = 0;
			iVar5 = 0;
			pbVar10 = pbVar4;
			do {
				uVar11 = 0;
				do {
					iVar12 = uVar11 + iVar5;
					uVar9 = 0;
					iVar15 = iVar12 + 1;
					do {
						bVar1 = 0x80;
						if (uVar9 != 0) {
							bVar1 = 0;
						}
						uVar9 = uVar9 + 1;
						*pbVar10 = bVar1 | (byte)iVar12;
						pbVar10[1] = bVar1 | (byte)iVar15;
						iVar12 = iVar12 + 3;
						iVar15 = iVar15 + 3;
						pbVar10 = pbVar10 + 2;
					} while (uVar9 < 3);
					uVar11 = uVar11 + 1;
				} while (uVar11 < 2);
				uVar16 = uVar16 + 1;
				iVar5 = iVar5 + 9;
			} while (uVar16 < 2);

			edDListLoadMatrix(&local_40);
			edDListUseMaterial(&this->pOwner->aCombinedMaterials[4].material);
			edDListBegin(0.0f, 0.0f, 0.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, this->field_0x4 * 4 + 0x18);
			this->whirlStateA.index = 0;
			uVar16 = this->pOwner->flags_0x54;
			_Create_FakePlane(0.0f, 0.5f, 1.0f, -0.05f, pVtx, (uint)((uVar16 & 8) != 0), (uint)((uVar16 & 0x10) != 0));
			uVar16 = 0;
			if ((this->field_0x4 & 0x3fffffff) != 0) {
				do {
					p_Var18 = pVtx + (pbVar3[uVar16] & 0x7f);
					this->whirlStateA.pAlphaMultiplier[uVar16] = (int)(p_Var18->field_0xc * 128.0f);
					edDListTexCoo2f(p_Var18->field_0x10, p_Var18->field_0x14);
					edDListColor4u8(this->field_0x8c.r, this->field_0x8c.g, this->field_0x8c.b, 0);
					if ((pbVar3[uVar16] & 0x80) == 0) {
						fVar20 = 1.0f;
						fVar19 = p_Var18->field_0x0;
					}
					else {
						fVar20 = 49152.0f;
						fVar19 = p_Var18->field_0x0;
					}

					edDListVertex4f(fVar19, p_Var18->field_0x4, p_Var18->field_0x8, fVar20);
					uVar16 = uVar16 + 1;
				} while (uVar16 < this->field_0x4 << 2);
			}

			_Create_Whirl(pVtx);

			uVar16 = 0;
			this->whirlStateB.index = this->whirlStateA.index + this->field_0x4 * 4;
			do {
				p_Var18 = pVtx + (pbVar4[uVar16] & 0x7f);
				if (uVar16 < 0xc) {
					this->whirlStateB.pAlphaMultiplier[uVar16] = (int)(p_Var18->field_0xc * 128.0f);
				}

				edDListTexCoo2f(p_Var18->field_0x10, p_Var18->field_0x14);
				edDListColor4u8(this->field_0x8c.r, this->field_0x8c.g, this->field_0x8c.b, 0);

				if ((pbVar4[uVar16] & 0x80) == 0) {
					fVar20 = 1.0f;
					fVar19 = p_Var18->field_0x0;
				}
				else {
					fVar20 = 49152.0f;
					fVar19 = p_Var18->field_0x0;
				}

				edDListVertex4f(fVar19, p_Var18->field_0x4, p_Var18->field_0x8, fVar20);
				uVar16 = uVar16 + 1;
			} while (uVar16 < 0x18);

			edDListEnd();
			gSP_Manager.ReleaseBuffer(pbVar4);
			gSP_Manager.ReleaseBuffer(pbVar3);
		}
		else {
			if (patchId != this->fakePlanePatchId) {
				return false;
			}
			pbVar3 = (byte*)gSP_Manager.GetFreeBuffer(this->field_0x4 << 2);
			uVar16 = 0;
			pbVar10 = pbVar3;
			do {
				uVar11 = 0;
				if (this->field_0x4 != 0) {
					iVar5 = uVar16 + 1;
					uVar9 = uVar16;
					do {
						bVar1 = 0x80;
						if (uVar11 != 0) {
							bVar1 = 0;
						}
						uVar11 = uVar11 + 1;
						*pbVar10 = bVar1 | (byte)uVar9;
						pbVar10[1] = bVar1 | (byte)iVar5;
						uVar9 = uVar9 + 3;
						iVar5 = iVar5 + 3;
						pbVar10 = pbVar10 + 2;
					} while (uVar11 < this->field_0x4);
				}
				uVar16 = uVar16 + 1;
			} while (uVar16 < 2);

			edDListLoadMatrix(&local_40);
			edDListUseMaterial(&this->pOwner->aCombinedMaterials[3].material);
			edDListBegin(0.0f, 0.0f, 0.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, this->field_0x4 << 2);
			this->fakePlaneState.index = 0;
			uVar16 = this->pOwner->flags_0x54;
			_Create_FakePlane(0.0f, 3.0f, 3.0f, 0.05f, pVtx, (uint)((uVar16 & 8) != 0), (uint)((uVar16 & 0x10) != 0));
			uVar16 = 0;
			if ((this->field_0x4 & 0x3fffffff) != 0) {
				do {
					p_Var18 = pVtx + (pbVar3[uVar16] & 0x7f);
					this->fakePlaneState.pAlphaMultiplier[uVar16] = (int)(p_Var18->field_0xc * 128.0f);
					edDListTexCoo2f(p_Var18->field_0x10, p_Var18->field_0x14);
					edDListColor4u8(0, 0x80, 0, 0);

					if ((pbVar3[uVar16] & 0x80) == 0) {
						fVar20 = 1.0f;
						fVar19 = p_Var18->field_0x0;
					}
					else {
						fVar20 = 49152.0f;
						fVar19 = p_Var18->field_0x0;
					}

					edDListVertex4f(fVar19, p_Var18->field_0x4, p_Var18->field_0x8, fVar20);
					uVar16 = uVar16 + 1;
				} while (uVar16 < this->field_0x4 << 2);
			}

			edDListEnd();
			gSP_Manager.ReleaseBuffer(pbVar3);
		}
	}

	gSP_Manager.ReleaseBuffer(pVtx);

	return true;
}

// HACK
extern ed_3d_strip* gCurStripPatchable;

void CFxWindSolid::Draw(edF32MATRIX4* pMatrix)
{
	CGlobalDListPatch* pPatch;
	int iVar2;
	uint patchActiveState;
	uint uVar4;
	_rgba newColor;
	uint uVar6;
	CGlobalDListManager* pDlistManager;

	pDlistManager = CScene::ptable.g_GlobalDListManager_004516bc;
	(this->field_0x8c).a = 0;

	if (this->whirlStateA.alpha != this->whirlStateA.prevAlpha) {
		this->whirlStateA.drawState = 2;
	}

	if (this->outerStateA.alpha != this->outerStateA.prevAlpha) {
		this->outerStateA.drawState = 2;
	}

	if (this->outerStateB.alpha != this->outerStateB.prevAlpha) {
		this->outerStateB.drawState = 2;
	}

	if (this->fakePlaneState.alpha != this->fakePlaneState.prevAlpha) {
		this->fakePlaneState.drawState = 2;
	}

	if (this->whirlStateB.alpha != this->whirlStateB.prevAlpha) {
		this->whirlStateB.drawState = 2;
	}

	uVar6 = this->outerPatchId;
	if ((pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0) || ((iVar2 = (int)uVar6 >> 0x10, pDlistManager->activeSectorPatchId != iVar2 && (iVar2 != 0)))) {
	LAB_0020cca8:
		patchActiveState = 0xffffffff;
	}
	else {
		pPatch = pDlistManager->ppGlobalDlist[iVar2].pDlistPatch;

		if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_0020cca8;

		patchActiveState = (uint)((pPatch->aPatches[uVar6 & 0xffff]->flags & 1) != 0);
	}

	if (patchActiveState == 1) {
		pPatch = GameDListPatch_BeginCurrent(uVar6);

		ACTOR_LOG(LogLevel::Info, "Drawing outer patch 0x{:x}", (uintptr_t)gCurStripPatchable);

		// Update the position via matrix.
		edF32Matrix4CopyHard(&pPatch->pCurrentPatch->pDisplayListCommand->matrix, pMatrix);

		if ((this->outerStateA.drawState != 0) || (this->outerStateB.drawState != 0)) {
			uVar6 = 0;

			if (((this->field_0x4 - 1) * 7 & 0x7fffffff) != 0) {
				do {
					if (this->outerStateA.drawState != 0) {
						newColor = (_rgba)(this->field_0x8c.rgba | (uint)(this->outerStateA.alpha * (float)(uint)this->outerStateA.pAlphaMultiplier[uVar6]) << 0x18);
						edDListPatchRGBA_Inline(pPatch->pCurrentPatch->pRgba, newColor, this->outerStateA.index + uVar6, pPatch->pCurrentPatch->nbMatrices);
					}

					if (this->outerStateB.drawState != 0) {
						newColor = (_rgba)(this->field_0x8c.rgba | (uint)(this->outerStateB.alpha * (float)(uint)this->outerStateB.pAlphaMultiplier[uVar6]) << 0x18);
						edDListPatchRGBA_Inline(pPatch->pCurrentPatch->pRgba, newColor, this->outerStateB.index + uVar6, pPatch->pCurrentPatch->nbMatrices);
					}

					uVar6 = uVar6 + 1;
				} while (uVar6 < (this->field_0x4 - 1) * 0xe);
			}

			if (this->outerStateA.drawState != 0) {
				this->outerStateA.drawState = this->outerStateA.drawState + -1;
			}

			if (this->outerStateB.drawState != 0) {
				this->outerStateB.drawState = this->outerStateB.drawState + -1;
			}
		}

		iVar2 = -1;

		if (this->outerStateB.alpha == 0.0f) {
			iVar2 = (this->field_0x4 - 1) * 0xe;
		}

		GameDListPatch_EndCurrent(iVar2, 0);
	}

	uVar6 = this->fakePlanePatchId;
	if ((pDlistManager->ppGlobalDlist == (GlobalDlistEntry*)0x0) ||
		((iVar2 = (int)uVar6 >> 0x10, pDlistManager->activeSectorPatchId != iVar2 && (iVar2 != 0)))) {
	LAB_0020d070:
		patchActiveState = 0xffffffff;
	}
	else {
		pPatch = pDlistManager->ppGlobalDlist[iVar2].pDlistPatch;
		if (pPatch == (CGlobalDListPatch*)0x0) goto LAB_0020d070;
		patchActiveState = (uint)((pPatch->aPatches[uVar6 & 0xffff]->flags & 1) != 0);
	}

	if (patchActiveState == 1) {
		pPatch = GameDListPatch_BeginCurrent(uVar6);

		ACTOR_LOG(LogLevel::Info, "Drawing fake plane patch 0x{:x}", (uintptr_t)gCurStripPatchable);

		// Update the position via matrix.
		edF32Matrix4CopyHard(&pPatch->pCurrentPatch->pDisplayListCommand->matrix, pMatrix);

		if (this->fakePlaneState.drawState != 0) {
			uVar6 = 0;
			if ((this->field_0x4 & 0x3fffffff) != 0) {
				do {
					newColor = (_rgba)((uint)(this->fakePlaneState.alpha * (float)(uint)this->fakePlaneState.pAlphaMultiplier[uVar6]) << 0x18 | 0x808080);
					edDListPatchRGBA_Inline(pPatch->pCurrentPatch->pRgba, newColor, this->fakePlaneState.index + uVar6, pPatch->pCurrentPatch->nbMatrices);
					uVar6 = uVar6 + 1;
				} while (uVar6 < this->field_0x4 << 2);
			}

			if (this->fakePlaneState.drawState != 0) {
				this->fakePlaneState.drawState = this->fakePlaneState.drawState + -1;
			}
		}

		GameDListPatch_EndCurrent(-(uint)(this->fakePlaneState.alpha != 0.0f), 0);
	}

	uVar6 = this->whirlPatchId;
	if ((pDlistManager->ppGlobalDlist != (GlobalDlistEntry*)0x0) &&
		((iVar2 = (int)uVar6 >> 0x10, pDlistManager->activeSectorPatchId == iVar2 || (iVar2 == 0)))) {
		pPatch = pDlistManager->ppGlobalDlist[iVar2].pDlistPatch;
		if (pPatch != (CGlobalDListPatch*)0x0) {
			patchActiveState = (uint)((pPatch->aPatches[uVar6 & 0xffff]->flags & 1) != 0);
			goto LAB_0020d2a0;
		}
	}

	patchActiveState = 0xffffffff;
LAB_0020d2a0:
	if (patchActiveState == 1) {
		pPatch = GameDListPatch_BeginCurrent(uVar6);

		ACTOR_LOG(LogLevel::Info, "Drawing whirl patch 0x{:x}", (uintptr_t)gCurStripPatchable);

		// Update the position via matrix.
		edF32Matrix4CopyHard(&pPatch->pCurrentPatch->pDisplayListCommand->matrix, pMatrix);

		if (this->whirlStateA.drawState != 0) {
			uVar6 = 0;
			if ((this->field_0x4 & 0x3fffffff) != 0) {
				do {
					newColor = (_rgba)(this->field_0x8c.rgba | (uint)(this->whirlStateA.alpha * (float)(uint)this->whirlStateA.pAlphaMultiplier[uVar6]) << 0x18);
					edDListPatchRGBA_Inline(pPatch->pCurrentPatch->pRgba, newColor, this->whirlStateA.index + uVar6, pPatch->pCurrentPatch->nbMatrices);
					uVar6 = uVar6 + 1;
				} while (uVar6 < this->field_0x4 << 2);
			}

			this->whirlStateA.drawState = this->whirlStateA.drawState + -1;
		}

		uVar6 = 0;
		if (this->whirlStateB.drawState != 0) {
			do {
				newColor = (_rgba)(this->field_0x8c.rgba | (uint)(this->whirlStateB.alpha * (float)(uint)this->whirlStateB.pAlphaMultiplier[uVar6]) << 0x18);
				edDListPatchRGBA_Inline(pPatch->pCurrentPatch->pRgba, newColor, this->whirlStateB.index + uVar6, pPatch->pCurrentPatch->nbMatrices);
				edDListPatchRGBA_Inline(pPatch->pCurrentPatch->pRgba, newColor, this->whirlStateB.index + uVar6 + 0xc, pPatch->pCurrentPatch->nbMatrices);
				uVar6 = uVar6 + 1;
			} while (uVar6 < 0xc);

			this->whirlStateB.drawState = this->whirlStateB.drawState + -1;
		}

		iVar2 = -1;
		if (this->whirlStateB.alpha == 0.0f) {
			iVar2 = this->field_0x4 << 2;
		}

		GameDListPatch_EndCurrent(iVar2, 0);
	}

	return;
}
