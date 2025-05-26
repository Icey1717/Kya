#include "ActorNativCmd.h"
#include "MemoryStream.h"
#include "ActorNativ.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "MathOps.h"
#include "CinematicManager.h"
#include "SectorManager.h"
#include "EdFileBase.h"

void CActorNativCmd::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pSVar1;
	uint count;
	int iVar2;
	int* pBase;
	CTalkParam* pCVar3;
	void* pvVar4;
	CTalkParam* iVar6;

	CActor::Create(pByteCode);

	pSVar1 = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * 4;
	}
	this->aNativs = pSVar1;

	this->field_0x164 = pByteCode->GetS32();

	if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->aNativs->entryCount;
	}

	this->nbTalkParams = (int)((float)iVar2 / 2.0f);
	count = this->nbTalkParams;
	if (count != 0) {
		this->aTalkParams = new CTalkParam[count];
	}

	iVar2 = 0;
	if (0 < this->nbTalkParams) {
		do {
			iVar6 = this->aTalkParams + iVar2;
			iVar6->field_0x8 = new CTalkParamSubObj[2];
			iVar2 = iVar2 + 1;

			iVar6->nbSubObj = 2;
		} while (iVar2 < this->nbTalkParams);
	}

	this->addOnA.Create(pByteCode);
	this->addOnB.Create(pByteCode);
	this->addOnC.Create(pByteCode);
	this->addOnD.Create(pByteCode);
	return;
}

void CActorNativCmd::Init()
{
	int iVar1;
	int iVar2;
	int iVar3;
	CTalkParam* iVar4;
	S_ACTOR_STREAM_REF* pActorRef;
	int iVar5;
	CActorNativMsgParam local_20;

	CActor::Init();

	this->addOnA.Init(this);
	this->addOnB.Init(this);
	this->addOnC.Init(this);
	this->addOnD.Init(this);

	pActorRef = this->aNativs;
	for (int i = 0; i < pActorRef->entryCount; i++) {
		pActorRef->aEntries[i].Init();
	}

	iVar5 = 0;
	if (0 < this->nbTalkParams) {
		do {
			iVar4 = this->aTalkParams + iVar5;
			iVar4->field_0x10 = 0;
			iVar4->field_0x1c = 0;
			iVar4->field_0x14 = 0;
			iVar4->field_0x18 = 0;
			iVar4->field_0x1d = 0;

			iVar3 = 0;
			if (0 < iVar4->nbSubObj) {
				iVar1 = 0;
				do {
					iVar4->field_0x8[iVar3].field_0x0 = 0;
					iVar4->field_0x8[iVar3].field_0x4 = 1;
					iVar3 = iVar3 + 1;
				} while (iVar3 < iVar4->nbSubObj);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbTalkParams);
	}

	this->field_0x1d0 = 0;

	this->addOnA.Reset();
	this->addOnB.Reset();
	this->addOnC.Reset();
	this->addOnD.Reset();

	iVar2 = 0;
	iVar5 = 0;
	while (true) {
		if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = 0;
		}
		else {
			iVar3 = this->aNativs->entryCount;
		}

		if (iVar3 <= iVar2) break;

		local_20.type = 1;
		local_20.field_0x4 = this->field_0x164;

		DoMessage(this->aNativs->aEntries[iVar2].Get(), (ACTOR_MESSAGE)0x4e, &local_20);

		iVar5 = iVar5 + 4;
		iVar2 = iVar2 + 1;
	}

	return;
}

void CActorNativCmd::Reset()
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorNativCmd::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 0x2) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorNativCmd::_gStateCfg_NVC[2] = {
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0)
};

StateConfig* CActorNativCmd::GetStateCfg(int state)
{
	StateConfig* pSVar1;

	if (state < 5) {
		pSVar1 = CActor::GetStateCfg(state);
	}
	else {
		pSVar1 = _gStateCfg_NVC + state + -5;
	}

	return pSVar1;
}

