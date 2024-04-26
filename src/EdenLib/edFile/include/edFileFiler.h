#ifndef _ED_FILE_FILER_H
#define _ED_FILE_FILER_H

#include "Types.h"

struct edFILEH;

enum ETableOfContentsInitMode {
	IM_CALC_SIZE = 2,
	IM_INIT = 1
};

struct edCFiler_28_Internal {
	EBankAction nextAction;
	int mode;
	edFILEH* pDataBank;
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
	edCFiler_28_Internal internalBank;
};

struct edFILE_PARAMETER {
	void* field_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int* field_0x8;
};

class edCFiler {
public:
	edCFiler();
	~edCFiler();

	virtual bool configure(uint);
	virtual bool configure(char* path, ETableOfContentsInitMode mode, edFILE_PARAMETER* param_4);
	virtual bool initialize();
	virtual edCFiler_28* GetGlobalC_0x1c();
	virtual void set_default_unit(char* szDriveLetter);
	virtual int get_default_unit(char* outString);
	virtual bool get_physical_filename(char* outFilePath, char* pathBuff);
	virtual bool mount_unit(char* filePath, char* bankPath);
	virtual bool unmount_unit(char* filePath);
	virtual bool open(edFILEH* outFile, char* unformatedFilePath);
	virtual bool close(edFILEH* pDebugBank);
	virtual uint read(edFILEH* pDebugBank, char* destination, uint requiredSize);
	virtual bool seek(edFILEH* pDebugBank);
	virtual bool isnowaitcmdend(edCFiler_28_Internal* pEdFilerInternal);
	virtual uint getalignedsize(uint inSize);
	virtual bool terminate();
	virtual bool findfile();
	virtual bool create(/*missing args*/);
	virtual bool write(/*missing args*/);
	virtual bool erase(/*missing args*/);
	virtual bool waitcmdend(/*missing args*/);
	virtual bool cmdbreak(/*missing args*/);
	virtual bool mkdir(/*missing args*/);
	virtual bool setattr(/*missing args*/);
	virtual bool format(/*missing args*/);
	virtual bool getfreespace(/*missing args*/);

	char* pDriveName_0x0;
	int field_0x4;
	struct edCFiler* pPrevEd;
	struct edCFiler* pNextEd;
	char filePath[16];
};


class edCFilerList {
public:
	edCFiler* get_root();
	void remove_filer(edCFiler* pFiler);
	void add_filer(edCFiler* pFiler);
	edCFiler* pRoot;
};

extern edCFilerList edFilerList;

edCFiler* edFileGetFiler(char* outString, char* filePath, long mode);
bool edFileFilerConfigure(char* path, ETableOfContentsInitMode mode, void* param_3, int* param_4);
void edFileGetFiler(edCFiler* pFiler);


#endif //_ED_FILE_FILER_H