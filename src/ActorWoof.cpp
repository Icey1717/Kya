#include "ActorWoof.h"
#include "MemoryStream.h"

void CActorWoof::Create(ByteCode* pByteCode)
{
	int* piVar1;
	uint uVar2;
	int iVar3;
	float fVar4;

	CActorAutonomous::Create(pByteCode);
	uVar2 = pByteCode->GetU32();
	//this->field_0x420 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->field_0x424 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->field_0x428 = uVar2;
	fVar4 = pByteCode->GetF32();
	//this->field_0x390 = fVar4;
	fVar4 = pByteCode->GetF32();
	//this->field_0x394 = fVar4;
	uVar2 = pByteCode->GetU32();
	//this->field_0x398 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->field_0x39c = uVar2;
	this->pathFollowReader.Create(pByteCode);
	iVar3 = pByteCode->GetS32();
	//this->field_0x41c = iVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//this->field_0x3a0 = piVar1;
	fVar4 = pByteCode->GetF32();
	//this->field_0x418 = fVar4;
	uVar2 = pByteCode->GetU32();
	//this->field_0x414 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->field_0x410 = uVar2;
	iVar3 = pByteCode->GetS32();
	//this->field_0x3a4 = iVar3;
	this->vision.Create(this, pByteCode);
	return;
}

CBehaviour* CActorWoof::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 4) {
		pBehaviour = &this->behaviourWoofVerticalJump;
	}
	else {
		if (behaviourType == 3) {
			pBehaviour = &this->behaviourWoofGuard;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}