void CActorNativCmd::ChangeManageState(int state)
{
	int iVar1;
	int iVar3;
	CActorNativMsgParam local_20;

	CActor::ChangeManageState(state);

	iVar3 = 0;
	if (state != 0) {
		while (true) {
			if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
				iVar1 = 0;
			}
			else {
				iVar1 = this->aNativs->entryCount;
			}

			if (iVar1 <= iVar3) break;

			local_20.type = 1;
			local_20.field_0x4 = this->field_0x164;

			DoMessage(this->aNativs->aEntries[iVar3].Get(), (ACTOR_MESSAGE)0x4e, &local_20);
			iVar3 = iVar3 + 1;
		}
	}

	return;
}

int CActorNativCmd::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
}

CAddOnNativ::CAddOnNativ()
{
	this->nbSubObjs = 0;
	this->aSubObjs = (CAddOnSubObj*)0x0;

	return;
}

void CAddOnNativ::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int* piVar2;
	void* pvVar3;
	int iVar4;
	int iVar7;
	int iVar9;
	float fVar10;

	this->nbSubObjs = pByteCode->GetS32();
	uVar1 = this->nbSubObjs;

	if (uVar1 != 0) {
		this->aSubObjs = new CAddOnSubObj[uVar1];
		iVar9 = 0;
		if (0 < this->nbSubObjs) {
			do {
				CAddOnSubObj* puVar9 = this->aSubObjs + iVar9;
				puVar9->field_0x0 = pByteCode->GetU32();
				puVar9->field_0x8 = pByteCode->GetS32();
				if (0 < puVar9->field_0x8) {
					puVar9->field_0x4 = new int[puVar9->field_0x8];
				}

				iVar7 = 0;
				if (0 < puVar9->field_0x8) {
					do {
						puVar9->field_0x4[iVar7] = pByteCode->GetS32();
						iVar7 = iVar7 + 1;
					} while (iVar7 < puVar9->field_0x8);
				}

				puVar9->field_0x10 = -1;
				iVar9 = iVar9 + 1;
			} while (iVar9 < this->nbSubObjs);
		}
	}
}

void CAddOnNativ::Init(CActor* pActor)
{
	CAddOn::Init(pActor);

	this->field_0xd = 0;

	return;
}

void CAddOnNativ::Manage()
{
	CAddOn::Manage();

	int iVar5;

	bool bVar1;
	if (GetCinematic()) {
		if ((GetCinematic()->state != CS_Stopped) && (bVar1 = true, this->field_0xc != 0)) goto LAB_003e1a88;
	}

	bVar1 = false;

LAB_003e1a88:

	if ((!bVar1) && (iVar5 = 0, 0 < this->nbSubObjs)) {
		do {
			CAddOnSubObj* pSubObj = this->aSubObjs + iVar5;
			if (pSubObj != (CAddOnSubObj*)0x0) {
				float fVar7 = pSubObj->field_0x14 + GetTimer()->cutsceneDeltaTime;
				pSubObj->field_0x14 = fVar7;

				if ((1.0f <= fVar7) && (pSubObj->pCinematic != (CCinematic*)0x0)) {
					pSubObj->SetCinematic((CCinematic*)0x0);
				}
			}

			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbSubObjs);
	}

	return;
}

void CAddOnNativ::Reset()
{
	CAddOn::Reset();

	this->field_0xd = 0;
	int iVar3 = 0;
	if (0 < this->nbSubObjs) {
		do {
			CAddOnSubObj* pSubObj = this->aSubObjs + iVar3;

			pSubObj->pCinematic = (CCinematic*)0x0;
			pSubObj->field_0x10 = -1;
			pSubObj->field_0x14 = 0.0f;

			iVar3 = iVar3 + 1;
		} while (iVar3 < this->nbSubObjs);
	}

	return;
}

