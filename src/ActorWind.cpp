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

	pGVar5 = CScene::ptable.g_GlobalSoundPtr_00451698;
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
		piVar2 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
		}
		this->field_0x1d0 = reinterpret_cast<S_TARGET_STREAM_REF*>(piVar2);

		pcVar3 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar3 + 0x20;
		this->field_0x1d4 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pcVar3);

		piVar2 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
		}
		this->field_0x1d8 = reinterpret_cast<S_TARGET_STREAM_REF*>(piVar2);

		pcVar3 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar3 + 0x20;
		this->field_0x1dc = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pcVar3);

		piVar2 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
		}
		this->field_0x1e0 = reinterpret_cast<S_TARGET_STREAM_REF*>(piVar2);

		pcVar3 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar3 + 0x20;
		this->field_0x1e4 = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pcVar3);
	}

	this->field_0x170 = 4.0f;
	fVar9 = pByteCode->GetF32();
	this->field_0x194 = fVar9;
	fVar9 = pByteCode->GetF32();
	this->field_0x198 = fVar9;
	fVar9 = pByteCode->GetF32();
	this->field_0x19c = fVar9;
	fVar9 = pByteCode->GetF32();
	this->field_0x1a0 = fVar9;
	fVar9 = pByteCode->GetF32();
	this->field_0x1a4 = fVar9;

	iVar7 = pByteCode->GetS32();
	this->pWayPointRef.index = iVar7;

	fVar9 = pByteCode->GetF32();
	this->field_0x1ac = fVar9;
	fVar9 = pByteCode->GetF32();
	this->field_0x1b0 = fVar9;

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

	//iVar7 = FUN_00184590((int)pGVar5, (long)(int)uVar6);
	//this->field_0x1b4 = iVar7;
	this->aFxWind = (CFxWind*)0x0;
	this->nbFxWind = 0;
	this->field_0x1e8 = (edF32MATRIX4*)0x0;
	//this->field_0x1ec = 0;
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
	S_TARGET_STREAM_REF* pSVar8;
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

	this->sectorObj.Init(this->objectId);

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		pSVar8 = this->field_0x1d0;
		iVar11 = 0;
		if (0 < pSVar8->entryCount) {
			do {
				pSVar8->aEntries[iVar11].Init();
				iVar11 = iVar11 + 1;
			} while (iVar11 < pSVar8->entryCount);
		}
		this->field_0x1d4->Init();

		pSVar8 = this->field_0x1d8;
		iVar11 = 0;
		if (0 < pSVar8->entryCount) {
			do {
				pSVar8->aEntries[iVar11].Init();
				iVar11 = iVar11 + 1;
			} while (iVar11 < pSVar8->entryCount);
		}
		this->field_0x1dc->Init();

		pSVar8 = this->field_0x1e0;
		iVar11 = 0;
		if (0 < pSVar8->entryCount) {
			do {
				pSVar8->aEntries[iVar11].Init();
				iVar11 = iVar11 + 1;
			} while (iVar11 < pSVar8->entryCount);
		}
		this->field_0x1e4->Init();
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

			IMPLEMENTATION_GUARD_WIND_FX(
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

			ReComputeBoundingSphere((int)this, &eStack128);)
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
	this->sectorObj.Term(this->objectId);
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
	S_TARGET_STREAM_REF* pSVar3;
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

		for (int i = 0; i < this->field_0x1d0->entryCount; i++) {
			this->field_0x1d0->aEntries[i].Reset();
		}

		this->field_0x1d4->Reset(this);

		for (int i = 0; i < this->field_0x1d8->entryCount; i++) {
			this->field_0x1d8->aEntries[i].Reset();
		}

		this->field_0x1dc->Reset(this);

		for (int i = 0; i < this->field_0x1e0->entryCount; i++) {
			this->field_0x1e0->aEntries[i].Reset();
		}
		
		this->field_0x1e4->Reset(this);
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
			IMPLEMENTATION_GUARD(
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			CActor::EvaluateDisplayState(this);
			CSectorHierarchy::SetHiddenOn(&this->sectorObj);)
		}
		else {
			this->flags = this->flags & 0xfffffffc;
			this->flags = this->flags & 0xffffff5f;
			IMPLEMENTATION_GUARD(
			CActor::EvaluateDisplayState(this);
			ResetTiming(this);
			CSectorHierarchy::SetHiddenOff(&this->sectorObj);)
		}

		iVar1 = 0;
		if (0 < (int)this->nbFxWind) {
			iVar4 = 0;
			IMPLEMENTATION_GUARD(
			do {
				iVar2 = &this->aFxWind->field_0x0 + iVar4;
				if (uVar5 == 0) {
					*(uint*)(iVar2 + 0x54) = *(uint*)(iVar2 + 0x54) & 0xfffffffe;
				}
				else {
					*(uint*)(iVar2 + 0x54) = *(uint*)(iVar2 + 0x54) | 1;
				}
				iVar1 = iVar1 + 1;
				iVar4 = iVar4 + 0x3c0;
			} while (iVar1 < (int)this->nbFxWind);)
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

			IMPLEMENTATION_GUARD_WIND_FX(
			iVar1 = 0;
			if (0 < (int)this->nbFxWind) {
				iVar4 = 0;
				do {
					iVar1 = iVar1 + 1;
					iVar2 = &this->aFxWind->field_0x0 + iVar4;
					*(uint*)(iVar2 + 0x54) = *(uint*)(iVar2 + 0x54) | 1;
					iVar4 = iVar4 + 0x3c0;
				} while (iVar1 < (int)this->nbFxWind);
			})

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

				IMPLEMENTATION_GUARD_WIND_FX(
				iVar1 = 0;
				if (0 < (int)this->nbFxWind) {
					iVar4 = 0;
					do {
						iVar1 = iVar1 + 1;
						iVar2 = &this->aFxWind->field_0x0 + iVar4;
						*(uint*)(iVar2 + 0x54) = *(uint*)(iVar2 + 0x54) & 0xfffffffe;
						iVar4 = iVar4 + 0x3c0;
					} while (iVar1 < (int)this->nbFxWind);
				})

				iVar1 = 1;
			}
			else {
				if (msg == 0xd) {
					IMPLEMENTATION_GUARD(
					iVar1 = *(int*)((int)pMsgParam + 4);
					if (iVar1 == 0) {
						this->flags = this->flags & 0xfffffffd;
						this->flags = this->flags | 1;
						this->flags = this->flags & 0xffffff7f;
						this->flags = this->flags | 0x20;
						CActor::EvaluateDisplayState(this);
						CSectorHierarchy::SetHiddenOn(&this->sectorObj);
					}
					else {
						this->flags = this->flags & 0xfffffffc;
						this->flags = this->flags & 0xffffff5f;
						CActor::EvaluateDisplayState(this);
						ResetTiming(this);
						CSectorHierarchy::SetHiddenOff(&this->sectorObj);
					}
					iVar4 = 0;
					if (0 < (int)this->nbFxWind) {
						iVar2 = 0;
						do {
							iVar3 = &this->aFxWind->field_0x0 + iVar2;
							if (iVar1 == 0) {
								*(uint*)(iVar3 + 0x54) = *(uint*)(iVar3 + 0x54) & 0xfffffffe;
							}
							else {
								*(uint*)(iVar3 + 0x54) = *(uint*)(iVar3 + 0x54) | 1;
							}
							iVar4 = iVar4 + 1;
							iVar2 = iVar2 + 0x3c0;
						} while (iVar4 < (int)this->nbFxWind);
					}
					iVar1 = 1;)
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
						iVar1 = this->actorState;
						uVar5 = 0;

						if (iVar1 != -1) {
							pAVar8 = GetStateCfg(iVar1);
							uVar5 = pAVar8->flags_0x4;
						}

						if (((uVar5 & 0x100) != 0) && (this->curWindAlpha != 0.0f)) {
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
			this->field_0x1d4->Manage(this);
			this->field_0x1dc->Manage(this);
			this->field_0x1e4->Manage(this);
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

			IMPLEMENTATION_GUARD_WIND_FX(
			iVar4 = 0;
			if (0 < (int)this->nbFxWind) {
				iVar3 = 0;
				do {
					iVar4 = iVar4 + 1;
					iVar2 = &this->aFxWind->field_0x0 + iVar3;
					*(uint*)(iVar2 + 0x54) = *(uint*)(iVar2 + 0x54) & 0xfffffffe;
					iVar3 = iVar3 + 0x3c0;
				} while (iVar4 < (int)this->nbFxWind);
			})

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

	IMPLEMENTATION_GUARD_WIND_FX(
	iVar4 = 0;
	if (0 < (int)this->nbFxWind) {
		iVar3 = 0;
		do {
			iVar4 = iVar4 + 1;
			iVar2 = &this->aFxWind->field_0x0 + iVar3;
			*(uint*)(iVar2 + 0x54) = *(uint*)(iVar2 + 0x54) & 0xfffffffe;
			iVar3 = iVar3 + 0x3c0;
		} while (iVar4 < (int)this->nbFxWind);
	})

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

	pNotifyActor->IsKindOfObject(4);

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
			entryIndex = 0;
			if (0 < pActorWind->field_0x1e0->entryCount) {
				do {
					pActorWind->field_0x1e0->aEntries[entryIndex].Switch(pActorWind);
					entryIndex = entryIndex + 1;
				} while (entryIndex < pActorWind->field_0x1e0->entryCount);
			}

			pActorWind->field_0x1e4->SwitchOn(pActorWind);
		}
	}
	else {
		if (newState == WIND_STATE_ON) {
			if (2.2f <= CScene::_pinstance->field_0x1c) {
				entryIndex = 0;
				if (0 < pActorWind->field_0x1d8->entryCount) {
					do {
						pActorWind->field_0x1d8->aEntries[entryIndex].Switch(pActorWind);
						entryIndex = entryIndex + 1;
					} while (entryIndex < pActorWind->field_0x1d8->entryCount);
				}

				pActorWind->field_0x1dc->SwitchOn(pActorWind);
			}
		}
		else {
			if ((newState == WIND_STATE_OFF) && (2.2f <= CScene::_pinstance->field_0x1c)) {
				entryIndex = 0;
				if (0 < pActorWind->field_0x1d0->entryCount) {
					do {
						pActorWind->field_0x1d0->aEntries[entryIndex].Switch(pActorWind);
						entryIndex = entryIndex + 1;
					} while (entryIndex < pActorWind->field_0x1d0->entryCount);
				}

				pActorWind->field_0x1d4->SwitchOn(pActorWind);
			}
		}
	}

	return;
}
