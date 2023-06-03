#ifndef _EDCFILER_H
#define _EDCFILER_H

#include "Types.h"
#include "edMem.h"

enum EBankAction
{
	BANK_ACTION_3 = 3,
	BANK_ACTION_5 = 5,
	CLOSE = 4,
	LOAD = 0,
	OPEN = 6,
	READ_STREAM = 2,
	SEEK = 1
};

struct edSysHandlerFile {
	edSysHandlerFile(struct edSysHandlersNodeTable* inNodeParent, int inMaxEventID, int inMainIdentifier)
		: nodeParent(inNodeParent)
		, maxEventID(inMaxEventID)
		, mainIdentifier(inMainIdentifier)
	{

	}

	struct edSysHandlersNodeTable* nodeParent;
	struct edSysHandlersPoolEntry* entries[16];
	int maxEventID;
	int mainIdentifier;
};

//struct edCFilerVTable {
//	void* field_0x0;
//	void* field_0x4;
//	void* free;
//	void* field_0xc;
//	void* initTableOfContents;
//	void* init;
//	void* field_0x18;
//	struct edCFiler_28* (*getGlobalC_0x1c)(struct edCFiler*);
//	void* field_0x20;
//	void* field_0x24;
//	void* field_0x28;
//	void* loadToInternal;
//	void* cleanupFunc;
//	bool (*open)(struct edCFiler*, struct edFILEH*, char*);
//	void* field_0x38;
//	void* field_0x3c;
//	void* field_0x40;
//	void* field_0x44;
//	bool (*seek)(struct edCFiler*, struct edFILEH*);
//	void* field_0x4c;
//	void* field_0x50;
//	void* field_0x54;
//	void* readCallback;
//	void* field_0x5c;
//	void* field_0x60;
//	void* field_0x64;
//	void* field_0x68;
//	void* field_0x6c;
//	void* field_0x70;
//	void* field_0x74;
//};

struct edCFiler_28_Internal {
	EBankAction nextAction;
	int mode;
	struct edFILEH* pDataBank;
	int seekOffset;
	char* pReadBuffer;
	int seekOffset2;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
};

struct edCFiler_28 {
	int freeIndexes;
	int currentIndex;
	EBankAction nextAction;
	struct edCFiler_28_Internal internalBank;
};

struct EdFileBaseData {
	char* pDriveName_0x0;
	int field_0x4;
	struct edCFiler* pPrevEd;
	struct edCFiler* pNextEd;
};

enum ETableOfContentsInitMode {
	IM_CALC_SIZE = 2,
	IM_INIT = 1
};

struct edFILE_PARAMETER {
	void* field_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int* field_0x8;
};


struct edCdlFolder {
	char name[12];
	undefined4 field_0xc;
	uint lsn;
	uint size;
	edCdlFolder* pChild;
	edCdlFolder* pParent;
	uint folderCount;
	uint fileCount;
	uint flag;
};

class edCFiler {
public:
	edCFiler();

	virtual bool configure(char* path, ETableOfContentsInitMode mode, edFILE_PARAMETER* param_4) { return false; }
	virtual bool initialize() { return true; };
	virtual edCFiler_28* GetGlobalC_0x1c();
	virtual void set_default_unit(char* szDriveLetter);
	virtual int get_default_unit(char* outString);
	virtual bool get_physical_filename(char* outFilePath, char* pathBuff) { return false; }
	virtual bool mount_unit(char* filePath, char* bankPath) { return false; }
	virtual bool open(edFILEH* outFile, char* unformatedFilePath) { return true; }
	virtual bool close(edFILEH* pDebugBank) { return false; }
	virtual uint read(edFILEH* pDebugBank, char* destination, uint requiredSize) { return 0; }
	virtual bool seek(edFILEH* pDebugBank) { return false; }
	virtual bool isnowaitcmdend(edCFiler_28_Internal* pEdFilerInternal) { return false; }
	virtual uint getalignedsize(uint inSize) { return inSize; }

	//struct edCFilerVTable* pVTable;
	struct EdFileBaseData baseData;
	char filePath[16];
};

void edFileNoWaitStackCallBack(edCFiler_28* param_1);

void edFilePathSplit(char* param_1, char* param_2, char* param_3, char* param_4, char* param_5);
char* edStrCat(char* buffer, char* suffix);

edCFiler* edFileGetFiler(char* outString, char* filePath, long mode);
void edFileSetPath(char* mode);
bool edFileInit(void);
edCFiler* GetFirstEdFileHandler_00260e00(edCFiler** param_1);
void edFileGetFiler(edCFiler* pFiler);
bool edFileFilerConfigure(char* path, ETableOfContentsInitMode mode, void* param_3, int* param_4);
bool get_physical_filename(char* filePathOut, char* filePathIn);
void Link_00260ec0(edCFiler** param_1, edCFiler* param_2);
void* GetInternalData_0025b2e0(edFILEH* pDebugBankData);

extern edSysHandlerFile g_edSysHandlerFile_00469b84;

struct edFileLoadConfig {
	EHeap heap;
	ushort align;
	ushort offset;
};

extern edFileLoadConfig edFileLoadInfo;

#endif //_EDCFILER_H
