#include "LargeObject.h"
#include <stdlib.h>
#include "LevelScheduleManager.h"
#include "PauseManager.h"
#include "CinematicManager.h"
#include "Rendering/DisplayList.h"
#include "LocalizationManager.h"
#include "edVideo/VideoA.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoD.h"
#include "Camera.h"
#include "edDlist.h"
#include "edMem.h"
#include <string.h>
#include <assert.h>
#include "ed3D.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "edVideo/VideoC.h"
#include "TimeController.h"
#include "MemoryStream.h"
#include "FrontendManager.h"
#include "CameraViewManager.h"
#include "FileManager3D.h"
#include "MathOps.h"
#include "SectorManager.h"

LargeObject* g_LargeObject_006db450 = NULL;

ManagerContainer g_ManagerSingletonArray_00451660 = { 0 };

StaticMeshMaster* g_StaticMeshMasterA_00448808 = NULL;
StaticMeshMaster* g_CameraPanMasterB_0044880c = NULL;

StaticMeshMaster* g_CameraPanStaticMasterArray_00451630[10] = { 0 };

uint g_DebugCameraFlag_00448ea4 = 0;

FrameBuffer* AllocateFrameBuffer_002b7100(VidModeData* pVidModeData)
{
	VidModeData* pVidModeCopy;
	FrameBuffer* pFrameBuffer;
	ZBufferTags* pTags;
	int zBufferFormat;

	pVidModeCopy = (VidModeData*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(VidModeData));
	*pVidModeCopy = *pVidModeData;

	pFrameBuffer = (FrameBuffer*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(FrameBuffer));
	memset(pFrameBuffer, 0, sizeof(FrameBuffer));

	pFrameBuffer->pVidModeData_0x0 = pVidModeCopy;
	pFrameBuffer->pVidModeData_0x0->pLink_0xc = pFrameBuffer;

	if (pVidModeData->bUseGlobalFrameBuffer == false) {
		pFrameBuffer->frameBasePtr = (uint)ed3D::g_FrameBufferPtr_004491f0 >> 5;
		pFrameBuffer->data_0xc = (char*)ed3D::g_FrameBufferPtr_004491f0;
	}
	if ((pVidModeData->flags_0x8 & 8) != 0) {
		pTags = (ZBufferTags*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ZBufferTags));
		pFrameBuffer->pZTags = pTags;

		// TAG
		pTags->commandBuffer[0].cmdA = SCE_GIF_SET_TAG(
			2,					// NLOOP
			SCE_GS_TRUE,		// EOP
			SCE_GS_FALSE,		// PRE
			0,					// PRIM
			SCE_GIF_PACKED,		// FLG
			1					// NREG
		);
		pTags->commandBuffer[0].cmdB = SCE_GIF_PACKED_AD;

		// Z BUFFER
		zBufferFormat = GetZBufferTextureFormat(pFrameBuffer->pVidModeData_0x0->pixelStoreMode);
		pTags->commandBuffer[1].cmdA = SCE_GS_SET_ZBUF(
			pFrameBuffer->frameBasePtr,	// ZBP
			zBufferFormat,				// PSM
			0							// ZMASK
		);
		pTags->commandBuffer[1].cmdB = SCE_GS_ZBUF_1;

		// Z TEST
		pTags->commandBuffer[2].cmdA = SCE_GS_SET_TEST(
			0,	// ATE 
			0,	// ATST
			0,	// AREF
			0,	// AFAIL
			0,	// DATE
			0,	// DATM
			1,	// ZTE
			3	// ZTST
		);
		pTags->commandBuffer[2].cmdB = SCE_GS_TEST_1;
	}
	pFrameBuffer->pNext = pFrameBuffer;
	return pFrameBuffer;
}

CameraPanStatic_50* GetCameraPanStatic_50_002a5600(StaticMeshMaster* pStaticMeshMaster)
{
	CameraPanStatic_50* pCVar1;

	pCVar1 = (CameraPanStatic_50*)0x0;
	if (pStaticMeshMaster != (StaticMeshMaster*)0xffffffff) {
		pCVar1 = &pStaticMeshMaster->field_0x50;
	}
	return pCVar1;
}

StaticMeshMaster* g_CameraPanStaticMaster_00449574 = NULL;

