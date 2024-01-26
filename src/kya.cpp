#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include "kya.h"

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
#include <thread>
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
#include "TranslatedTextData.h"
#include "edVideo/VideoA.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoD.h"
#include "PauseManager.h"

#include "Rendering/CustomShell.h"
#include "InputManager.h"

#include <assert.h>
#include "edStr.h"
#include "Rendering/edCTextFont.h"
#include "BootData.h"
#include "SaveManagement.h"
#include "EventManager.h"

template<class T>
T* CreateNew()
{
	return new T;
}

InputSetupParams g_InputSetupParams = { 0, 0, 0, 0x8C };

InputSetupParams* edSysGetConfig(void)
{
	return &g_InputSetupParams;
}

EFileLoadMode GetFileLoadMode_00424d9c(void)
{
	return g_InputSetupParams.fileLoadMode;
}

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
		edStrCat(g_edPsx2ModulePath, "");
		edStrCat(g_edPsx2ModulePath, path);
	}
	else {
		if (EVar2 == FLM_CD_DVD) {
			strcpy(g_edPsx2ModulePath, "cdrom0:");
			if ((*path != '\\') && (*path != '/')) {
				edStrCat(g_edPsx2ModulePath, "\\");
			}
			edStrCat(g_edPsx2ModulePath, path);
		}
		else {
			//#Unimplemented
			//if (EVar2 == New_Name_(1)) {
			//	g_edPsx2ModulePath._0_7_ = CONCAT16(s_host0:_00433148[6], CONCAT24(s_host0 : _00433148._4_2_, s_host0 : _00433148._0_4_));
			//	g_edPsx2ModulePath = g_edPsx2ModulePath & 0xff00000000000000 | (ulong)g_edPsx2ModulePath._0_7_;
			//	if ((*param_1 == '\\') || (*param_1 == '/')) {
			//		param_1 = param_1 + 1;
			//	}
			//	edStrCat(g_edPsx2ModulePath, param_1);
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
	edDebugPrintf("edPsx2Module.path = %s\n", g_edPsx2ModulePath);
	return;
}

void StartCDDVDInit(void)
{
#if defined(PLATFORM_PS2)
	int iVar1;

	/* CDVD Drive Initialization... Please wait...\n */
	edDebugPrintf("CDVD Drive Initialization... Please wait...\n");
	sceSifInitRpc(0);
	iVar1 = sceCdInit(0);
	if (iVar1 == 0) {
		/* CDVD drive initialization failed\n */
		edDebugPrintf("CDVD drive initialization failed\n");
	}
	else {
		iVar1 = sceCdGetDiskType();
		if (iVar1 == 0x14) {
			sceCdMmode(2);
			/* Media type : DVD-ROM\n */
			edDebugPrintf("Media type : DVD-ROM\n");
		}
		else {
			sceCdMmode(1);
			/* Media type : CD-ROM\n */
			edDebugPrintf("Media type : CD-ROM\n");
		}
		sceFsReset();
		sceCdDiskReady(0);
		/* CDVD drive successfully initialized\n */
		edDebugPrintf("CDVD drive successfully initialized\n");
	}
#endif
	return;
}

void InitIopAndRPC_00291de0(void)
{
	int iVar1;

#if defined(PLATFORM_PS2)
	sceSifInitRpc(0);
	edDebugPrintf("RPC initialized\n");
	iVar1 = sceSifInitIopHeap();
	if (iVar1 == 0) {
		edDebugPrintf("IOP heap initialized\n");
	}
	else {
		edDebugPrintf("IOP heap initialization failed\n");
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
		edStrCat(stringParameter, __src);
		do {
			iVar3 = sceSifRebootIop(stringParameter);
		} while (iVar3 == 0);
		do {
			iVar3 = sceSifSyncIop();
		} while (iVar3 == 0);
		/* CD boot module %s successfully loaded\n */
		edDebugPrintf("CD boot module %s successfully loaded\n", __src);
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

	pIVar2 = edSysGetConfig();
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
		edDebugPrintf("module binary file : %s error: %d\n", g_edPsx2ModulePath, errorC);
	}
	else {
		nbyte = sceLseek(iVar4, 0, 2);
		sceLseek(iVar4, 0, 0);
		buf = (byte*)edMemAllocAlign(TO_HEAP(H_MAIN), nbyte, 0x40);
		sceRead(iVar4, buf, nbyte);
		sceClose(iVar4);
		pModule = (edModule*)(buf + 4);
		edDebugPrintf("Binary modules Sony Libs version : %d.%d\n", (ulong)*buf, (ulong)buf[1]);
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
			edDebugPrintf("Module %s (id = %02d) :\n", pModule->name, pModule->id);
			bVar1 = IsModuleLoaded_002921b0(pModule->id);
			if (bVar1 == false) {
				pvVar5 = sceSifAllocSysMemory(1, pModule->size, (void*)0x0);
				if (pvVar5 == (void*)0x0) {
					edDebugPrintf("-> module %s (%d)\n", pModule->name, pvVar5);
				}
				local_10.size = pModule->size;
				local_10.mode = 0;
				local_10.data = (uint)paVar9;
				local_10.addr = (uint)pvVar5;
				sceSifSetDma(&local_10, 1);
				if (pModule->args != 0) {
					edDebugPrintf("command line : %s", pModule->commandLine);
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
					edDebugPrintf("-> module %s (%d), err : %d\n", pModule->name, pModule->id, result);
				}
			}
			else {
				edDebugPrintf("edPsx2ModuleLoad : hand-load your IOP module NOW then press F5 to continue\n");
				//Break();
			}
			//FUN_0022fa30(uVar5);
			edDebugPrintf("-> successfully loaded\n");
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

	pIVar2 = edSysGetConfig();
	if (pIVar2->field_0x20 != (char*)0x0) {
		ToUppercase(pIVar2->field_0x20);
	}
	for (; count != 0; count = count + -1) {
		edDebugPrintf("Module %s (id = %02d) :\n", aModules->name, aModules->id);
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
			edDebugPrintf("\tmodule full path/name : %s\n", g_edPsx2ModulePath);
			pcVar8 = (char*)0x0;
			if (aModules->cmdLine != (char*)0x0) {
				edDebugPrintf("command line : %s", aModules->cmdLine);
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
					edDebugPrintf("** could not load module %s, error : %d (retrying) **\n", aModules->name, -iVar5);
					iVar5 = sceSifLoadModule(g_edPsx2ModulePath, (int)pcVar8, aModules->cmdLine);
				}
				if ((uVar7 < 0xb) || (-1 < iVar5)) {
					edDebugPrintf("-> successfully loaded\n");
				}
			}
			else {
				edDebugPrintf("edPsx2ModuleLoad : hand-load your IOP module NOW then press F5 to continue\n");
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



void _edSystemInitSpecific(void)
{
	IopPaths* pIopPaths;
	InputSetupParams* pIVar1;
	uint uVar2;
	undefined4 uVar3;

	pIVar1 = edSysGetConfig();
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
	//edDebugPrintf(s_IOP_available_memory_:_Total_:_ % _00432fb0, uVar2, uVar3);
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
			edDebugPrintf("[EE] edDev : libpad2 init went wrong.\n");
			success = -1;
		}
	}
	else {
		/* [EE] edDev : libdbc init went wrong.\n */
		edDebugPrintf("[EE] edDev : libdbc init went wrong.\n");
		success = -1;
	}
#else
	success = 0;
#endif
	return success;
}

int edTime = 0;
int DAT_00449174 = 0;

void _edSystemTimerReset(void)
{
	DAT_00449174 = 0;
	return;
}

void _edSystemSetTimer(int a, void(pFunc)(void))
{
	return;
}

void edTimerInit(void)
{
	edTime = 0;
	_edSystemTimerReset();
	//_edSystemSetTimer(edSysConfiguration.field_0x14, edTimerDefault);
	//EdSysTimerNodeFunc_00290f50(&edSysHandlersNodeParent_00489100, 6, TimerFunc_00290e80, 1, 0);
	return;
}

void edSysInit(void)
{
	_edSystemInit(g_InputSetupParams.argc, g_InputSetupParams.argv);
	edMemInit((short)g_InputSetupParams.headerCount);
	edSysHandlersInit(g_InputSetupParams.handlersSize);
	edTimerInit();
	_edSystemInitSpecific();

	SetupLibdcbAndLibpad(); // HACK
	return;
}

EFileLoadMode g_FileLoadMode_00448810 = FLM_CD_DVD;

char* LoadFileFromDisk(char* fileName, uint* outSize)
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
		edStrCat(filePath, fileName);
		edStrCat(filePath, ";1");
		scePrintf("Loading ini %s\n", filePath);
		ret = sceCdSearchFile(&inFile, filePath);
		scePrintf("Loading ini %d | %d\n", ret, sceCdGetError());
		if (ret != 0) {
			strcpy(cdPath, g_szCdRomPrefix_00438548);
			edStrCat(cdPath, filePath);
			iVar1 = sceOpen(cdPath, 1);
			if (-1 < iVar1) {
				size = sceLseek(iVar1, 0, 2);
				if (outSize != (uint*)0x0) {
					*outSize = size;
				}
				unaff_s3_lo = (char*)edMemAlloc(TO_HEAP(H_MAIN), (long)(int)size);
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
		edStrCat(filePath, fileName);
		iVar1 = sceOpen(filePath, 1);
		if (-1 < iVar1) {
			size = sceLseek(iVar1, 0, 2);
			if (outSize != (uint*)0x0) {
				*outSize = size;
			}
			unaff_s3_lo = (char*)edMemAlloc(TO_HEAP(H_MAIN), (long)(int)size);
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

		unaff_s3_lo = (char*)edMemAlloc(TO_HEAP(H_MAIN), (long)(int)size);
		if (unaff_s3_lo != (char*)0x0) {
			fseek(fp, 0, SEEK_SET);
			fread(unaff_s3_lo, 1, size, fp);
		}
		fclose(fp);
	}
#endif

	return unaff_s3_lo;
}

char* g_szIni_0042b5b8 = "BWITCH.INI";
char* s_Video_0042b490 = "Video";

GlobalVideoConfig gVideoConfig = {};

void VideoReadConfig(IniFile* file)
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
		videoMode = SCE_GS_PAL;
		/* Video - SetVideoMode */
		bVar1 = g_IniFile_00450750.ReadString_001aa520(s_Video_0042b490, "SetVideoMode", outVideoMode);
		if ((bVar1 != false) && (iVar2 = edStrCmp(outVideoMode, "NTSC"), iVar2 == 0)) {
			screenHeight = 0x1c0;
			isNTSC = 0;
			videoMode = SCE_GS_NTSC;
		}
		/* Video - SetFrequency */
		videoFrequency = 0;
		bVar1 = g_IniFile_00450750.ReadInt_001a9830(s_Video_0042b490, "SetFrequency", &videoFrequency);
		if ((bVar1 != false) && (videoFrequency == 0x3c)) {
			isNTSC = 0;
		}
		gVideoConfig.screenWidth = 0x200;
		gVideoConfig.field_0x8 = 0;
		gVideoConfig.screenHeight = screenHeight;
		gVideoConfig.isNTSC = isNTSC;
		gVideoConfig.omode = videoMode;
		g_IniFile_00450750.ReadInt_001a9830(s_Video_0042b490, "SetOffsetX", &gVideoConfig.offsetX);
		g_IniFile_00450750.ReadInt_001a9830(s_Video_0042b490, "SetOffsetY", &gVideoConfig.offsetY);
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
	// set GIF tag
	sceGifPkAddGsData(&gifpkt, 0x100000000000800a);
	sceGifPkAddGsData(&gifpkt, 0xe);

	// enable PRIM register
	sceGifPkAddGsData(&gifpkt, SCE_GS_SET_PRMODECONT(1));
	sceGifPkAddGsData(&gifpkt, SCE_GS_PRMODECONT);

	// frame buffer settting
	sceGifPkAddGsData(&gifpkt, 0x100000);
	sceGifPkAddGsData(&gifpkt, SCE_GS_FRAME_1);
	sceGifPkAddGsData(&gifpkt, g_DoubleBuffer.draw0.zbuf1addr | 0x130000000);
	sceGifPkAddGsData(&gifpkt, SCE_GS_ZBUF_1);

	// offset value ( PRIM coord -> WIN coord )
	sceGifPkAddGsData(&gifpkt, SCE_GS_SET_XYOFFSET_1(0, 0));
	sceGifPkAddGsData(&gifpkt, SCE_GS_XYOFFSET_1);

	// scissor settings ( WIN coordinates x0,x1,y0,y1 )
	sceGifPkAddGsData(&gifpkt, 0x400000004000000);
	sceGifPkAddGsData(&gifpkt, SCE_GS_SCISSOR_1);

	// pixel test control
	sceGifPkAddGsData(&gifpkt, 0x30000);
	sceGifPkAddGsData(&gifpkt, SCE_GS_TEST_1);

	// set sprite primitive
	sceGifPkAddGsData(&gifpkt, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0));
	sceGifPkAddGsData(&gifpkt, SCE_GS_PRIM);

	// set RGBA of sprite
	sceGifPkAddGsData(&gifpkt, (a & 0xff) << 0x18 | (b & 0xff) << 0x10 | r & 0xff | (g & 0xff) << 8 | 0x3f80000000000000);
	sceGifPkAddGsData(&gifpkt, SCE_GS_RGBAQ);

	// set upper-left position of sprite
	sceGifPkAddGsData(&gifpkt, SCE_GS_SET_XYZ(0, 0, 0));
	sceGifPkAddGsData(&gifpkt, SCE_GS_XYZF2);

	// set lower-right position of sprite
	sceGifPkAddGsData(&gifpkt, 0x40004000);
	sceGifPkAddGsData(&gifpkt, SCE_GS_XYZF2);
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

	screenHeight = (short)gVideoConfig.screenHeight;
	screenWidth = (short)gVideoConfig.screenWidth;
	/* Start video mode in NTCS and change to PAL based on global */
	videoMode = SCE_GS_NTSC;
	if (gVideoConfig.omode != SCE_GS_NTSC) {
		videoMode = SCE_GS_PAL;
	}
#if defined(PLATFORM_PS2)
	g_DmaChan_00449820 = sceDmaGetChan(2);
	sceGsResetGraph(0, SCE_GS_INTERLACE, videoMode, SCE_GS_FIELD);
#endif
	GraphicsClear_004059a0(0xff, 0xff, 0, 0);
#if defined(PLATFORM_PS2)
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
	edDmaFlushCache();
	RENDER_LOG("DMA Begin SplashFunc_002ba880\n");
	edDmaSend(SHELLDMA_CHANNEL_GIF, (uint)packet.pBuffer);
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

	edDmaSyncPath();
	auStack32.field_0x4 = &local_8;
	local_8.field_0x0 = 0;
	local_8.field_0x2 = 0;
	auStack32.field_0x8 = 0;
	local_8.width = (short)gVideoConfig.screenWidth;
	local_8.height = (short)gVideoConfig.screenHeight;
	auStack32.field_0xc = auStack32.field_0x4;
	auStack32.pSplashFile = pSplashFile;
	for (counter = 0; counter < count; counter = counter + 2) {
		edDmaSyncPath();
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
	Renderer::RenderImage(pSplashFile, gVideoConfig.screenWidth, gVideoConfig.screenHeight);
#endif
	return;
}

char* sz_ModeNet_0042b7f8 = "<net>";
char* sz_ModeCdvd_0042b800 = "<cdvd>";
char* sz_ModePc_0042b808 = "<pc>";
char* sz_ModeFullPC_0042b810 = "<pc>0:d:\\Projects\\BWitch\\Psx2\\";

edCdlFolder* PTR_edCdlFolder_00448ef4;

void Init_edFile(void)
{
	bool bVar1;
	InputSetupParams* pIVar2;
	undefined4 uVar3;
	int local_4;

	pIVar2 = edSysGetConfig();
	//uVar3 = edFileGetConfig();
	//uVar3->field_0x0 = g_ProfileModeSetup_00448edc != 0;
	edFileInit();
	if (g_FileLoadMode_00448810 == FLM_Net) {
		edFileSetPath(sz_ModeNet_0042b7f8);
	}
	else {
		if (g_FileLoadMode_00448810 == FLM_CD_DVD) {
			edFileSetPath(sz_ModeCdvd_0042b800);
		}
		else {
			edFileSetPath(sz_ModePc_0042b808);
		}
	}
	/* <cdvd> */
	local_4 = 0;
	bVar1 = edFileFilerConfigure(sz_ModeCdvd_0042b800, IM_CALC_SIZE, (void*)0x898, &local_4);
	if (bVar1 != false) {
		PTR_edCdlFolder_00448ef4 = (edCdlFolder*)edMemAlloc(TO_HEAP(H_MAIN), local_4);
		/* <cdvd> */
		edFileFilerConfigure(sz_ModeCdvd_0042b800, IM_INIT, PTR_edCdlFolder_00448ef4, (int*)local_4);
	}
	if (pIVar2->field_0x20 != (char*)0x0) {
		/* <pc>0:d:\\Projects\\BWitch\\Psx2\\ */
		edFileSetPath(sz_ModeFullPC_0042b810);
	}
	return;
}

bool LoadBNK(char* bnkTitle, char* bnkPath)
{
	bool uVar1;
	edCFiler* peVar1;
	char filePath[512];

	/* Param_1: "<BNK>0:" Param_2: "CDEURO/menu/Messages.bnk" */
	peVar1 = edFileGetFiler(filePath, bnkTitle, 1);
	if (peVar1 == (edCFiler*)0x0) {
		uVar1 = false;
	}
	else {
		uVar1 = peVar1->mount_unit(filePath, bnkPath);
	}
	return uVar1;
}

#define WIN_ENGLISH_LANGUAGE 0;

int GetSystemLanguage(void)
{
	int systemLanguageID;
#ifdef PLATFORM_PS2
	systemLanguageID = sceScfGetLanguage();
	switch (systemLanguageID) {
	case SCE_JAPANESE_LANGUAGE:
		systemLanguageID = SCE_JAPANESE_LANGUAGE;
		break;
	case SCE_ENGLISH_LANGUAGE:
		systemLanguageID = SCE_ENGLISH_LANGUAGE;
		break;
	case SCE_FRENCH_LANGUAGE:
		systemLanguageID = SCE_FRENCH_LANGUAGE;
		break;
	case SCE_SPANISH_LANGUAGE:
		systemLanguageID = SCE_SPANISH_LANGUAGE;
		break;
	case SCE_GERMAN_LANGUAGE:
		systemLanguageID = SCE_GERMAN_LANGUAGE;
		break;
	case SCE_ITALIAN_LANGUAGE:
		systemLanguageID = SCE_ITALIAN_LANGUAGE;
		break;
	case SCE_DUTCH_LANGUAGE:
		systemLanguageID = SCE_DUTCH_LANGUAGE;
		break;
	case SCE_PORTUGUESE_LANGUAGE:
		systemLanguageID = SCE_PORTUGUESE_LANGUAGE;
		break;
	default:
		systemLanguageID = 8;
	}
#else
	systemLanguageID = WIN_ENGLISH_LANGUAGE;
#endif
	return systemLanguageID;
}

void SetLanguageID_00336b40(void)
{
	int systemLanguageID;

	if (gVideoConfig.omode == SCE_GS_PAL) {
		systemLanguageID = GetSystemLanguage();
		switch (systemLanguageID) {
		default:
			CMessageFile::sm_default_language = GB;
			break;
		case 2:
			CMessageFile::sm_default_language = FR;
			break;
		case 3:
			CMessageFile::sm_default_language = SP;
			break;
		case 4:
			CMessageFile::sm_default_language = GE;
			break;
		case 5:
			CMessageFile::sm_default_language = IT;
		}
	}
	else {
		CMessageFile::sm_default_language = GB;
	}
	return;
}

char* sz_BNK_Messages_Drive = "<BNK>0:";
char* sz_BNK_Messages_Path = "CDEURO/menu/Messages.bnk";

void MainInit(int argc,char **argv)
{
	InputSetupParams *pIVar1;
	char* pFileBuffer;
	uint fileSize;
	
	printf("----------------- Main Init begin -----------\n");
	printf("-------- eden lib init --------------\n");
	printf("---- Init edSys \n");
	
	pIVar1 = edSysGetConfig();
	pIVar1->argc = argc;
	pIVar1->argv = argv;
	pIVar1->field_0x14 = 10000;
	pIVar1->headerCount = 0x1000;

	gCompatibilityHandlingPtr->SetupIopPaths(pIVar1);

	edSysInit();

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
		VideoReadConfig(&g_IniFile_00450750);
	}

	SetupDoubleBuffer_00405ba0();

	/* If using NTSC then load SPLASH_N, otherwise load SPLASH_P */
	if (gVideoConfig.omode == SCE_GS_NTSC) {
		pFileBuffer = LoadFileFromDisk("CDEURO\\FRONTEND\\SPLASH_N.RAW", (uint*)0x0);
	}
	else {
		pFileBuffer = LoadFileFromDisk("CDEURO\\FRONTEND\\SPLASH_P.RAW", (uint*)0x0);
	}
	if (pFileBuffer != (char*)0x0) {
		SetupSplash(pFileBuffer, 2);
		edMemFree(pFileBuffer);
	}

	//edDebugPrintf(s_----_Init_edDebug_0042b610);
	//edDebugInit();
	//edDebugPrintf(s_----_Init_edCluster_0042b630);
	//edClusterConfig = edClusterGetConfig();
	//edClusterConfig->field_0x0 = 6;
	//edClusterInit();
	//ProfileInit(0);
	gCompatibilityHandlingPtr->IOPFunc_0x10(0);
	edDebugPrintf("---- Init edDlist \n");
	//edDListConfig_ = edDListGetConfig();
	//edDListConfig_->field_0x0 = 0x14;
	//edDListConfig_->field_0xc = edDListConfig_->field_0xc * 3;
	//edDListConfig_->field_0x10 = 500;
	edDListInit();
	edDebugPrintf("---- Init ed3D \n");
	Init3D();
	//edDebugPrintf(s_----_Init_edFile_0042b690);
	Init_edFile();
	//puVar2 = edTextGetConfig();
	//puVar2->field_0x4 = 0x26;
	edTextInit();
	//edDebugPrintf(s_----_Init_edMath_0042b6b0);
	//edMathInit();
	//edDebugPrintf(s_----_Init_edDev_0042b6d0);
	//edDevInit();
	///* Should we init sound and music? */
	//if (DAT_00448ef0 == 0) {
	//	edDebugPrintf(s_----_Init_edMusic_0042b6f0); 6f0);
	//	musicConfig = edMusicGetConfig();
	//	uVar2 = edSoundVoicesNumberGet();
	//	if (uVar2 < 0x30) {
	//		musicConfig->field_0x0 = uVar2 >> 1;
	//	}
	//	else {
	//		musicConfig->field_0x0 = 0x18;
	//	}
	//	musicConfig->field_0x4 = 10;
	//	musicConfig->field_0x8 = 0xf;
	//	musicConfig->field_0xc = 5;
	//	edMusic::Init();
	//	edDebugPrintf(s_----_Init_Sound_0042b710); 0);
	//	soundConfig = edSoundGetConfig();
	//	soundConfig->field_0xc = musicConfig->field_0x0;
	//	soundConfig->field_0x10 = uVar2 - 1;
	//	soundConfig->field_0x0 = 0x80;
	//	if (gVideoConfig.omode == SCE_GS_PAL) {
	//		soundConfig->field_0x14 = 0x32;
	//	}
	//	else {
	//		soundConfig->field_0x14 = 0x3c;
	//	}
	//	edMusicInit();
	//	edSoundInit((float)&DAT_3f4ccccd);
	//}
	edEventInit();
	/* Init_edBank */
	edDebugPrintf("---- Init edBank \n");
	edBankInit();
	///* game_init */
	//edDebugPrintf(s_--------_game_init_------------ - _0042b750);
	//edDebugPrintf(s_--------------------------------_0042b780);
	//Game_Init();
	///* <BNK>0: CDEURO/menu/Messages.bnk */
	LoadBNK(sz_BNK_Messages_Drive, sz_BNK_Messages_Path);
	SetLanguageID_00336b40();
	InstallBootData();
	/* Init_edVideo */
	edDebugPrintf("---- Init edVideo \n");
	edVideoConfig* pVVar4 = edVideoGetConfig();
	pVVar4->field_0x1 = 1;
	pVVar4->field_0x4 = 0;
	edVideoInit();
	SetVideoMode((short)gVideoConfig.omode, (short)gVideoConfig.screenWidth, (short)gVideoConfig.screenHeight, (short)gVideoConfig.field_0x8, 0);
	//CheckControllers(&g_IniFile_00450750);
	CScene::CreateScene();
	Game_Init();

#ifdef PLATFORM_WIN
	Renderer::WaitUntilReady();
#endif

	///* May jump to 003965B8 */
	bool bVar1 = gCompatibilityHandlingPtr->GetAnyControllerConnected();
	/* This doesn't seem to trigger on main run. */
	if (bVar1 != false) {
		IMPLEMENTATION_GUARD();
		//DrawPopup_0034e1f0(8, 0, 0x52525f503700080c, 0x171d0d0b190f111a, 0);
	}
	SaveManagementBootCheck();
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

	display->videoIsPAL = gVideoConfig.omode == SCE_GS_PAL;
	display->fbp0 = 0;
	display->fileReadSuccess = 0;
	//UpdateSoundManager();
	//SetPerfFlag_0023e2b0();
#ifdef PLATFORM_PS2
	DisableIntc(INTC_VBLANK_S);
	DisableIntc(INTC_GS);

	uint workArea = 0x16976c + 0x1000;

	puVar2 = (u_char*)edMemAllocAlign(TO_HEAP(H_MAIN), workArea, 0x40);
	(display->pssResources).mpegBuff = puVar2;
	(display->pssResources).mpegBuffSize = workArea;
	psVar3 = (sceIpuRGB32*)edMemAllocAlign(TO_HEAP(H_MAIN), 0x140000, 0x40);
	(display->pssResources).rgb32 = psVar3;
	puVar4 = (u_int*)edMemAllocAlign(TO_HEAP(H_MAIN), 0x3c200, 0x40);
	(display->pssResources).path3tag = puVar4;
	pvVar5 = edMemAllocAlign(TO_HEAP(H_MAIN), 0x30000, 0x40);
	(display->pssResources).demuxBuff = pvVar5;
	(display->pssResources).demuxBuffSize = 0x30000;
	puVar2 = (u_char*)edMemAllocAlign(TO_HEAP(H_MAIN), 0xc000, 0x40);
	(display->pssResources).audioBuff = puVar2;
	(display->pssResources).audioBuffSize = 0xc000;

	pvVar5 = sceSifAllocIopHeap(0x6000);
	(display->pssResources).iopBuff = (int)pvVar5;
	(display->pssResources).iopBuffSize = 0x6000;
	puVar2 = (u_char*)edMemAllocAlign(TO_HEAP(H_MAIN), ZERO_BUFF_SIZE, 0x40);
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

	get_physical_filename(fileName, inFileName);

#ifdef PLATFORM_WIN
	char* pcVar6 = edFilePathGetFilePath(fileName);

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
	EnableIntc(INTC_VBLANK_S);
	EnableIntc(INTC_GS);
	edVideoSetAttribute((ed_video_attr*)0x0);
	edVideoSetBackgroundColor(0, 0, 0);
	edVideoSetOffset((short)gVideoConfig.offsetX, (short)gVideoConfig.offsetY);
	edVideoFlip();
#endif
	return;
}

void ShowCompanySplashScreen(char* file_name, bool param_2, bool param_3)
{
	byte bVar1;
	Timer* inTimeController;
	VideoFile* display;
	uint uVar2;
	int iVar3;
	float fVar4;
	char file_path[512];

#ifdef SKIP_MOVIES
	return;
#endif

	inTimeController = GetTimer();
	display = new(VideoFile);
	/* file_path = CDEURO/movies/ + file_name + .pss */
	edStrCatMulti(file_path, "CDEURO/movies/", file_name, ".pss", 0);
	LoadVideoFromFilePath(display, file_path);

	do {
		//PlayerInput::Update(inTimeController->cutsceneDeltaTime);
		inTimeController->Update();
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
				Renderer::RenderImage(image + 0x10, 0x280, 0x200);
			}
#endif
		}
		if (param_2 != false) {
			if (param_3 == false) {
				uVar2 = 1;
				if (((gPlayerInput.pressedBitfield & 0x40000) == 0) && ((gPlayerInput.pressedBitfield & 0x1000000) == 0)) {
					uVar2 = 0;
				}
			}
			else {
				uVar2 = gPlayerInput.pressedBitfield & 0x40000;
			}
			if (uVar2 != 0) break;
		}
		iVar3 = 1;
		if (display->fileReadSuccess != 0) {
			iVar3 = hasVideoEnded();
		}
	} while (iVar3 == 0);
	DestroyVideoFile(display);
	if (display != (VideoFile*)0x0) {
		free(display);
	}
	return;
}


