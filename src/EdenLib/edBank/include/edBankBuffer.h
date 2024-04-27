#ifndef _ED_BANK_BUFFER_H
#define _ED_BANK_BUFFER_H

#include "Types.h"
#include "edMem.h"

class edCBankInstall;
class edCBankBuffer;
class edCBankFileHeader;

struct edCFiler_Bnk_static_header;
struct edFILEH;
struct edCBankCallback;

typedef void(*LoadBankFileFunc)(bool, void*);

#define FILE_DATA_TAG_SIZE 0x8

struct edCBankInstall {
	char* filePath;
	int fileFlagA;
	void* pObjectReference;
	LoadBankFileFunc fileFunc;
	EHeap heapID;
	int fileFlagE;
};

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
	bool load(edCBankInstall* pBankInstall);
	bool install(edCBankInstall* pBankInstall);
	bool close();
	int get_index(const char* inFileName);
	bool get_info(int inFileIndex, edBANK_ENTRY_INFO* pBankEntryInfoOut, char* outIopPath);
	int get_element_count();
	char* get_element(int fileIndex);

	edCBankBuffer* pBankBuffer;
	char* pFileBuffer;
	edCBankFileHeader* pFileHeader;
	void* pObjectReference;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined4 field_0x14;
	edFILEH* pFile;
	int headerComponent;
	int accessFlag;
};

class edCBankBuffer 
{
public:
	edCFiler_Bnk_static_header* get_entry(int index);
	void terminate();
	bool file_access(edCBankBufferEntry* pBankBufferEntry, edCBankInstall* pBankInstall, long param_4, bool someFlag);
	edCBankBufferEntry* get_free_entry();
	void initialize(int size, int nbBankBufferEntries, edCBankInstall* pBankInstall);
	void bank_buffer_setcb(edCBankCallback* pNewBankCallback);

	int firstField;
	edCBankBufferEntry* aBankBufferEntries;
	int size;
	int nbBankBufferEntries;
	int flagB;
	int nbBankBufferEntriesInUse;
	edCBankCallback* pBankCallback;
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
	edCBankBuffer* pNextBank;
	edCBankBuffer* pPrevBank;
	edCBankBufferEntry* pBankFileAccessObject;
};

#endif //_ED_BANK_BUFFER_H