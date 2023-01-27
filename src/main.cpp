#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#if defined(PLATFORM_PS2)
#include <libsdr.h>
#include <libdma.h>
#include <libcdvd.h>
#include <libgraph.h>
#include <libgifpk.h>
#include <sifdev.h>
#include <sif.h>
#include <malloc.h>

#include <libdbc.h>
#include <libpad2.h>

#include <shellCore.h>
#include <shellDma.h>

#include <libscf.h>
#include <libipu.h>

#include <devvif0.h>
#include <devvif1.h>
#include <devvu0.h>
#include <devvu1.h>

#include <libmc.h>

extern "C" {
#include "ezmpegstr/ezmpegstr.h"
#include "ezmpegstr/playpss.h"
#include "ezmpegstr/strfile.h"
}
#else
#include <port.h>
#include <renderer.h>
#include <decode.h>
#include "edC/edCFiler_CDVD.h"
#endif

#include "IniFile.h"
#include "Iop.h"
#include "edMem.h"

#include "Types.h"
#include "edPacket.h"
#include "edSystem.h"
#include "LargeObject.h"
#include "edC/edCFiler.h"
#include "edC/edCBank.h"
#include "TimeController.h"
#include "LevelScheduleManager.h"
#include "edText.h"
#include "ed3D.h"
#include "edDlist.h"

uint g_DebugCameraFlag_00448ea4 = 0;

template<class T>
T* CreateNew()
{
	return new T;
}

InputSetupParams g_InputSetupParams = { 0, 0, 0, 0x8C };

InputSetupParams* GetInputSetupParams(void)
{
	return &g_InputSetupParams;
}

EFileLoadMode GetFileLoadMode_00424d9c(void)
{
	return g_InputSetupParams.fileLoadMode;
}

EdFileGlobal_10* g_NextFreeFilePtr_00457888 = NULL;

void SetupEd10_00217720(void* pObj, void* pFreeFunc, EdFileGlobal_10* pHeader)
{
	pHeader->pPrevFileHeader = g_NextFreeFilePtr_00457888;
	pHeader->pFreeFunc = pFreeFunc;
	pHeader->pFileData = pObj;
	g_NextFreeFilePtr_00457888 = pHeader;
	return;
}

IopManager IopManager_004497c4;
IopManager* PTR_IopManager_00448cf8 = &IopManager_004497c4;

char* g_szCdRomPrefix_00438548 = "cdrom0:";
char* g_szHostPrefix_00438550 = "host0:";

bool edSysHandlersInit(int size)
{
	bool bVar1;

	bVar1 = edCSysHandlerPool::initialize(&g_SysHandlersNodeTable_00489170, size);
	if (bVar1 != false) {
		// AddSystemMessageNodeHandler_002910f0(&edSysHandlersNodeParent_00489140, 0, SystemMessage, 1, 0);
	}
	else {
		/* edSysHandlersInit: Unable to initialize handler system */
		//SystemMessage(0, 0, s_edSysHandlersInit:_Unable_to_ini_00432b80);
		//Break();
	}
	return bVar1 != false;
}

void ToUppercase(char* string)
{
	char cVar1;

	while (true) {
		cVar1 = *string;
		if (cVar1 == '\0') break;
		if (('`' < cVar1) && (cVar1 < '{')) {
			cVar1 = cVar1 + -0x20;
		}
		*string = cVar1;
		string = string + 1;
	}
	return;
}

char g_edPsx2ModulePath[32];
char* g_ModuleNamePtr_004892a0;

void Set_edPsx2ModulePath(char* path)
{
	char cVar1;
	EFileLoadMode EVar2;
	int iVar3;
	char* pcVar4;

	ToUppercase(path);
	EVar2 = GetFileLoadMode_00424d9c();
	if (EVar2 == FLM_Net) {
		strcpy(g_edPsx2ModulePath, "");
		strcat(g_edPsx2ModulePath, "");
		strcat(g_edPsx2ModulePath, path);
	}
	else {
		if (EVar2 == FLM_CD_DVD) {
			strcpy(g_edPsx2ModulePath, "cdrom0:");
			if ((*path != '\\') && (*path != '/')) {
				strcat(g_edPsx2ModulePath, "\\");
			}
			strcat(g_edPsx2ModulePath, path);
		}
		else {
			//#Unimplemented
			//if (EVar2 == New_Name_(1)) {
			//	g_edPsx2ModulePath._0_7_ = CONCAT16(s_host0:_00433148[6], CONCAT24(s_host0 : _00433148._4_2_, s_host0 : _00433148._0_4_));
			//	g_edPsx2ModulePath = g_edPsx2ModulePath & 0xff00000000000000 | (ulong)g_edPsx2ModulePath._0_7_;
			//	if ((*param_1 == '\\') || (*param_1 == '/')) {
			//		param_1 = param_1 + 1;
			//	}
			//	strcat(g_edPsx2ModulePath, param_1);
			//}
			//else {
			//	g_edPsx2ModulePath = g_edPsx2ModulePath & 0xffffffffffffff00;
			//}
		}
	}
	iVar3 = strlen(g_edPsx2ModulePath);
	pcVar4 = g_edPsx2ModulePath + iVar3;
	if (0 < iVar3) {
		cVar1 = *(char*)((long long)pcVar4 - 1);//*(char*)((int)&g_SysHandlersNodeTable_00489170.maxPoolEntries + iVar3 + 3);
		if ((cVar1 != '/') && (g_ModuleNamePtr_004892a0 = g_edPsx2ModulePath + iVar3 + 1, cVar1 != '\\')) {
			*pcVar4 = '\\';
			*g_ModuleNamePtr_004892a0 = 0;
			pcVar4 = g_ModuleNamePtr_004892a0;
		}
	}
	g_ModuleNamePtr_004892a0 = pcVar4;
	/* edPsx2Module.path = %s\n */
	PrintString("edPsx2Module.path = %s\n", g_edPsx2ModulePath);
	return;
}

void StartCDDVDInit(void)
{
#if defined(PLATFORM_PS2)
	int iVar1;

	/* CDVD Drive Initialization... Please wait...\n */
	PrintString("CDVD Drive Initialization... Please wait...\n");
	sceSifInitRpc(0);
	iVar1 = sceCdInit(0);
	if (iVar1 == 0) {
		/* CDVD drive initialization failed\n */
		PrintString("CDVD drive initialization failed\n");
	}
	else {
		iVar1 = sceCdGetDiskType();
		if (iVar1 == 0x14) {
			sceCdMmode(2);
			/* Media type : DVD-ROM\n */
			PrintString("Media type : DVD-ROM\n");
		}
		else {
			sceCdMmode(1);
			/* Media type : CD-ROM\n */
			PrintString("Media type : CD-ROM\n");
		}
		sceFsReset();
		sceCdDiskReady(0);
		/* CDVD drive successfully initialized\n */
		PrintString("CDVD drive successfully initialized\n");
	}
#endif
	return;
}

