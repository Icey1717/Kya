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
