#include "EdFileBase.h"
#include "edFile/edFileFiler.h"
#include "edFile/edFileNoWaitStack.h"

StaticEdFileBase StaticEdFileBase_004497f0 = { 0 };
char gFilePath[8];

void StaticEdFileBase::Remove()
{
	this->nbActiveUsers = this->nbActiveUsers + -1;

	// HACK
	if (this->nbActiveUsers < 0) {
		this->nbActiveUsers = 0;
	}

	return;
}

void StaticEdFileBase::Add()
{
	this->nbActiveUsers = this->nbActiveUsers + 1;
	return;
}

bool StaticEdFileBase::IsAvailable()
{
	bool bVar1;
	edCFiler* peVar2;
	edCFileNoWaitStack* peVar3;
	char acStack512[512];

	bVar1 = this->nbActiveUsers == 0;

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
	return this->nbActiveUsers == 0;
}
