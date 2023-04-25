#include "Iop.h"
#include <stdlib.h>
#include <stdio.h>

#define USE_IOP_300

extern EFileLoadMode g_FileLoadMode_00448810;

IopPaths IopPaths_00496aa0;

IopManager IopManager_004497c4;
IopManager* PTR_IopManager_00448cf8 = &IopManager_004497c4;

char g_IopPath[8] = "\\IOP\\";
#if defined(USE_IOP_300)
char g_ImagePath[16] = "IOPRP300.IMG";
#else
char g_ImagePath[16] = "IOPRP271.IMG";
#endif
char g_ModulesName[16] = "modules.bin";

char g_sdName[16] = "libsd.irx";
char g_sdrdrvName[16] = "sdrdrv.irx";
char g_sio2manName[16] = "sio2man.irx";
char g_usbdName[16] = "usbd.irx";
char g_sio2dName[16] = "sio2d.irx";
char g_dbcName[16] = "dbcman.irx";
char g_ds2Name[16] = "ds2o_d.irx";
char g_mcmanName[16] = "mcman.irx";
char g_mcservName[16] = "mcserv.irx";
char g_modmidiName[16] = "modmidi.irx";
char g_modhsynName[16] = "modhsyn.irx";
char g_modcdvdstmName[16] = "cdvdstm.irx";

edPsx2Module g_Modules[12] = 
{ 
	{g_sdName, 0, 0, "" },
	{g_sdrdrvName, 1, 0, "" },
	{g_sio2manName, 2, 0, "" },
	{g_usbdName, 3, 0, "" },
	{g_sio2dName, 4, 0, "" },
	{g_dbcName, 5, 0, "" },
	{g_ds2Name, 6, 0, "" },
	{g_mcmanName, 7, 0, "" },
	{g_mcservName, 8, 0, "" },
	{g_modmidiName, 9, 0, "" },
	{g_modhsynName, 10, 0, "" },
	{g_modcdvdstmName, 11, 0, "" },
};

#define LOAD_INVIDIDUAL_MODULES


void IopManager::SetupIopPaths(InputSetupParams* pSetupParams)
{
	pSetupParams->pIopPaths = &IopPaths_00496aa0;
	pSetupParams->field_0x20 = 0;
	IopPaths_00496aa0.pImgName = g_ImagePath;
	IopPaths_00496aa0.pIopRoot = g_IopPath;
#if defined(LOAD_INVIDIDUAL_MODULES)
	IopPaths_00496aa0.pModulesName = (char*)g_Modules;
	IopPaths_00496aa0.modulesCount = sizeof(g_Modules) / sizeof(edPsx2Module);
#else
	IopPaths_00496aa0.pModulesName = g_ModulesName;
	IopPaths_00496aa0.modulesCount = 0;
#endif
	pSetupParams->fileLoadMode = g_FileLoadMode_00448810;
	return;
}

bool IopManager::GetAnyControllerConnected()
{
	return disconnectedController != 0;
}