StaticMeshMaster* LinkChildStaticMeshMaster_002b6ed0(StaticMeshMaster* pTemplate, CameraObj_130* pCameraObj130)
{
	FrameBuffer* pVidModeDataA;
	FrameBuffer* pVidModeDataB;
	CameraObjParams* pCVar1;
	uint uVar2;
	uint uVar3;
	FrameBuffer* pVidModeDataA_00;
	CameraObj_28* pCVar4;
	CameraObj_28* pCVar5;
	StaticMeshMaster* pStaticMeshMaster;
	CameraPanStatic_50* pCVar6;
	CameraObjParams* pCVar7;
	CameraObjParams* pCVar8;
	VidModeData local_20;
	CameraObjParams local_8;

	pVidModeDataA = pTemplate->pCameraObj28_0xc->pColorBuffer;
	pVidModeDataB = pTemplate->pCameraObj28_0xc->pZBuffer;
	if (0x100 < (uint)(pTemplate->field_0x50).field_0x100.field_0x1c) {
		(pTemplate->field_0x50).field_0x100.field_0x1c = 0x100;
	}
	uVar2 = ToPowerOfTwo_0029e3f0((pTemplate->field_0x50).field_0x100.field_0x18);
	uVar3 = ToPowerOfTwo_0029e3f0((pTemplate->field_0x50).field_0x100.field_0x1c);
	pCVar7 = (CameraObjParams*)0x8;
	pCVar8 = &local_8;
	pCVar1 = pCVar8;
	//while (pCVar1 != (CameraObjParams*)0x0) {
	//	*(undefined*)&pCVar8->field_0x0 = 0;
	//	pCVar8 = (CameraObjParams*)((int)&pCVar8->field_0x0 + 1);
	//	pCVar7 = (CameraObjParams*)((int)&pCVar7[-1].screenHeight + 1);
	//	pCVar1 = pCVar7;
	//}
	local_20.field_0x6 = 0x10;
	local_20.flags_0x8 = 4;
	local_20.frameBufferCount = 1;
	local_20.screenHeight = (ushort)uVar3;
	local_20.screenWidth = (ushort)uVar2;
	local_20.pixelStoreMode = SCE_GS_PSMCT32;
	local_20.bUseGlobalFrameBuffer = false;
	local_8.screenWidth = local_20.screenWidth;
	local_8.screenHeight = local_20.screenHeight;
	pVidModeDataA_00 = AllocateFrameBuffer_002b7100(&local_20);
	pCVar4 = AllocateCameraObj28_002bae70(&local_8, pVidModeDataA_00, pVidModeDataB, 1);
	SetCameraClear_002bb960(pCVar4, 0, 0, 0);
	pCVar5 = AllocateCameraObj28_002bae70(&local_8, pVidModeDataA, pVidModeDataB, 0);
	pStaticMeshMaster = ed3D::FindFreeStaticMeshMaster_002b4600(pCameraObj130, pCVar4, 1);
	pStaticMeshMaster->isChild = 1;
	pCVar6 = GetCameraPanStatic_50_002a5600(pStaticMeshMaster);
	pCVar6->field_0x4 = (pTemplate->field_0x50).field_0x100.field_0x4;
	pCVar6->field_0x18 = (pTemplate->field_0x50).field_0x100.field_0x4;
	pCVar6->farClip = (pTemplate->field_0x50).field_0x100.field_0xc;
	pCVar6->nearClip = (pTemplate->field_0x50).field_0x100.field_0x8;
	pCVar6->field_0x14 = (pTemplate->field_0x50).field_0x12c;
	pCVar6->field_0x10 = (pTemplate->field_0x50).field_0x128;
	(pCVar6->field_0x100).pCamera_0x10 = pCVar4;
	(pCVar6->field_0x100).pCamera_0x14 = pCVar5;
	(pCVar6->field_0x100).field_0x20 = (pTemplate->field_0x50).field_0x100.field_0x20;
	(pCVar6->field_0x100).field_0x23 = (pTemplate->field_0x50).field_0x100.field_0x23;
	(pCVar6->field_0x100).field_0x22 = (pTemplate->field_0x50).field_0x100.field_0x22;
	(pCVar6->field_0x100).field_0x24 = (pTemplate->field_0x50).field_0x100.field_0x24;
	(pCVar6->field_0x100).field_0x18 = (pTemplate->field_0x50).field_0x100.field_0x18;
	(pCVar6->field_0x100).field_0x1c = (pTemplate->field_0x50).field_0x100.field_0x1c;
	(pTemplate->field_0x50).field_0x100.field_0x20 = 0;
	g_CameraPanStaticMaster_00449574 = pStaticMeshMaster;
	return pStaticMeshMaster;
}

StaticMeshMaster* LinkChildStaticMeshMaster_002b4790(StaticMeshMaster* pStaticMeshMaster, CameraObj_130* param_2)
{
	StaticMeshMaster* pDisplayList;

	pStaticMeshMaster->flags_0x4 = pStaticMeshMaster->flags_0x4 | 2;
	pDisplayList = LinkChildStaticMeshMaster_002b6ed0(pStaticMeshMaster, param_2);
	edDlist::WillSetActiveDisplayList_00290cb0(&pStaticMeshMaster->headerA, (DisplayListInternal*)pDisplayList);
	return pDisplayList;
}

