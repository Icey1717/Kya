#ifndef _ED_BANK_FILE_H
#define _ED_BANK_FILE_H

#include "Types.h"
#include "edMem.h"

#define IO_LOG_DISABLED(...)
#define IO_LOG(level, format, ...) MY_LOG_CATEGORY("io", level, format, ##__VA_ARGS__)

struct edFILEH;

#define BANK_CALLBACK_MESH 0x4
#define BANK_CALLBACK_TEXTURE 0x5

PACK(struct FileTypeData {
	ushort stype;
	ushort type;
});

struct edCBankCallback {
	uint type;
	uint stype;
	bool (*pFunction[6])(char*, int);
};

struct FileHeaderFileData {
	int offset;
	int size;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	int crc;
};

static_assert(sizeof(FileHeaderFileData) == 0x10, "FileHeaderFileData is the wrong size");

class edCBankFileHeader {
public:
	void apply_callback(edCBankCallback* pTypePairData, int mode);
	void unpack();
	void analyse();
	uint get_index(int inIndex, int mode);
	char* get_entry_filename(int inIndex);
	char* get_entry_data(int fileIndex);
	int get_entryindex_from_filename(const char* inFileName);
	FileHeaderFileData* get_entry(int fileIndex);
	FileTypeData* get_entry_typepair(int inFileIndex);
	bool is_heterogeneous();

	char header[4];
	uint flags_0x4;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	FileTypeData typeData;
	int sizePacked;
	int sizeUnpacked;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	int field_0x1c;
	uint fileCount;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	int fileHeaderDataOffset;
	int fileTypeDataOffset;
	int field_0x30;
	uint field_0x34;
	int field_0x38;
};

struct edCFiler_Bnk_static_header {
	char header[8];
	edCBankFileHeader fileHeader;
};

static_assert(sizeof(edCBankFileHeader) == 0x3c, "edCBankFileHeader is the wrong size");

char* edBankFilerReadHeader(EHeap heapID, char* filePath, uint flags, edFILEH** ppFile);

void edDataLZ77Unpack(byte* param_1, byte* param_2, int param_3);

char* TreeInfo_RecurseWhileCountingIndexesUsingReference(char* headerBaseFilePath, int* outIndex, char* fileName, char* searchBuffer);
char* TreeInfo_Recurse(char* input);
void TreeInfo_UnOptimizeFilePath(char* param_1, char* param_2);
int TreeInfo_OptimizeFilePath(char* outFileName, const char* inFileName);

// Debug
char* DebugFindFilePath(edCBankFileHeader* pBVar3, int inFileIndex);

#endif // _ED_BANK_FILE_H