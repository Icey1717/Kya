#include "ActorMoney.h"
#include "MemoryStream.h"

void CActorMoney::Create(ByteCode* pByteCode)
{
	KyaUpdateObjA* pKVar1;
	uint uVar2;
	int puVar3;
	int* piVar3;
	float fVar4;
	float fVar5;

	CActorMovable::Create(pByteCode);
	uVar2 = pByteCode->GetU32();
	//this->field_0x1d0 = uVar2;
	puVar3 = pByteCode->GetS32();
	//this->field_0x284 = puVar3;
	uVar2 = pByteCode->GetS32();
	//this->field_0x1d4 = uVar2;
	//CActor::SV_InstallMaterialId(this->field_0x1d4);
	//piVar3 = CActor::CreateActorSound((CActor*)this, 1);
	//*(int**)&this->field_0x280 = piVar3;
	//pKVar1 = (this->base).data.subObjA;
	//fVar4 = (pKVar1->boundingSphere).y;
	//fVar5 = (pKVar1->boundingSphere).z;
	//this->field_0x288 = (int)(pKVar1->boundingSphere).x;
	//(this->field_0x28c).x = fVar4;
	//(this->field_0x28c).y = fVar5;
	//(this->field_0x28c).z = (pKVar1->boundingSphere).w;
	return;
}

class CBehaviourMoneyFlock : public CBehaviour
{

};

class CBehaviourMoneyAddOn : public CBehaviourMoneyFlock
{

};

CBehaviour* CActorMoney::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = new CBehaviourMoneyAddOn;
	}
	else {
		if (behaviourType == 2) {
			pBehaviour = new CBehaviourMoneyFlock;
		}
		else {
			pBehaviour = (CBehaviour*)0x0;
		}
	}
	return pBehaviour;
}
