#include "CompatibilityHandlingPS2.h"
#include <stdlib.h>
#include <stdio.h>
#include "EdenLib/edDev/Sources/edDevGetChanClick.h"
#include "InputManager.h"

#define USE_IOP_300

extern EFileLoadMode VERSION;

ED_PSX2_MODULES_INIT ModulesInit;

CCompatibilityHandlingPS2 IopManager_004497c4;
CCompatibilityHandling* gCompatibilityHandlingPtr = &IopManager_004497c4;

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


void CCompatibilityHandlingPS2::edSysInit(edSYS_CONFIG* pSetupParams)
{
	pSetupParams->pIopPaths = &ModulesInit;
	pSetupParams->field_0x20 = 0;
	ModulesInit.pImgName = g_ImagePath;
	ModulesInit.pIopRoot = g_IopPath;
#if defined(LOAD_INVIDIDUAL_MODULES)
	ModulesInit.pModulesName = (char*)g_Modules;
	ModulesInit.modulesCount = sizeof(g_Modules) / sizeof(edPsx2Module);
#else
	ModulesInit.pModulesName = g_ModulesName;
	ModulesInit.modulesCount = 0;
#endif
	pSetupParams->version = VERSION;
	return;
}

bool CCompatibilityHandlingPS2::HandleDisconnectedDevices(int area)
{
	return disconnectedController != 0;
}

bool CCompatibilityHandlingPS2::TestCheatCode_CheatMenu()
{
	float fVar1;

	if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
		fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 2);
	}
	else {
		fVar1 = 0.0f;
	}

	if (fVar1 != 0.0f) {
		if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
			fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 0);
		}
		else {
			fVar1 = 0.0f;
		}

		if (fVar1 != 0.0f) {
			if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
				fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 7);
			}
			else {
				fVar1 = 0.0f;
			}

			if (fVar1 != 0.0f) {
				if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
					fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 5);
				}
				else {
					fVar1 = 0.0f;
				}

				if (fVar1 != 0.0f) {
					return true;
				}
			}
		}
	}

	return false;
}

bool CCompatibilityHandlingPS2::TestCheatCode_IdleCamera()
{
	float fVar1;

	if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
		fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 0);
	}
	else {
		fVar1 = 0.0f;
	}

	if (fVar1 != 0.0f) {
		if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
			fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 1);
		}
		else {
			fVar1 = 0.0f;
		}

		if (fVar1 != 0.0f) {
			if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
				fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 0xe);
			}
			else {
				fVar1 = 0.0f;
			}

			if (fVar1 != 0.0f) {
				if ((gPlayerInput.disconnectedController == 0) && (gPlayerInput.bDisconnected == 0)) {
					fVar1 = edDevGetChanClick(gPlayerInput.portIndex, 6);
				}
				else {
					fVar1 = 0.0f;
				}

				if (fVar1 != 0.0f) {
					return true;
				}
			}
		}
	}

	return false;
}

void CCompatibilityHandlingPS2::ActivatePadAutoRepeat()
{
	return;
}

void CCompatibilityHandlingPS2::DisactivatePadAutoRepeat()
{
	return;
}
