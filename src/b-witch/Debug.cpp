#include "Debug.h"
#include "CameraViewManager.h"
#include "CameraMouseQuake.h"

_eddebug edDebugMenu;

_eddebug_module* edDebugMenuGetCurrentModule(void)
{
	return edDebugMenu.curModule;
}

void _edDebugMenuInit(void)

{
	int iVar1;
	int iVar2;

	//edDebugMenuSetDefaultMaster((_eddebug_module*)&edDebugConfig.field_0x38);
	//memset(_edDebugMenuTitle, 0, 0);
	//edDebugMenuSetActive(1);
	//edDebugMenuForceDisplay(0);
	edDebugMenu.curModule = (_eddebug_module*)0x0;
	//iVar1 = 0;
	//edDebugMenu.DrawDebugTextLineHeight_00468f3c = 0x14;
	//edDebugMenu.DrawDebugTextLocationX_00468f34 = 0x10e;
	//edDebugMenu.DrawDebugTextLocationY_00468f38 = 0x28;
	//edDebugMenu.NumDebugMenuLibraries = 0;
	//edDebugMenu.ShortcutCount_00468f1d = 0;
	//edDebugMenu.Flags_00468f18 = edDebugMenu.Flags_00468f18 | 9;
	//iVar2 = 0;
	//do {
	//	iVar1 = iVar1 + 8;
	//	*(undefined4*)((int)&edDebugShortCuts[0].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[0].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[1].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[1].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[2].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[2].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[3].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[3].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[4].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[4].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[5].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[5].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[6].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[6].FuncPtr + iVar2) = 0;
	//	*(undefined4*)((int)&edDebugShortCuts[7].Number + iVar2) = 0xffffffff;
	//	*(undefined4*)((int)&edDebugShortCuts[7].FuncPtr + iVar2) = 0;
	//	iVar2 = iVar2 + 0x60;
	//} while (iVar1 < 0x40);
	//edDebugAddShortCut(0x2b, _edDebugToggleFilterMode, 1);
	//edDebugMenu.field_0x2e = 0;
	//edDebugMenu.field_0x2f = 0;
	//edDebugMenu.field_0x31 = 0xff;
	edDebugMenu.KeyboardID_00468f24 = 0x80000000;
	//edDebugMenu.field_0x38 = 0;
	//edDebugMenu.field_0x3c = 0;
	//edDebugMenu.GamePadID_00468f30 = 0x80000000;
	return;
}

void edDebugInit(void)
{
	//DAT_00468a40 = 0;
	//edText::RegisterFontToEdDebug = (undefined*)0x0;
	//DAT_00468a4c = 0;
	//edText::RemoveTextHandler_0048f90 = (undefined*)0x0;
	//DAT_00468a48 = 0;
	//edText::DrawText_00448f94 = (undefined*)0x0;
	//DAT_00468a44 = 0;
	//_edDebugTextSetColorFunc = (undefined*)0x0;
	//FLOAT_00468a58 = 1.0;
	//PTR_FUN_00448f9c = (undefined*)0x0;
	//FLOAT_00468a54 = 1.0;
	//FLOAT_00468a50 = 1.0;
	//edDebugRegisterDefaultTextFunctions();
	_edDebugMenuInit();
	//_edDebugTableVarAddInit();
	//edDebugRegisterLibrary(PTR_s_LIB_EDDEBUG_00448920, (undefined*)&_eddebug_module_0041d970, 0, 0);
	return;
}

bool edDebugMenuIsDisplayed(void)
{
	_eddebug_module* p_Var1;

	p_Var1 = edDebugMenuGetCurrentModule();
	return p_Var1 != (_eddebug_module*)0x0;
}

void ToggleCameraHack(void)
{
	CCameraManager* pCameraManager;

	pCameraManager = CScene::ptable.g_CameraManager_0045167c;

	if ((GameFlags & 0x40000000) == 0) {
		GameFlags = GameFlags | 0x40000000;
		pCameraManager->flags = pCameraManager->flags | CAMERA_HACK_ACTIVE;
		pCameraManager->KeepSameParam(pCameraManager->pMouseQuakeCamera_0x4e8, 0x7d);
	}
	else {
		GameFlags = GameFlags & 0xbfffffff;
		pCameraManager->flags = pCameraManager->flags & 0xefffffff;
	}

	return;
}
