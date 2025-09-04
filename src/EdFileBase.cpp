#include "EdFileBase.h"
#include "EdenLib/edFile/include/edFileFiler.h"
#include "EdenLib/edFile/include/edFileNoWaitStack.h"

StaticEdFileBase StaticEdFileBase_004497f0 = { 0 };
char gFilePath[8];

void StaticEdFileBase::Remove()
{
	this->field_0x4 = this->field_0x4 + -1;
	return;
}

void StaticEdFileBase::Add()
{
	this->field_0x4 = this->field_0x4 + 1;
	return;
}

bool StaticEdFileBase::FUN_00401f30()
{
	bool bVar1;
	edCFiler* peVar2;
	edCFileNoWaitStack* peVar3;
	char acStack512[512];

	bVar1 = this->field_0x4 == 0;

	if (bVar1) {
		if (this->pEdFileBase == (edCFiler*)0x0) {
			peVar2 = edFileGetFiler(acStack512, gFilePath, 1);
			this->pEdFileBase = peVar2;
		}

		peVar2 = this->pEdFileBase;
		bVar1 = peVar2 != (edCFiler*)0x0;
		if (bVar1) {
			bVar1 = peVar2->getnowaitfilestack()->nbInUse != 0;
		}

		bVar1 = !bVar1;
	}

	return bVar1;
}

bool StaticEdFileBase::Check()
{
	return this->field_0x4 == 0;
}
