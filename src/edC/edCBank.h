#ifndef _EDCBANK_H
#define _EDCBANK_H

#include "Types.h"

#include "edCBankBuffer.h"

typedef struct BankFileData BankFileData, * PBankFileData;

struct BankFileData {
	uint unknownA;
	uint unknownB;
	uint length;
	uint unknownC;
	char* fileBufferStart;
};

typedef struct BankFilePathContainer BankFilePathContainer, * PBankFilePathContainer;

struct BankFilePathContainer {
	char* filePath;
	int fileFlagA;
	int pObjectReference;
	int fileFunc;
	int heapID;
	int fileFlagE;
};

typedef struct edCBank edCBank, * PedCBank;

struct edCBank {
	int firstField;
	struct edCBankBuffer* pBankBuffer;
	int size;
	int createFlagA;
	int flagB;
	int flagC;
	struct TypePairData* pBankTypePairData;
	int fileFlagA;
	int heapID;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	struct edCBank* pNextBank;
	struct edCBank* pPrevBank;
	struct edCBankBuffer* pBankFileAccessObject;
};

struct edBank_Manager {
	byte field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	int bankCount;
	struct edCBank* pNextBank;
	undefined4 field_0xc;
	undefined4 field_0x10;
};

void edCBank_Setup(edCBank* pBankObj, int size, int param_3, BankFilePathContainer* bankPathContainer);
void edCBank_SetDeserializeData(edCBank* pBank, TypePairData* pTypePairData);
edCBankBuffer* edCBank_GetBankBuffer(edCBank* bankObj);
DebugBankData_234* LoadFile(char* filePath, uint flags);
byte UsedInFileLoad(DebugBankData_234* pDebugBank, char* pReadBuffer, uint someSize);
void edCBank_Free_00244e10(edCBank* pBank);

void Init_edBank(void);

extern int g_FileSystemSysHandlerID_00469bc4;
extern int g_FileSystemHandlers_00469bc8;
extern ReadBank_158 g_ReadBank_00466e60;

extern edCFiler* g_edCFiler_MCPtr_00448fd8;

#endif //_EDCBANK_H
