#include "ActorShockerCmd.h"
#include "MemoryStream.h"

CActorShockerCmd::CActorShockerCmd()
{
	this->sequenceParamsCount = 0;
	this->aSequenceParams = (CSequenceParam*)0x0;

	return;
}

void CActorShockerCmd::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	int* piVar4;
	CSequenceParam* pCVar5;
	int iVar6;
	float* pfVar7;
	CSequenceParam* piVar3;
	float fVar10;

	CActor::Create(pByteCode);

	this->field_0x168 = pByteCode->GetU32();
	this->sequenceParamsCount = pByteCode->GetS32();

	uVar1 = this->sequenceParamsCount;
	if (uVar1 != 0) {
		this->aSequenceParams = new CSequenceParam[uVar1];
		iVar2 = 0;
		if (0 < this->sequenceParamsCount) {
			do {
				piVar3 = &this->aSequenceParams[iVar2];
				piVar3->actorRef.index = pByteCode->GetS32();
				piVar3->count_0x4 = pByteCode->GetS32();

				uVar1 = piVar3->count_0x4;
				if (uVar1 != 0) {
					piVar3->aFloats = new float[uVar1];
					iVar6 = 0;
					if (0 < piVar3->count_0x4) {
						do {
							piVar3->aFloats[iVar6] = pByteCode->GetF32();
							iVar6 = iVar6 + 1;
						} while (iVar6 < piVar3->count_0x4);
					}
				}

				iVar2 = iVar2 + 1;
			} while (iVar2 < this->sequenceParamsCount);
		}
	}

	return;
}

void CActorShockerCmd::Init()
{
	int iVar1;
	CSequenceParam* pRef;
	undefined4 local_4;

	iVar1 = 0;
	if (0 < this->sequenceParamsCount) {
		do {
			pRef = this->aSequenceParams + iVar1;
			pRef->actorRef.Init();
			pRef->field_0xc = 0;
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->sequenceParamsCount);
	}

	CActor::Init();

	this->field_0x180 = -1;
	this->field_0x178 = 0;
	this->field_0x174 = 0;

	iVar1 = 0;
	if (0 < this->sequenceParamsCount) {
		do {
			this->aSequenceParams[iVar1].field_0xc = 0;
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->sequenceParamsCount);
	}

	iVar1 = 0;
	if (0 < this->sequenceParamsCount) {
		for (; iVar1 < this->sequenceParamsCount; iVar1 = iVar1 + 1) {
			local_4 = 0;
			DoMessage(this->aSequenceParams[iVar1].actorRef.Get(), (ACTOR_MESSAGE)0x69, &local_4);
		}
	}
	return;
}

void CActorShockerCmd::Reset()
{
	int iVar2;
	undefined4 local_4;

	CActor::Reset();

	this->field_0x180 = -1;
	this->field_0x178 = 0;
	this->field_0x174 = 0;

	iVar2 = 0;
	if (0 < this->sequenceParamsCount) {
		do {
			this->aSequenceParams[iVar2].field_0xc = 0;
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->sequenceParamsCount);
	}

	iVar2 = 0;
	if (0 < this->sequenceParamsCount) {
		for (; iVar2 < this->sequenceParamsCount; iVar2 = iVar2 + 1) {
			local_4 = 0;
			DoMessage(this->aSequenceParams[iVar2].actorRef.Get(), (ACTOR_MESSAGE)0x69, &local_4);
		}
	}

	return;
}

CBehaviour* CActorShockerCmd::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 2) {
		pNewBehaviour = &this->behaviourStand;
	}
	else {
		pNewBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pNewBehaviour;
}

StateConfig CActorShockerCmd::_gStateCfg_SCM[2] =
{
	StateConfig(0, 0),
	StateConfig(0, 0)
};

StateConfig* CActorShockerCmd::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 2);
		pStateConfig = _gStateCfg_SCM + state + -5;
	}

	return pStateConfig;
}

