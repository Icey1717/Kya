#ifndef _EDCFILER_H
#define _EDCFILER_H

#include "Types.h"

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
//	bool (*open)(struct edCFiler*, struct DebugBankData_234*, char*);
//	void* field_0x38;
//	void* field_0x3c;
//	void* field_0x40;
//	void* field_0x44;
//	bool (*seek)(struct edCFiler*, struct DebugBankData_234*);
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
	struct DebugBankData_234* pDataBank;
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

struct InitTableOfContentsParams {
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

	virtual bool InitTableOfContents(char* path, ETableOfContentsInitMode mode, InitTableOfContentsParams* param_4) { return false; }
	virtual bool Init() { return true; };
	virtual edCFiler_28* GetGlobalC_0x1c();
	virtual void SetDriveLetter(char* szDriveLetter);
	virtual int AppendDriveLetter(char* outString);
	virtual bool FormatStreamPath(char* outFilePath, char* pathBuff) { return false; }
	virtual bool LoadAndStoreInternal(char* filePath, char* bankPath) { return false; }
	virtual bool Open(DebugBankData_234* outFile, char* unformatedFilePath) { return true; }
	virtual bool Close(DebugBankData_234* pDebugBank) { return false; }
	virtual uint ReadStream(DebugBankData_234* pDebugBank, char* destination, uint requiredSize) { return 0; }
	virtual bool Seek(DebugBankData_234* pDebugBank) { return false; }
	virtual bool ReadCallback(edCFiler_28_Internal* pEdFilerInternal) { return false; }
	virtual uint Function74(uint inSize) { return inSize; }

	//struct edCFilerVTable* pVTable;
	struct EdFileBaseData baseData;
	char filePath[16];
};

void ReadsBankFile(edCFiler_28* param_1);

void FormatFilePath_002618e0(char* param_1, char* param_2, char* param_3, char* param_4, char* param_5);

edCFiler* FindEdCFiler(char* outString, char* filePath, long mode);
void SetFilePathMode_00261810(char* mode);
bool InitFileHandlers_0025c300(void);
edCFiler* GetFirstEdFileHandler_00260e00(edCFiler** param_1);
void Func_0025b0c0(edCFiler* pFiler);
bool edCFilerInitTOC_00260f80(char* path, ETableOfContentsInitMode mode, void* param_3, int* param_4);
bool FormatStreamPath(char* filePathOut, char* filePathIn);
void Link_00260ec0(edCFiler** param_1, edCFiler* param_2);
void* GetInternalData_0025b2e0(DebugBankData_234* pDebugBankData);

#endif //_EDCFILER_H
