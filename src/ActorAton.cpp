#include "ActorAton.h"
#include "MemoryStream.h"

CActorAton::CActorAton()
{
	
}

void CActorAton::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);

	// This could be inlined static mesh component create.
	this->field_0x354 = pByteCode->GetU32();
	this->field_0x358 = pByteCode->GetU32();
	this->field_0x35c = pByteCode->GetU32();
	this->field_0x360 = pByteCode->GetU32();
	this->field_0x63c = pByteCode->GetS32();
	this->field_0x644 = pByteCode->GetS32();
	this->field_0x648 = pByteCode->GetS32();

	staticMeshComponent.Reset();

	this->trailMaterialId = pByteCode->GetS32();
	if (this->trailMaterialId != -1) {
		SV_InstallMaterialId(this->trailMaterialId);
		this->fxTailA.Create(1.0f, 0x10, 4, this->trailMaterialId);
		this->fxTailB.Create(1.0f, 0x10, 4, this->trailMaterialId);
	}

	this->field_0x350 = pByteCode->GetF32();
	this->pathPlaneArray.Create(pByteCode);
	behaviourAddOn.Create(pByteCode);
	return;
}

CBehaviour* CActorAton::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = &this->behaviourAtonEscape;
	}
	else {
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}

CPathPlaneArray::CPathPlaneArray()
{
	this->field_0x0 = 0;
	this->field_0x8 = 0;
	this->field_0x4 = 0;
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

CBehaviourAddOnAton::CBehaviourAddOnAton()
{
	this->field_0x10 = 0;
	this->field_0x14 = (int*)0x0;
	this->field_0x1c = 0;
	this->field_0x20 = (int*)0x0;
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

void CFxTail::Create(float param_1, int count, int param_4, int materialId)
{
	IMPLEMENTATION_GUARD_LOG();
}

CStaticMeshComponent::CStaticMeshComponent()
{
	//this->field_0x634 = 0;
	//this->field_0x638 = 0;
	//this->staticMeshComponent = &StaticMeshComponent::VTable_0043a7a0;
	//this->field_0x690 = 0;
	//this->field_0x694 = 0;
	//this->field_0x648 = -1;
	//this->field_0x644 = -1;
	//this->staticMeshComponent = &StaticMeshComponent::VTable_0043a6f0;
	//this->staticMeshComponent = &StaticMeshComponent::VTable_0043a6d0;
}

void CStaticMeshComponent::Reset()
{
	IMPLEMENTATION_GUARD_LOG();
}