CAddOnSubObj* CAddOnNativ::GetSubObj(uint param_2, int pActor)
{
	CCinematic* pCVar1;
	CAddOnSubObj* pCVar2;
	bool bVar3;
	CCinematic* pCVar4;
	ulong uVar5;
	int iVar6;
	CAddOnSubObj* pCVar7;

	iVar6 = 0;
	if (0 < this->nbSubObjs) {
		pCVar7 = this->aSubObjs;
		do {
			if (param_2 == pCVar7->field_0x0) goto LAB_003e1c88;
			iVar6 = iVar6 + 1;
			pCVar7 = pCVar7 + 1;
		} while (iVar6 < this->nbSubObjs);
	}

	pCVar7 = (CAddOnSubObj*)0x0;
LAB_003e1c88:
	if (pCVar7 == (CAddOnSubObj*)0x0) {
		return (CAddOnSubObj*)0x0;
	}

	pCVar1 = pCVar7->pCinematic;
	if (pCVar1 == (CCinematic*)0x0) {
		return (CAddOnSubObj*)0x0;
	}

	if (pActor != 0x0) {
		return pCVar7;
	}

	pCVar2 = this->pSubObj;
	pCVar4 = (CCinematic*)0x0;
	if (pCVar2 != (CAddOnSubObj*)0x0) {
		pCVar4 = pCVar2->pCinematic;
	}

	if (pCVar4 != (CCinematic*)0x0) {
		pCVar4 = (CCinematic*)0x0;
		if (pCVar2 != (CAddOnSubObj*)0x0) {
			pCVar4 = pCVar2->pCinematic;
		}
		if ((pCVar4->state != CS_Stopped) && (bVar3 = true, this->field_0xc != 0)) goto LAB_003e1d00;
	}

	bVar3 = false;
LAB_003e1d00:
	if ((((!bVar3) && (uVar5 = StaticEdFileBase_004497f0.FUN_00401f30(), uVar5 != 0)) &&
		(pCVar1->cineBankLoadStage_0x2b4 == 4)) && ((pCVar1->flags_0x8 & 0x80) == 0)) {
		return pCVar7;
	}

	return (CAddOnSubObj*)0x0;
}

bool CAddOnNativ::Func_0x20(uint param_2, CActor* param_3, int pActor)
{
	CAddOnSubObj* pCVar1;
	bool bVar2;
	CCinematic* pCinematic;

	if (param_3 == 0) {
		param_3 = this->pOwner;
	}

	pCVar1 = this->pSubObj;
	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	if (pCinematic == (CCinematic*)0x0) {
	LAB_003e15a8:
		bVar2 = false;
	}
	else {
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}

		if ((pCinematic->state == CS_Stopped) || (bVar2 = true, this->field_0xc == 0)) goto LAB_003e15a8;
	}

	if (((bVar2) || (this->field_0xc != 0)) && (pActor == 0x0)) {
		return false;
	}

	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	if (pCinematic != (CCinematic*)0x0) {
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}
		if ((pCinematic->state != CS_Stopped) && (bVar2 = true, this->field_0xc != 0)) goto LAB_003e1620;
	}
	bVar2 = false;
LAB_003e1620:
	if (bVar2) {
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}

		pCinematic->FUN_001c92b0();

		pCVar1 = this->pSubObj;
		pCinematic = (CCinematic*)0x0;
		if (pCVar1 != (CAddOnSubObj*)0x0) {
			pCinematic = pCVar1->pCinematic;
		}

		bVar2 = pCinematic->Has_0x2d8();
		if (bVar2 != false) {
			pCVar1 = this->pSubObj;
			pCinematic = (CCinematic*)0x0;
			if (pCVar1 != (CAddOnSubObj*)0x0) {
				pCinematic = pCVar1->pCinematic;
			}

			pCinematic->Remove_0x2d8();
		}
	}

	Func_0x34(param_2, param_3);

	pCVar1 = GetSubObj(param_2, pActor);
	if ((pCVar1 == (CAddOnSubObj*)0x0) && (pCVar1 = this->pSubObj, pCVar1 != (CAddOnSubObj*)0x0)) {
		pCVar1->SetCinematic((CCinematic*)0x0);
	}

	this->pSubObj = pCVar1;
	pCVar1 = this->pSubObj;

	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	if (pCinematic == (CCinematic*)0x0) {
		return false;
	}

	pCinematic = (CCinematic*)0x0;
	if (pCVar1 != (CAddOnSubObj*)0x0) {
		pCinematic = pCVar1->pCinematic;
	}

	pCinematic->TryTriggerCutscene(param_3, 0);

	this->field_0xc = 1;

	return true;
}