void InitIopAndRPC_00291de0(void)
{
	int iVar1;

#if defined(PLATFORM_PS2)
	sceSifInitRpc(0);
	PrintString("RPC initialized\n");
	iVar1 = sceSifInitIopHeap();
	if (iVar1 == 0) {
		PrintString("IOP heap initialized\n");
	}
	else {
		PrintString("IOP heap initialization failed\n");
	}
#endif
	return;
}

byte g_LoadedModulesIDs_00489280[0x20];
int g_LoadedModulesCount_004892a4;

void StartCDBootModuleAndInitCDDVD(IopPaths* pIopPaths)
{
	char* pcVar1;
	char* __src;
	EFileLoadMode EVar2;
	int iVar3;
	char stringParameter[256];

	pcVar1 = pIopPaths->pIopRoot;
	__src = pIopPaths->pImgName;
	memset(g_LoadedModulesIDs_00489280, 0xff, sizeof(g_LoadedModulesIDs_00489280));
	g_edPsx2ModulePath[0] = '\0';
	g_LoadedModulesCount_004892a4 = 0;
	g_ModuleNamePtr_004892a0 = g_edPsx2ModulePath;
	Set_edPsx2ModulePath(pcVar1);
#if defined(PLATFORM_PS2)
	sceSifInitRpc(0);
	int a = sceCdGetDiskType();
	if ((__src != (char*)0x0) && (EVar2 = GetFileLoadMode_00424d9c(), EVar2 == FLM_CD_DVD)) {
		ToUppercase(__src);
		strcpy(stringParameter, g_edPsx2ModulePath);
		strcat(stringParameter, __src);
		do {
			iVar3 = sceSifRebootIop(stringParameter);
		} while (iVar3 == 0);
		do {
			iVar3 = sceSifSyncIop();
		} while (iVar3 == 0);
		/* CD boot module %s successfully loaded\n */
		PrintString("CD boot module %s successfully loaded\n", __src);
	}
	InitIopAndRPC_00291de0();
	StartCDDVDInit();
#endif
	return;
}

bool IsModuleLoaded_002921b0(byte id)
{
	byte* pbVar1;
	uint uVar2;

	pbVar1 = g_LoadedModulesIDs_00489280;
	uVar2 = 0;
	while (true) {
		if ((uint)g_LoadedModulesCount_004892a4 <= uVar2) {
			*pbVar1 = id;
			g_LoadedModulesCount_004892a4 = g_LoadedModulesCount_004892a4 + 1;
			return false;
		}
		if (*pbVar1 == id) break;
		uVar2 = uVar2 + 1;
		pbVar1 = pbVar1 + 1;
	}
	return true;
}


struct edModule {
	byte id;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	uint size;
	undefined4 args;
	char commandLine[10];
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
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
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	undefined field_0x60;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined field_0xcc;
	undefined field_0xcd;
	undefined field_0xce;
	undefined field_0xcf;
	undefined field_0xd0;
	undefined field_0xd1;
	undefined field_0xd2;
	undefined field_0xd3;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
	undefined field_0xe0;
	undefined field_0xe1;
	undefined field_0xe2;
	undefined field_0xe3;
	undefined field_0xe4;
	undefined field_0xe5;
	undefined field_0xe6;
	undefined field_0xe7;
	undefined field_0xe8;
	undefined field_0xe9;
	undefined field_0xea;
	undefined field_0xeb;
	undefined field_0xec;
	undefined field_0xed;
	undefined field_0xee;
	undefined field_0xef;
	undefined field_0xf0;
	undefined field_0xf1;
	undefined field_0xf2;
	undefined field_0xf3;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined field_0xf8;
	undefined field_0xf9;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
	undefined field_0x100;
	undefined field_0x101;
	undefined field_0x102;
	undefined field_0x103;
	undefined field_0x104;
	undefined field_0x105;
	undefined field_0x106;
	undefined field_0x107;
	undefined field_0x108;
	undefined field_0x109;
	undefined field_0x10a;
	undefined field_0x10b;
	char name[10];
	undefined field_0x116;
	undefined field_0x117;
	undefined field_0x118;
	undefined field_0x119;
	undefined field_0x11a;
	undefined field_0x11b;
	undefined field_0x11c;
	undefined field_0x11d;
	undefined field_0x11e;
	undefined field_0x11f;
	undefined field_0x120;
	undefined field_0x121;
	undefined field_0x122;
	undefined field_0x123;
	undefined field_0x124;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	undefined field_0x128;
	undefined field_0x129;
	undefined field_0x12a;
	undefined field_0x12b;
	undefined field_0x12c;
	undefined field_0x12d;
	undefined field_0x12e;
	undefined field_0x12f;
	undefined field_0x130;
	undefined field_0x131;
	undefined field_0x132;
	undefined field_0x133;
	undefined field_0x134;
	undefined field_0x135;
	undefined field_0x136;
	undefined field_0x137;
	undefined field_0x138;
	undefined field_0x139;
	undefined field_0x13a;
	undefined field_0x13b;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
	undefined field_0x140;
	undefined field_0x141;
	undefined field_0x142;
	undefined field_0x143;
	undefined field_0x144;
	undefined field_0x145;
	undefined field_0x146;
	undefined field_0x147;
	undefined field_0x148;
	undefined field_0x149;
	undefined field_0x14a;
	undefined field_0x14b;
};

