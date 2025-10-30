#ifndef _ED_FILE_H
#define _ED_FILE_H

#include "Types.h"
#include "edMem.h"
#include "EdenLib/edSys/sources/EdHandlers.h"

class edCFiler;

struct CDFileContainer;

struct FileTime
{
	uchar sec;
	uchar min;
	uchar hour;
	uchar _pad;
	uchar day;
	uchar month;
	ushort year;
};

struct edFILE_INFO
{
	FileTime create;
	FileTime modify;
	uint fileSize;
	uint flags;
	char name[512];
};

struct edFILEH
{
	edCFiler* pOwningFiler;
	void* pFileData;
	int seekOffset; /* Created by retype action */
	uint openFlags;
	edFILE_INFO fileInfo;
	int nbQueuedActions;
	byte bInUse;
	undefined field_0x22d;
	undefined field_0x22e;
	undefined field_0x22f;
	EBankAction action;
};

enum ESeekMode
{
	ED_SEEK_SET,
	ED_SEEK_CUR,
	ED_SEEK_END
};

class edCFileFind
{
public:
	edCFileFind()
		: bOpen(0)
	{
	}

	byte bOpen;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	edCFiler* pFiler;
	int field_0x8;
	edFILE_INFO* pFileInfo;
	char szSlotPathA[512];
	char szSlotPathB[512];
};

enum EFindMode
{
	ED_FIND_FIRST = 0,
	ED_FIND_NEXT = 1,
	ED_FIND_CLOSE = 2
};

typedef edCSysHandlerSystem<ED_SYSTEM_HANDLER_FILE, &edSysHandlerMainPool, 16> edSysHandlerFile;

extern byte edFileHandleTable[16];
extern edFILEH edFileHandleData[16];

void* GetInternalData_0025b2e0(edFILEH* pFile);

char* edFileOpen(char* filePath, uint* outSize, uint flags);
edFILEH* edFileOpen(char* filePath, uint flags);
bool edFileMakeDir(char* szPath);
uint edFileLoadSize(edFILEH* pFile);
int edFileIsIdle(char* szPath, int param_2);
bool edFileGetFreeSpace(char* szPath, ulong* param_2, ulong* param_3, byte* param_4);
bool edFileRead(edFILEH* pFile, char* pDst, uint size);
bool edFileSeek(edFILEH* pFile, uint seekOffset, ESeekMode mode);
bool edFileClose(edFILEH* pFile);
bool edFileCmdBreak(edFILEH* pFile);

bool edFileWrite(edFILEH* pFile, void* pData, uint size);
bool edFileFlush(void);

void edFileNoWaitStackFlush(void);

uint edFileGetSize(edFILEH* param_1);

void edFileSetPath(char* mode);
bool edFileInit(void);
bool edFileGetPhysicalFileName(char* filePathOut, char* filePathIn);

bool edFileFindFirst(edFILE_INFO* pFileInfo, char* pName);
bool edFileFindClose(void);

bool edFileDelete(char* szPath);

edFILE_INFO* GetFileInfoFromFile(edFILEH* pFile);

struct edFileLoadConfig {
	EHeap heap;
	ushort align;
	ushort offset;
};

extern edFileLoadConfig edFileLoadInfo;

#define ED_HANDLER_FILE_OPEN 3
#define ED_HANDLER_FILE_READ 4
#define ED_HANDLER_FILE_WRITE 5
#define ED_HANDLER_FILE_SEEK 6

extern edSysHandlerFile edFileHandlers;

#endif //_ED_FILE_H