LargeObject::LargeObject()
{
	void** errorPrinter;
	//LocalizationManager* pManager;
	//Manager_170* pMVar1;
	FrontendManager* pFrontendManager;
	//HelpManager* pHelpManager;
	PauseManager* pPVar2;
	//MapManager* pMVar3;
	CameraViewManager* pCameraViewmanager;
	SectorManager* pSectorManager;
	//LightManager* pLightManager;
	FileManager3D* p3DFileManager;
	//Manager_100* pMVar4;
	//Manager_208* pMVar5;
	//GlobalSound_00451698* inAllocatedMemory;
	//Manager_C* pMVar6;
	//Manager_10* pMVar7;
	//ActorManager* pActorManager;
	CinematicManager* pNewCinematicObject;
	//Manager_C_Alt* pMVar8;
	//EffectsManager* pEVar9;
	//EventManager* pEventManager;
	//Manager_29b4* pMVar10;
	edVideo_Globals* peVar11;
	FrameBuffer* pVVar12;
	FrameBuffer* pVVar13;
	CameraObj_28* pCVar14;
	CameraPanStatic_50* pCVar15;
	StaticMeshMaster* pSVar16;
	undefined8 uVar17;
	//Manager_29b4** ppMVar18;
	LargeObject* puVar5;
	int iVar19;
	StaticMeshMaster** ppSVar20;
	CameraObj_130* pCVar21;
	CameraObjParams local_8;
	LocalizationManager* pManager;

	this->field_0x38 = 1;
	//errorPrinter = (void**)Allocate(4);
	//if (errorPrinter != (void**)0x0) {
	//	errorPrinter = PrintCActorErrors(errorPrinter);
	//}
	//this->field_0x0 = (int)errorPrinter;
	this->field_0x38 = 1;
	iVar19 = 0x17;
	this->field_0xcc = -1;
	this->field_0xd0 = 0.0;
	//ppMVar18 = &g_Manager29B4_004516bc;
	this->field_0xd4 = 0.0;
	this->field_0xe8 = 0;
	this->field_0xec = 0;
	this->field_0xf0 = 0;
	this->field_0xf4 = 0;
	this->field_0xd8 = this->field_0xe8;
	this->field_0xdc = this->field_0xec;
	this->field_0xe0 = this->field_0xf0;
	this->field_0xe4 = this->field_0xf4;
	this->pCameraObj28_0x4 = (CameraObj_28*)0x0;
	this->pCameraObj28_0x8 = (CameraObj_28*)0x0;
	this->field_0x48 = 0;
	//do {
	//	*ppMVar18 = (Manager_29b4*)0x0;
	//	ppMVar18[-1] = (Manager_29b4*)0x0;
	//	iVar19 = iVar19 + -8;
	//	ppMVar18[-2] = (Manager_29b4*)0x0;
	//	ppMVar18[-3] = (Manager_29b4*)0x0;
	//	ppMVar18[-4] = (Manager_29b4*)0x0;
	//	ppMVar18[-5] = (Manager_29b4*)0x0;
	//	ppMVar18[-6] = (Manager_29b4*)0x0;
	//	ppMVar18[-7] = (Manager_29b4*)0x0;
	//	ppMVar18 = ppMVar18 + -8;
	//} while (-1 < iVar19);
	g_ManagerSingletonArray_00451660.g_LevelScheduleManager_00451660 = new LevelScheduleManager();
	/* Initial load point at 006db5b0 */
	if (g_ManagerSingletonArray_00451660.g_LevelScheduleManager_00451660 != (LevelScheduleManager*)0x0) {
		g_LevelScheduleManager_00449728 = g_ManagerSingletonArray_00451660.g_LevelScheduleManager_00451660;
	}
	pManager = new LocalizationManager();
	g_ManagerSingletonArray_00451660.g_LocalizationManager_00451678 = pManager;
	//g_Manager1e4_00451678 = pManager;
	//pMVar1 = (Manager_170*)Allocate(0x170);
	//if (pMVar1 != (Manager_170*)0x0) {
	//	uVar17 = ManagerConstructor_001dba90(pMVar1);
	//	pMVar1 = (Manager_170*)uVar17;
	//}
	//g_Manager170_00451674 = pMVar1;
	pFrontendManager = new FrontendManager();
	g_ManagerSingletonArray_00451660.g_FrontendManager_00451680 = pFrontendManager;
	//pHelpManager = (HelpManager*)Allocate(0x1e4);
	//if (pHelpManager != (HelpManager*)0x0) {
	//	uVar17 = SetupHelpManager_0037bc70(pHelpManager);
	//	pHelpManager = (HelpManager*)uVar17;
	//}
	//g_HelpManager_00451684 = pHelpManager;
	pPVar2 = new PauseManager();
	g_ManagerSingletonArray_00451660.g_PauseManager_00451688 = pPVar2;
	//pMVar3 = (MapManager*)Allocate(0x3a0);
	//if (pMVar3 != (MapManager*)0x0) {
	//	pMVar3 = (MapManager*)SetupMapManager_003f9ef0(pMVar3);
	//}
	//g_MapManager_0045168c = pMVar3;
	pCameraViewmanager = new CameraViewManager();
	g_ManagerSingletonArray_00451660.g_CameraManager_0045167c = pCameraViewmanager;
	pSectorManager = new SectorManager();
	g_ManagerSingletonArray_00451660.g_SectorManager_00451670 = pSectorManager;
	//pLightManager = (LightManager*)Allocate(0x120);
	//if (pLightManager != (LightManager*)0x0) {
	//	pLightManager = LightManager::Constructor_002170a0(pLightManager);
	//}
	//g_LightManager_004516b0 = pLightManager;
	p3DFileManager = new FileManager3D();
	g_ManagerSingletonArray_00451660.g_FileManager3D_00451664 = p3DFileManager;
	//pMVar4 = (Manager_100*)Allocate(100);
	//if (pMVar4 != (Manager_100*)0x0) {
	//	uVar17 = SetupManager100_00211eb0(pMVar4);
	//	pMVar4 = (Manager_100*)uVar17;
	//}
	//g_Manager100_00451690 = pMVar4;
	//pMVar5 = (Manager_208*)Allocate(0x208);
	//if (pMVar5 != (Manager_208*)0x0) {
	//	uVar17 = SetupManager208_001ad8f0(pMVar5);
	//	pMVar5 = (Manager_208*)uVar17;
	//}
	//g_Manager208_00451694 = pMVar5;
	//inAllocatedMemory = (GlobalSound_00451698*)Allocate(0x2e8);
	//if (inAllocatedMemory != (GlobalSound_00451698*)0x0) {
	//	inAllocatedMemory = (GlobalSound_00451698*)GlobalSoundConstructor(inAllocatedMemory);
	//}
	//g_GlobalSoundPtr_00451698 = inAllocatedMemory;
	//pMVar6 = (Manager_C*)Allocate(0xc);
	//if (pMVar6 != (Manager_C*)0x0) {
	//	uVar17 = SetupManagerC_001bfc20(pMVar6);
	//	pMVar6 = (Manager_C*)uVar17;
	//}
	//g_ManagerC_0045169c = pMVar6;
	//pMVar7 = (Manager_10*)Allocate(0x10);
	//if (pMVar7 != (Manager_10*)0x0) {
	//	uVar17 = SetupManager10_001bf9c0(pMVar7);
	//	pMVar7 = (Manager_10*)uVar17;
	//}
	//g_Manager10_004516a0 = pMVar7;
	//pActorManager = (ActorManager*)Allocate(0x600);
	//if (pActorManager != (ActorManager*)0x0) {
	//	pActorManager = ActorManager::Setup_00108040(pActorManager);
	//}
	//g_ActorManager_004516a4 = pActorManager;
	//g_CinematicManager_0045166c = (ManagerFunctionData**)Allocate(4);
	//if (g_CinematicManager_0045166c != (ManagerFunctionData**)0x0) {
	//	*g_CinematicManager_0045166c = &g_ManagerDefaultFuncData_0043a4e0;
	//	*g_CinematicManager_0045166c = &ManagerFunctionData_0043dc90;
	//}
	pNewCinematicObject = new CinematicManager();
	g_ManagerSingletonArray_00451660.g_CinematicManagerPtr_004516ac = pNewCinematicObject;
	//g_AnimManager_00451668 = (AnimManager*)Allocate(0x1c);
	//if (g_AnimManager_00451668 != (AnimManager*)0x0) {
	//	g_AnimManager_00451668->pManagerFunctionData = &g_ManagerDefaultFuncData_0043a4e0;
	//	g_AnimManager_00451668->pManagerFunctionData = &g_levelLoadFuncPtr;
	//}
	//pMVar8 = (Manager_C_Alt*)Allocate(0xc);
	//if (pMVar8 != (Manager_C_Alt*)0x0) {
	//	uVar17 = SetupManagerC_Alt_0019ef50(pMVar8);
	//	pMVar8 = (Manager_C_Alt*)uVar17;
	//}
	//g_ManagerC_Alt_004516b4 = pMVar8;
	//pEVar9 = (EffectsManager*)Allocate(0x70);
	//if (pEVar9 != (EffectsManager*)0x0) {
	//	pEVar9 = (EffectsManager*)SetupEffectsManager_001a1dd0(pEVar9);
	//}
	//g_EffectsManager_004516b8 = pEVar9;
	//pEventManager = (EventManager*)Allocate(0xc);
	//if (pEventManager != (EventManager*)0x0) {
	//	pEventManager = EventManager::Setup_0019e980(pEventManager);
	//}
	//g_EventManager_006f5080 = pEventManager;
	//pMVar10 = (Manager_29b4*)Allocate(0x29b4);
	//if (pMVar10 != (Manager_29b4*)0x0) {
	//	uVar17 = SetupManager29b4_002d88c0(pMVar10);
	//	pMVar10 = (Manager_29b4*)uVar17;
	//}
	//g_Manager29B4_004516bc = pMVar10;
	peVar11 = GetVideoParams_002ba3e0();
	local_8.field_0x2 = 0;
	local_8.field_0x0 = 0;
	local_8.screenWidth = peVar11->pActiveVidParams->params26.screenWidth;
	local_8.screenHeight = peVar11->pActiveVidParams->params26.screenHeight;
	pVVar12 = GetFrameBuffer_001ba9c0();
	pVVar13 = GetFrameBuffer_001ba9d0();
	pCVar14 = AllocateCameraObj28_002bae70(&local_8, pVVar12, pVVar13, 3);
	this->pCameraObj28_0x4 = pCVar14;
	SetCameraClear_002bb960(this->pCameraObj28_0x4, 0, 0, 0);
	g_StaticMeshMasterA_00448808 = ed3D::FindFreeStaticMeshMaster_002b4600(&CameraObj_130_0044a100, this->pCameraObj28_0x4, 1);
	g_StaticMeshMasterA_00448808->SetFlag_002a5400(0x20);
	g_StaticMeshMasterA_00448808->SetFlag_002a5410(1);
	g_StaticMeshMasterA_00448808->SetFlag_002a5440(1);
	pCVar15 = GetCameraPanStatic_50_002a5600(g_StaticMeshMasterA_00448808);
	pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	(pCVar15->field_0x100).field_0x18 = 0x200;
	(pCVar15->field_0x100).field_0x1c = 0x100;
	pCVar15->field_0x12c = pCVar15->field_0x14 * 800;
	pCVar15->field_0x128 = 1;
	(pCVar15->field_0x100).field_0x0 = 0;
	(pCVar15->field_0x100).field_0x4 = 90.0;
	(pCVar15->field_0x100).field_0x8 = -0.01;
	(pCVar15->field_0x100).field_0xc = -150.0;
	(pCVar15->field_0x100).field_0x22 = 0x32;
	pVVar12 = GetFrameBuffer_001ba9c0();
	pVVar13 = GetFrameBuffer_001ba9d0();
	pCVar14 = AllocateCameraObj28_002bae70(&local_8, pVVar12, pVVar13, 0);
	this->pCameraObj28_0x8 = pCVar14;
	SetCameraClear_002bb960(this->pCameraObj28_0x8, 0, 0, 0);
	g_CameraPanMasterB_0044880c = ed3D::FindFreeStaticMeshMaster_002b4600(&CameraObj_130_0044a100, this->pCameraObj28_0x8, 1);
	g_CameraPanMasterB_0044880c->SetFlag_002a5400(0);
	g_CameraPanMasterB_0044880c->SetFlag_002a5410(0);
	pCVar15 = GetCameraPanStatic_50_002a5600(g_CameraPanMasterB_0044880c);
	pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	GetCameraPanStatic_50_002a5600(g_StaticMeshMasterA_00448808);
	iVar19 = 0;
	pCVar21 = CameraObj_130_ARRAY_0044a230;
	ppSVar20 = g_CameraPanStaticMasterArray_00451630;
	do {
		pSVar16 = LinkChildStaticMeshMaster_002b4790(g_StaticMeshMasterA_00448808, pCVar21);
		*ppSVar20 = pSVar16;
		(*ppSVar20)->SetFlag_002a5400(4);
		iVar19 = iVar19 + 1;
		pCVar21 = pCVar21 + 1;
		ppSVar20 = ppSVar20 + 1;
	} while (iVar19 < 10);
	this->count_0x120 = 0;
	this->field_0x11c = 0;
	g_LargeObject_006db450 = this;
	return;
}


