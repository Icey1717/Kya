#ifndef ED_FILE_NOWAITSTACK_H
#define ED_FILE_NOWAITSTACK_H

#include "Types.h"

class edCFiler;
struct edFILEH;

struct edFILE_STACK_ELEMENT
{
	EBankAction action;
	int bIsFiler;
	void* pData;
	int seekOffset;
	void* pReadBuffer;
	int nbReadBytes;
	void* pWriteDst;
	uint nbWriteBytes;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	char* szPath;
};

class edCFileNoWaitStack
{
public:
	edCFileNoWaitStack();

	edFILE_STACK_ELEMENT* AddFiler(edCFiler* pFiler, EBankAction action);
	edFILE_STACK_ELEMENT* AddFile(edFILEH* pFile, EBankAction action);

	bool AddFilerSync(edCFiler* pFiler);
	bool AddFileClose(edFILEH* pFile);
	bool AddFileWrite(edFILEH* pFile, void* pData, uint nbWriteBytes);
	bool AddFileRead(edFILEH* pFile, void* pData, uint nbReadBytes);
	bool AddFileSeek(edFILEH* pFile, uint seekOffset);
	bool AddFileOpen(edFILEH* pFile, char* szPath);

	int nbInUse;
	int currentIndex;
	EBankAction nextAction;
	edFILE_STACK_ELEMENT aElements[0x18];
};

void edFileNoWaitStackCallBack(edCFileNoWaitStack* pStack);
void edFileNoWaitStackFlush(void);
void edFileNoWaitStackFlush(edCFiler* pFiler);

extern edCFileNoWaitStack edCFiler_28_ARRAY_004697a0;

#endif // ED_FILE_NOWAITSTACK_H