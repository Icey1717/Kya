#include "ActorBonus.h"
#include "MemoryStream.h"

void CActorBonus::Create(ByteCode* pByteCode)
{
	KyaUpdateObjA* pKVar1;
	int iVar2;
	int* piVar3;
	float fVar4;
	float fVar5;

	CActorMovable::Create(pByteCode);
	//(this->vector_0x1e4).x = 0.0;
	iVar2 = pByteCode->GetS32();
	//this->field_0x1dc = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->particleID_0x1d4 = iVar2;
	//CActor::SV_InstallMaterialId(this->particleID_0x1d4);
	iVar2 = pByteCode->GetS32();
	//this->field_0x1d4 = iVar2;
	//CActor::SV_InstallMaterialId(this->field_0x1d4);
	//piVar3 = CActor::CreateActorSound((CActor*)this, 1);
	//*(int**)&this->field_0x1d8 = piVar3;
	//pKVar1 = (this->base).base.data.subObjA;
	//fVar4 = (pKVar1->boundingSphere).y;
	//fVar5 = (pKVar1->boundingSphere).z;
	//(this->vector_0x1e4).y = (pKVar1->boundingSphere).x;
	//(this->vector_0x1e4).z = fVar4;
	//(this->vector_0x1e4).w = fVar5;
	//this->field_0x1f0 = (pKVar1->boundingSphere).w;
	return;
}



CBehaviour* CActorBonus::BuildBehaviour(int behaviourType)
{
	CBehaviour* pCVar1;

	if (behaviourType == 2) {
		pCVar1 = (CBehaviour*)0x0;
	}
	else {
		if (behaviourType == 6) {
			pCVar1 = new CCBehaviourBonusAddOn;
		}
		else {
			if (behaviourType == 5) {
					pCVar1 = &this->field_0x200;
			}
			else {
				if (behaviourType == 4) {
					pCVar1 = new CBehaviourBonusPath;
				}
				else {
					if (behaviourType == 3) {
						pCVar1 = new CBehaviourBonusTurn;
					}
					else {
						pCVar1 = CActor::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}
	return pCVar1;
}
