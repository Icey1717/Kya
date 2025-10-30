#ifndef _ED_BANK_STACK_FILE_H
#define _ED_BANK_STACK_FILE_H

#include "Types.h"
#include "edMem.h"
#include "edBank/edBankBuffer.h"


class edCBankStackElement {
public:
	edCBankBufferEntry* pBankHeader_0x0;
	char* pReadBuffer;
	edFILEH* pDebugBankData;
	edCBankCallback* pBankTypePairData_0xc;
	void* pObjectReference_0x10;
	LoadBankFileFunc fileFunc_0x14;
	int fileFlagB_0x18;
};

class edCBankStack {
public:
	edCBankStack();
	bool add_file(edCBankStackElement* pInBank);
	void initialize();
	void terminate();
	int loadedBanks;
	int currentIndex;
	edCBankStackElement aBankQueue[12];
};

extern edCBankStack edBankStack;

#endif // _ED_BANK_STACK_FILE_H