bool CAddOnNativ::Func_0x24(uint param_2, CActor* pActor)
{
	bool bVar1;
	int iVar2;
	CAddOnSubObj* pSubObj;
	CCinematic* pCinematic;

	if (pActor == (CActor*)0x0) {
		pActor = this->pOwner;
	}

	Func_0x30(param_2);

	if (Func_0x2c(param_2) == 0) {
		bVar1 = false;
	}
	else {
		iVar2 = 0;
		if (0 < this->nbSubObjs) {
			pSubObj = this->aSubObjs;

			do {
				if (param_2 == pSubObj->field_0x0) {
					iVar2 = pSubObj->FUN_003e37e0();
					if (iVar2 == -1) {
						pSubObj = (CAddOnSubObj*)0x0;
					}
					goto LAB_003e1878;
				}

				iVar2 = iVar2 + 1;
				pSubObj = pSubObj + 1;
			} while (iVar2 < this->nbSubObjs);
		}

		pSubObj = (CAddOnSubObj*)0x0;

	LAB_003e1878:
		if (pSubObj == (CAddOnSubObj*)0x0) {
			bVar1 = false;
		}
		else {
			pCinematic = pSubObj->pCinematic;
			bVar1 = pCinematic->Has_0x2d8();
			if (bVar1 == false) {
				pCinematic->Add_0x2d8();
			}

			bVar1 = true;
		}
	}

	return bVar1;
}

bool CAddOnNativ::Func_0x2c(uint param_2)
{
	CAddOnSubObj* pSubObj;
	int curIndex;

	pSubObj = (CAddOnSubObj*)0x0;
	curIndex = 0;
	if (0 < this->nbSubObjs) {
		do {
			pSubObj = this->aSubObjs + curIndex;

			if (param_2 == pSubObj->field_0x0) break;

			curIndex = curIndex + 1;
		} while (curIndex < this->nbSubObjs);
	}

	if (pSubObj != (CAddOnSubObj*)0x0) {
		if (pSubObj->pCinematic == (CCinematic*)0x0) {
			return true;
		}

		if (pSubObj->pCinematic->Has_0x2d8() == false) {
			return true;
		}
	}

	return false;
}

void CAddOnNativ::Func_0x30(uint param_2)
{
	int curIndex;
	CAddOnSubObj* pSubObj;

	curIndex = 0;
	if (0 < this->nbSubObjs) {
		pSubObj = this->aSubObjs;
		do {

			if (param_2 == pSubObj->field_0x0) goto LAB_003e1b78;

			curIndex = curIndex + 1;
			pSubObj = pSubObj + 1;
		} while (curIndex < this->nbSubObjs);
	}

	pSubObj = (CAddOnSubObj*)0x0;
LAB_003e1b78:
	if (pSubObj != (CAddOnSubObj*)0x0) {
		pSubObj->field_0x14 = 0.0f;
	}

	return;
}