char* PTR_ARRAY_ARRAY_00427ac0[4][2] = {
	{"teas_nt", "teas_nt"},
	{"teas_pt", "teas_pt"},
	{"teasa_nt", "teasa_nt"},
	{"teasa_pt", "teasa_pt"}
};


char* PTR_IntroVideoTitleStart[2][2] = {
	{"int_nw", "int_nt"},
	{"int_pw", "int_pt"}
};

char* s_ufgsi_004381a8 = "ufgsi";
char* s_SubtitleSuffix = "s";

float FLOAT_004837b0 = 0.5f;
float FUN_00284900(void)
{
	return FLOAT_004837b0;
}

char* szMoviesPath = "CDEURO/movies/";

void PlayIntroVideo(long mode)
{
	undefined4 uVar1;
	bool bVar2;
	Timer* pTVar3;
	int currentLanguage;
	VideoFile* currentLanguage_videoPointer;
	uint WTVersion;
	uint uVar4;
	float fVar5;
	char mysteryFilePathA[512];
	char teaserFilePath[512];
	char promoFilePath[512];
	char introFilePath[512];
	char mysteryFilePathB[512];
	char introFileName[128];
	char* fileName;
	undefined* promoFileName;

	uVar4 = (uint)(gVideoConfig.omode == 3);
	WTVersion = (uint)(gSettings.bWidescreen == 0);
	if (mode == 0) {
		if (LevelScheduleManager::gThis->nextLevelID == 1) {
			bVar2 = false; // LevelScheduleManager::GetFirstRunOrSave(LevelScheduleManager::gThis, 1);
			if (bVar2 == false) {
				if (uVar4 == 1) {
					WTVersion = 1;
				}
				currentLanguage = CMessageFile::get_default_language();
				/* Format is &s%c
				   First param is = "int_nw"
				   Second param is "ufgsi" (US, French, German, Spanish, Italian) */
				sprintf(introFileName, "%s%c", PTR_IntroVideoTitleStart[uVar4][WTVersion], s_ufgsi_004381a8[currentLanguage]);
				if (gSettings.field_0x8 != false) {
					/* If we have subtitles on, append an 's' */
					strcat(introFileName, s_SubtitleSuffix);
				}
				pTVar3 = GetTimer();
				currentLanguage_videoPointer = new(VideoFile);
				/* CDEURO/movies/ */
				edStrCatMulti(introFilePath, szMoviesPath, introFileName, ".pss", NULL);
				LoadVideoFromFilePath(currentLanguage_videoPointer, introFilePath);
				do {
					//PlayerInput::Update(pTVar3->cutsceneDeltaTime);
					pTVar3->Update();
					if (currentLanguage_videoPointer->fileReadSuccess != 0) {
						fVar5 = FUN_00284900();
#ifdef PLATFORM_PS2
						continuePss(&currentLanguage_videoPointer->pssResources, &currentLanguage_videoPointer->dispenv,
							(uint)currentLanguage_videoPointer->videoIsPAL); // , fVar5);
#else
						char* image = continuePss();

						if (image) {
							Renderer::RenderImage(image + 0x10, 0x280, 0x200);
						}
#endif
					}
					if (((uint)gPlayerInput.pressedBitfield & 0x40000) != 0) break;
					currentLanguage = 1;
					if (currentLanguage_videoPointer->fileReadSuccess != 0) {
						currentLanguage = hasVideoEnded();
					}
				} while (currentLanguage == 0);
				DestroyVideoFile(currentLanguage_videoPointer);
				if (currentLanguage_videoPointer != (VideoFile*)0x0) {
					Free(currentLanguage_videoPointer);
				}
			}
		}
		else {
			if ((LevelScheduleManager::gThis->nextLevelID == 0xe) &&
				//(currentLanguage = LevelScheduleManager::GetParamFromDat004253d4(1), currentLanguage == 0xe)) {
				(currentLanguage = 0, currentLanguage == 0xe)) {
				/* Fail the above check on normal run, then return to end of function. */
				if ((GameFlags & 0x1000) == 0) {
					if ((GameFlags & 0x2000) == 0) {
						fileName = PTR_ARRAY_ARRAY_00427ac0[uVar4][WTVersion];
						pTVar3 = GetTimer();
						currentLanguage_videoPointer = new(VideoFile);
						edStrCatMulti(mysteryFilePathA, szMoviesPath, fileName, ".pss", "\0");
						LoadVideoFromFilePath(currentLanguage_videoPointer, mysteryFilePathA);
						do {
							//PlayerInput::Update(pTVar3->cutsceneDeltaTime);
							pTVar3->Update();
							if (currentLanguage_videoPointer->fileReadSuccess != 0) {
								fVar5 = FUN_00284900();
#ifdef PLATFORM_PS2
								continuePss(&currentLanguage_videoPointer->pssResources, &currentLanguage_videoPointer->dispenv,
									(uint)currentLanguage_videoPointer->videoIsPAL); //, fVar5);
#else
								char* image = continuePss();

								if (image) {
									Renderer::RenderImage(image + 0x10, 0x280, 0x200);
								}
#endif
							}
							bVar2 = true;
							if ((((uint)gPlayerInput.pressedBitfield & 0x40000) == 0) &&
								(((uint)gPlayerInput.pressedBitfield & 0x1000000) == 0)) {
								bVar2 = false;
							}
							if (bVar2) break;
							currentLanguage = 1;
							if (currentLanguage_videoPointer->fileReadSuccess != 0) {
								currentLanguage = hasVideoEnded();
							}
						} while (currentLanguage == 0);
						DestroyVideoFile(currentLanguage_videoPointer);
						if (currentLanguage_videoPointer != (VideoFile*)0x0) {
							Free(currentLanguage_videoPointer);
						}
					}
					else {
						/* teasa_nt */
						uVar1 = 0; // *(undefined4*)(WTVersion * 4 + uVar4 * 8 + 0x427ad0);
						pTVar3 = GetTimer();
						currentLanguage_videoPointer = new(VideoFile);
						edStrCatMulti(teaserFilePath, szMoviesPath, fileName, ".pss", "\0");
						LoadVideoFromFilePath(currentLanguage_videoPointer, teaserFilePath);
						do {
							//PlayerInput::Update(pTVar3->cutsceneDeltaTime);
							pTVar3->Update();
							if (currentLanguage_videoPointer->fileReadSuccess != 0) {
								fVar5 = FUN_00284900();
#ifdef PLATFORM_PS2
								continuePss(&currentLanguage_videoPointer->pssResources, &currentLanguage_videoPointer->dispenv,
									(uint)currentLanguage_videoPointer->videoIsPAL); //, fVar5);
#else
								char* image = continuePss();

								if (image) {
									Renderer::RenderImage(image + 0x10, 0x280, 0x200);
								}
#endif
							}
							bVar2 = true;
							if ((((uint)gPlayerInput.pressedBitfield & 0x40000) == 0) &&
								(((uint)gPlayerInput.pressedBitfield & 0x1000000) == 0)) {
								bVar2 = false;
							}
							if (bVar2) break;
							currentLanguage = 1;
							if (currentLanguage_videoPointer->fileReadSuccess != 0) {
								currentLanguage = hasVideoEnded();
							}
						} while (currentLanguage == 0);
						DestroyVideoFile(currentLanguage_videoPointer);
						if (currentLanguage_videoPointer != (VideoFile*)0x0) {
							Free(currentLanguage_videoPointer);
						}
					}
				}
				else {
					/* promo_nt */
					promoFileName = NULL; // (&PTR_PromoVideoTitleStart)[uVar4 * 2 + WTVersion];
					pTVar3 = GetTimer();
					currentLanguage_videoPointer = new(VideoFile);
					edStrCatMulti(promoFilePath, szMoviesPath, fileName, ".pss", "\0");
					LoadVideoFromFilePath(currentLanguage_videoPointer, promoFilePath);
					do {
						//PlayerInput::Update(pTVar3->cutsceneDeltaTime);
						pTVar3->Update();
						if (currentLanguage_videoPointer->fileReadSuccess != 0) {
							fVar5 = FUN_00284900();
#ifdef PLATFORM_PS2
							continuePss(&currentLanguage_videoPointer->pssResources, &currentLanguage_videoPointer->dispenv,
								(uint)currentLanguage_videoPointer->videoIsPAL); //, fVar5);
#else
							char* image = continuePss();

							if (image) {
								Renderer::RenderImage(image + 0x10, 0x280, 0x200);
							}
#endif
						}
						bVar2 = true;
						if ((((uint)gPlayerInput.pressedBitfield & 0x40000) == 0) &&
							(((uint)gPlayerInput.pressedBitfield & 0x1000000) == 0)) {
							bVar2 = false;
						}
						if (bVar2) break;
						currentLanguage = 1;
						if (currentLanguage_videoPointer->fileReadSuccess != 0) {
							currentLanguage = hasVideoEnded();
						}
					} while (currentLanguage == 0);
					DestroyVideoFile(currentLanguage_videoPointer);
					if (currentLanguage_videoPointer != (VideoFile*)0x0) {
						Free(currentLanguage_videoPointer);
					}
					GameFlags = GameFlags & 0xffffefff;
				}
			}
		}
	}
	else {
		fileName = PTR_ARRAY_ARRAY_00427ac0[uVar4][WTVersion];
		pTVar3 = GetTimer();
		currentLanguage_videoPointer = new(VideoFile);
		/* CDEURO/movies/ */
		edStrCatMulti(mysteryFilePathB, szMoviesPath, fileName, ".pss", "\0");
		LoadVideoFromFilePath(currentLanguage_videoPointer, mysteryFilePathB);
		do {
			//PlayerInput::Update(pTVar3->cutsceneDeltaTime);
			pTVar3->Update();
			if (currentLanguage_videoPointer->fileReadSuccess != 0) {
				fVar5 = FUN_00284900();
#ifdef PLATFORM_PS2
				continuePss(&currentLanguage_videoPointer->pssResources, &currentLanguage_videoPointer->dispenv,
					(uint)currentLanguage_videoPointer->videoIsPAL); //, fVar5);
#else
				char* image = continuePss();

				if (image) {
					Renderer::RenderImage(image + 0x10, 0x280, 0x200);
				}
#endif
			}
			bVar2 = true;
			if ((((uint)gPlayerInput.pressedBitfield & 0x40000) == 0) &&
				(((uint)gPlayerInput.pressedBitfield & 0x1000000) == 0)) {
				bVar2 = false;
			}
			if (bVar2) break;
			currentLanguage = 1;
			if (currentLanguage_videoPointer->fileReadSuccess != 0) {
				currentLanguage = hasVideoEnded();
			}
		} while (currentLanguage == 0);
		DestroyVideoFile(currentLanguage_videoPointer);
		if (currentLanguage_videoPointer != (VideoFile*)0x0) {
			Free(currentLanguage_videoPointer);
		}
	}
	return;
}