void SetupGameCreateObject(void)
{
	new LargeObject;
}

void WillSetupDisplayListAndRunConstructors(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->OnBeginGame();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	SetupDisplayLists();
	return;
}

void LoadLevelUpdate_001b9c60(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->LoadLevelUpdate();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	ActivateDisplayLists_002d6490();
	return;
}

int INT_0044934c = 0;

void SetuGpffff8e5cTo0(void)
{
	INT_0044934c = 0;
	return;
}


void LoadStageOne_001b9dc0(void)
{
	TimeController* pTimeController;
	Manager** loadLoopObject;
	int iVar1;

	pTimeController = GetTimeController();
	pTimeController->ResetTimeController_001ba6d0();
	SetuGpffff8e5cTo0();
	iVar1 = 0;
	loadLoopObject = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*loadLoopObject != (Manager*)0x0) {
			(*loadLoopObject)->LoadStageOne();
		}
		iVar1 = iVar1 + 1;
		loadLoopObject = loadLoopObject + 1;
	} while (iVar1 < 0x18);
	return;
}

bool LargeObject::AsyncLoad_001b9cd0()
{
	bool bVar1;
	Manager** loopFunc;
	int iVar2;
	bool bVar3;

	bVar3 = false;
	//LargeObjectFunc_001b97f0(param_1);
	iVar2 = 0;
	loopFunc = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if ((*loopFunc != (Manager*)0x0) && (bVar1 = (*loopFunc)->AsyncLoad(), bVar1 != false)) {
			bVar3 = true;
		}
		iVar2 = iVar2 + 1;
		loopFunc = loopFunc + 1;
	} while (iVar2 < 0x18);
	return bVar3;
}

