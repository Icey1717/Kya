#ifndef _EDCBANK_H
#define _EDCBANK_H

#include "Types.h"
#include "edMem.h"

#include "edCBankBuffer.h"

enum ESeekMode {
	ED_SEEK_SET,
	ED_SEEK_CUR,
	ED_SEEK_END
};

typedef struct edBANK_ENTRY_INFO edBANK_ENTRY_INFO, * PBankFileData;

struct edBANK_ENTRY_INFO {
	uint type;
	uint stype;
	uint size;
	uint crc;
	char* fileBufferStart;
};

typedef struct BankFilePathContainer BankFilePathContainer, * PBankFilePathContainer;

struct BankFilePathContainer {
	char* filePath;
	int fileFlagA;
	void* pObjectReference;
	LoadBankFileFunc fileFunc;
	EHeap heapID;
	int fileFlagE;
};

typedef struct edCBank edCBank, * PedCBank;

struct edCBank {
	int firstField;
	struct edCBankBufferEntry* pBankBuffer;
	int size;
	int createFlagA;
	int flagB;
	int flagC;
	struct TypePairData* pBankTypePairData;
	int fileFlagA;
	EHeap heapID;
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
	struct edCBankBufferEntry* pBankFileAccessObject;
};

struct edBank_Manager {
	byte field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	int bankCount;
	struct edCBank* pNextBank;
	undefined4 field_0xc;
	int selectedFileIndex;
};

void initialize(edCBank* pBankObj, int size, int param_3, BankFilePathContainer* bankPathContainer);
void edCBank_SetDeserializeData(edCBank* pBank, TypePairData* pTypePairData);
edCBankBufferEntry* get_free_entry(edCBank* bankObj);
edFILEH* edFileOpen(char* filePath, uint flags);
uint GetFileSize_0025bd70(edFILEH* pDebugBank);
bool SetRead_0025be80(edFILEH* pDebugBank, char* param_2, uint size);
bool SetClose_0025bf60(edFILEH* pDebugBank);
byte edFileRead(edFILEH* pDebugBank, char* pReadBuffer, uint someSize);
void edCBank_Free_00244e10(edCBank* pBank);

bool SetBankReadStream(class edCFiler_28* param_1, edFILEH* pDebugBank, char* pReadBuffer, uint someSize);

void edBankInit(void);

char* ReadFileToBuffer(EHeap heapID, char* filePath, uint flags, edFILEH** outLoadedData);
bool edFileSeek(edFILEH* pDebugBank, uint seekOffset, ESeekMode mode);
bool edFileClose(edFILEH* pDebugBank);
bool SetBankClose(edCFiler_28* param_1, edFILEH* pDataBank);

extern int g_FileSystemSysHandlerID_00469bc4;
extern int g_FileSystemHandlers_00469bc8;
extern AsyncBankReader g_AsyncBankReader_00466e60;

extern edCFiler* g_edCFiler_MCPtr_00448fd8;

extern byte g_DebugBankLoadFlag_00469be0[16];
extern edFILEH g_DebugBankDataArray_00469bf0[16];

#endif //_EDCBANK_H