void LoadingLoop(void)
{
	CScene* pSceneInstance;
	bool bVar2;
	Timer* inTimeController;

	MY_LOG("LoadLevel Begin\n");

	/* These functions just run once */
	IMPLEMENTATION_GUARD_LOG(
	PlayIntroVideo(0);)
	pSceneInstance = CScene::_pinstance;
	inTimeController = GetTimer();
	pSceneInstance->LevelLoading_Begin();
	do {
		/* This is the main loop that plays cutscenes

		   This does not control any cutscene elements */
		//PlayerInput::Update(inTimeController->cutsceneDeltaTime);
		inTimeController->Update();
		/* Play cutscene */
		bVar2 = pSceneInstance->LevelLoading_Manage();
		/* Responsible for drawing the loading screen. */
		pSceneInstance->LevelLoading_Draw();
		//SaveRelated_002f37d0(&SaveDataLoadStruct_0048ee30);
		/* Update rendering */
		edVideoFlip();
#if defined(LOAD_FOREVER)
	} while (1);
#else
	} while (bVar2 != false);
#endif
	pSceneInstance->LevelLoading_End();
	MY_LOG("LoadLevel End\n");
	return;
}

struct APlayer;
APlayer* g_PlayerActor_00448e10 = NULL;

void GameLoop(void)
{
	//ActorState AVar1;
	APlayer* pAVar2;
	CScene* pLVar3;
	bool cVar4;
	bool bVar4;
	Timer* timeController;
	int* piVar5;
	//AnimResult* pAVar6;
	uint uVar7;
	ulong uVar8;
	float fVar9;

	MY_LOG("GameLoop Begin\n");

	pLVar3 = CScene::_pinstance;
	timeController = GetTimer();
	do {
		gCompatibilityHandlingPtr->IOPFunc_0x14(0);
		//PlayerInput::Update(timeController->cutsceneDeltaTime);
		cVar4 = gCompatibilityHandlingPtr->GetAnyControllerConnected();
		bVar4 = gPlayerInput.SoftReset();
		if (bVar4 != false) {
			IMPLEMENTATION_GUARD();
			//Scene::_pinstance->InitiateReset();
		}
		pAVar2 = g_PlayerActor_00448e10;
		if ((GameFlags & 0xc0) == 0) {
			if (g_PlayerActor_00448e10 != (APlayer*)0x0) {
				IMPLEMENTATION_GUARD();
				//piVar5 = (int*)(*(code*)g_PlayerActor_00448e10->pVTable->field_0x138)(g_PlayerActor_00448e10);
				//fVar9 = (float)(**(code**)(*piVar5 + 0x24))();
				//bVar4 = fVar9 - (pAVar2->character).characterBase.field_0x2e0 <= 0.0;
				//if (!bVar4) {
				//	AVar1 = (pAVar2->character).characterBase.actorBase.actorState;
				//	if (AVar1 == AS_None) {
				//		uVar7 = 0;
				//	}
				//	else {
				//		pAVar6 = (*(pAVar2->pVTable->actorBase).getAnimForState)((AActorBase*)pAVar2, AVar1);
				//		uVar7 = pAVar6->flags_0x4 & 1;
				//	}
				//	bVar4 = uVar7 != 0;
				//}
				//if (bVar4) goto LAB_001a7608;
			}
			if (cVar4 != false) {
				if ((GameFlags & 8) != 0) {
					IMPLEMENTATION_GUARD();
					//LoadFrontendWithuRam00451684();
				}
				if ((GameFlags & 0x10) != 0) {
					IMPLEMENTATION_GUARD();
					//OpenMapScreen_003f59b0();
				}
				if ((GameFlags & 0x3c) == 0) {
					if ((GameFlags & 0x800) == 0) {
						IMPLEMENTATION_GUARD();
						//ActivatePause_001b5320(PM_PauseMenu);
					}
					else {
						IMPLEMENTATION_GUARD();
						//ActivatePause_001b5320(PM_MiniGame);
					}
				}
			}
			if ((((GameFlags & 0x8000) == 0) &&
				((gPlayerInput.pressedBitfield & 0x80000) != 0)) &&
				(gPlayerInput.buttonArray[18].floatFieldB == 0.0)) {
				if ((GameFlags & 8) == 0) {
					if ((GameFlags & 0x3c) == 0) {
						IMPLEMENTATION_GUARD();
						//GameLoopFunc_0037a670();
					}
				}
				else {
					IMPLEMENTATION_GUARD();
					//LoadFrontendWithuRam00451684();
				}
			}
			bVar4 = gPlayerInput.buttonArray[6].floatFieldB != 0.0 ||
				gPlayerInput.buttonArray[10].floatFieldB != 0.0;
			if ((gPlayerInput.field_0x58 != 4) || (gPlayerInput.field_0x34 != 4)) {
				bVar4 = true;
			}
			if ((((gPlayerInput.pressedBitfield & 0x8000000) != 0) && (!bVar4)) ||
				((GameFlags & 0x400) != 0)) {
				if (((GameFlags & 0x10) == 0) || ((GameFlags & 0x400) != 0)) {
					if ((GameFlags & 0x3c) == 0) {
						IMPLEMENTATION_GUARD();
						//LoadMaps();
					}
				}
				else {
					IMPLEMENTATION_GUARD();
					//OpenMapScreen_003f59b0();
				}
				GameFlags = GameFlags & 0xfffffbff;
			}
			if (((gPlayerInput.pressedBitfield & 0x40000) != 0) &&
				((gPlayerInput.pressedBitfield & 0x80000) == 0)) {
				if ((GameFlags & 4) == 0) {
					if ((GameFlags & 0x3c) == 0) {
						if ((GameFlags & 0x800) == 0) {
							IMPLEMENTATION_GUARD();
							//ActivatePause_001b5320(PM_PauseMenu);
						}
						else {
							IMPLEMENTATION_GUARD();
							//ActivatePause_001b5320(PM_MiniGame);
						}
					}
				}
				else {
					IMPLEMENTATION_GUARD();
					//uVar8 = FUN_001b5d90();
					//if (uVar8 == 0) {
					//	FUN_001b5110();
					//}
				}
			}
		}
	LAB_001a7608:
		if (((long)(int)GameFlags & 0xffffffff80000000U) != 0) {
			GameFlags = (uint)((ulong)((long)(int)GameFlags << 0x21) >> 0x21);
			IMPLEMENTATION_GUARD();
			//SetPaused_001b8c40(Scene::_pinstance, 1);
		}
		timeController->Update();
		/* This may control cine cutscenes */
		pLVar3->Level_Manage();
		UpdateObjectsMain();
		//SaveRelated_002f37d0(&SaveDataLoadStruct_0048ee30);
		edVideoFlip();
		/* Determine if we should exit the main game loop here. */
		if ((GameFlags & 2) != 0) {
			GameFlags = GameFlags & 0xfffffe7d;
			return;
		}
	} while (true);

	MY_LOG("GameLoop End\n");
}