int CActorShockerCmd::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActor* pCVar1;
	StateConfig* pSVar2;
	uint uVar3;
	int iVar4;
	ulong uVar5;
	int iVar6;
	int iVar7;
	CActor* piVar1;

	if (msg == 0x69) {
		int* pMsg = static_cast<int*>(pMsgParam);
		if (*pMsg == 3) {
			iVar4 = 1;
			this->field_0x174 = this->field_0x174 + -1;
		}
		else {
			if (*pMsg == 2) {
				iVar4 = 0;
				this->field_0x174 = this->field_0x174 + 1;
				iVar7 = 0;
				if (0 < this->sequenceParamsCount) {
					do {
						pCVar1 = this->aSequenceParams[iVar7].actorRef.Get();
						if (pCVar1->actorState == -1) {
							uVar3 = 0;
						}
						else {
							pSVar2 = pCVar1->GetStateCfg(pCVar1->actorState);
							uVar3 = pSVar2->flags_0x4 & 1;
						}

						if ((uVar3 == 0) && ((this->aSequenceParams[iVar7].actorRef.Get()->flags & 4) != 0)) {
							iVar4 = iVar4 + 1;
						}

						iVar7 = iVar7 + 1;
					} while (iVar7 < this->sequenceParamsCount);
				}

				if ((this->field_0x174 == iVar4) && (this->field_0x178 == 0)) {
					iVar4 = this->aSequenceParams->count_0x4;
					if (this->field_0x168 == 0) {
						uVar5 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
						CScene::_pinstance->field_0x38 = uVar5;
						iVar4 = iVar4 * (static_cast<uint>(uVar5 >> 0x10) & 0x7fff);
						if (iVar4 < 0) {
							iVar4 = iVar4 + 0x7fff;
						}

						this->field_0x180 = iVar4 >> 0xf;
					}
					else {
						if (iVar4 == 0) {
							trap(7);
						}

						this->field_0x180 = (this->field_0x180 + 1) % iVar4;
					}

					SetState(6, -1);
				}

				iVar4 = 1;
			}
			else {
				iVar4 = 0;
			}
		}
	}
	else {
		iVar4 = CActor::InterpretMessage(pSender, msg, pMsgParam);
	}

	return iVar4;
}

void CActorShockerCmd::StateShockerCmdSequence()
{
	int iVar2;
	CSequenceParam* pParam;
	int iVar4;
	undefined4 local_4;

	iVar4 = 0;
	if (0 < this->sequenceParamsCount) {
		do {
			pParam = &this->aSequenceParams[iVar4];
			if ((pParam->field_0xc == 0) && (pParam->aFloats[this->field_0x180] < this->timeInAir)) {
				local_4 = 1;
				iVar2 = DoMessage(pParam->actorRef.Get(), (ACTOR_MESSAGE)0x69, &local_4);
				if (iVar2 != 0) {
					this->field_0x178 = this->field_0x178 + 1;
					pParam->field_0xc = 1;
				}
			}

			iVar4 = iVar4 + 1;
		} while (iVar4 < this->sequenceParamsCount);
	}

	if (this->field_0x178 == this->field_0x17c) {
		this->field_0x178 = 0;
		iVar4 = 0;
		if (0 < this->sequenceParamsCount) {
			do {
				this->aSequenceParams[iVar4].field_0xc = 0;
				iVar4 = iVar4 + 1;
			} while (iVar4 < this->sequenceParamsCount);
		}

		SetState(5, -1);
	}

	return;
}

void CBehaviourShockerCmd::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShockerCmd::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorShockerCmd*>(pOwner);

	return;
}

int CBehaviourShockerCmd::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourShockerCmd::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourShockerCmdStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShockerCmdStand::Manage()
{
	if (this->pOwner->actorState == 6) {
		this->pOwner->StateShockerCmdSequence();
	}

	return;
}

void CBehaviourShockerCmdStand::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourShockerCmd::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourShockerCmdStand::InitState(int newState)
{
	StateConfig* pSVar1;
	CActor* pCVar2;
	uint uVar3;
	int iVar5;
	int iVar6;
	CActor* piVar1;
	CActorShockerCmd* pShockerCmd;

	pShockerCmd = this->pOwner;
	if (newState == 6) {
		pShockerCmd->field_0x178 = 0;
		iVar5 = 0;
		iVar6 = 0;
		if (0 < pShockerCmd->sequenceParamsCount) {
			do {
				pCVar2 = pShockerCmd->aSequenceParams[iVar6].actorRef.Get();
				if (pCVar2->actorState == -1) {
					uVar3 = 0;
				}
				else {
					pSVar1 = pCVar2->GetStateCfg(pCVar2->actorState);
					uVar3 = pSVar1->flags_0x4 & 1;
				}

				if ((uVar3 == 0) && (pCVar2 = pShockerCmd->aSequenceParams[iVar6].GetActor(), (pCVar2->flags & 4) != 0)) {
					iVar5 = iVar5 + 1;
				}

				iVar6 = iVar6 + 1;
			} while (iVar6 < pShockerCmd->sequenceParamsCount);
		}

		pShockerCmd->field_0x17c = iVar5;
	}

	return;
}

void CBehaviourShockerCmdStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourShockerCmdStand::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

CActor* CSequenceParam::GetActor()
{
	return this->actorRef.Get();
}
