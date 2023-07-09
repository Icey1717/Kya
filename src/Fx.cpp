#include "Fx.h"
#include "MemoryStream.h"
#include <string.h>

CFxManager::CFxManager()
{
	CFxManager* local_a0_lo_24;

	this->count_0x4 = 0;
	this->field_0x8 = (undefined*)0x0;
	this->orderedCountArray = (undefined*)0x0;
	this->totalEffectCount_0x2c = 0;
	this->field_0x68 = (undefined*)0x0;
	this->field_0x6c = 2;
	memset(this->aEffectCategory, 0, sizeof(this->aEffectCategory));
	memset(this->effectCountByType, 0, sizeof(this->effectCountByType));
	this->field_0x30 = 0xd0;
	this->field_0x34 = 0x70;
	this->field_0x38 = 0x60;
	this->field_0x3c = 0x90;
	this->field_0x40 = 0xe0;
	this->field_0x44 = 0;
	this->field_0x48 = 0;
	return;
}

void CFxManager::AddPool(ByteCode* pMemoryStream)
{
	EffectsManagerObjBase* pEVar1;
	//EffectsManagerObjAdvancedVTable* pEVar2;
	uint uVar3;
	uint uVar4;

	//pEVar1 = (EffectsManagerObjBase*)operator.new(0x2c);
	//if ((long)(int)pEVar1 != 0) {
	//	FUN_001a48c0((long)(int)pEVar1);
	//	pEVar1->pVTable = &EffectsManagerObjAdvancedVTable_004428f0;
	//}
	//this->field_0x4c[0] = pEVar1;
	//pEVar1 = (EffectsManagerObjBase*)operator.new(0x34);
	//if ((long)(int)pEVar1 != 0) {
	//	FUN_001a4070((long)(int)pEVar1);
	//	pEVar1->pVTable = &EffectsManagerObjAdvancedVTable_00441f20;
	//	pEVar2 = (EffectsManagerObjAdvancedVTable*)edMemAlloc(H_MAIN, 0x3e800);
	//	(&pEVar1->pVTable)[1] = pEVar2;
	//}
	//this->field_0x4c[3] = pEVar1;
	//pEVar1 = (EffectsManagerObjBase*)operator.new(0x34);
	//if ((long)(int)pEVar1 != 0) {
	//	FUN_001a5140((long)(int)pEVar1);
	//	pEVar1->pVTable = &EffectsManagerObjAdvancedVTable_00442990;
	//}
	//this->field_0x4c[4] = pEVar1;
	//pEVar1 = (EffectsManagerObjBase*)operator.new(0x2c);
	//if ((long)(int)pEVar1 != 0) {
	//	FUN_001a6160((long)(int)pEVar1);
	//	pEVar1->pVTable = &EffectsManagerObjAdvancedVTable_00443c40;
	//}
	//this->field_0x4c[5] = pEVar1;
	//pEVar1 = (EffectsManagerObjBase*)operator.new(0x2c);
	//if ((long)(int)pEVar1 != 0) {
	//	FUN_001a5950((long)(int)pEVar1);
	//	pEVar1->pVTable = &EffectsManagerObjAdvancedVTable_00443ba0;
	//}
	//this->field_0x4c[6] = pEVar1;
	uVar4 = 0;
	do {
		uVar3 = pMemoryStream->GetU32();
		pEVar1 = this->aEffectCategory[uVar4];
		if (pEVar1 != (EffectsManagerObjBase*)0x0) {
			IMPLEMENTATION_GUARD(
			(*(code*)(pEVar1->pVTable->field_0x0).field_0x2c)(pEVar1, pMemoryStream, uVar3);)
		}
		uVar4 = uVar4 + 1;
	} while (uVar4 < 7);
	return;
}