void CBehaviourNativCmdStand::Manage()
{
	CActorNativCmd* pNativCmd;
	CActorNativ* pNativ;
	S_ACTOR_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;
	int iVar6;

	pNativCmd = this->pOwner;
	iVar5 = pNativCmd->actorState;
	if ((iVar5 != 6) && (iVar5 == 5)) {
		iVar6 = 0;
		iVar5 = 0;
		while (true) {
			pSVar3 = pNativCmd->aNativs;
			iVar4 = 0;
			if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar4 = pSVar3->entryCount;
			}

			if (iVar4 <= iVar6) break;

			pNativ = reinterpret_cast<CActorNativ*>(pSVar3->aEntries[iVar6].Get());

			if ((pNativ != (CActorNativ*)0x0) && ((pNativ->flags & 4) != 0)) {
				pNativCmd->FUN_00394250(pNativ);
			}

			iVar5 = iVar5 + 4;
			iVar6 = iVar6 + 1;
		}

		pNativCmd->ManageDiscussion();
		pNativCmd->ManageNativWithKim();
		//pNativCmd->FUN_00393620();
		//pNativCmd->FUN_00393470();
	}

	IMPLEMENTATION_GUARD_ADD_ON(
	pNativCmd->addOnA.Manage();
	pNativCmd->addOnB.Manage();
	pNativCmd->addOnC.Manage();
	pNativCmd->addOnD.Manage();)

	return;
}

void CBehaviourNativCmdStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorNativCmd* pCVar1;

	this->pOwner = reinterpret_cast<CActorNativCmd*>(pOwner);
	if (newState == -1) {
		pCVar1 = this->pOwner;
		pCVar1->SetState(5, -1);
	}
	else {
		pCVar1 = this->pOwner;
		pCVar1->SetState(newState, newAnimationType);
	}
	return;
}

void CActorNativCmd::ManageDiscussion()
{
	bool bVar1;
	Timer* pTVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	CActor* pReceiver;
	CTalkParam* pTalkParam;
	long lVar6;
	int iVar7;
	float fVar9;
	undefined4 local_20[2];
	undefined4 local_18;
	undefined4* local_4;

	iVar5 = this->nbTalkParams;
	iVar7 = 0;
	if (0 < iVar5) {
		do {
			pTalkParam = (CTalkParam*)0x0;
			if (iVar7 < iVar5) {
				pTalkParam = this->aTalkParams + iVar7;
			}

			if (pTalkParam->field_0x1d == 1) {
				IMPLEMENTATION_GUARD(
				pTVar2 = GetTimer();
				fVar9 = (float)pTalkParam->field_0x14 + pTVar2->cutsceneDeltaTime;
				pTalkParam->field_0x14 = fVar9;
				bVar1 = true;
				if (fVar9 <= (float)pTalkParam->field_0x18) {
					iVar5 = 0;
					if (0 < (int)pTalkParam->field_0x10) {
						iVar4 = 0;
						do {
							iVar3 = 0;
							if (iVar5 < pTalkParam->nbSubObj) {
								iVar3 = *(int*)((int)&pTalkParam->field_0x8->field_0x0 + iVar4);
							}
							if ((*(uint*)(iVar3 + 8) & 4) == 0) {
								bVar1 = true;
								goto LAB_00394120;
							}
							iVar5 = iVar5 + 1;
							iVar4 = iVar4 + 8;
						} while (iVar5 < (int)pTalkParam->field_0x10);
					}
					bVar1 = false;
				}
			LAB_00394120:
				if (bVar1) {
					lVar6 = 0;
					if (0 < pTalkParam->nbSubObj) {
						iVar5 = 0;
						do {
							pReceiver = (CActor*)0x0;
							if (lVar6 < pTalkParam->nbSubObj) {
								pReceiver = *(CActor**)((int)&pTalkParam->field_0x8->field_0x0 + iVar5);
							}
							local_20[0] = CTalkParam::ChooseFinishOrder(pTalkParam, lVar6);
							local_18 = 0xffffffff;
							if (pReceiver != (CActor*)0x0) {
								local_4 = local_20;
								CActor::DoMessage((CActor*)this, pReceiver, 0x4e, (uint)local_20);
							}
							lVar6 = (long)((int)lVar6 + 1);
							iVar5 = iVar5 + 8;
						} while (lVar6 < pTalkParam->nbSubObj);
					}
					pTalkParam->field_0x10 = 0;
					pTalkParam->field_0x1c = 0;
					pTalkParam->field_0x14 = 0;
					pTalkParam->field_0x18 = 0;
					pTalkParam->field_0x1d = 0;
					iVar5 = 0;
					if (0 < pTalkParam->nbSubObj) {
						iVar4 = 0;
						do {
							iVar5 = iVar5 + 1;
							*(undefined4*)((int)&pTalkParam->field_0x8->field_0x0 + iVar4) = 0;
							*(undefined4*)((int)&pTalkParam->field_0x8->field_0x4 + iVar4) = 1;
							iVar4 = iVar4 + 8;
						} while (iVar5 < pTalkParam->nbSubObj);
					}
				})
			}
			iVar5 = this->nbTalkParams;
			iVar7 = iVar7 + 1;
		} while (iVar7 < iVar5);
	}

	return;
}

