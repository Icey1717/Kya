#include "edBank.h"
#include "edBankFiler.h"
#include "edBankStackFile.h"

edCBankConfig edBankConfig = { 0, 0, 0 };

byte _edBankAllowNoWait;

void edBankTerm()
{
	edBankFilerRemove();
	edBankStack.terminate();
	//edBankDebug.terminate();
	return;
}

void edBankInit(void)
{
	//edBankDebug.initialize();
	edBankStack.initialize();
	edBankFilerInstall();
	return;
}