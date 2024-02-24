#include "ActorEventGenerator.h"
#include "MemoryStream.h"

void CActorEventGenerator::Create(ByteCode* pByteCode)

{
	undefined* puVar1;
	uint uVar2;
	int iVar3;
	//ActorVTable** ppAVar4;
	int iVar5;
	CActorEventGenerator* pCVar6;

	SkipToNextActor(pByteCode);

	//CActor::Create(pByteCode);
	//this->flags_0x164 = 0;
	//uVar2 = pByteCode->GetU32();
	//iVar3 = pByteCode->GetS32();
	//this->count_0x16c = iVar3;
	//if (uVar2 == 1) {
	//	g_EventGeneratorPtr_004497bc = this;
	//	this->flags_0x164 = this->flags_0x164 | 2;
	//}
	//else {
	//	if (-1 < this->count_0x16c) {
	//		this->flags_0x164 = this->flags_0x164 | 2;
	//	}
	//}
	//this->count_0x2f0 = 0;
	//puVar1 = EventGenerator_12_004265a8.field_0x8;
	//iVar3 = EventGenerator_12_004265a8.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_004265a8.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//iVar5 = 0;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x530;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_004265b8.field_0x8;
	//iVar3 = EventGenerator_12_004265b8.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_004265b8.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x538;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_004265c8.field_0x8;
	//iVar3 = EventGenerator_12_004265c8.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_004265c8.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x53c;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_004265d8.field_0x8;
	//iVar3 = EventGenerator_12_004265d8.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_004265d8.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x540;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_004265e8.field_0x8;
	//iVar3 = EventGenerator_12_004265e8.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_004265e8.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x544;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_004265f8.field_0x8;
	//iVar3 = EventGenerator_12_004265f8.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_004265f8.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x550;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_00426608.field_0x8;
	//iVar3 = EventGenerator_12_00426608.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_00426608.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x558;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_00426618.field_0x8;
	//iVar3 = EventGenerator_12_00426618.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_00426618.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x564;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_00426628.field_0x8;
	//iVar3 = EventGenerator_12_00426628.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_00426628.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x570;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_00426638.field_0x8;
	//iVar3 = EventGenerator_12_00426638.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_00426638.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x574;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_00426648.field_0x8;
	//iVar3 = EventGenerator_12_00426648.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_00426648.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x580;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//puVar1 = EventGenerator_12_00426658.field_0x8;
	//iVar3 = EventGenerator_12_00426658.field_0x4;
	//ppAVar4 = &this->pVTable + this->count_0x2f0 * 6;
	//ppAVar4[0x5d] = EventGenerator_12_00426658.field_0x0;
	//ppAVar4[0x5e] = (ActorVTable*)iVar3;
	//ppAVar4[0x5f] = (ActorVTable*)puVar1;
	//*(undefined2*)((int)this->field_0x174 + this->count_0x2f0 * 0x18 + 0xe) = 0;
	//*(undefined2*)(this->field_0x174 + this->count_0x2f0 * 6 + 3) = 0;
	//this->field_0x174[this->count_0x2f0 * 6 + 4] = &this->field_0x578;
	//this->count_0x2f0 = this->count_0x2f0 + 1;
	//pCVar6 = this;
	//if (0 < this->count_0x2f0) {
	//	do {
	//		FUN_00391330((long)&pCVar6->field_0x170, 0);
	//		iVar5 = iVar5 + 1;
	//		pCVar6 = (CActorEventGenerator*)&(pCVar6->field_0x4).pCinData;
	//	} while (iVar5 < this->count_0x2f0);
	//}
	return;
}

CBehaviour* CActorEventGenerator::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &behaviourEventGen;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}