void edPsx2ModuleLoad(char* pModulesName)
{
	bool bVar1;
	InputSetupParams* pIVar2;
	EFileLoadMode EVar3;
	int iVar4;
	int nbyte;
	byte* buf;
	void* pvVar5;
	char* pcVar6;
	int result;
	uint uVar7;
	edModule* pModule;
	char* paVar9;
#if defined(PLATFORM_PS2)
	sceSifDmaData local_10;

	pIVar2 = GetInputSetupParams();
	if (pIVar2->field_0x20 != (char*)0x0) {
		ToUppercase(pIVar2->field_0x20);
	}
	ToUppercase(pModulesName);
	strcpy(g_ModuleNamePtr_004892a0, pModulesName);
	EVar3 = GetFileLoadMode_00424d9c();
	if (EVar3 == FLM_CD_DVD) {
		iVar4 = strlen(pModulesName);
		g_ModuleNamePtr_004892a0 = g_ModuleNamePtr_004892a0 + iVar4;
		strcpy(g_ModuleNamePtr_004892a0, ";1");
		iVar4 = strlen(pModulesName);
		g_ModuleNamePtr_004892a0 = g_ModuleNamePtr_004892a0 + -iVar4;
	}
	iVar4 = sceOpen(g_edPsx2ModulePath, 1);
	if (iVar4 < 0) {
		int errorC = sceStdioConvertError(SCE_STDIO_FUNC_ANYTHING, iVar4);
		PrintString("module binary file : %s error: %d\n", g_edPsx2ModulePath, errorC);
	}
	else {
		nbyte = sceLseek(iVar4, 0, 2);
		sceLseek(iVar4, 0, 0);
		buf = (byte*)edMemAllocAlign(1, nbyte, 0x40);
		sceRead(iVar4, buf, nbyte);
		sceClose(iVar4);
		pModule = (edModule*)(buf + 4);
		PrintString("Binary modules Sony Libs version : %d.%d\n", (ulong)*buf, (ulong)buf[1]);
		for (uVar7 = 0; uVar7 < buf[2]; uVar7 = uVar7 + 1) {
			if (((uint)pModule & 3) != 0) {
				pModule = (edModule*)((((uint)pModule >> 2) + 1) * 4);
			}
			if (((uint)(pModule + 1) & 0x3f) == 0) {
				paVar9 = (char*)(pModule + 1);
			}
			else {
				paVar9 = (char*)((((uint)(pModule + 1) >> 6) + 1) * 0x40);
			}
			PrintString("Module %s (id = %02d) :\n", pModule->name, pModule->id);
			bVar1 = IsModuleLoaded_002921b0(pModule->id);
			if (bVar1 == false) {
				pvVar5 = sceSifAllocSysMemory(1, pModule->size, (void*)0x0);
				if (pvVar5 == (void*)0x0) {
					PrintString("-> module %s (%d)\n", pModule->name, pvVar5);
				}
				local_10.size = pModule->size;
				local_10.mode = 0;
				local_10.data = (uint)paVar9;
				local_10.addr = (uint)pvVar5;
				sceSifSetDma(&local_10, 1);
				if (pModule->args != 0) {
					PrintString("command line : %s", pModule->commandLine);
					for (pcVar6 = pModule->commandLine; *pcVar6 != '\0'; pcVar6 = pcVar6 + 1) {
						if (*pcVar6 == ' ') {
							*pcVar6 = '\0';
						}
					}
				}
				ToUppercase(pModule->name);
				if ((pIVar2->field_0x20 == (char*)0x0)) // || (pcVar6 = FUN_0021ce50(pModule->name, pIVar2->field_0x20), pcVar6 == (char*)0x0)) {
					result = sceSifLoadModuleBuffer(pvVar5, pModule->args, pModule->commandLine);
				if (result < 0) {
					PrintString("-> module %s (%d), err : %d\n", pModule->name, pModule->id, result);
				}
			}
			else {
				PrintString("edPsx2ModuleLoad : hand-load your IOP module NOW then press F5 to continue\n");
				//Break();
			}
			//FUN_0022fa30(uVar5);
			PrintString("-> successfully loaded\n");
		}
		pModule = (edModule*)((char*)((int)paVar9 + 0xc) + (pModule->size - 0xc));
	}
	edMemFree(buf);
	EVar3 = GetFileLoadMode_00424d9c();
	if (EVar3 == FLM_CD_DVD) {
		sceCdDiskReady(0);
	}
#endif
	return;
}


void edPsx2ModuleLoad(int count, edPsx2Module* aModules)
{
	bool bVar1;
	InputSetupParams* pIVar2;
	EFileLoadMode EVar3;
	char* pcVar4;
	int iVar5;
	size_t sVar6;
	uint uVar7;
	char* pcVar8;

	pIVar2 = GetInputSetupParams();
	if (pIVar2->field_0x20 != (char*)0x0) {
		ToUppercase(pIVar2->field_0x20);
	}
	for (; count != 0; count = count + -1) {
		PrintString("Module %s (id = %02d) :\n", aModules->name, aModules->id);
		bVar1 = IsModuleLoaded_002921b0(*(byte*)&aModules->id);
		if (bVar1 == false) {
			ToUppercase(aModules->name);
			strcpy(g_ModuleNamePtr_004892a0, aModules->name);
			EVar3 = GetFileLoadMode_00424d9c();
			if (EVar3 == FLM_CD_DVD) {
				sVar6 = strlen(aModules->name);
				g_ModuleNamePtr_004892a0 = g_ModuleNamePtr_004892a0 + (int)sVar6;
				strcpy(g_ModuleNamePtr_004892a0, ";1");
				sVar6 = strlen(aModules->name);
				g_ModuleNamePtr_004892a0 = g_ModuleNamePtr_004892a0 + -(int)sVar6;
			}
			PrintString("\tmodule full path/name : %s\n", g_edPsx2ModulePath);
			pcVar8 = (char*)0x0;
			if (aModules->cmdLine != (char*)0x0) {
				PrintString("command line : %s", aModules->cmdLine);
				for (pcVar8 = aModules->cmdLine; *pcVar8 != '\0'; pcVar8 = pcVar8 + 1) {
					if (*pcVar8 == ' ') {
						*pcVar8 = '\0';
					}
				}
				pcVar8 = pcVar8 + -(int)aModules->cmdLine;
			}
			uVar7 = 0;
#if defined(PLATFORM_PS2)
			if ((pIVar2->field_0x20 == (char*)0x0)) // || (pcVar4 = FUN_0021ce50(aModules->name, pIVar2->field_0x20), pcVar4 == (char*)0x0)) 
			{
				iVar5 = sceSifLoadModule(g_edPsx2ModulePath, (int)pcVar8, aModules->cmdLine);
				while ((iVar5 < 0 && (bVar1 = uVar7 < 10, uVar7 = uVar7 + 1, bVar1))) {
					PrintString("** could not load module %s, error : %d (retrying) **\n", aModules->name, -iVar5);
					iVar5 = sceSifLoadModule(g_edPsx2ModulePath, (int)pcVar8, aModules->cmdLine);
				}
				if ((uVar7 < 0xb) || (-1 < iVar5)) {
					PrintString("-> successfully loaded\n");
				}
			}
			else {
				PrintString("edPsx2ModuleLoad : hand-load your IOP module NOW then press F5 to continue\n");
				//Break();
			}
			EVar3 = GetFileLoadMode_00424d9c();
			if (EVar3 == FLM_CD_DVD) {
				sceCdDiskReady(0);
			}
#endif

		}
		aModules = aModules + 1;
	}
	return;
}



void IOPFunc_00291d10(void)
{
	IopPaths* pIopPaths;
	InputSetupParams* pIVar1;
	uint uVar2;
	undefined4 uVar3;

	pIVar1 = GetInputSetupParams();
	pIopPaths = pIVar1->pIopPaths;
	StartCDBootModuleAndInitCDDVD(pIopPaths);
	if (pIopPaths->modulesCount == 0) {
		edPsx2ModuleLoad(pIopPaths->pModulesName);
	}
	else {
		edPsx2ModuleLoad(pIopPaths->modulesCount, (edPsx2Module*)pIopPaths->pModulesName);
	}
	//IOPSetupFunc_00292cc0();
	//uVar2 = GetTotalIOPMemory_00292b80();
	//uVar3 = GetLargestIOPMemory_00292c70();
	/* IOP available memory :\nTotal : %010d\nLargest : %010d\n */
	//PrintString(s_IOP_available_memory_:_Total_:_ % _00432fb0, uVar2, uVar3);
	return;
}