void LargeObject::EndLoadStageOne(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->EndLoadStageOne();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	return;
}

void LoadA_001b9bf0(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->LoadA();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	return;
}

void SetParam1_0x20ToParam2(CameraObj_28* param_1, uint param_2)
{
	param_1->altFbMask = param_2;
	return;
}

byte BYTE_004489dc = 0;
uint UINT_004489e0 = 0;
uint UINT_004493fc = 0;

void FUN_002ab4a0(byte param_1, uint param_2, uint param_3)
{
	BYTE_004489dc = param_1;
	UINT_004489e0 = param_3;
	UINT_004493fc = param_2;
	return;
}

void LoadB_001b95c0(LargeObject* param_1)
{
	undefined4* puVar1;
	TimeController* pTimeController;
	Manager** loadFuncPtr;
	int loopCounter;

	if (param_1->field_0x40 != 0) {
		param_1->field_0x44 = 0.0;
		param_1->field_0x40 = 0;
	}
	param_1->field_0x44 = 0.0;
	if ((param_1->field_0xc[3] & 1) == 0) {
		param_1->field_0xcc = param_1->field_0xcc + 1;
		param_1->field_0xd8 = param_1->field_0xe8;
		param_1->field_0xdc = param_1->field_0xec;
		param_1->field_0xe0 = param_1->field_0xf0;
		param_1->field_0xe4 = param_1->field_0xf4;
		param_1->field_0xd0 = 0.0;
		param_1->field_0xd4 = 0.0;
		loopCounter = param_1->field_0xcc;
		(&param_1->field_0x0 + loopCounter * 2)[0x13] = (int)param_1->field_0xc;
		(&param_1->field_0x0 + loopCounter * 2)[0x14] = (int)0x0;
	}
	puVar1 = param_1->field_0xc;
	param_1->field_0xd8 = *puVar1;
	param_1->field_0xdc = puVar1[1];
	param_1->field_0xe0 = (undefined4)puVar1[2];
	param_1->field_0xe4 = puVar1[3];
	puVar1 = param_1->field_0xc;
	param_1->field_0xe8 = *puVar1;
	param_1->field_0xec = puVar1[1];
	param_1->field_0xf0 = (undefined4)puVar1[2];
	param_1->field_0xf4 = puVar1[3];
	param_1->field_0x48 = 0;
	FUN_002ab4a0(1, param_1->field_0x14, param_1->field_0x10);
	//if (g_ManagerSingletonArray_00451660[1][2].pVTable == (ManagerFunctionData*)0x0) {
	//	SetParam1_0x20ToParam2(param_1->pCameraObj28_0x4, 0);
	//}
	//else {
		SetParam1_0x20ToParam2(param_1->pCameraObj28_0x4, 0xffffffff);
	//}
	SetParam1_0x20ToParam2(param_1->pCameraObj28_0x8, 0);
	pTimeController = GetTimeController();
	pTimeController->ResetTimeController_001ba6d0();
	SetuGpffff8e5cTo0();
	param_1->field_0x38 = 1;
	//EffectsManager::LoadFunc_001a14d0((EffectsManager*)g_ManagerSingletonArray_00451660[22]);
	loopCounter = 0;
	/* Load loop Initially points at 006db5b0 */
	loadFuncPtr = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		/* This will call load functions */
		if (*loadFuncPtr != (Manager*)0x0) {
			(*loadFuncPtr)->LoadB();
		}
		loopCounter = loopCounter + 1;
		loadFuncPtr = loadFuncPtr + 1;
	} while (loopCounter < 0x18);
	param_1->field_0x38 = 1;
	//FUN_0024ae40();
	//(*(code*)g_LevelScheduleManager_00449728->pManagerFunctionData[1].field_0x0)();
	//WillLoadCinematic(g_CinematicManager_0048efc);
	//FUN_003f95f0((MapManager*)g_ManagerSingletonArray_00451660[11]);
	if (param_1->field_0xcc != -1) {
		//puVar1 = (undefined4*)(&param_1->field_0x4c)[param_1->field_0xcc * 2];
		//param_1->field_0xe8 = *puVar1;
		//param_1->field_0xec = puVar1[1];
		//param_1->field_0xf0 = (undefined4)puVar1[2];
		//param_1->field_0xf4 = puVar1[3];
		//param_1->field_0xd0 = param_1->field_0xd4;
	}
	return;
}

