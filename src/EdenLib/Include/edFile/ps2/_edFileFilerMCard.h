#ifndef _EDCFILER_MCARD_H
#define _EDCFILER_MCARD_H

#include "Types.h"
#include "edFile/edFileFiler.h"
#include "edFile/edFileNoWaitStack.h"

struct MCSpaceStruct_0x10
{
	MCSpaceStruct_0x10();

	byte field_0x0;
	byte field_0x1;
	byte bIsSyncIdle;
	byte field_0x3;

	int port;
	int slot;
	int free;
};

struct MC_OpenFile
{
#ifdef PLATFORM_PS2
	int syncResult;
#else
	void* pStream;
#endif
	MCSpaceStruct_0x10* pSpaceStruct;
};

class edCFiler_MemoryCard : public edCFiler
{
public:
	virtual bool configure(uint);
	virtual bool initialize();
	virtual bool terminate();
	virtual edCFileNoWaitStack* getnowaitfilestack();

	virtual bool get_physical_filename(char* outFilePath, char* pathBuff);

	virtual bool open(edFILEH* pFile, char* szPath);
	virtual bool close(edFILEH* pFile);
	virtual uint read(edFILEH* pFile, void* pDst, uint requiredSize);
	virtual bool write(edFILEH* pFile, void* pDst, uint size);
	virtual bool seek(edFILEH* pFile);
	virtual bool erase(char* szPath);
	virtual bool waitcmdend(/*missing args*/);
	virtual bool findfile(edCFileFind* pFileFind, int mode);
	virtual bool isnowaitcmdend(edFILE_STACK_ELEMENT* pElement);
	virtual bool mkdir(char* szPath);
	virtual bool setattr(/*missing args*/);
	virtual bool getfreespace(char* szPath, ulong* pFreeBytes, ulong* pFreeClusters, byte* param_5);
	virtual bool isidle(char* szPath, int param_3);
	virtual bool format(/*missing args*/);

	byte field_0x214;

	byte field_0x218[4];
	MC_OpenFile field_0x21c[4];

	edCFileNoWaitStack field_0x23c;

	MCSpaceStruct_0x10 aSpaceStructs[2];

	int activeMemCardIndex;

	int infoType;
	int infoFree;
	int infoFormat;
};

extern edCFiler_MemoryCard edFiler_MemoryCard;

void _edFileMCardAddFiler(void);

#endif // _EDCFILER_MCARD_H