int SetupLibdcbAndLibpad(void)
{
	int success;

#if defined(PLATFORM_PS2)
	success = sceDbcInit();
	if (success == 1) {
		success = scePad2Init(0);
		if (success == 1) {
			success = 0;
		}
		else {
			/* [EE] edDev : libpad2 init went wrong.\n */
			PrintString("[EE] edDev : libpad2 init went wrong.\n");
			success = -1;
		}
	}
	else {
		/* [EE] edDev : libdbc init went wrong.\n */
		PrintString("[EE] edDev : libdbc init went wrong.\n");
		success = -1;
	}
#else
	success = 0;
#endif
	return success;
}

void InitEdSys1(void)
{
	edSystemInit(g_InputSetupParams.argc, g_InputSetupParams.argv);
	InitHeap_002902a0((short)g_InputSetupParams.headerCount);
	edSysHandlersInit(g_InputSetupParams.handlersSize);
	//EdSysFunc_00290f00();
	IOPFunc_00291d10();

	SetupLibdcbAndLibpad(); // HACK
	return;
}

EFileLoadMode g_FileLoadMode_00448810 = FLM_CD_DVD;

char* LoadFileFromDisk(const char* fileName, uint* outSize)
{
	int iVar1;
	uint size;
	long ret;
	char* unaff_s3_lo = NULL;
	char cdPath[256];
	char filePath[256];

#if defined(PLATFORM_PS2)
	scePrintf("Loading ini %d\n", g_FileLoadMode_00448810);
	sceCdlFILE inFile;

	if (g_FileLoadMode_00448810 == FLM_CD_DVD) {
		/* Format: '\Param 1;1' */
		strcpy(filePath, "\\");
		strcat(filePath, fileName);
		strcat(filePath, ";1");
		scePrintf("Loading ini %s\n", filePath);
		ret = sceCdSearchFile(&inFile, filePath);
		scePrintf("Loading ini %d | %d\n", ret, sceCdGetError());
		if (ret != 0) {
			strcpy(cdPath, g_szCdRomPrefix_00438548);
			strcat(cdPath, filePath);
			iVar1 = sceOpen(cdPath, 1);
			if (-1 < iVar1) {
				size = sceLseek(iVar1, 0, 2);
				if (outSize != (uint*)0x0) {
					*outSize = size;
				}
				unaff_s3_lo = (char*)edMemAlloc(1, (long)(int)size);
				if (unaff_s3_lo != (char*)0x0) {
					sceLseek(iVar1, 0, 0);
					sceRead(iVar1, unaff_s3_lo, size);
				}
				sceClose(iVar1);
			}
		}
	}
	else {
		/* host0: */
		strcpy(filePath, g_szHostPrefix_00438550);
		strcat(filePath, fileName);
		iVar1 = sceOpen(filePath, 1);
		if (-1 < iVar1) {
			size = sceLseek(iVar1, 0, 2);
			if (outSize != (uint*)0x0) {
				*outSize = size;
			}
			unaff_s3_lo = (char*)edMemAlloc(1, (long)(int)size);
			if (unaff_s3_lo != (char*)0x0) {
				sceLseek(iVar1, 0, 0);
				sceRead(iVar1, unaff_s3_lo, size);
			}
			sceClose(iVar1);
		}
	}
#else
	FILE* fp;

	fp = fopen(fileName, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		if (outSize != (uint*)0x0) {
			*outSize = size;
		}

		unaff_s3_lo = (char*)edMemAlloc(1, (long)(int)size);
		if (unaff_s3_lo != (char*)0x0) {
			fseek(fp, 0, SEEK_SET);
			fread(unaff_s3_lo, 1, size, fp);
		}
		fclose(fp);
	}
#endif

	return unaff_s3_lo;
}

const char* g_szIni_0042b5b8 = "BWITCH.INI";


int g_ScreenWidth;
int g_VideoMode1;
int g_ScreenHeight;
int g_VideoMode2;

char* s_Video_0042b490 = "Video";

int g_SetOffsetX;
int g_SetOffsetY;

void SetupVideo(IniFile* file)
{
	bool bVar1;
	int iVar2;
	int videoMode;
	int isNTSC;
	uint screenHeight;
	char outVideoMode[140];
	int videoFrequency;

	if (file->fileBufferStart != (char*)0x0) {
		outVideoMode[0] = '\0';
		screenHeight = 0x200;
		isNTSC = 1;
		videoMode = 3;
		/* Video - SetVideoMode */
		bVar1 = g_IniFile_00450750.ReadString_001aa520(s_Video_0042b490, "SetVideoMode", outVideoMode);
		if ((bVar1 != false) && (iVar2 = strcmp(outVideoMode, "NTSC"), iVar2 == 0)) {
			screenHeight = 0x1c0;
			isNTSC = 0;
			videoMode = 2;
		}
		/* Video - SetFrequency */
		videoFrequency = 0;
		bVar1 = g_IniFile_00450750.ReadInt_001a9830(s_Video_0042b490, "SetFrequency", &videoFrequency);
		if ((bVar1 != false) && (videoFrequency == 0x3c)) {
			isNTSC = 0;
		}
		g_ScreenWidth = 0x200;
		g_VideoMode1 = 0;
		g_ScreenHeight = screenHeight;
		g_isNTSC = isNTSC;
		g_VideoMode2 = videoMode;
		g_IniFile_00450750.ReadInt_001a9830(s_Video_0042b490, "SetOffsetX", &g_SetOffsetX);
		g_IniFile_00450750.ReadInt_001a9830(s_Video_0042b490, "SetOffsetY", &g_SetOffsetY);
	}
	return;
}

#if defined(PLATFORM_PS2)
sceDmaChan* g_DmaChan_00449820;
sceGsDBuff g_DoubleBuffer;
#endif

