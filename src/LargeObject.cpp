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

LargeObject* g_LargeObject_006db450 = NULL;

LevelScheduleManager* g_LevelScheduleManager_00451660 = NULL;
LocalizationManager* g_LocalizationManager_00451678 = NULL;

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
	VidModeData_30* pVVar12;
	VidModeData_30* pVVar13;
	CameraObj_28* pCVar14;
	//CameraPanStatic_50* pCVar15;
	//StaticMeshMaster* pSVar16;
	undefined8 uVar17;
	//Manager_29b4** ppMVar18;
	LargeObject* puVar5;
	int iVar19;
	//StaticMeshMaster** ppSVar20;
	//CameraObj_130* pCVar21;
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
	pVVar12 = GetVidModeData30_001ba9c0();
	pVVar13 = GetVidModeData30_001ba9d0();
	pCVar14 = AllocateCameraObj28_002bae70(&local_8, pVVar12, pVVar13, 3);
	this->pCameraObj28_0x4 = pCVar14;
	SetCameraObjBytes_002bb960(this->pCameraObj28_0x4, 0, 0, 0);
	//g_StaticMeshMasterA_00448808 = SetupCameraPanStaticMaster_002b4600(&CameraObj_130_0044a100, this->pCameraObj28_0x4, 1);
	//StaticMeshMaster::SetFlag_002a5400(g_StaticMeshMasterA_00448808, 0x20);
	//CameraPanMasterSetFlag_002a5410(g_StaticMeshMasterA_00448808, 1);
	//CameraPanMasterSetFlag_002a5440(g_StaticMeshMasterA_00448808, 1);
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
	pVVar12 = GetVidModeData30_001ba9c0();
	pVVar13 = GetVidModeData30_001ba9d0();
	//pCVar14 = AllocateCameraObj28_002bae70(&local_8, pVVar12, pVVar13, 0);
	//this->pCameraObj28_0x8 = pCVar14;
	//SetCameraObjBytes_002bb960(this->pCameraObj28_0x8, 0, 0, 0);
	//g_CameraPanMasterB_0044880c = SetupCameraPanStaticMaster_002b4600(&CameraObj_130_0044a100, this->pCameraObj28_0x8, 1);
	//StaticMeshMaster::SetFlag_002a5400(g_CameraPanMasterB_0044880c, 0);
	//CameraPanMasterSetFlag_002a5410(g_CameraPanMasterB_0044880c, 0);
	//pCVar15 = StaticMeshMaster::GetCameraPanStatic_50_002a5600(g_CameraPanMasterB_0044880c);
	//pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	//StaticMeshMaster::GetCameraPanStatic_50_002a5600(g_StaticMeshMasterA_00448808);
	//iVar19 = 0;
	//pCVar21 = CameraObj_130_ARRAY_0044a230;
	//ppSVar20 = g_CameraPanStaticMasterArray_00451630;
	//do {
	//	pSVar16 = CopyCameraPanStaticMaster_002b4790(g_StaticMeshMasterA_00448808, pCVar21);
	//	*ppSVar20 = pSVar16;
	//	StaticMeshMaster::SetFlag_002a5400(*ppSVar20, 4);
	//	iVar19 = iVar19 + 1;
	//	pCVar21 = pCVar21 + 1;
	//	ppSVar20 = ppSVar20 + 1;
	//} while (iVar19 < 10);
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
