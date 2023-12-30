#include "ActorBlazer.h"
#include "MemoryStream.h"

void CActorBlazer::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	uint uVar3;
	int iVar4;
	float fVar5;

	CActor::Create(pByteCode);
	this->vision.Create(this, pByteCode);
	uVar3 = pByteCode->GetU32();
	//this->field_0x1d0 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1d4 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1d8 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1dc = uVar3;
	this->pathFollowReader.Create(pByteCode);
	fVar5 = pByteCode->GetF32();
	//this->field_0x1e0 = fVar5;

	pByteCode->GetF32();
	//FUN_00115040(&this->field_0x1e4, pByteCode);

	fVar5 = pByteCode->GetF32();
	//this->field_0x1ec = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x1f0 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x1f4 = fVar5;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1f8 = uVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//this->field_0x1fc = piVar1;
	iVar4 = pByteCode->GetS32();
	//this->field_0x200 = iVar4;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x204 = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x208 = pcVar2;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x20c = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x210 = pcVar2;
	iVar4 = pByteCode->GetS32();
	//this->field_0x214 = iVar4;
	addOnGenerator.Create(this, pByteCode);
	return;
}

void CVision::Create(CActor* pActor, ByteCode* pByteCode)
{
	uint uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	fVar2 = pByteCode->GetF32();
	fVar3 = pByteCode->GetF32();
	fVar4 = pByteCode->GetF32();
	fVar5 = pByteCode->GetF32();
	uVar1 = pByteCode->GetU32();
	//pPerception->visionRange_0x34 = fVar2;
	//pPerception->field_0x38 = fVar4 * 0.5;
	//pPerception->field_0x3c = fVar5 * 0.5;
	//fVar2 = fVar3 * 0.5 * 0.01745329;
	//pPerception->flags_0x0 = 0;
	//pPerception->field_0x30 = fVar2;
	//pPerception->field_0x44 = g_FloatSineCurve_00472260[(int)(ABS(fVar2 * 1303.797) + 0.5) & 0x1fff];
	//pPerception->field_0x3c = pPerception->field_0x3c;
	//fVar2 = 1.570796 - pPerception->field_0x30;
	//pPerception->field_0x40 =
	//	pPerception->field_0x3c *
	//	(g_FloatSineCurve_00472260[(int)(ABS((fVar2 - 1.570796) * 1303.797) + 0.5) & 0x1fff] /
	//		g_FloatSineCurve_00472260[(int)(ABS(fVar2 * 1303.797) + 0.5) & 0x1fff]);
	//pPerception->pActor_0x48 = (CActor*)0x0;
	//pPerception->field_0x4c = 0;
	//pPerception->field_0x54 = 0;
	//pPerception->field_0x50 = 0;
	//pPerception->field_0x58 = 0.0;
	//pPerception->flags_0x0 = uVar1;
	//pPerception->pActor = pActor;
	return;
}

void CPathFollowReader::Create(ByteCode* pByteCode)
{
	int iVar1;

	iVar1 = pByteCode->GetS32();
	this->field_0x0 = iVar1;
	return;
}
