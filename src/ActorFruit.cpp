#include "ActorFruit.h"
#include "MemoryStream.h"

void CActorFruit::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	uint uVar3;

	CActorAutonomous::Create(pByteCode);
	uVar3 = pByteCode->GetU32();
	//this->field_0x360 = uVar3;
	if (2.2f <= CScene::_pinstance->field_0x1c) {
		piVar1 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar1 + 1);
		if (*piVar1 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
		}
		//this->field_0x350 = piVar1;
		pcVar2 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar2 + 0x20;
		//this->field_0x354 = pcVar2;
	}
	else {
		piVar1 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar1 + 1);
		if (*piVar1 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x10;
		}
		//this->field_0x358 = piVar1;
		pcVar2 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar2 + 0x20;
		//this->field_0x35c = pcVar2;
	}
	//this->field_0x374 = 0;
	//this->field_0x378 = 0;
	//this->field_0x37c = 0;
	return;
}
