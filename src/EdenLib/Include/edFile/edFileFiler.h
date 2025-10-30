#ifndef _ED_FILE_FILER_H
#define _ED_FILE_FILER_H

#include "Types.h"

struct edFILEH;
class edCFileFind;
class edCFileNoWaitStack;
struct edFILE_STACK_ELEMENT;

enum ETableOfContentsInitMode
{
	IM_CALC_SIZE = 2,
	IM_INIT = 1
};

struct edFILE_PARAMETER
{
	void* field_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	int* field_0x8;
};

class edCFiler
{
public:
	edCFiler();
	~edCFiler();

	virtual bool configure(uint);
	virtual bool configure(char* path, ETableOfContentsInitMode mode, edFILE_PARAMETER* param_4);
	virtual bool initialize();
	virtual bool terminate();
	virtual edCFileNoWaitStack* getnowaitfilestack();
	virtual void set_default_unit(char* szDriveLetter);
	virtual int get_default_unit(char* outString);
	virtual bool get_physical_filename(char* outFilePath, char* pathBuff);
	virtual bool mount_unit(char* filePath, char* bankPath);
	virtual bool unmount_unit(char* filePath);
	virtual bool open(edFILEH* outFile, char* unformatedFilePath);
	virtual bool close(edFILEH* pDebugBank);
	virtual uint read(edFILEH* pFile, void* pDst, uint requiredSize);
	virtual bool seek(edFILEH* pDebugBank);
	virtual bool isnowaitcmdend(edFILE_STACK_ELEMENT* pEdFilerInternal);
	virtual bool findfile(edCFileFind* pFileFind, int mode);
	virtual bool create(/*missing args*/);
	virtual bool write(edFILEH* pFile, void* pDst, uint size);
	virtual bool erase(char* szPath);
	virtual bool waitcmdend(/*missing args*/);
	virtual bool cmdbreak(/*missing args*/);
	virtual bool mkdir(char* szPath);
	virtual bool setattr(/*missing args*/);
	virtual bool getfreespace(char* szPath, ulong* pFreeBytes, ulong* pFreeClusters, byte* param_5);
	virtual bool isidle(char* szPath, int);
	virtual bool format(/*missing args*/);
	virtual uint getalignedsize(uint inSize);

	char* pDriveName_0x0;
	uint flags;
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
void edFileNoWaitStackFlush(edCFiler* pFiler);


#endif //_ED_FILE_FILER_H