void CActorNativCmd::ManageNativWithKim()
{
	CActorNativ* pNativ;
	CActorNativ* pActor;
	bool bVar1;
	CActorHero* pCVar2;
	CBehaviour* pCVar3;
	long lVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 eStack48;
	undefined4 local_20[7];
	undefined4* local_4;

	iVar7 = 0;
	iVar6 = -1;
	fVar11 = 3.402823e+38;

	do {
		if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
			iVar5 = 0;
		}
		else {
			iVar5 = this->aNativs->entryCount;
		}
		if (iVar5 <= iVar7) {
			iVar7 = 0;
			while (true) {
				if (this->aNativs == (S_ACTOR_STREAM_REF*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = this->aNativs->entryCount;
				}

				if (iVar5 <= iVar7) break;

				pActor = (CActorNativ*)this->aNativs->aEntries[iVar7].Get();
				if ((((pActor != (CActor*)0x0) && (pCVar3 = pActor->GetBehaviour(3), pCVar3 != (CBehaviour*)0x0)) && ((pActor->flags & 4) != 0)) && pActor->field_0x504 != 0) {
					IMPLEMENTATION_GUARD(
					FUN_00394d00((long)(int)this, (int)pActor);
					if (iVar7 == iVar6) {
						local_20[0] = 10;
					}
					else {
						local_20[0] = 0xb;
					}
					local_4 = local_20;
					CActor::DoMessage((CActor*)this, this->aNativs->aEntries[iVar7].pActor, 0x4e, (uint)local_4);)
				}
				iVar7 = iVar7 + 1;
			}
			return;
		}

		pNativ = (CActorNativ*)this->aNativs->aEntries[iVar7].Get();
		fVar10 = fVar11;
		iVar5 = iVar6;
		if (((pNativ != (CActorNativ*)0x0) && (pCVar3 = pNativ->GetBehaviour(3), pCVar3 != (CBehaviour*)0x0)) && (((pNativ->flags & 4) != 0 && (pNativ->field_0x504 != 0)))) {
			if (pNativ == (CActorNativ*)0x0) {
			LAB_00393de0:
				bVar1 = false;
			}
			else {
				if (pNativ->field_0x378 == 1) {
					IMPLEMENTATION_GUARD(
					CActorNativ::FUN_00162a70(pNativ);
					lVar4 = (*(code*)((this->addOnB).base.pVTable)->GetSubObj)();)
				}
				else {
					IMPLEMENTATION_GUARD(
					CActorNativ::FUN_00162a70(pNativ);
					lVar4 = (*(code*)((this->addOnA).base.pVTable)->GetSubObj)();)
				}
				bVar1 = true;
				if (lVar4 == 0) goto LAB_00393de0;
			}

			pCVar2 = CActorHero::_gThis;
			if (bVar1) {
				edF32Vector4SubHard(&eStack48, &CActorHero::_gThis->currentLocation, &pNativ->currentLocation);
				edF32Vector4SafeNormalize0Hard(&eStack48, &eStack48);
				fVar10 = pCVar2->currentLocation.x - pNativ->currentLocation.x;
				fVar8 = pCVar2->currentLocation.y - pNativ->currentLocation.y;
				fVar9 = pCVar2->currentLocation.z - pNativ->currentLocation.z;
				fVar10 = sqrtf(fVar10 * fVar10 + fVar8 * fVar8 + fVar9 * fVar9);
				fVar8 = edF32Vector4DotProductHard(&pNativ->rotationQuat, &eStack48);
				fVar9 = edF32Vector4DotProductHard(&pCVar2->rotationQuat, &eStack48);
				if (((fVar8 <= 0.0f) || (0.0f <= fVar9)) || (3.0f <= fVar10)) {
					fVar10 = 3.402823e+38f;
				}

				iVar5 = iVar7;
				if (fVar11 <= fVar10) {
					fVar10 = fVar11;
					iVar5 = iVar6;
				}
			}
		}

		iVar6 = iVar5;
		iVar7 = iVar7 + 1;
		fVar11 = fVar10;
	} while (true);
}

