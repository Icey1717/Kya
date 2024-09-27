#include "ActorWind.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "MathOps.h"
#include "EventManager.h"
#include "ActorFactory.h"
#include "ActorAutonomous.h"
#include "WayPoint.h"

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
	this->field_0x164 = pByteCode->GetF32();
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
	this->field_0x1b8.index = pByteCode->GetS32();
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	this->field_0x1bc = reinterpret_cast<S_ACTOR_STREAM_REF*>(piVar2);

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
	this->field_0x1a8.index = iVar7;

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
	pSVar10 = this->field_0x1bc->aEntries;
	for (iVar11 = this->field_0x1bc->entryCount; iVar11 != 0; iVar11 = iVar11 + -1) {
		pSVar10->Init();
		pSVar10 = pSVar10 + 1;
	}

	this->field_0x1b8.Init();
	this->field_0x1c4.Init();

	if (this->field_0x1e8 != (edF32MATRIX4*)0x0) {
		peVar1 = (this->field_0x1b8).Get();
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
	pSVar12 = this->field_0x1bc;
	iVar11 = 0;
	if (pSVar12 != (S_ACTOR_STREAM_REF*)0x0) {
		iVar11 = pSVar12->entryCount;
	}
	if (0 < iVar11) {
		iVar11 = 0;
		if (pSVar12 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar11 = pSVar12->entryCount;
		}

		this->field_0x1ec = new int[iVar11];
	}

	this->field_0x21c = (this->field_0x1c4).Get();
	if (this->field_0x21c == (ed_zone_3d*)0x0) {
		this->field_0x21c = (this->field_0x1b8).Get();
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

	this->field_0x1a8.Init();
	this->field_0x1cc.Init();

	this->field_0x1f0 = 0;
	iVar9 = 0;
	while (true) {
		if (this->field_0x1bc == (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = 0;
		}
		else {
			iVar6 = this->field_0x1bc->entryCount;
		}

		if (iVar6 <= iVar9) break;
		this->field_0x1ec[iVar9] = 0x0;
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
	int iVar1;
	int iVar2;
	_wind_param_in local_40;
	_wind_param_in* local_4;

	CActor::ChangeManageState(state);

	if (state == 0) {
		iVar2 = 0;
		while (true) {
			if (this->field_0x1bc == (S_ACTOR_STREAM_REF*)0x0) {
				iVar1 = 0;
			}
			else {
				iVar1 = this->field_0x1bc->entryCount;
			}

			if (iVar1 <= iVar2) break;

			if (this->field_0x1ec[iVar2] != 0x0) {
				local_40.field_0x0 = this->field_0x16c;
				local_40.field_0x4 = this->field_0x170;
				local_40.field_0x8 = 1;

				if ((((this->field_0x160 & 0x20) == 0) && (local_40.field_0x8 = 0, (this->field_0x160 & 0x40) == 0)) &&
					(local_40.field_0x8 = 1, 0.17398384f <= fabs(this->rotationQuat.y))) {
					local_40.field_0x8 = 0;
				}

				local_40.field_0x10 = this->field_0x1fc;
				local_40.field_0x14 = this->field_0x200;
				local_40.field_0x18 = this->field_0x19c;
				local_40.field_0x1c = this->field_0x1a0;
				local_40.field_0x20 = this->field_0x1a4;
				local_40.field_0x24 = (uint)((this->field_0x160 & 0x400) != 0);
				local_40.field_0x28 = this->field_0x1a8.Get();
				if (local_40.field_0x28 == (CWayPoint*)0x0) {
					local_40.field_0x28 = (CWayPoint*)0x0;
				}
				local_4 = &local_40;
				local_40.field_0x2c = this->field_0x1ac;
				local_40.field_0x30 = this->field_0x1b0;
				DoMessage(this->field_0x1bc->aEntries[iVar2].Get(), (ACTOR_MESSAGE)0x18, (MSG_PARAM)local_4);
				this->field_0x1ec[iVar2] = 0x0;
			}

			iVar2 = iVar2 + 1;
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
			CActor::EvaluateDisplayState((CActor*)this);
			CSectorHierarchy::SetHiddenOn(&this->sectorObj);)
		}
		else {
			this->flags = this->flags & 0xfffffffc;
			this->flags = this->flags & 0xffffff5f;
			IMPLEMENTATION_GUARD(
			CActor::EvaluateDisplayState((CActor*)this);
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
						CActor::EvaluateDisplayState((CActor*)this);
						CSectorHierarchy::SetHiddenOn(&this->sectorObj);
					}
					else {
						this->flags = this->flags & 0xfffffffc;
						this->flags = this->flags & 0xffffff5f;
						CActor::EvaluateDisplayState((CActor*)this);
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
	CEventManager* pCVar2;
	bool bVar3;
	int iVar4;
	uint uVar5;
	edF32VECTOR4* peVar6;
	uint uVar7;
	StateConfig* pAVar8;
	CActor* pCVar9;
	CFxWind* pCVar10;
	float fVar11;
	_wind_param_in local_a0;
	_wind_param_in local_60;
	edF32VECTOR4 local_20;
	_wind_param_in* local_c;
	_wind_param_in* local_8;
	float local_4;

	if (this->pTiedActor != (CActor*)0x0) {
		local_20 = this->baseLocation;
		SV_UpdatePosition_Rel(&local_20, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	local_4 = ComputeCurTime();
	if (local_4 != -1.0f) {
		iVar4 = ComputeStateFromTime(&local_4);
		SetState(iVar4, -1);

		switch (this->actorState) {
		case 5:
			SV_UpdateValue(0.0f, this->field_0x20c, &this->field_0x1f4);
			break;
		case 6:
			this->field_0x1f4 = 0.0f;
			break;
		case 7:
			SV_UpdateValue(this->field_0x164, this->field_0x210, &this->field_0x1f4);
			break;
		case 8:
			this->field_0x1f4 = this->field_0x164;
			break;
		case 9:
			SV_UpdateValue(0.0f, this->field_0x208, &this->field_0x1f4);
			break;
		case 10:
			this->field_0x1f4 = 0.0f;
		}

		pCVar2 = CScene::ptable.g_EventManager_006f5080;
		if ((this->field_0x1b8).Get() != (ed_zone_3d*)0x0) {
			fVar11 = edFIntervalLERP(this->field_0x1f4, 0.0f, this->field_0x164, 0.1f, this->field_0x168);
			iVar4 = 0;
			while (true) {
				int entryCount;
				if (this->field_0x1bc == (S_ACTOR_STREAM_REF*)0x0) {
					entryCount = 0x0;
				}
				else {
					entryCount = this->field_0x1bc->entryCount;
				}

				if (entryCount <= iVar4) break;

				pCVar9 = this->field_0x1bc->aEntries[iVar4].Get();
				if (((pCVar9 != (CActor*)0x0) && ((pCVar9->flags & 4) != 0)) &&
					((CActorFactory::gClassProperties[pCVar9->typeID].flags & 4) != 0)) {
					uVar5 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, (this->field_0x1b8).Get(), &pCVar9->currentLocation, 0);
					bVar3 = pCVar9->IsKindOfObject(2);
					if (bVar3 != false) {
						peVar6 = pCVar9->GetTopPosition();
						uVar7 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, (this->field_0x1b8).Get(), peVar6, 0);
						uVar5 = uVar5 | uVar7;
					}

					if ((uVar5 & 1) == 0) {
						if (((uVar5 & 2) != 0) && (field_0x1ec[iVar4] != 0)) {
							local_a0.field_0x0 = this->field_0x16c;
							local_a0.field_0x4 = this->field_0x170;

							local_a0.field_0x8 = 1;
							if (((this->field_0x160 & 0x20) == 0) &&
								((local_a0.field_0x8 = 0, (this->field_0x160 & 0x40) == 0 &&
									(local_a0.field_0x8 = 1, 0.17398384f <= fabs(this->rotationQuat.y))))) {
								local_a0.field_0x8 = 0;
							}

							local_a0.field_0xc = 0;
							if ((this->field_0x19c < -1.0f) && (this->field_0x1a0 < -1.0f)) {
								local_a0.field_0xc = 1;
							}

							local_a0.field_0x10 = this->field_0x1fc;
							local_a0.field_0x14 = this->field_0x200;
							local_a0.field_0x18 = this->field_0x19c;
							local_a0.field_0x1c = this->field_0x1a0;
							local_a0.field_0x20 = this->field_0x1a4;
							local_a0.field_0x24 = (uint)((this->field_0x160 & 0x400) != 0);
							local_a0.field_0x28 = this->field_0x1a8.Get();
							if (local_a0.field_0x28 == (CWayPoint*)0x0) {
								local_a0.field_0x28 = (CWayPoint*)0x0;
							}

							local_c = &local_a0;
							local_a0.field_0x2c = this->field_0x1ac;
							local_a0.field_0x30 = this->field_0x1b0;

							DoMessage(pCVar9, (ACTOR_MESSAGE)0x18, (MSG_PARAM)local_c);
							field_0x1ec[iVar4] = 0;
						}
					}
					else {
						if (field_0x1ec[iVar4] == 0) {
							local_60.field_0x0 = this->field_0x16c;
							local_60.field_0x4 = this->field_0x170;

							local_60.field_0x8 = 1;
							if ((((this->field_0x160 & 0x20) == 0) && (local_60.field_0x8 = 0, (this->field_0x160 & 0x40) == 0)) &&
								(local_60.field_0x8 = 1, 0.17398384f <= fabs(this->rotationQuat.y))) {
								local_60.field_0x8 = 0;
							}

							local_60.field_0xc = 0;
							if ((this->field_0x19c < -1.0f) && (this->field_0x1a0 < -1.0f)) {
								local_60.field_0xc = 1;
							}

							local_60.field_0x10 = this->field_0x1fc;
							local_60.field_0x14 = this->field_0x200;
							local_60.field_0x18 = this->field_0x19c;
							local_60.field_0x1c = this->field_0x1a0;
							local_60.field_0x20 = this->field_0x1a4;
							local_60.field_0x24 = (uint)((this->field_0x160 & 0x400) != 0);
							local_60.field_0x28 = this->field_0x1a8.Get();
							if (local_60.field_0x28 == (CWayPoint*)0x0) {
								local_60.field_0x28 = (CWayPoint*)0x0;
							}

							local_8 = &local_60;
							local_60.field_0x2c = this->field_0x1ac;
							local_60.field_0x30 = this->field_0x1b0;

							DoMessage(pCVar9, (ACTOR_MESSAGE)0x17, (MSG_PARAM)local_8);
							field_0x1ec[iVar4] = 1;
						}
					}

					if (field_0x1ec[iVar4] != 0) {
						iVar1 = this->actorState;
						uVar5 = 0;

						if (iVar1 != -1) {
							pAVar8 = GetStateCfg(iVar1);
							uVar5 = pAVar8->flags_0x4;
						}

						if (((uVar5 & 0x100) != 0) && (this->field_0x1f4 != 0.0f)) {
							NotifyActorInWindArea(fVar11, pCVar9);
						}
					}
				}

				iVar4 = iVar4 + 1;
			}
		}

		this->field_0x1f8 = this->field_0x1f4;

		IMPLEMENTATION_GUARD_WIND_FX(
		pCVar10 = this->aFxWind;
		for (iVar4 = 0; iVar4 < (int)this->nbFxWind; iVar4 = iVar4 + 1) {
			pCVar10->Manage((uint)(0.0 < this->field_0x1f4));
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
	int iVar1;

	iVar1 = 5;
	if (this->field_0x180 <= *pTime) {
		*pTime = *pTime - this->field_0x180;
		iVar1 = 6;
		if (this->field_0x184 <= *pTime) {
			*pTime = *pTime - this->field_0x184;
			iVar1 = 7;
			if (this->field_0x188 <= *pTime) {
				*pTime = *pTime - this->field_0x188;
				iVar1 = 8;
				if (this->field_0x18c <= *pTime) {
					*pTime = *pTime - this->field_0x18c;
					iVar1 = 9;
					if (this->field_0x178 <= *pTime) {
						iVar1 = 10;
						*pTime = *pTime - this->field_0x178;
					}
				}
			}
		}
	}
	return iVar1;
}

void CActorWind::ComputeSpeedsInterpoltation()
{
	float fVar1;

	if (this->field_0x180 != 0.0f) {
		this->field_0x20c = 0.0f / this->field_0x180;
	}

	if (this->field_0x188 != 0.0f) {
		this->field_0x210 = fabs(this->field_0x164) / this->field_0x188;
	}

	if (this->field_0x178 != 0.0f) {
		this->field_0x208 = fabs(-this->field_0x164) / this->field_0x178;
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

	this->field_0x1f4 = 0.0f;
	this->field_0x218 = 0;

	local_4 = ComputeCurTime();
	if (local_4 != -1.0f) {
		iVar2 = ComputeStateFromTime(&local_4);
		switch (iVar2) {
		case 5:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x180, 0.0f, 0.0f);
			this->field_0x1f4 = fVar3;
			break;
		case 6:
			this->field_0x1f4 = 0.0f;
			break;
		case 7:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x188, 0.0f, this->field_0x164);
			this->field_0x1f4 = fVar3;
			break;
		case 8:
			this->field_0x1f4 = this->field_0x164;
			break;
		case 9:
			fVar3 = edFIntervalLERP(local_4, 0.0f, this->field_0x178, this->field_0x164, 0.0f);
			this->field_0x1f4 = fVar3;
			break;
		case 10:
			this->field_0x1f4 = 0.0f;
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
	NotifyWindParam local_40;
	edF32VECTOR4 local_20;
	NotifyWindParam* local_4;

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

		fVar3 = this->field_0x1f4;
		if (-CDynamicExt::gForceGravity.y < fVar3) {
			local_40.field_0x10 = fVar3 + ((-CDynamicExt::gForceGravity.y - fVar3) / param_1) * fVar2;
		}
		else {
			local_40.field_0x10 = edFIntervalLERP(fVar2, 0.0f, param_1, fVar3, 0.0f);
		}
	}
	else {
		local_40.field_0x10 = this->field_0x1f4;
	}

	if (local_40.field_0x10 != 0.0f) {
		pfVar1 = this->field_0x1cc.Get();
		if ((pfVar1 != (CWayPoint*)0x0) && (pNotifyActor->typeID != 6)) {
			local_50.xyz = pfVar1->field_0x0;
			local_50.w = 1.0f;
			edF32Vector4SubHard(&local_50, &local_50, &pNotifyActor->currentLocation);
			local_50.y = 0.0f;
			edF32Vector4ScaleHard(local_40.field_0x10 * 0.01f, &local_50, &local_50);
			edF32Vector4ScaleHard(local_40.field_0x10, &local_20, &local_20);
			edF32Vector4AddHard(&local_20, &local_20, &local_50);
			edF32Vector4NormalizeHard(&local_20, &local_20);
		}

		local_4 = &local_40;
		local_40.field_0x0 = local_20;
		DoMessage(pNotifyActor, (ACTOR_MESSAGE)0x16, (MSG_PARAM)local_4);
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
		this->pOwner->SetState(5, -1);
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

	if (newState == 10) {
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
		if (newState == 8) {
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
			if ((newState == 6) && (2.2f <= CScene::_pinstance->field_0x1c)) {
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
