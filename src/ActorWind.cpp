#include "ActorWind.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "MathOps.h"
#include "EventManager.h"
#include "ActorFactory.h"
#include "ActorAutonomous.h"
#include "WayPoint.h"

#define WIND_STATE_INTERPOLATE 0x5
#define WIND_STATE_OFF 0x6
#define WIND_STATE_TURN_ON 0x7
#define WIND_STATE_ON 0x8
#define WIND_STATE_TURN_OFF 0x9
#define WIND_STATE_RECHARGE 0xa

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
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	this->pActorStreamRef = reinterpret_cast<S_ACTOR_STREAM_REF*>(piVar2);

	this->materialId = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(this->materialId);

	this->field_0x1c4.index = pByteCode->GetS32();
	this->field_0x1c8 = pByteCode->GetU32();
	//if (*(byte*)&this->field_0x1c8 < 0x80) {
	//	*(byte*)&this->field_0x1c8 = *(byte*)&this->field_0x1c8 << 1;
	//}
	//else {
	//	*(undefined*)&this->field_0x1c8 = 0xff;
	//}
	//bVar1 = *(byte*)((int)&this->field_0x1c8 + 1);
	//if (bVar1 < 0x80) {
	//	*(byte*)((int)&this->field_0x1c8 + 1) = bVar1 << 1;
	//}
	//else {
	//	*(undefined*)((int)&this->field_0x1c8 + 1) = 0xff;
	//}
	//bVar1 = *(byte*)((int)&this->field_0x1c8 + 2);
	//if (bVar1 < 0x80) {
	//	*(byte*)((int)&this->field_0x1c8 + 2) = bVar1 << 1;
	//}
	//else {
	//	*(undefined*)((int)&this->field_0x1c8 + 2) = 0xff;
	//}

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
		if (this->field_0x16c < 0.0) {
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
			peVar1->pMatrix = STORE_SECTION(this->field_0x1e8);
		}

		peVar1 = (this->field_0x1c4).Get();
		if (peVar1 != (ed_zone_3d*)0x0) {
			peVar1->pMatrix = STORE_SECTION(this->field_0x1e8);
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
					pFxWind->field_0x54 = pFxWind->field_0x54 & 0xfffffffe;
				}
				else {
					pFxWind->field_0x54 = pFxWind->field_0x54 | 1;
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

	IMPLEMENTATION_GUARD_WIND_FX(
	iVar2 = 0;
	if (0 < (int)this->nbFxWind) {
		iVar1 = 0;
		do {
			CFxWind::ChangeVisibleState(&this->aFxWind->field_0x0 + iVar1, (long)state);
			iVar2 = iVar2 + 1;
			iVar1 = iVar1 + 0x3c0;
		} while (iVar2 < (int)this->nbFxWind);
	})

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
					pFxWind->field_0x54 = pFxWind->field_0x54 & 0xfffffffe;
				}
				else {
					pFxWind->field_0x54 = pFxWind->field_0x54 | 1;
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
					pFxWind->field_0x54 = pFxWind->field_0x54 | 1;

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
						pFxWind->field_0x54 = pFxWind->field_0x54 & 0xfffffffe;

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
								pFxWind->field_0x54 = pFxWind->field_0x54 & 0xfffffffe;
							}
							else {
								pFxWind->field_0x54 = pFxWind->field_0x54 | 1;
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
	CFxWind* pCVar10;
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
		case WIND_STATE_INTERPOLATE:
			SV_UpdateValue(0.0f, this->field_0x20c, &this->curWindAlpha);
			break;
		case WIND_STATE_OFF:
			this->curWindAlpha = 0.0f;
			break;
		case WIND_STATE_TURN_ON:
			SV_UpdateValue(this->maxWind, this->field_0x210, &this->curWindAlpha);
			break;
		case WIND_STATE_ON:
			this->curWindAlpha = this->maxWind;
			break;
		case WIND_STATE_TURN_OFF:
			SV_UpdateValue(0.0f, this->field_0x208, &this->curWindAlpha);
			break;
		case WIND_STATE_RECHARGE:
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

		IMPLEMENTATION_GUARD_WIND_FX(
		pCVar10 = this->aFxWind;
		for (iVar4 = 0; iVar4 < (int)this->nbFxWind; iVar4 = iVar4 + 1) {
			pCVar10->Manage((uint)(0.0 < this->curWindAlpha));
			pCVar10 = pCVar10 + 1;
		})

		IMPLEMENTATION_GUARD_AUDIO(
		if ((this->aFxWind != (CFxWind*)0x0) && (this->pSoundWind != (CSoundWind*)0x0)) {
			pCVar10 = this->aFxWind + this->nbFxWind;
			while (bVar3 = this->aFxWind < pCVar10, pCVar10 = pCVar10 + -1, bVar3) {
				this->pSoundWind->Play(this, pCVar10);
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
					pFxWind->field_0x54 = pFxWind->field_0x54 & 0xfffffffe;

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
			pFxWind->field_0x54 = pFxWind->field_0x54 & 0xfffffffe;

			iVar4 = iVar4 + 1;
		} while (iVar4 < this->nbFxWind);
	}

	return  -1.0f;
}

int CActorWind::ComputeStateFromTime(float* pTime)
{
	int newState = WIND_STATE_INTERPOLATE;

	if (this->field_0x180 <= *pTime) {
		*pTime = *pTime - this->field_0x180;
		newState = WIND_STATE_OFF;
		if (this->field_0x184 <= *pTime) {
			*pTime = *pTime - this->field_0x184;
			newState = WIND_STATE_TURN_ON;
			if (this->field_0x188 <= *pTime) {
				*pTime = *pTime - this->field_0x188;
				newState = WIND_STATE_ON;
				if (this->field_0x18c <= *pTime) {
					*pTime = *pTime - this->field_0x18c;
					newState = WIND_STATE_TURN_OFF;
					if (this->field_0x178 <= *pTime) {
						newState = WIND_STATE_RECHARGE;
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
		case WIND_STATE_INTERPOLATE:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x180, 0.0f, 0.0f);
			this->curWindAlpha = fVar3;
			break;
		case WIND_STATE_OFF:
			this->curWindAlpha = 0.0f;
			break;
		case WIND_STATE_TURN_ON:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x188, 0.0f, this->maxWind);
			this->curWindAlpha = fVar3;
			break;
		case WIND_STATE_ON:
			this->curWindAlpha = this->maxWind;
			break;
		case WIND_STATE_TURN_OFF:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x178, this->maxWind, 0.0f);
			this->curWindAlpha = fVar3;
			break;
		case WIND_STATE_RECHARGE:
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

	if ((bVar3) && (pKVar2 = this->subObjA, fVar6 = pKVar2->visibilityDistance, pKVar2->field_0x1c < fVar6)) {
		pKVar2 = this->subObjA;
		pKVar2->floatFieldB = fVar6;
		pKVar2->field_0x1c = fVar6;
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

		pFx->field_0x60 = this->pMeshTransform->base.transformA;
		pFx->field_0x58 = param_3;
		
		pFx->field_0xa0 = local_40;

		pFx->field_0x130 = local_50;
		pFx->field_0x150 = local_f0;
		edF32Matrix4MulF32Vector4Hard(&local_f0, &this->pMeshTransform->base.transformA, &local_f0);
		pFx->field_0x140 = local_f0;

		local_100.x = local_50.x;
		local_100.z = local_50.z;
		local_100.w = local_50.w;
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
		this->pOwner->SetState(WIND_STATE_INTERPOLATE, -1);
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

	if (newState == WIND_STATE_RECHARGE) {
		if (2.2f <= CScene::_pinstance->field_0x1c) {
			pActorWind->field_0x1e0.Switch(pActorWind);
		}
	}
	else {
		if (newState == WIND_STATE_ON) {
			if (2.2f <= CScene::_pinstance->field_0x1c) {
				pActorWind->field_0x1d8.Switch(pActorWind);
			}
		}
		else {
			if ((newState == WIND_STATE_OFF) && (2.2f <= CScene::_pinstance->field_0x1c)) {
				pActorWind->field_0x1d0.Switch(pActorWind);
			}
		}
	}

	return;
}

CFxEmitterPool* gpWIND_PartPool = (CFxEmitterPool*)0x0;

void CFxWind::Create(CActorWind* pOwner, int param_3)
{
	CActorSound* pCVar1;
	CFxEmitterPool* pCVar2;

	this->pOwner = pOwner;
	//this->field_0x4 = pOwner->objectId;
	this->field_0x164 = param_3;
	//this->field_0x2c8 = 1.0f;
	//this->field_0x2cc = 0.5f;
	//this->field_0x2d0 = 1.0f;
	//this->field_0x2d4 = 1.0f;
	//this->field_0x180 = (ed_g2d_manager*)0x0;
	//this->field_0x58 = 6;
	//this->field_0x54 = 0;
	//this->field_0x374 = pOwner->CreateActorSound(3);

	//(this->windSolid).pOwner = this;
	//(this->windSolid).field_0x24 = 0;

	pCVar2 = gpWIND_PartPool;
	if ((gpWIND_PartPool == (CFxEmitterPool*)0x0)) {
		pCVar2 = new CFxEmitterPool;
	}
	gpWIND_PartPool = pCVar2;

	return;
}

void CFxWind::Init(edF32MATRIX4* pMatrix)
{
	byte* pbVar1;
	byte* pbVar2;
	CFxEmitterPool* pCVar3;
	C3DFileManager* this_00;
	edF32MATRIX4* peVar4;
	edDList_material* peVar5;
	ed_hash_code* peVar6;
	int iVar7;
	char* pcVar8;
	ed_g2d_manager* peVar9;
	ed_g2d_texture* peVar10;
	void* pvVar11;
	CFxEmitterPool* pCVar12;
	int iVar13;
	int* piVar14;
	ed_hash_code** ppeVar15;
	int* piVar16;
	ed_hash_code** ppeVar17;
	int iVar18;
	//WindAnimST* pWVar19;
	undefined4 uVar20;
	undefined4 uVar21;
	float fVar22;
	int local_40[8];
	ed_hash_code* local_20[7];
	e_ed_event_prim3d_type local_4;

	this_00 = CScene::ptable.g_C3DFileManager_00451664;
	peVar4 = edEventGetChunkZonePrimitive((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->pOwner->field_0x21c, this->field_0x164, &local_4);
	this->field_0x5c = peVar4;
	edF32Matrix4GetInverseGaussSoft(&this->field_0x10, peVar4);
	this->pOwner->IsAValidFxPrimitive(this, local_4, peVar4, pMatrix);

	IMPLEMENTATION_GUARD_WIND_FX(
	RetrieveFlags(this, this->pOwner);
	edCSound3DPrim::Init((edCSound3DPrim*)&this->field_0x3a0, this->field_0x58, &this->field_0x10);
	if (((this->field_0x54 & 0x100) == 0) || ((this->field_0x54 & 2) != 0)) {
		ppeVar15 = local_20;
		piVar14 = local_40;
		iVar18 = 0;
		iVar13 = 0;
		pWVar19 = gUseAnimST;
		ppeVar17 = ppeVar15;
		piVar16 = piVar14;
		do {
			if (pWVar19->field_0x0 != 0) {
				peVar5 = C3DFileManager::GetMaterialFromId(this_00, this->pOwner->materialId, iVar13);
				peVar6 = (ed_hash_code*)ed3DG2DGetHASHFromMat(peVar5->pManager, peVar5->pMaterial);
				*ppeVar17 = peVar6;
				iVar7 = ed3DG2DGetNeededSizeForDuplicateMaterial(*ppeVar17);
				*piVar16 = iVar7;
				iVar18 = iVar18 + *piVar16;
			}
			iVar13 = iVar13 + 1;
			pWVar19 = pWVar19 + 1;
			ppeVar17 = ppeVar17 + 1;
			piVar16 = piVar16 + 1;
		} while (iVar13 < 5);
		pcVar8 = (char*)edMemAlloc(H_MAIN, iVar18);
		this->field_0x180 = pcVar8;
		memset(this->field_0x180, 0, iVar18);
		pcVar8 = this->field_0x180;
		pMaterial = (MaterialManagerCombined*)&this->field_0x184;
		iVar13 = 0;
		pWVar19 = gUseAnimST;
		do {
			if (pWVar19->field_0x0 == 0) {
				peVar5 = C3DFileManager::GetMaterialFromId(this_00, this->pOwner->materialId, iVar13);
				(pMaterial->field_0x0).pManager = peVar5->pManager;
				(pMaterial->field_0x0).pMaterial = peVar5->pMaterial;
				(pMaterial->field_0x0).mode = peVar5->mode;
				(pMaterial->field_0x0).index = peVar5->index;
				peVar9 = (pMaterial->field_0x0).pManager;
				(pMaterial->field_0x10).pFileBuffer = peVar9->pFileBuffer;
				(pMaterial->field_0x10).textureFileLengthA = peVar9->textureFileLengthA;
				(pMaterial->field_0x10).pTextureChunk = peVar9->pTextureChunk;
				(pMaterial->field_0x10).pMATA_HASH = peVar9->pMATA_HASH;
				(pMaterial->field_0x10).pT2DA = peVar9->pT2DA;
				(pMaterial->field_0x10).pPALL = peVar9->pPALL;
				*(undefined4*)&(pMaterial->field_0x10).field_0x18 = *(undefined4*)&peVar9->field_0x18;
				(pMaterial->field_0x10).textureFileLengthB = peVar9->textureFileLengthB;
				(pMaterial->field_0x10).pANMA = peVar9->pANMA;
				uVar20 = *(undefined4*)&peVar9->field_0x28;
				uVar21 = *(undefined4*)&peVar9->field_0x2c;
				*(undefined4*)&(pMaterial->field_0x10).field_0x24 = *(undefined4*)&peVar9->field_0x24;
				*(undefined4*)&(pMaterial->field_0x10).field_0x28 = uVar20;
				*(undefined4*)&(pMaterial->field_0x10).field_0x2c = uVar21;
			}
			else {
				peVar9 = ed3DG2DDuplicateMaterial(*ppeVar15, pcVar8, &pMaterial->field_0x10);
				ed3DG2DGetG2DMaterial(peVar9, (*ppeVar15)->hash);
				edDListCreatMaterialFromHashCode((edDList_material*)pMaterial, (*ppeVar15)->hash, peVar9, 0);
				peVar10 = ed3DG2DGetTextureFromMaterial((pMaterial->field_0x0).pMaterial, 0);
				if (peVar10->pAnimSpeedNormalExtruder != (edF32VECTOR4*)0x0) {
					peVar10->pAnimSpeedNormalExtruder->x = 0.0;
					peVar10->pAnimSpeedNormalExtruder->y = 0.0;
					peVar10->pAnimSpeedNormalExtruder->z = 0.0;
					peVar10->pAnimSpeedNormalExtruder->w = 0.0;
				}
				pcVar8 = pcVar8 + *piVar14;
			}
			iVar13 = iVar13 + 1;
			pWVar19 = pWVar19 + 1;
			ppeVar15 = ppeVar15 + 1;
			piVar14 = piVar14 + 1;
			pMaterial = pMaterial + 1;
		} while (iVar13 < 5);
	}
	if ((this->field_0x54 & 2) != 0) {
		*(undefined4*)&(this->windSolid).field_0x10 = 0x3f800000;
		*(undefined4*)&(this->windSolid).field_0x14 = 0x3f800000;
		*(undefined4*)&(this->windSolid).field_0xc = 0;
		*(undefined4*)&(this->windSolid).field_0x8 = 0;
		*(undefined4*)&this->field_0x308 = 0;
		*(undefined4*)&this->field_0x304 = 0;
		*(undefined4*)&this->field_0x31c = 0;
		*(undefined4*)&this->field_0x318 = 0;
		*(undefined4*)&this->field_0x330 = 0;
		*(undefined4*)&this->field_0x32c = 0;
		*(undefined4*)&this->field_0x344 = 0;
		*(undefined4*)&this->field_0x340 = 0;
		*(undefined4*)&this->field_0x358 = 0;
		*(undefined4*)&this->field_0x354 = 0;
		fVar22 = edF32Vector4GetDistHard((edF32VECTOR4*)&((this->windSolid).pOwner)->field_0x130);
		(this->windSolid).field_0x4 = (int)(fVar22 / 20.0 + 1.0);
		if ((int)(this->windSolid).field_0x4 < 3) {
			(this->windSolid).field_0x4 = 3;
		}
		pvVar11 = edMemAlloc(H_MAIN, (this->windSolid).field_0x4 * 0x16 + -2);
		this->field_0x2fc = pvVar11;
		*(undefined4*)&this->field_0x314 = this->field_0x2fc;
		*(undefined4*)&this->field_0x300 = this->field_0x2fc;
		*(uint*)&this->field_0x328 = *(int*)&this->field_0x300 + ((this->windSolid).field_0x4 - 1) * 0xe;
		*(uint*)&this->field_0x33c = *(int*)&this->field_0x328 + (this->windSolid).field_0x4 * 4;
		*(uint*)&this->field_0x350 = *(int*)&this->field_0x33c + (this->windSolid).field_0x4 * 4;
		iVar13 = GameDListPatch_Register((CObject*)this, ((this->windSolid).field_0x4 - 1) * 0x1c, 0);
		*(int*)&this->field_0x368 = iVar13;
		iVar13 = GameDListPatch_Register((CObject*)this, (this->windSolid).field_0x4 * 4 + 0x18, 0);
		*(int*)&this->field_0x36c = iVar13;
		iVar13 = GameDListPatch_Register((CObject*)this, (this->windSolid).field_0x4 << 2, 0);
		*(int*)&this->field_0x370 = iVar13;
		pbVar1 = *(byte**)&this->field_0x370;
		pbVar2 = *(byte**)&this->field_0x36c;
		(this->windSolid).field_0x18 = *(byte**)&this->field_0x368;
		(this->windSolid).field_0x1c = pbVar2;
		(this->windSolid).field_0x20 = pbVar1;
	}
	pCVar3 = gpWIND_PartPool;
	iVar13 = -1;
	if ((((this->pOwner->base).objectId == -1) && ((int)gpWIND_PartPool->field_0x10 < 0x40)) &&
		(iVar18 = 0, pCVar12 = gpWIND_PartPool, gpWIND_PartPool->field_0x10 != 0x40)) {
		do {
			if (pCVar12->field_0x20 == 0) {
				iVar13 = iVar18;
			}
			iVar18 = iVar18 + 1;
		} while ((iVar18 < 0x40) && (pCVar12 = (CFxEmitterPool*)&pCVar12->field_0x8, iVar13 == -1));
		(&gpWIND_PartPool->field_0x20)[iVar13 * 2] = this;
		pCVar3->field_0x10 = pCVar3->field_0x10 + 1;
	}
	CSoundWind::Init(this->pOwner->pSoundWind, this);)

	return;
}