void FUN_001b90b0(LargeObject* param_1)

{
	TimeController* pTVar1;
	ulong uVar2;

	//LargeObjectFunc_001b97f0(param_1);
	pTVar1 = GetTimeController();
	param_1->field_0x44 = param_1->field_0x44 + pTVar1->lastFrameTime;
	switch (param_1->field_0x40) {
	case 1:
		assert(false);
		//FUN_001a0180(1.0, 2, 0);
		//if (param_1->field_0x40 != 2) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 2;
		//}
		break;
	case 2:
		assert(false);
		//uVar2 = GetbRam0048cdf8();
		//if (uVar2 == 0) {
		//	g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 & 0xfffffe7f;
		//	LargeObject::OnSaveLoaded_001b8d40(param_1);
		//	FUN_001a0180(1.0, 1, 0);
		//	if (param_1->field_0x40 != 0) {
		//		param_1->field_0x44 = 0.0;
		//		param_1->field_0x40 = 0;
		//	}
		//}
		break;
	case 3:
		assert(false);
		//g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 & 0xfffffe7f;
		//LargeObject::OnSaveLoaded_001b8d40(param_1);
		//FUN_001a0180(1.0, 1, 0);
		//if (param_1->field_0x40 != 0) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 0;
		//}
		break;
	case 4:
		assert(false);
		//FUN_001a0180(1.0, 2, 0);
		//if (param_1->field_0x40 != 5) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 5;
		//}
		break;
	case 5:
		assert(false);
		//uVar2 = GetbRam0048cdf8();
		//if (uVar2 == 0) {
		//	g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 & 0xfffffe7f;
		//	FUN_001b8e30((int)param_1);
		//	FUN_001a0180(1.0, 1, 0);
		//	if (param_1->field_0x40 != 0) {
		//		param_1->field_0x44 = 0.0;
		//		param_1->field_0x40 = 0;
		//	}
		//}
		break;
	case 6:
		assert(false);
		//FUN_001a0180(1.0, 2, 0);
		//if (param_1->field_0x40 != 7) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 7;
		//}
		break;
	case 7:
		assert(false);
		//uVar2 = GetbRam0048cdf8();
		//if (uVar2 == 0) {
		//	FUN_001a0180(1.0, 1, 0);
		//	g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 | 2;
		//	if (param_1->field_0x40 != 0) {
		//		param_1->field_0x44 = 0.0;
		//		param_1->field_0x40 = 0;
		//	}
		//}
	}
	return;
}

