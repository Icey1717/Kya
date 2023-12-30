#include "ActorAton.h"
#include "MemoryStream.h"

void CActorAton::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);
	uVar1 = pByteCode->GetU32();
	//this->field_0x354 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x358 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x35c = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x360 = uVar1;
	iVar2 = pByteCode->GetS32();
	//this->field_0x63c = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0x644 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0x648 = iVar2;
	//(*(code*)this->field_0x640->field_0x10)(&this->field_0x640);
	iVar2 = pByteCode->GetS32();
	//this->field_0x39c = iVar2;
	//if (this->field_0x39c != -1) {
	//	CActor::SV_InstallMaterialId(this->field_0x39c);
	//	CFxTail::Create(1.0, &this->fxTailA, 0x10, 4, this->field_0x39c);
	//	CFxTail::Create(1.0, &this->fxTailB, 0x10, 4, this->field_0x39c);
	//}
	fVar3 = pByteCode->GetF32();
	//this->field_0x350 = fVar3;
	this->pathPlaneArray.Create(pByteCode);
	behaviourAddOn.Create(pByteCode);
	return;
}

void CPathPlaneArray::Create(ByteCode* pByteCode)
{
	int iVar1;
	uint uVar2;
	int* piVar3;
	int iVar4;
	int iVar5;

	uVar2 = pByteCode->GetU32();
	this->field_0x0 = uVar2;
	uVar2 = this->field_0x0;
	if (uVar2 != 0) {
		//piVar3 = (int*)operator.new.array((long)(int)(uVar2 * 0x20 + 0x10));
		//piVar3 = __construct_new_array(piVar3, CPathPlane::CPathPlane, FUN_001c0d90, 0x20, uVar2);
		//this->field_0x8 = piVar3;
	}
	if ((this->field_0x0 != 0) && (uVar2 = 0, this->field_0x0 != 0)) {
		iVar5 = 0;
		do {
			//iVar1 = this->field_0x8;
			iVar4 = pByteCode->GetS32();
			//*(int*)(iVar1 + iVar5) = iVar4;
			uVar2 = uVar2 + 1;
			iVar5 = iVar5 + 0x20;
		} while (uVar2 < (uint)this->field_0x0);
	}
	return;
}

void CBehaviourAddOnAton::Create(ByteCode* pByteCode)
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

	uVar1 = pByteCode->GetU32();
	//this->field_0x18 = uVar1;
	uVar1 = pByteCode->GetS32();
	this->field_0x1c = uVar1;
	uVar1 = this->field_0x1c;
	if (uVar1 != 0) {
		//piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0x18 + 0x10));
		//piVar2 = __construct_new_array(piVar2, (ActorConstructorA*)&LAB_003e1f60, FUN_003e3a70, 0x18, uVar1);
		//this->field_0x20 = piVar2;
		iVar9 = 0;
		if (0 < this->field_0x1c) {
			iVar6 = 0;
			do {
				//puVar8 = (uint*)((int)this->field_0x20 + iVar6);
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
			} while (iVar9 < this->field_0x1c);
		}
	}
	uVar1 = pByteCode->GetS32();
	this->field_0x10 = uVar1;
	uVar1 = this->field_0x10;
	if (uVar1 != 0) {
		//piVar2 = (int*)operator.new.array((long)(int)(uVar1 * 0x1c + 0x10));
		//piVar2 = __construct_new_array(piVar2, (ActorConstructorA*)&LAB_003e3450, FUN_0010df00, 0x1c, uVar1);
		//this->field_0x14 = piVar2;
		iVar9 = 0;
		if (0 < (int)this->field_0x10) {
			iVar6 = 0;
			do {
				//puVar8 = (uint*)((int)this->field_0x14 + iVar6);
				uVar1 = pByteCode->GetU32();
				//puVar8[6] = uVar1;
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
				iVar6 = iVar6 + 0x1c;
			} while (iVar9 < this->field_0x10);
		}
	}
	fVar10 = pByteCode->GetF32();
	//this->field_0x24 = fVar10;
	//this->field_0x24 = 10.0;
	return;
}
