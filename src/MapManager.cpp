#include "MapManager.h"
#include "MemoryStream.h"

void CMapManager::OnLoadLevelBnk_003f9a60(ByteCode* pMemoryStream)
{
	int* piVar1;
	int iVar2;
	uint uVar3;
	float fVar4;

	iVar2 = pMemoryStream->GetS32();
	this->field_0x4 = iVar2;
	uVar3 = pMemoryStream->GetU32();
	this->field_0x8 = uVar3;
	fVar4 = pMemoryStream->GetF32();
	this->field_0xc = (fVar4 * 3.141593f) / 180.0f;
	piVar1 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar1 * 4;
	}
	this->field_0x10 = piVar1;
	iVar2 = pMemoryStream->GetS32();
	this->field_0x14 = iVar2;
	iVar2 = pMemoryStream->GetS32();
	this->field_0x18 = iVar2;
	if (this->field_0x18 == -1) {
		this->field_0x394 = 1;
	}
	else {
		this->field_0x394 = 0;
	}
	return;
}