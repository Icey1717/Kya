#include "ActorNativCmd.h"
#include "MemoryStream.h"

void CActorNativCmd::Create(ByteCode* pByteCode)
{
	uint count;
	int iVar1;
	int* piVar2;
	void* pvVar3;
	int iVar4;
	int iVar5;

	CActor::Create(pByteCode);
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	//this->field_0x160 = piVar2;
	iVar1 = pByteCode->GetS32();
	IMPLEMENTATION_GUARD_LOG(
		this->field_0x164 = iVar1;
	if (this->field_0x160 == (int*)0x0) {
		iVar1 = 0;
	}
	else {
		iVar1 = *this->field_0x160;
	}
	this->field_0x1cc = (int)((float)iVar1 / 2.0);
	count = this->field_0x1cc;
	if (count != 0) {
		piVar2 = (int*)operator.new.array((long)(int)(count * 0x20 + 0x10));
		piVar2 = __construct_new_array(piVar2, CTalkParam::CTalkParam, Particle::Free_00392fa0, 0x20, count);
		this->field_0x1c8 = piVar2;
	}
	iVar1 = 0;
	if (0 < (int)this->field_0x1cc) {
		iVar4 = 0;
		do {
			iVar5 = (int)this->field_0x1c8 + iVar4;
			pvVar3 = operator.new.array(0x10);
			*(void**)(iVar5 + 8) = pvVar3;
			iVar1 = iVar1 + 1;
			*(undefined4*)(iVar5 + 0xc) = 2;
			iVar4 = iVar4 + 0x20;
		} while (iVar1 < (int)this->field_0x1cc);
	});

	this->addOnA.Create(pByteCode);
	this->addOnB.Create(pByteCode);
	this->addOnC.Create(pByteCode);
	this->addOnD.Create(pByteCode);
	return;
}

void CBehaviourAddOnNativ::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int* piVar2;
	void* pvVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	uint* puVar8;
	int iVar9;
	float fVar10;

	uVar1 = pByteCode->GetS32();
	this->field_0x10 = uVar1;
	uVar1 = this->field_0x10;
	if (uVar1 != 0) {
		//piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0x18 + 0x10));
		//piVar2 = __construct_new_array(piVar2, (ActorConstructorA*)&LAB_003e1f60, FUN_003e3a70, 0x18, uVar1);
		//this->field_0x14 = piVar2;
		iVar9 = 0;
		if (0 < this->field_0x10) {
			iVar6 = 0;
			do {
				//puVar8 = (uint*)((int)this->field_0x14 + iVar6);
				uVar1 = pByteCode->GetU32();
				//*puVar8 = uVar1;
				uVar1 = pByteCode->GetS32();
				//puVar8[2] = uVar1;
				//if (0 < (int)puVar8[2]) {
				//	pvVar3 = operator.new.array((long)(int)(puVar8[2] << 2));
				//	puVar8[1] = (uint)pvVar3;
				//}
				iVar7 = 0;
				if (0 < uVar1) {
					iVar5 = 0;
					do {
						iVar4 = pByteCode->GetS32();
						iVar7 = iVar7 + 1;
						//*(int*)(puVar8[1] + iVar5) = iVar4;
						iVar5 = iVar5 + 4;
					} while (iVar7 < uVar1);
				}
				//puVar8[4] = 0xffffffff;
				iVar9 = iVar9 + 1;
				iVar6 = iVar6 + 0x18;
			} while (iVar9 < this->field_0x10);
		}
	}
}