void GraphicsClear_004059a0(ulong r, ulong g, ulong b, ulong a)
{
	uint size;
#if defined(PLATFORM_PS2)
	u_long128 auStack256[240];
	sceGifPacket gifpkt;

	sceGifPkInit(&gifpkt, auStack256);
	sceGifPkReset(&gifpkt);
	sceGifPkAddGsData(&gifpkt, 0x100000000000800a);
	sceGifPkAddGsData(&gifpkt, 0xe);
	sceGifPkAddGsData(&gifpkt, 1);
	sceGifPkAddGsData(&gifpkt, 0x1a);
	sceGifPkAddGsData(&gifpkt, 0x100000);
	sceGifPkAddGsData(&gifpkt, 0x4c);
	sceGifPkAddGsData(&gifpkt, g_DoubleBuffer.draw0.zbuf1addr | 0x130000000);
	sceGifPkAddGsData(&gifpkt, 0x4e);
	sceGifPkAddGsData(&gifpkt, 0);
	sceGifPkAddGsData(&gifpkt, 0x18);
	sceGifPkAddGsData(&gifpkt, 0x400000004000000);
	sceGifPkAddGsData(&gifpkt, 0x40);
	sceGifPkAddGsData(&gifpkt, 0x30000);
	sceGifPkAddGsData(&gifpkt, 0x47);
	sceGifPkAddGsData(&gifpkt, 6);
	sceGifPkAddGsData(&gifpkt, 0);
	sceGifPkAddGsData(&gifpkt, (a & 0xff) << 0x18 | (b & 0xff) << 0x10 | r & 0xff | (g & 0xff) << 8 | 0x3f80000000000000);
	sceGifPkAddGsData(&gifpkt, 1);
	sceGifPkAddGsData(&gifpkt, 0);
	sceGifPkAddGsData(&gifpkt, 4);
	sceGifPkAddGsData(&gifpkt, 0x40004000);
	sceGifPkAddGsData(&gifpkt, 4);
	sceGifPkTerminate(&gifpkt);
	FlushCache(0);
	size = sceGifPkSize(&gifpkt);
	sceDmaSendN(g_DmaChan_00449820, (u_long128*)((u_int)gifpkt.pBase & 0x0FFFFFFF), size);
	sceGsSyncPath(0, 0);
#endif
	return;
}

void SetupDoubleBuffer_00405ba0(void)
{
	short videoMode;
	int iVar1;
	short screenHeight;
	short screenWidth;

	screenHeight = (short)g_ScreenHeight;
	screenWidth = (short)g_ScreenWidth;
	/* Start video mode in NTCS and change to PAL based on global */
	videoMode = 2;
	if (g_VideoMode2 != 2) {
		videoMode = 3;
	}
#if defined(PLATFORM_PS2)
	g_DmaChan_00449820 = sceDmaGetChan(2);
	sceGsResetGraph(0, SCE_GS_INTERLACE, videoMode, SCE_GS_FIELD);
	GraphicsClear_004059a0(0xff, 0xff, 0, 0);
	sceGsSetDefDBuff(&g_DoubleBuffer, SCE_GS_PSMCT32, screenWidth, screenHeight, SCE_GS_ZGREATER, SCE_GS_PSMZ32, SCE_GS_NOCLEAR);
	iVar1 = 0;
	do {
		sceGsSyncV(0);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 0x14);
	iVar1 = sceGsSyncV(0);
	sceGsSetHalfOffset(&g_DoubleBuffer.draw1, 0x800, 0x800, (ushort)iVar1 ^ 1);
	FlushCache(0);
	sceGsSwapDBuff(&g_DoubleBuffer, 0);
#endif
	return;
}

#if defined(PLATFORM_PS2)
void InitSystem()
{
	// resets the entire VIF0 including VIF0-FIF0
	sceDevVif0Reset();

	// resets the entire VIF1 including VIF1-FIF0
	sceDevVif1Reset();

	// reset Vu0
	sceDevVu0Reset();

	// reset Vu1
	sceDevVu1Reset();

	// resets devices on the data path to GS (VIF1,Vu0,GIF)
	sceGsResetPath();

	// resets the DMAC
	sceDmaReset(1);
	return;
}
#endif

struct astruct_9 {
	short field_0x0;
	short field_0x2;
	short width;
	short height;
};

struct SplashParams {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	struct astruct_9* field_0x4;
	int field_0x8;
	struct astruct_9* field_0xc;
	char* pSplashFile;
};

int ConvFunc_002ba830(long inVal)
{
	int outVal;

	if (inVal == 2) {
		outVal = 2;
	}
	else {
		if (inVal == 1) {
			outVal = 3;
		}
		else {
			if (inVal == 0) {
				outVal = 4;
			}
		}
	}
	return outVal;
}

void SplashFunc_002ba880(SplashParams* pParams, long param_2, uint param_3)
{
	short sVar1;
	astruct_9* paVar2;
	int iVar3;
	uint uVar4;
	int iVar5;
	char* pcVar6;
	uint uVar7;
	uint uVar8;
	int iVar9;
	uint uVar10;
	uint uVar11;
	ulonglong auStack832[100];
	edPacket packet;

	pcVar6 = pParams->pSplashFile;
	paVar2 = pParams->field_0xc;
	iVar3 = ConvFunc_002ba830(param_2);
	uVar10 = paVar2->height;
	sVar1 = paVar2->width;
	uVar4 = (uint)(iVar3 * (int)sVar1 * (int)paVar2->height) / 0x7fff0;
	uVar11 = uVar4 + 1;
	if (1 < uVar11) {
		uVar10 = 0x7fff0 / (sVar1 * iVar3);
	}
	uVar8 = iVar3 * uVar10 * (int)sVar1;
	edPacketInit(&packet, 800, auStack832);
	iVar9 = 0;
	uVar7 = 0;
	if (uVar4 != 0xffffffff) {
		do {
			edPacketFunc_0026a2b0(&packet, 0);
			edPacketWriteLong_0026a000(&packet, 0x1000000000000004);
			edPacketWriteLong_0026a000(&packet, 0xe);
			iVar5 = (int)paVar2->width;
			if (paVar2->width < 0) {
				iVar5 = iVar5 + 0x3f;
			}
			edPacketWriteLong_0026a000(&packet, param_2 << 0x38 | (ulong)param_3 << 0x20 | (long)(iVar5 >> 6) << 0x30);
			edPacketWriteLong_0026a000(&packet, 0x50);
			edPacketWriteLong_0026a000(&packet, (long)paVar2->field_0x0 << 0x20 | (long)(paVar2->field_0x2 + iVar9) << 0x30);
			edPacketWriteLong_0026a000(&packet, 0x51);
			edPacketWriteLong_0026a000(&packet, (long)paVar2->width | (ulong)uVar10 << 0x20);
			edPacketWriteLong_0026a000(&packet, 0x52);
			edPacketWriteLong_0026a000(&packet, 0);
			edPacketWriteLong_0026a000(&packet, 0x53);
			edPacketFunc_0026a2b0(&packet, 0);
			edPacketWriteLong_0026a000(&packet, (long)(int)(uVar8 >> 4) | 0x800000000008000);
			edPacketFunc_0026a440(&packet, (long)(int)pcVar6, uVar8, 0);
			iVar9 = iVar9 + uVar10;
			pcVar6 = pcVar6 + iVar3 * uVar10 * (int)paVar2->width;
			if ((ulong)(long)paVar2->height < (ulong)(long)(int)(iVar9 + uVar10)) {
				uVar10 = paVar2->height - iVar9;
				uVar8 = iVar3 * uVar10 * (int)paVar2->width;
			}
			uVar7 = uVar7 + 1;
		} while (uVar7 < uVar11);
	}
	edPacketFunc_0026a6d0(&packet, 0);
	edPacketClose_00269e70(&packet);
#if defined(PLATFORM_PS2)
	//WaitDMA();
	shellDmaStartChain(SHELLDMA_CHANNEL_GIF, (ShellDmaTag*)packet.pBuffer);
#endif
	return;
}

