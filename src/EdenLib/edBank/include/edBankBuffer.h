#ifndef _ED_BANK_BUFFER_H
#define _ED_BANK_BUFFER_H

#include "Types.h"
#include "edMem.h"

class edCBankInstall;
class edCBankBuffer;
class edCBankFileHeader;

struct edCFiler_Bnk_static_header;
struct edFILEH;
struct TypePairData;

struct edBANK_ENTRY_INFO {
	uint type;
	uint stype;
	uint size;
	uint crc;
	char* fileBufferStart;
};

class edCBankBufferEntry {
public:
	void wait();
	bool is_loaded();
	bool load(edCBankInstall* pLoadData);
	bool install(edCBankInstall* pLoadData);
	bool close();
	int get_index(const char* inFileName);
	bool get_info(int inFileIndex, edBANK_ENTRY_INFO* outFileData, char* outIopPath);
	int get_element_count();
	char* get_element(int fileIndex);

	edCBankBuffer* header;
	int* field_0x4;
	edCBankFileHeader* fileBuffer;
	void* pObjectReference;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined4 field_0x14;
	edFILEH* pLoadedData;
	int headerComponent;
	int accessFlag;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	int field_0x28;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
};

class edCBankBuffer 
{
public:
	edCFiler_Bnk_static_header* get_entry(int index);
	void terminate();
	bool file_access(edCBankBufferEntry* pBankFileAccessObject, edCBankInstall* filePathPtr, long param_4, bool someFlag);
	edCBankBufferEntry* get_free_entry();
	void initialize(int size, int param_3, edCBankInstall* bankPathContainer);
	void bank_buffer_setcb(TypePairData* pTypePairData);

	int firstField;
	struct edCBankBufferEntry* pBankBuffer;
	int size;
	int createFlagA;
	int flagB;
	int flagC;
	TypePairData* pBankTypePairData;
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
	struct edCBankBuffer* pNextBank;
	struct edCBankBuffer* pPrevBank;
	struct edCBankBufferEntry* pBankFileAccessObject;
};

#endif //_ED_BANK_BUFFER_H