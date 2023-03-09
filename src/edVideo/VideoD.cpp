#include "VideoD.h"
#include "VideoB.h"
#include "../edSystem.h"

edVideo_Globals edVideo_Globals_00449590 = { 0 };

void FUN_002b8f30(void)
{
	if (g_ActiveVidParams_0048cd90.params26.omode == 3) {
		//edVideo_Globals_00449590.field_0x4 = &DAT_0059f740;
	}
	else {
		//edVideo_Globals_00449590.field_0x4 = (undefined*)&g_heap_004a5780[0x59b].pStartAddr;
	}
	edVideo_Globals_00449590.pActiveVidParams = &g_ActiveVidParams_0048cd90;
	return;
}

void SetVideoDefaults_002ba440(edVideoData* param_1)
{
	(param_1->params26).screenWidth = 0x200;
	(param_1->params26).screenHeight = 0x200;
	(param_1->params26).vidMode1 = 0;
	(param_1->params26).omode = 3;
	(param_1->params26).isNTSC = 1;
	(param_1->params26).inter = 1;
	(param_1->params26).field_0xb = 1;
	(param_1->params26).field_0xc = 1;
	(param_1->params26).field_0xd = 1;
	(param_1->params26).field_0xe = 1;
	(param_1->params26).field_0xf = 1;
	(param_1->params26).field_0x10[0] = '\x01';
	return;
}

edSysHandlerVideo edSysHandlerVideo_0048cee0;

edSysHandlerVideo::edSysHandlerVideo()
{
	SetVideoDefaults_002ba440(&g_ActiveVidParams_0048cd90);
	nodeParent = &g_SysHandlersNodeTable_00489170;
	maxEventID = 0xb;
	mainIdentifier = 2;
	//SetupEd10_00217720((undefined*)&edSysHandlerVideo_0048cee0, Destructor, &EdFileGlobal_10_0048ced0);
	return;
}

void CallRenderFunc_002ba0f0(void)
{
	VidModeData_30* pVVar1;

	pVVar1 = (g_ActiveVidParams_0048cd90.data30)->pVidModeData_0x0->pLink_0xc;
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 0, pVVar1);
	if (g_ActiveVidParams_0048cd90.params26.field_0xf != 0) {
		(g_ActiveVidParams_0048cd90.data30)->pVidModeData_0x0->pLink_0xc = pVVar1->pLink_0x1c;
	}
	return;
}

void RefreshScreenRender(void)
{
	/* Render scene */
	//edSysHandlersCall(g_SysHandlersMainIdentifier_0048cf14, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4,
	//	g_SysHandlersMainMaxEventID_0048cf10, 6, (void*)0x0);
	//WaitForDraw_00258230();
	//FUN_002b9550();
	/* Render UI */
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 10, (void*)0x0);
	//WaitForDraw_00258230();
	//edSysHandlersCall(g_SysHandlersMainIdentifier_0048cf14, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4,
	//	g_SysHandlersMainMaxEventID_0048cf10, 9, (void*)0x0);
	//WaitSomeTime2? ('\x01');
	//if (g_ActiveVidParams_0048cd90.params26.field_0xf != 0) {
	//	FUN_002ba050();
	//	FUN_002ba0c0();
	//}
	CallRenderFunc_002ba0f0();
	//edSysHandlersCall(g_SysHandlersMainIdentifier_0048cf14, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4,
	//	g_SysHandlersMainMaxEventID_0048cf10, 7, (void*)0x0);
	return;
}

VidModeData_30* GetVidModeData_002ba360(void)
{
	VidModeData_30* pVVar1;

	pVVar1 = (VidModeData_30*)0x0;
	if (g_ActiveVidParams_0048cd90.data30 != (VidModeData_30*)0x0) {
		pVVar1 = g_ActiveVidParams_0048cd90.data30->pVidModeData_0x0->pLink_0xc;
	}
	if ((pVVar1 != (VidModeData_30*)0x0) && (pVVar1->pLink_0x1c != (VidModeData_30*)0x0)) {
		pVVar1 = pVVar1->pLink_0x1c;
	}
	return pVVar1;
}

edVideo_Globals* GetVideoParams_002ba3e0(void)
{
	return &edVideo_Globals_00449590;
}