void SetupSplash(char* pSplashFile, uint count)
{
#if defined(PLATFORM_PS2)
	int iVar1;
	uint counter;
	sceGsDrawEnv1* draw;
	SplashParams auStack32;
	astruct_9 local_8;

	//WaitForDraw_00258230();
	auStack32.field_0x4 = &local_8;
	local_8.field_0x0 = 0;
	local_8.field_0x2 = 0;
	auStack32.field_0x8 = 0;
	local_8.width = (short)g_ScreenWidth;
	local_8.height = (short)g_ScreenHeight;
	auStack32.field_0xc = auStack32.field_0x4;
	auStack32.pSplashFile = pSplashFile;
	for (counter = 0; counter < count; counter = counter + 2) {
		//WaitForDraw_00258230();
		SplashFunc_002ba880(&auStack32, 1, 0);
		if (true) {
			draw = &g_DoubleBuffer.draw0;
		}
		else {
			draw = &g_DoubleBuffer.draw1;
		}
		iVar1 = sceGsSyncV(0);
		sceGsSetHalfOffset(draw, 0x800, 0x800, (ushort)iVar1 ^ 1);
		FlushCache(0);
		sceGsSwapDBuff(&g_DoubleBuffer, counter);
	}
#else
	Renderer::ShowSplash(pSplashFile, g_ScreenWidth, g_ScreenHeight);
#endif
	return;
}

char* sz_ModeNet_0042b7f8 = "<net>";
char* sz_ModeCdvd_0042b800 = "<cdvd>";
char* sz_ModePc_0042b808 = "<pc>";
char* sz_ModeFullPC_0042b810 = "<pc>0:d:\\Projects\\BWitch\\Psx2\\";

edCdlFolder* PTR_edCdlFolder_00448ef4;

void Init_edFileA(void)
{
	bool bVar1;
	InputSetupParams* pIVar2;
	undefined4 uVar3;
	int local_4;

	pIVar2 = GetInputSetupParams();
	//uVar3 = Get0x448fd0();
	//*(bool*)uVar3 = g_ProfileModeSetup_00448edc != 0;
	InitFileHandlers_0025c300();
	if (g_FileLoadMode_00448810 == FLM_Net) {
		SetFilePathMode_00261810(sz_ModeNet_0042b7f8);
	}
	else {
		if (g_FileLoadMode_00448810 == FLM_CD_DVD) {
			SetFilePathMode_00261810(sz_ModeCdvd_0042b800);
		}
		else {
			SetFilePathMode_00261810(sz_ModePc_0042b808);
		}
	}
	/* <cdvd> */
	local_4 = 0;
	bVar1 = edCFilerInitTOC_00260f80(sz_ModeCdvd_0042b800, IM_CALC_SIZE, (void*)0x898, &local_4);
	if (bVar1 != false) {
		PTR_edCdlFolder_00448ef4 = (edCdlFolder*)edMemAlloc(1, local_4);
		/* <cdvd> */
		edCFilerInitTOC_00260f80(sz_ModeCdvd_0042b800, IM_INIT, PTR_edCdlFolder_00448ef4, (int*)local_4);
	}
	if (pIVar2->field_0x20 != (char*)0x0) {
		/* <pc>0:d:\\Projects\\BWitch\\Psx2\\ */
		SetFilePathMode_00261810(sz_ModeFullPC_0042b810);
	}
	return;
}

void SetupGame(int argc,char **argv)
{
	InputSetupParams *pIVar1;
	char* pFileBuffer;
	uint fileSize;
	
	printf("----------------- Main Init begin -----------\n");
	printf("-------- eden lib init --------------\n");
	printf("---- Init edSys \n");
	
	pIVar1 = GetInputSetupParams();
	pIVar1->argc = argc;
	pIVar1->argv = argv;
	pIVar1->field_0x14 = 10000;
	pIVar1->headerCount = 0x1000;

	PTR_IopManager_00448cf8->SetupIopPaths(pIVar1);

	InitEdSys1();

	//ScratchPadAllocator::Init_00191940(&ScratchPadAllocator_00449ed0);

#if defined(PLATFORM_PS2)
	sceSdRemoteInit();
	scePrintf("sceSdRemoteInit Done\n");

	InitSystem();
#endif

	pFileBuffer = LoadFileFromDisk(g_szIni_0042b5b8, &fileSize);
	if (pFileBuffer != (char*)0x0) 
	{
		g_IniFile_00450750.Load_001ab200(g_szIni_0042b5b8, pFileBuffer, fileSize);
		SetupVideo(&g_IniFile_00450750);
	}

#if defined(PLATFORM_PS2)
	SetupDoubleBuffer_00405ba0();
#else
	Renderer::Setup();
#endif

	/* If using NTSC then load SPLASH_N, otherwise load SPLASH_P */
	if (g_VideoMode2 == 2) {
		pFileBuffer = LoadFileFromDisk("CDEURO\\FRONTEND\\SPLASH_N.RAW", (uint*)0x0);
	}
	else {
		pFileBuffer = LoadFileFromDisk("CDEURO\\FRONTEND\\SPLASH_P.RAW", (uint*)0x0);
	}
	if (pFileBuffer != (char*)0x0) {
#if defined(PLATFORM_PS2)
		shellDmaInit();
#endif
		SetupSplash(pFileBuffer, 2);
		edMemFree(pFileBuffer);
	}

	//PrintString(s_----_Init_edDebug_0042b610);
	//edDebug::Init();
	//PrintString(s_----_Init_edCluster_0042b630);
	//soundController ? = Init_edClusterA();
	//*soundController ? = 6;
	//Init_edClusterB();
	//edProfile::Init_001b8390(0);
	//(*(code*)(*PTR_IopManager_00448cf8)->field_0x10)(PTR_IopManager_00448cf8, 0);
	PrintString("---- Init edDlist \n");
	//soundController ? = GetDAT_004250d0();
	//*soundController ? = 0x14;
	//soundController ? [3] = soundController ? [3] * 3;
	//soundController ? [4] = 500;
	edDlist::Init();
	PrintString("---- Init ed3D \n");
	ed3D::Init();
	//PrintString(s_----_Init_edFile_0042b690);
	Init_edFileA();
	//puVar1 = GetPtrgp0xffff8c78();
	//*(undefined4*)(puVar1 + 4) = 0x26;
	Init_edText();
	//PrintString(s_----_Init_edMath_0042b6b0);
	//Init_edMath();
	//PrintString(s_----_Init_edDev_0042b6d0);
	//Init_edDevA();
	///* Should we init sound and music? */
	//if (DAT_00448ef0 == 0) {
	//	PrintString(s_----_Init_edMusic_0042b6f0);
	//	puVar2 = FUN_002699e0();
	//	uVar3 = Get0x30();
	//	if (uVar3 < 0x30) {
	//		*puVar2 = uVar3 >> 1;
	//	}
	//	else {
	//		*puVar2 = 0x18;
	//	}
	//	puVar2[1] = 10;
	//	puVar2[2] = 0xf;
	//	puVar2[3] = 5;
	//	edMusic::Init();
	//	PrintString(s_----_Init_Sound_0042b710);
	//	soundController ? = Init_SoundA();
	//	soundController ? [3] = *puVar2;
	//	soundController ? [4] = uVar3 - 1;
	//	*soundController ? = 0x80;
	//	if (g_VideoMode2 == 3) {
	//		soundController ? [5] = 0x32;
	//	}
	//	else {
	//		soundController ? [5] = 0x3c;
	//	}
	//	Init_SoundB();
	//	Init_SoundC((float)&DAT_3f4ccccd);
	//}
	//Init_SoundD();
	/* Init_edBank */
	PrintString("---- Init edBank \n");
	Init_edBank();
	///* game_init */
	//PrintString(s_--------_game_init_------------ - _0042b750);
	//PrintString(s_--------------------------------_0042b780);
	//Game_Init();
	///* <BNK>0: CDEURO/menu/Messages.bnk */
	//LoadBNK(s_ < BNK>0:_0042b528, s_CDEURO / menu / Messages.bnk_0042b7c0);
	//FUN_00336b40();
	//LoadAndVerifyMenuBnk();
	///* Init_edVideo */
	//PrintString(s_----_Init_edVideo_0042b7e0);
	//puVar1 = GetAddressuGpffff8510();
	//puVar1[1] = 1;
	//puVar1[4] = 0;
	//FUN_002b9e70();
	//SetupVidParams_001baa30((short)g_VideoMode2, (short)g_ScreenWidth, (short)g_ScreenHeight, (short)g_VideoMode1, 0);
	//CheckControllers(&g_IniFile_00450750);
	SetupGameCreateObject();
	WillSetupDisplayListAndRunConstructors();
	///* May jump to 003965B8 */
	//lVar4 = (*(code*)(*PTR_IopManager_00448cf8)->field_0x18)(PTR_IopManager_00448cf8, 0);
	///* This doesn't seem to trigger on main run. */
	//if (lVar4 != 0) {
	//	DrawPopup_0034e1f0(8, 0, 0x52525f503700080c, 0x171d0d0b190f111a, 0);
	//}
	//SetupIconSaveAndSerial();
	return;
}