void PreUpdateObjects(LargeObject* param_1)
{
	Manager** ppMVar1;
	int iVar2;

	if (param_1->field_0x48 == 0) {
		if ((g_DebugCameraFlag_00448ea4 & 0x200) != 0) {
			g_StaticMeshMasterA_00448808->RemoveFlag_002a53e0(4);
			g_CameraPanMasterB_0044880c->RemoveFlag_002a53e0(4);
			g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 & 0xfffffdff;
		}
	}
	else {
		if ((g_DebugCameraFlag_00448ea4 & 0x200) == 0) {
			g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 | 0x200;
			g_StaticMeshMasterA_00448808->SetFlag_002a5400(4);
			g_CameraPanMasterB_0044880c->SetFlag_002a5400(4);
		}
	}
	//EmptyFunction();
	if ((g_DebugCameraFlag_00448ea4 & 0x20) == 0) {
		iVar2 = 0;
		ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
		do {
			if (*ppMVar1 != (Manager*)0x0) {
				(*ppMVar1)->PreUpdateA();
			}
			iVar2 = iVar2 + 1;
			ppMVar1 = ppMVar1 + 1;
		} while (iVar2 < 0x18);
	}
	else {
		iVar2 = 0;
		ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
		do {
			if (*ppMVar1 != (Manager*)0x0) {
				(*ppMVar1)->PreUpdateB();
			}
			iVar2 = iVar2 + 1;
			ppMVar1 = ppMVar1 + 1;
		} while (iVar2 < 0x18);
	}
	FUN_001b90b0(param_1);
	return;
}

void UpdateObjectsMain(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->Update();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	ActivateDisplayLists_002d6490();
	return;
}

char* s_Main_Camera_0042b460 = "Main Camera";

void LargeObject::OnLoadLevelBnk_001b8920(MemoryStream* pMemoryStream)
{
	undefined4* puVar1;
	int* piVar2;
	uint uVar3;
	int iVar4;
	MainCamera* pCVar5;
	float fVar5;
	uint* local_4;

	pMemoryStream->ReadByte_00189bb0();
	pMemoryStream->ReadByte_00189bb0();
	pMemoryStream->ReadByte_00189bb0();
	pMemoryStream->ReadByte_00189bb0();
	pMemoryStream->ReadLongFunc_00189b00();
	pMemoryStream->ReadInt_00189b70();
	pMemoryStream->ReadInt_00189b70();
	uVar3 = pMemoryStream->ReadUint_00189b50();
	this->field_0x18 = uVar3;
	puVar1 = (undefined4*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(puVar1 + 4);
	this->field_0xc = puVar1;
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->field_0x14 = iVar4;
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->field_0x10 = iVar4;
	/* Main Camera */
	pCVar5 = (MainCamera*)g_ManagerSingletonArray_00451660.g_CameraManager_0045167c->LoadViewOrCamera(CT_MainCamera, pMemoryStream, s_Main_Camera_0042b460);
	fVar5 = pMemoryStream->ReadFloat_00189b30();
	this->field_0x118 = fVar5;
	//pCVar5->field_0x2dc = fVar5;
	uVar3 = pMemoryStream->ReadUint_00189b50();
	this->field_0x11c = uVar3;
	uVar3 = pMemoryStream->ReadUint_00189b50();
	this->count_0x120 = uVar3;
	LoadFunc_001b87b0();
	//MapManager::OnLoadLevelBnk_003f9a60(g_ManagerSingletonArray_00451660.g_MapManager_0045168c, pMemoryStream);
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->field_0x28 = iVar4;
	if (this->field_0x28 != -1) {
		IMPLEMENTATION_GUARD(FileManager3D::OnLoadLevelBnk_001a6b80(g_ManagerSingletonArray_00451660.g_FileManager3D_00451664, this->field_0x28));
	}
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->defaultTextureIndex_0x2c = iVar4;
	//FUN_002cbc90((undefined4*)&this->field_0xfc);
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->field_0x30 = iVar4;
	//EffectsManager::OnLoadLevelBnk_001a1790(g_ManagerSingletonArray_00451660.g_EffectsManager_004516b8, pMemoryStream);
	local_4 = (uint*)0x0;
	//MemoryStreamReadFunc_00117330(&local_4, pMemoryStream);
	//FUN_00117210(&local_4);
	//LevelScheduleManager::OnLoadLevelBnk_002d9f80(g_LevelScheduleManager_00449728, pMemoryStream);
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->field_0x10c = iVar4;
	iVar4 = pMemoryStream->ReadInt_00189b70();
	this->field_0x110 = iVar4;
	piVar2 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 4;
	}
	//this->field_0x114 = piVar2;
	return;
}

