#include "ActorCommander.h"
#include "MemoryStream.h"



void CActorCommander::Create(ByteCode* pByteCode)
{
	ulong uVar1;
	float fVar2;
	uint uVar3;
	float fVar4;
	CActor* pCVar5;
	int iVar6;
	int* piVar7;
	CTeamElt* pCVar8;
	int iVar9;
	ACommanderComponent_10* pAVar10;
	int iVar11;
	int iVar12;
	int iVar13;

	CActor::Create(pByteCode);
	uVar3 = pByteCode->GetU32();
	this->eventIndex_0x178 = uVar3;
	uVar3 = pByteCode->GetU32();
	this->eventID_0x17c = uVar3;
	fVar4 = (float)pByteCode->GetU32();
	//(this->field_0x180).x = fVar4;
	fVar4 = (float)pByteCode->GetS32();
	//(this->field_0x180).y = fVar4;
	pCVar5 = (CActor*)pByteCode->GetS32();
	//(this->checkpointData_0x190).pActor_0x0 = pCVar5;
	this->squad.Create(pByteCode);
	iVar6 = pByteCode->GetS32();
	this->count_0x2ec = iVar6;
	uVar3 = this->count_0x2ec;
	if ((int)uVar3 < 1) {
		this->aTeamElt = (CTeamElt*)0x0;
	}
	else {
		this->aTeamElt = new CTeamElt[uVar3];
	}
	iVar6 = 0;
	if (0 < this->count_0x2ec) {
		do {
			iVar9 = pByteCode->GetS32();
			this->aTeamElt[iVar6].enemyIndex = iVar9;
			iVar6 = iVar6 + 1;
		} while (iVar6 < this->count_0x2ec);
	}
	//(this->field_0x180).w = 0.0;
	//(this->field_0x180).z = -NAN;
	iVar6 = pByteCode->GetS32();
	this->count_0x16c = iVar6;
	if (this->count_0x16c == 0) {
		this->aComponent_0x170 = (ACommanderComponent_10*)0x0;
	}
	else {
		this->aComponent_0x170 = new ACommanderComponent_10[this->count_0x16c];
		iVar9 = 0;
		iVar12 = 0;
		iVar6 = 0;
		if (0 < this->count_0x16c) {
			iVar13 = 0;
			do {
				iVar11 = pByteCode->GetS32();
				//*(int*)(&this->field_0x170->field_0x0 + iVar13) = iVar11;
				iVar11 = pByteCode->GetS32();
				iVar6 = iVar6 + 1;
				//*(int*)(&this->field_0x170->field_0x8 + iVar13) = iVar11;
				//piVar7 = (int*)(&this->field_0x170->field_0x0 + iVar13);
				//iVar9 = iVar9 + *piVar7;
				//piVar7[1] = iVar9;
				//iVar12 = iVar12 + piVar7[2];
				//*(int*)(&this->field_0x170->field_0xc + iVar13) = iVar12;
				//iVar13 = iVar13 + 0x10;
			} while (iVar6 < this->count_0x16c);
		}
	}
	this->camFigData.Create(pByteCode);
	//fVar2 = edF32VECTOR4_0040e720.w;
	//fVar4 = edF32VECTOR4_0040e720.z;
	//uVar1 = (ulong)edF32VECTOR4_0040e720._0_8_ >> 0x20;
	//(this->camFigData).field_0x290.x = (float)edF32VECTOR4_0040e720._0_8_;
	//(this->camFigData).field_0x290.y = (float)uVar1;
	//(this->camFigData).field_0x290.z = fVar4;
	//(this->camFigData).field_0x290.w = fVar2;
	//CCamFigData::IsValid(&this->camFigData, 0);
	return;
}

void CSquad::Create(ByteCode* pByteCode)
{
	uint uVar1;
	uint uVar2;
	float fVar3;
	float fVar4;

	fVar3 = pByteCode->GetF32();
	uVar1 = pByteCode->GetU32();
	uVar2 = pByteCode->GetU32();
	//if ((int)uVar1 < 0) {
	//	fVar4 = (float)(uVar1 >> 1 | uVar1 & 1);
	//	fVar4 = fVar4 + fVar4;
	//}
	//else {
	//	fVar4 = (float)uVar1;
	//}
	//CChessBoard::Init(fVar3, fVar3 * fVar4, &this->chessboard, uVar1, uVar2);
	//(this->field_0x0).x = 0.0;
	//(this->field_0x0).y = 0.6;
	//(this->field_0x0).w = 0.0;
	//(this->field_0x0).z = 0.0;
	//memset(this->field_0x10, 0, 0xa0);
	//this->field_0xb0 = 1;
	//this->field_0xb4 = 0;
	//this->field_0xbc = 0;
	//this->field_0xb8 = 0;
	//memset(this->field_0xc0, 0, 0xa0);
	return;
}