struct VideoFile {
	byte fbp0;
	byte fileReadSuccess; /* True if the file read successful */
	byte videoIsPAL;
	byte field_0x3;
#ifdef PLATFORM_PS2
	playPssRsrcs pssResources;
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
	sceGsDispEnv dispenv;
#endif
};

void LoadVideoFromFilePath(VideoFile* display, char* inFileName)
{
	u_char* puVar2;
#ifdef PLATFORM_PS2
	sceIpuRGB32* psVar3;
#endif
	u_int* puVar4;
	void* pvVar5;
	uint uVar6;
	int iVar7;
	char fileName[512];

	display->videoIsPAL = g_VideoMode2 == 3;
	display->fbp0 = 0;
	display->fileReadSuccess = 0;
	//UpdateSoundManager();
	//SetPerfFlag_0023e2b0();
#ifdef PLATFORM_PS2
	DisableIntc(2);
	DisableIntc(0);

	uint workArea = 0x16976c + 0x1000;

	puVar2 = (u_char*)edMemAllocAlign(1, workArea, 0x40);
	(display->pssResources).mpegBuff = puVar2;
	(display->pssResources).mpegBuffSize = workArea;
	psVar3 = (sceIpuRGB32*)edMemAllocAlign(1, 0x140000, 0x40);
	(display->pssResources).rgb32 = psVar3;
	puVar4 = (u_int*)edMemAllocAlign(1, 0x3c200, 0x40);
	(display->pssResources).path3tag = puVar4;
	pvVar5 = edMemAllocAlign(1, 0x30000, 0x40);
	(display->pssResources).demuxBuff = pvVar5;
	(display->pssResources).demuxBuffSize = 0x30000;
	puVar2 = (u_char*)edMemAllocAlign(1, 0xc000, 0x40);
	(display->pssResources).audioBuff = puVar2;
	(display->pssResources).audioBuffSize = 0xc000;

	pvVar5 = sceSifAllocIopHeap(0x6000);
	(display->pssResources).iopBuff = (int)pvVar5;
	(display->pssResources).iopBuffSize = 0x6000;
	puVar2 = (u_char*)edMemAllocAlign(1, ZERO_BUFF_SIZE, 0x40);
	(display->pssResources).zeroBuff = puVar2;
	pvVar5 = sceSifAllocIopHeap(ZERO_BUFF_SIZE);
	(display->pssResources).iopZeroBuff = (int)pvVar5;

	clearBackground(0x80000000, (display->pssResources).path3tag);
#endif
	uVar6 = 0x200;
	if (display->videoIsPAL == 0) {
		uVar6 = 0x1c0;
	}
#ifdef PLATFORM_PS2
	sceGsSetDefDispEnv(&display->dispenv, SCE_GS_PSMCT32, DISP_WIDTH, (short)uVar6, 0xc, 0);

	if (uVar6 < 0x200) {
		(display->dispenv).dispfb.DBX = (MAX_WIDTH - DISP_WIDTH) / 2;
	}
	if (MAX_HEIGHT > DISP_HEIGHT) {
		(display->dispenv).dispfb.DBY = (MAX_HEIGHT - DISP_HEIGHT) / 2;
	}

	(display->dispenv).dispfb.FBW = bound(MAX_WIDTH, 64) / 64;

	sceGsPutDispEnv(&display->dispenv);
	iVar7 = 0;
	do {
		sceGsSyncV(0);
		iVar7 = iVar7 + 1;
	} while (iVar7 < 0x14);

#endif

	FormatStreamPath(fileName, inFileName);

#ifdef PLATFORM_WIN
	char* pcVar6 = SearchForColon(fileName);

	char* pcFileFull = FormatForPC(pcVar6);
	memcpy(fileName, pcFileFull, strlen(pcFileFull));
	fileName[strlen(pcFileFull)] = '\0';
#endif

	iVar7 = strFileOpen(fileName);
	display->fileReadSuccess = iVar7 != false;

	if (display->fileReadSuccess != 0) {
#ifdef PLATFORM_PS2
		playPss(&display->pssResources);
#else
		playPss();
#endif
	}
	return;
}

void Free(void* ptr)
{
	edMemFree(ptr);
}