void CActorNativCmd::FUN_00394250(CActorNativ* pNativ)
{
	int iVar1;
	CTakePutTrajectoryParam* pCVar2;
	StateConfig* pSVar3;
	int iVar4;
	CPathFinderClient* lVar5;
	uint uVar6;
	ulong uVar7;
	ActionEntry* pAction;
	int iVar9;
	int iVar10;
	float fVar11;
	float fVar12;
	CActorNativMsgParamOther local_50;
	undefined4 local_30[10];
	undefined4* local_8;
	undefined4* local_4;

	if ((pNativ != (CActorNativ*)0x0) && (pNativ->field_0x3f0 == this)) {
		iVar9 = pNativ->actorState;
		if (iVar9 == -1) {
			uVar6 = 0;
		}
		else {
			pSVar3 = pNativ->GetStateCfg(iVar9);
			uVar6 = pSVar3->flags_0x4 & 0x800;
		}

		if (uVar6 != 0) {
			fVar11 = pNativ->FUN_00164070();
			iVar9 = -1;
			local_50.field_0x4 = 0;
			iVar10 = 0;
			fVar12 = 0.0f;
			fVar11 = (fVar11 * (float)CScene::Rand()) / 32767.0f;
			if (0 < pNativ->nbActions) {
				do {
					pAction = pNativ->aActions + iVar10;
					iVar1 = pAction->field_0x0;
					if (iVar9 != iVar1) {
						local_50.field_0x4 = 0;
						iVar9 = iVar1;
					}

					if (iVar1 == 2) {
						pCVar2 = (pNativ->behaviourTakeAndPut).aTrajectoryParams;
						lVar5 = pNativ->GetPathfinderClient();
						iVar4 = pCVar2[local_50.field_0x4].IsWayPointValidPosition(lVar5);
						if (iVar4 != 0) {
							fVar12 = fVar12 + pAction->field_0x4;
						}
					}
					else {
						fVar12 = fVar12 + (float)pAction->field_0x4;
					}

					if (fVar11 < fVar12) {
						if (iVar1 == 2) {
							local_50.type = 4;
							DoMessage(pNativ, (ACTOR_MESSAGE)0x4e, &local_50);
							return;
						}
						if (iVar1 != 1) {
							if (iVar1 != 0) {
								return;
							}
							local_4 = local_30;
							local_30[0] = 2;
							DoMessage(pNativ, (ACTOR_MESSAGE)0x4e, local_4);
							return;
						}

						IMPLEMENTATION_GUARD(
						FUN_003944e0(pNativ, local_50.field_0x4);)
						return;
					}
					iVar10 = iVar10 + 1;
					local_50.field_0x4 = local_50.field_0x4 + 1;
				} while (iVar10 < pNativ->nbActions);
			}
		}
	}

	return;
}