void LevelInit(void)
{
	CScene* pSceneInstance;

	MY_LOG("LevelInit Begin\n");

	/* Cutscenes are played in here */
	LoadingLoop();
	pSceneInstance = CScene::_pinstance;
	pSceneInstance->Level_Install();
	pSceneInstance->Level_Init();
	MY_LOG("LevelInit End\n");
	return;
}

char* sz_EdenFileFormat_0042b848 = "eden%d_%c";
char* sz_AtariFileFormat_0042b858 = "atari_%c";

#ifdef PLATFORM_PS2
int main(int argc,char **argv)
#else
int main_internal(int argc, char** argv)
#endif
{
	char videoModeSpecifier;
	uint cutsceneID;
	char atariFileName[16];
	char edenFileName[24];
	sceCdCLOCK clockTime;

	/* arcg = %d\nargv = %s\n */
	printf("arcg = %d\nargv = %s\n", argc, *argv);
	MainInit(argc, argv);
	//std::this_thread::sleep_for(std::chrono::duration<double>(10.0));
	if (LevelScheduleManager::gThis->nextLevelID == 0xe) {
		videoModeSpecifier = 'n';
		if (gVideoConfig.omode == SCE_GS_PAL) {
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
		LevelInit();
		GameLoop();
	//	LevelTerm();
	} while ((GameFlags & 1) == 0);
	//MainTerm();
	return 0;
}