bool LargeObject::CheckFunc_001b9300()
{
	return this->field_0x40 - 6U < 2;
}

void Func_002b6db0(StaticMeshMaster* pStaticMeshMaster, uint width, uint height)
{
	CameraObj_28* pCVar1;
	CameraObj_28* pCVar2;
	short* psVar3;
	CameraPanStatic_50* pCVar4;
	uint uVar5;
	uint uVar6;
	short* psVar7;
	short* psVar8;
	ushort uVar9;
	ushort uVar10;
	short local_8;
	short local_6;

	pCVar4 = GetCameraPanStatic_50_002a5600(pStaticMeshMaster);
	pCVar1 = (pCVar4->field_0x100).pCamera_0x10;
	pCVar2 = (pCVar4->field_0x100).pCamera_0x14;
	uVar5 = ToPowerOfTwo_0029e3f0(width);
	uVar6 = ToPowerOfTwo_0029e3f0(height);
	if (0x100 < (int)uVar6) {
		uVar6 = 0x100;
	}
	psVar8 = &local_8;
	if (0x200 < (int)uVar5) {
		uVar5 = 0x200;
	}
	psVar7 = (short*)0x8;
	psVar3 = psVar8;
	while (psVar3 != (short*)0x0) {
		*(undefined*)psVar8 = 0;
		psVar8 = (short*)((ulong)psVar8 + 1);
		psVar7 = (short*)((ulong)psVar7 + -1);
		psVar3 = psVar7;
	}
	uVar10 = (ushort)uVar5;
	uVar9 = (ushort)uVar6;
	(pCVar4->field_0x100).field_0x18 = uVar5;
	(pCVar4->field_0x100).field_0x1c = uVar6;
	pCVar1->posX = local_8;
	pCVar1->posY = local_6;
	pCVar1->screenWidth = uVar10;
	pCVar1->screenHeight = uVar9;
	pCVar1->pColorBuffer->pVidModeData_0x0->screenWidth = uVar10;
	pCVar1->pColorBuffer->pVidModeData_0x0->screenHeight = uVar9;
	pCVar2->posX = local_8;
	pCVar2->posY = local_6;
	pCVar2->screenWidth = uVar10;
	pCVar2->screenHeight = uVar9;
	return;
}

void LargeObject::LoadFunc_001b87b0()
{
	CameraPanStatic_50* pCVar1;
	StaticMeshMaster** ppSVar2;
	uint unaff_s1_lo;
	uint unaff_s2_lo;
	uint uVar3;

	if (this->field_0x11c == 0) {
		this->count_0x120 = 1;
	}
	if (10 < this->count_0x120) {
		this->count_0x120 = 10;
	}
	GetCameraPanStatic_50_002a5600(g_StaticMeshMasterA_00448808);
	if (this->field_0x11c == 0) {
		unaff_s2_lo = 0x200;
		unaff_s1_lo = 0x100;
	}
	if (this->field_0x11c == 1) {
		unaff_s2_lo = 0x80;
		unaff_s1_lo = unaff_s2_lo;
	}
	uVar3 = 0;
	ppSVar2 = g_CameraPanStaticMasterArray_00451630;
	do {
		if (uVar3 < this->count_0x120) {
			(*ppSVar2)->RemoveFlag_002a53e0(4);
		}
		else {
			(*ppSVar2)->SetFlag_002a5400(4);
		}
		Func_002b6db0(*ppSVar2, unaff_s2_lo, unaff_s1_lo);
		pCVar1 = GetCameraPanStatic_50_002a5600(*ppSVar2);
		(pCVar1->field_0x100).field_0x20 = (short)(1 << (uVar3 & 0x1f));
		uVar3 = uVar3 + 1;
		ppSVar2 = ppSVar2 + 1;
		pCVar1->field_0x12c = pCVar1->field_0x14 * 800;
		pCVar1->field_0x128 = 1;
		(pCVar1->field_0x100).field_0x0 = 0;
		(pCVar1->field_0x100).field_0x4 = 90.0;
		(pCVar1->field_0x100).field_0x8 = -0.01;
		(pCVar1->field_0x100).field_0xc = -150.0;
		(pCVar1->field_0x100).field_0x22 = 0x32;
		(pCVar1->field_0x100).field_0x23 = 0;
		(pCVar1->field_0x100).field_0x24 = 0;
	} while ((int)uVar3 < 10);
	return;
}
