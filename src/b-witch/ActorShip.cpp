#include "ActorShip.h"
#include "MemoryStream.h"

CActorShip::CActorShip()
{
	this->field_0x1b4 = 0.0f;
}

void CActorShip::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pSVar1;
	S_NTF_TARGET_STREAM_REF* pSVar2;
	S_STREAM_EVENT_CAMERA* pSVar3;
	int iVar4;
	int* piVar5;
	int iVar6;
	uint uVar10;
	int iVar11;
	float fVar12;
	
	CActor::Create(pByteCode);

	this->careBoyStreamRef = S_ACTOR_STREAM_REF::Create(pByteCode);
	
	this->shipPathManager.Create(pByteCode);

	this->field_0x180.index = pByteCode->GetS32();
	this->field_0x184.index = pByteCode->GetS32();
	this->hunterStreamRef.index = pByteCode->GetS32();

	this->field_0x214 = pByteCode->GetF32();
	this->field_0x210 = pByteCode->GetF32();
	this->field_0x218 = pByteCode->GetF32();
	this->field_0x21c = pByteCode->GetF32();
	this->field_0x220 = pByteCode->GetF32();
	this->field_0x224 = pByteCode->GetF32() * 0.01745329f;
	this->field_0x230 = pByteCode->GetF32();
	this->field_0x228 = pByteCode->GetF32();
	this->field_0x22c = pByteCode->GetF32();
	this->field_0x25c = this->field_0x224 / this->field_0x21c;
	this->field_0x260 = this->field_0x224 / this->field_0x220;
	(this->shipPathManager).field_0x18 = pByteCode->GetS32();
	(this->shipPathManager).nb = 0;
	(this->shipPathManager).field_0xc = 0;
	this->field_0x234 = pByteCode->GetS32();
	this->field_0x238 = pByteCode->GetS32();
	this->field_0x1ac.index = pByteCode->GetS32();

	this->field_0x18c[0] = pByteCode->GetU32();
	this->field_0x18c[1] = pByteCode->GetU32();
	this->field_0x18c[2] = pByteCode->GetU32();
	this->field_0x18c[3] = pByteCode->GetU32();

	this->field_0x19c = pByteCode->GetU32();
	this->field_0x1a0 = pByteCode->GetU32();
	this->field_0x1a4 = pByteCode->GetU32();
	this->field_0x1a8 = pByteCode->GetU32();
	
	this->field_0x1b0.Create(this, pByteCode);

	field_0x23c.Create(pByteCode);

	return;
}

void CBehaviourShip::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShip::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorShip*>(pOwner);

	return;
}

int CBehaviourShip::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourShip::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourShipFLY::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShipFLY::Manage()
{
	IMPLEMENTATION_GUARD(this->BehaviourShipFLY_Manage();)

	return;
}

void CBehaviourShipFLY::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourShip::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourShipFLY::InitState(int newState)
{
	IMPLEMENTATION_GUARD(this->BehaviourShipFLY_Manage(newState, this);)

	return;
}

void CBehaviourShipFLY::TermState(int oldState, int newState)
{
	int iVar1;
	bool bVar2;
	CNewFx* pNewFx;
	CActorShip* pShip;

	pShip = this->pOwner;
	if (oldState == 6) {
		pNewFx = (pShip->field_0x26c).pFx;
		if (((pNewFx == static_cast<CNewFx*>(0x0)) || (iVar1 = (pShip->field_0x26c).id, iVar1 == 0)) ||
			(iVar1 != pNewFx->id)) {
			bVar2 = false;
		}
		else {
			bVar2 = false;
			if (((pNewFx->flags & 2) != 0) && ((pNewFx->flags & 8) == 0)) {
				bVar2 = true;
			}
		}

		if (((bVar2) && (pNewFx != static_cast<CNewFx*>(0x0))) &&
			((iVar1 = (pShip->field_0x26c).id, iVar1 != 0 && (iVar1 == pNewFx->id)))) {
			pNewFx->Stop(-1.0f);
		}
	}

	return;
}

int CBehaviourShipFLY::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

void ShipPathManager::Create(ByteCode* pByteCode)
{
	this->nbInts = pByteCode->GetS32();

	int iVar4 = this->nbInts;
	if (iVar4 != 0) {
		this->aInts = new int[iVar4];
		iVar4 = 0;
		if (0 < this->nbInts) {
			do {
				this->aInts[iVar4] = pByteCode->GetS32();
				iVar4 = iVar4 + 1;
			} while (iVar4 < this->nbInts);
		}
	}

	return;
}