void DestroyVideoFile(VideoFile* pVideo)
{
	if (pVideo->fileReadSuccess != 0) {
		destroyPss();
		strFileClose();
		pVideo->fileReadSuccess = 0;
	}
#ifdef PLATFORM_PS2
	clearBackground(0x80000000, (pVideo->pssResources).path3tag);
	edMemFree((pVideo->pssResources).mpegBuff);
	edMemFree((pVideo->pssResources).rgb32);
	edMemFree((pVideo->pssResources).path3tag);
	edMemFree((pVideo->pssResources).demuxBuff);
	edMemFree((pVideo->pssResources).audioBuff);
	sceSifFreeIopHeap((void*)(pVideo->pssResources).iopBuff);
	edMemFree((pVideo->pssResources).zeroBuff);
	sceSifFreeIopHeap((void*)(pVideo->pssResources).iopZeroBuff);
	EnableIntc(2);
	EnableIntc(0);
	//FUN_002b9b80((VidParams26*)0x0);
	//FUN_002b9e00(0, 0, 0);
	//FUN_002b9dd0((short)g_SetOffsetX, (short)g_SetOffsetY);
	//RefreshScreenRender();
#endif
	return;
}

void ShowCompanySplashScreen(char* file_name, bool param_2, bool param_3)
{
	byte bVar1;
	TimeController* inTimeController;
	VideoFile* display;
	uint uVar2;
	int iVar3;
	float fVar4;
	char file_path[512];

	inTimeController = GetTimeController();
	display = (VideoFile*)Allocate(sizeof(VideoFile));
	/* file_path = CDEURO/movies/ + file_name + .pss */
	FormatFilePath(file_path, "CDEURO/movies/", file_name, ".pss", 0);
	LoadVideoFromFilePath(display, file_path);

	do {
		//ReadInput(inTimeController->cutsceneDeltaTime);
		inTimeController->UpdateDeltaTime();
		if (display->fileReadSuccess != 0) {
			fVar4 = 1.0;
			if (param_2 == false) {
				bVar1 = display->videoIsPAL;
			}
			else {
				fVar4 = 0.0f; //FUN_00284900();
				bVar1 = display->videoIsPAL;
			}
#ifdef PLATFORM_PS2
			continuePss(&display->pssResources, &display->dispenv, (int)bVar1);
#else
			char* image = continuePss();

			if (image) {
				Renderer::ShowSplash(image + 0x10, 0x280, 0x200);
			}
#endif
		}
		//if (param_2 != false) {
		//	if (param_3 == false) {
		//		uVar2 = 1;
		//		if ((((uint)InputManager_00450960.pressedBitfield & 0x40000) == 0) && (((uint)InputManager_00450960.pressedBitfield & 0x1000000) == 0)) {
		//			uVar2 = 0;
		//		}
		//	}
		//	else {
		//		uVar2 = (uint)InputManager_00450960.pressedBitfield & 0x40000;
		//	}
		//	if (uVar2 != 0) break;
		//}
		iVar3 = 1;
		if (display->fileReadSuccess != 0) {
			iVar3 = hasVideoEnded();
		}
	} while (iVar3 == 0);
	DestroyVideoFile(display);
	if (display != (VideoFile*)0x0) {
		Free(display);
	}
	return;
}

void LoadStageOne(void)
{
	TimeController* objAndCounter;
	LevelScheduleManager** loadLoopObject;
	int iVar1;

	objAndCounter = GetTimeController();
	objAndCounter->ResetTimeController_001ba6d0();
	//SetuGpffff8e5cTo0();
	//iVar1 = 0;
	//loadLoopObject = &g_LevelScheduleManager_00451660;
	//do {
	//	if (*loadLoopObject != (LevelScheduleManager*)0x0) {
	//		(*(code*)(*loadLoopObject)->pManagerFunctionData->loadStageOneFunc)();
	//	}
	//	iVar1 = iVar1 + 1;
	//	loadLoopObject = loadLoopObject + 1;
	//} while (iVar1 < 0x18);
	g_LevelScheduleManager_00449728->LoadStageOne();
	return;
}


void LoadLevel(void)
{
	LargeObject* pLVar1;
	bool bVar2;
	TimeController* inTimeController;

	/* These functions just run once */
	//PlayIntroVideo(0);
	pLVar1 = g_LargeObject_006db450;
	inTimeController = GetTimeController();
	LoadStageOne();
	do {
		/* This is the main loop that plays cutscenes

		   This does not control any cutscene elements */
		//ReadInput(inTimeController->cutsceneDeltaTime);
		inTimeController->UpdateDeltaTime();
		/* Play cutscene */
		bVar2 = true; // AsyncLoad_001b9cd0(pLVar1);
		/* Responsible for drawing the loading screen. */
		LoadLevelUpdate_001b9c60();
		//SaveRelated_002f37d0(&SaveDataLoadStruct_0048ee30);
		/* Update rendering */
		//RefreshScreenRender();
	} while (bVar2 != false);
	//EndLoadStageOne();
	return;
}

void LoadAndPlayCutscene(void)
{
	LargeObject* pLVar1;

	/* Cutscenes are played in here */
	LoadLevel();
	//pLVar1 = g_LargeObject_006db450;
	//LoadA();
	//LoadB(pLVar1);
	return;
}


char* sz_EdenFileFormat_0042b848 = "eden%d_%c";
char* sz_AtariFileFormat_0042b858 = "atari_%c";

int main(int argc,char **argv)
{
	char videoModeSpecifier;
	uint cutsceneID;
	char atariFileName[16];
	char edenFileName[24];
	sceCdCLOCK clockTime;

	/* arcg = %d\nargv = %s\n */
	printf("arcg = %d\nargv = %s\n", argc, *argv);
	SetupGame(argc, argv);
	if (g_LevelScheduleManager_00449728->nextLevelID == 0xe) {
		videoModeSpecifier = 'n';
		if (g_VideoMode2 == 3) {
			videoModeSpecifier = 'p';
		}

#ifdef PLATFORM_PS2
		sceCdReadClock(&clockTime);
#endif
		sceScfGetLocalTimefromRTC(&clockTime);
		cutsceneID = clockTime.second & 3;
		if ((false) && ((clockTime.second & 3) != 0)) {
			cutsceneID = cutsceneID - 4;
		}

		/* Generate the file for playing the eden cutscene. This is randomly selected based in the format: eden%d_%c. C is the video format (PAL or NTSC) and the byte is the random cutscene to play
		   between 0-4. */
		sprintf(edenFileName, sz_EdenFileFormat_0042b848, cutsceneID + 1, videoModeSpecifier);
		/* atari_%c */
		sprintf(atariFileName, sz_AtariFileFormat_0042b858, videoModeSpecifier);
		ShowCompanySplashScreen(atariFileName, 0, 0);
		ShowCompanySplashScreen(edenFileName, 0, 0);
	}
	do {
		LoadAndPlayCutscene();
	//	GameLoop();
	//	LoadGame();
	} while ((g_DebugCameraFlag_00448ea4 & 1) == 0);
	//MainCleanup();
	return 0;
}

