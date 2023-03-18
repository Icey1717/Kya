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
#include "ed3D.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "edVideo/VideoC.h"

LargeObject* g_LargeObject_006db450 = NULL;

LevelScheduleManager* g_LevelScheduleManager_00451660 = NULL;
LocalizationManager* g_LocalizationManager_00451678 = NULL;

StaticMeshMaster* g_StaticMeshMasterA_00448808 = NULL;
StaticMeshMaster* g_CameraPanMasterB_0044880c = NULL;

CameraObj_130 CameraObj_130_0044a100;

StaticMeshMaster* g_CameraPanStaticMasterArray_00451630[10] = { 0 };

FrameBuffer* AllocateFrameBuffer_002b7100(VidModeData* pVidModeData)
{
	VidModeData* pVidModeCopy;
	FrameBuffer* pFrameBuffer;
	ZBufferTags* pTags;
	int zBufferFormat;

	pVidModeCopy = (VidModeData*)edMemAlloc(H_MAIN, sizeof(VidModeData));
	*pVidModeCopy = *pVidModeData;

	pFrameBuffer = (FrameBuffer*)edMemAlloc(H_MAIN, sizeof(FrameBuffer));
	memset(pFrameBuffer, 0, sizeof(FrameBuffer));

	pFrameBuffer->pVidModeData_0x0 = pVidModeCopy;
	pFrameBuffer->pVidModeData_0x0->pLink_0xc = pFrameBuffer;

	if (pVidModeData->bUseGlobalFrameBuffer == false) {
		pFrameBuffer->frameBasePtr = (uint)ed3D::g_FrameBufferPtr_004491f0 >> 5;
		pFrameBuffer->data_0xc = (char*)ed3D::g_FrameBufferPtr_004491f0;
	}
	if ((pVidModeData->flags_0x8 & 8) != 0) {
		pTags = (ZBufferTags*)edMemAlloc(H_MAIN, sizeof(ZBufferTags));
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

StaticMeshMaster* CopyCameraPanStaticMaster_002b6ed0(StaticMeshMaster* pTemplate, CameraObj_130* pCameraObj130)
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

	pVidModeDataA = pTemplate->pCameraObj28_0xc->pVidModeData30_0x10;
	pVidModeDataB = pTemplate->pCameraObj28_0xc->pZBuffer;
	//if (0x100 < (uint)(pTemplate->field_0x50).field_0x100.field_0x1c) {
	//	(pTemplate->field_0x50).field_0x100.field_0x1c = 0x100;
	//}
	uVar2 = 0; //FUN_0029e3f0((pTemplate->field_0x50).field_0x100.field_0x18);
	uVar3 = 0; //FUN_0029e3f0((pTemplate->field_0x50).field_0x100.field_0x1c);
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
	SetCameraObjBytes_002bb960(pCVar4, 0, 0, 0);
	pCVar5 = AllocateCameraObj28_002bae70(&local_8, pVidModeDataA, pVidModeDataB, 0);
	pStaticMeshMaster = ed3D::SetupCameraPanStaticMaster_002b4600(pCameraObj130, pCVar4, 1);
	pStaticMeshMaster->field_0x0 = 1;
	//pCVar6 = StaticMeshMaster::GetCameraPanStatic_50_002a5600(pStaticMeshMaster);
	//pCVar6->field_0x4 = (pTemplate->field_0x50).field_0x100.field_0x4;
	//pCVar6->field_0x18 = (pTemplate->field_0x50).field_0x100.field_0x4;
	//pCVar6->field_0xc = (pTemplate->field_0x50).field_0x100.field_0xc;
	//pCVar6->field_0x8 = (pTemplate->field_0x50).field_0x100.field_0x8;
	//pCVar6->field_0x14 = (pTemplate->field_0x50).field_0x12c;
	//pCVar6->field_0x10 = (pTemplate->field_0x50).field_0x128;
	//(pCVar6->field_0x100).field_0x10 = pCVar4;
	//(pCVar6->field_0x100).field_0x14 = pCVar5;
	//(pCVar6->field_0x100).field_0x20 = (pTemplate->field_0x50).field_0x100.field_0x20;
	//(pCVar6->field_0x100).field_0x23 = (pTemplate->field_0x50).field_0x100.field_0x23;
	//(pCVar6->field_0x100).field_0x22 = (pTemplate->field_0x50).field_0x100.field_0x22;
	//(pCVar6->field_0x100).field_0x24 = (pTemplate->field_0x50).field_0x100.field_0x24;
	//(pCVar6->field_0x100).field_0x18 = (pTemplate->field_0x50).field_0x100.field_0x18;
	//(pCVar6->field_0x100).field_0x1c = (pTemplate->field_0x50).field_0x100.field_0x1c;
	//(pTemplate->field_0x50).field_0x100.field_0x20 = 0;
	//g_CameraPanStaticMaster_00449574 = pStaticMeshMaster;
	return pStaticMeshMaster;
}

StaticMeshMaster* CopyCameraPanStaticMaster_002b4790(StaticMeshMaster* param_1, CameraObj_130* param_2)
{
	StaticMeshMaster* pDisplayList;

	param_1->flags_0x4 = param_1->flags_0x4 | 2;
	pDisplayList = CopyCameraPanStaticMaster_002b6ed0(param_1, param_2);
	edDlist::WillSetActiveDisplayList_00290cb0(&param_1->field_0x10, (DisplayListInternal*)pDisplayList);
	return pDisplayList;
}


LargeObject::LargeObject()
{
	void** errorPrinter;
	//LocalizationManager* pManager;
	//Manager_170* pMVar1;
	//FrontendManager* pFrontendManager;
	//HelpManager* pHelpManager;
	PauseManager* pPVar2;
	//MapManager* pMVar3;
	//CameraViewManager* pCameraViewmanager;
	//SectorManager* pSectorManager;
	//LightManager* pLightManager;
	//3DFileManager * p3DFileManager;
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
	//CameraPanStatic_50* pCVar15;
	StaticMeshMaster* pSVar16;
	undefined8 uVar17;
	//Manager_29b4** ppMVar18;
	LargeObject* puVar5;
	int iVar19;
	StaticMeshMaster** ppSVar20;
	CameraObj_130* pCVar21;
	CameraObjParams local_8;

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
	g_LevelScheduleManager_00451660 = new LevelScheduleManager();
	/* Initial load point at 006db5b0 */
	if (g_LevelScheduleManager_00451660 != (LevelScheduleManager*)0x0) {
		g_LevelScheduleManager_00449728 = g_LevelScheduleManager_00451660;
	}
	g_LocalizationManager_00451678 = new LocalizationManager();

	//g_Manager1e4_00451678 = pManager;
	//pMVar1 = (Manager_170*)Allocate(0x170);
	//if (pMVar1 != (Manager_170*)0x0) {
	//	uVar17 = ManagerConstructor_001dba90(pMVar1);
	//	pMVar1 = (Manager_170*)uVar17;
	//}
	//g_Manager170_00451674 = pMVar1;
	//pFrontendManager = (FrontendManager*)Allocate(0x80);
	//if (pFrontendManager != (FrontendManager*)0x0) {
	//	uVar17 = FrontendManager::Setup_001d0030(pFrontendManager);
	//	pFrontendManager = (FrontendManager*)uVar17;
	//}
	//g_FrontendManager_00451680 = pFrontendManager;
	//pHelpManager = (HelpManager*)Allocate(0x1e4);
	//if (pHelpManager != (HelpManager*)0x0) {
	//	uVar17 = SetupHelpManager_0037bc70(pHelpManager);
	//	pHelpManager = (HelpManager*)uVar17;
	//}
	//g_HelpManager_00451684 = pHelpManager;
	pPVar2 = new PauseManager();
	if (pPVar2 != (PauseManager*)0x0) {
		pPVar2 = pPVar2->SetupPauseManager_001b0eb0();
	}
	g_PauseManager_00451688 = pPVar2;
	//pMVar3 = (MapManager*)Allocate(0x3a0);
	//if (pMVar3 != (MapManager*)0x0) {
	//	pMVar3 = (MapManager*)SetupMapManager_003f9ef0(pMVar3);
	//}
	//g_MapManager_0045168c = pMVar3;
	//pCameraViewmanager = (CameraViewManager*)Allocate(0xa80);
	//if (pCameraViewmanager != (CameraViewManager*)0x0) {
	//	uVar17 = SetupCameraViewManager_00197710(pCameraViewmanager);
	//	pCameraViewmanager = (CameraViewManager*)uVar17;
	//}
	//g_CameraManager_0045167c = pCameraViewmanager;
	//pSectorManager = (SectorManager*)Allocate(0x388);
	//if (pSectorManager != (SectorManager*)0x0) {
	//	pSectorManager = (SectorManager*)SectorManager::Setup_001ffdf0(pSectorManager);
	//}
	//g_SectorManager_00451670 = pSectorManager;
	//pLightManager = (LightManager*)Allocate(0x120);
	//if (pLightManager != (LightManager*)0x0) {
	//	pLightManager = LightManager::Constructor_002170a0(pLightManager);
	//}
	//g_LightManager_004516b0 = pLightManager;
	//p3DFileManager = (3DFileManager*)Allocate(0x54);
	//if (p3DFileManager != (3DFileManager*)0x0) {
	//	p3DFileManager = 3DFileManager::Constructor_001a70c0(p3DFileManager);
	//}
	//g_3DFileManager_00451664 = p3DFileManager;
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
	if (pNewCinematicObject != (CinematicManager*)0x0) {
		pNewCinematicObject = pNewCinematicObject->SetupCinematicManager_001c68b0();
	}
	g_CinematicManagerPtr_004516ac = pNewCinematicObject;
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
	SetCameraObjBytes_002bb960(this->pCameraObj28_0x4, 0, 0, 0);
	g_StaticMeshMasterA_00448808 = ed3D::SetupCameraPanStaticMaster_002b4600(&CameraObj_130_0044a100, this->pCameraObj28_0x4, 1);
	g_StaticMeshMasterA_00448808->SetFlag_002a5400(0x20);
	g_StaticMeshMasterA_00448808->SetFlag_002a5410(1);
	g_StaticMeshMasterA_00448808->SetFlag_002a5440(1);
	//pCVar15 = StaticMeshMaster::GetCameraPanStatic_50_002a5600(g_StaticMeshMasterA_00448808);
	//pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	//(pCVar15->field_0x100).field_0x18 = 0x200;
	//(pCVar15->field_0x100).field_0x1c = 0x100;
	//pCVar15->field_0x12c = pCVar15->field_0x14 * 800;
	//pCVar15->field_0x128 = 1;
	//(pCVar15->field_0x100).field_0x0 = 0;
	//(pCVar15->field_0x100).field_0x4 = 90.0;
	//(pCVar15->field_0x100).field_0x8 = -0.01;
	//(pCVar15->field_0x100).field_0xc = -150.0;
	//(pCVar15->field_0x100).field_0x22 = 0x32;
	pVVar12 = GetFrameBuffer_001ba9c0();
	pVVar13 = GetFrameBuffer_001ba9d0();
	pCVar14 = AllocateCameraObj28_002bae70(&local_8, pVVar12, pVVar13, 0);
	this->pCameraObj28_0x8 = pCVar14;
	SetCameraObjBytes_002bb960(this->pCameraObj28_0x8, 0, 0, 0);
	g_CameraPanMasterB_0044880c = ed3D::SetupCameraPanStaticMaster_002b4600(&CameraObj_130_0044a100, this->pCameraObj28_0x8, 1);
	g_CameraPanMasterB_0044880c->SetFlag_002a5400(0);
	g_CameraPanMasterB_0044880c->SetFlag_002a5410(0);
	//pCVar15 = StaticMeshMaster::GetCameraPanStatic_50_002a5600(g_CameraPanMasterB_0044880c);
	//pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	//StaticMeshMaster::GetCameraPanStatic_50_002a5600(g_StaticMeshMasterA_00448808);
	//iVar19 = 0;
	//pCVar21 = CameraObj_130_ARRAY_0044a230;
	ppSVar20 = g_CameraPanStaticMasterArray_00451630;
	do {
		pSVar16 = CopyCameraPanStaticMaster_002b4790(g_StaticMeshMasterA_00448808, pCVar21);
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
	LevelScheduleManager** ppLVar1;
	int iVar2;

	g_LevelScheduleManager_00449728->OnBeginGame(); // #HACK
	g_PauseManager_00451688->OnBeginGame(); // #HACK
	g_LocalizationManager_00449744->OnBeginGame(); // #HACK

	//iVar2 = 0;
	//ppLVar1 = &g_LevelScheduleManager_00451660;
	//do {
	//	if (*ppLVar1 != (LevelScheduleManager*)0x0) {
	//		(*(code*)(*ppLVar1)->pManagerFunctionData->constructorFunc)();
	//	}
	//	iVar2 = iVar2 + 1;
	//	ppLVar1 = ppLVar1 + 1;
	//} while (iVar2 < 0x18);
	SetupDisplayLists();
	return;
}

void LoadLevelUpdate_001b9c60(void)
{
	//LevelScheduleManager** ppLVar1;

	//g_LevelScheduleManager_00449728- // Hack
	g_PauseManager_00451688->LoadLevelUpdate();

	//int iVar2;
	//
	//iVar2 = 0;
	//ppLVar1 = &g_LevelScheduleManager_00451660;
	//do {
	//	if (*ppLVar1 != (LevelScheduleManager*)0x0) {
	//		(*(code*)(*ppLVar1)->pManagerFunctionData->loadLevelUpdateFunc)();
	//	}
	//	iVar2 = iVar2 + 1;
	//	ppLVar1 = ppLVar1 + 1;
	//} while (iVar2 < 0x18);
	ActivateDisplayLists_002d6490();
	return;
}
