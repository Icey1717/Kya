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
#include "renderer.h"
#endif
#include "edVideo/VideoC.h"
#include "TimeController.h"
#include "MemoryStream.h"
#include "FrontendManager.h"
#include "CameraViewManager.h"
#include "FileManager3D.h"
#include "MathOps.h"
#include "SectorManager.h"

Scene* Scene::_pinstance = NULL;

ManagerContainer g_ManagerSingletonArray_00451660 = { 0 };

ed_3D_Scene* Scene::_scene_handleA = NULL;
ed_3D_Scene* Scene::_scene_handleB = NULL;

ed_3D_Scene* g_CameraPanStaticMasterArray_00451630[10] = { 0 };

uint GameFlags = 0;

edSurface* ed3DShadowSurfaceNew(ed_surface_desc* pVidModeData)
{
	ed_surface_desc* pVidModeCopy;
	edSurface* pFrameBuffer;
	ZBufferTags* pTags;
	int zBufferFormat;

	pVidModeCopy = (ed_surface_desc*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), sizeof(ed_surface_desc));
	*pVidModeCopy = *pVidModeData;

	pFrameBuffer = (edSurface*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), sizeof(edSurface));
	memset(pFrameBuffer, 0, sizeof(edSurface));

	pFrameBuffer->pSurfaceDesc = pVidModeCopy;
	pFrameBuffer->pSurfaceDesc->pLink_0xc = pFrameBuffer;

	if (pVidModeData->bUseGlobalFrameBuffer == false) {
		pFrameBuffer->frameBasePtr = (uint)gFXBufAddr >> 5;
		pFrameBuffer->data_0xc = (char*)gFXBufAddr;
	}
	if ((pVidModeData->flags_0x8 & 8) != 0) {
		pTags = (ZBufferTags*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), sizeof(ZBufferTags));
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
		zBufferFormat = edSurfaceGetZBufferBpp(pFrameBuffer->pSurfaceDesc->pixelStoreMode);
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

SceneConfig* ed3DSceneGetConfig(ed_3D_Scene* pStaticMeshMaster)
{
	SceneConfig* pCVar1;

	pCVar1 = (SceneConfig*)0x0;
	if (pStaticMeshMaster != (ed_3D_Scene*)0xffffffff) {
		pCVar1 = &pStaticMeshMaster->sceneConfig;
	}
	return pCVar1;
}

ed_3D_Scene* gShadowScene = NULL;

ed_3D_Scene* ed3DShadowCreateScene(ed_3D_Scene* pTemplate, edFCamera* pCamera)
{
	edSurface* pVidModeDataA;
	edSurface* pVidModeDataB;
	CameraObjParams* pCVar1;
	uint uVar2;
	uint uVar3;
	edSurface* pVidModeDataA_00;
	ed_viewport* pViewport;
	ed_viewport* pCVar5;
	ed_3D_Scene* pStaticMeshMaster;
	SceneConfig* pCVar6;
	CameraObjParams* pCVar7;
	CameraObjParams* pCVar8;
	ed_surface_desc local_20;
	CameraObjParams local_8;

	pVidModeDataA = pTemplate->pViewport->pColorBuffer;
	pVidModeDataB = pTemplate->pViewport->pZBuffer;
	if (0x100 < (uint)(pTemplate->sceneConfig).pShadowConfig.field_0x1c) {
		(pTemplate->sceneConfig).pShadowConfig.field_0x1c = 0x100;
	}
	uVar2 = GetGreaterPower2Val((pTemplate->sceneConfig).pShadowConfig.field_0x18);
	uVar3 = GetGreaterPower2Val((pTemplate->sceneConfig).pShadowConfig.field_0x1c);
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
	pVidModeDataA_00 = ed3DShadowSurfaceNew(&local_20);
	pViewport = edViewportNew(&local_8, pVidModeDataA_00, pVidModeDataB, 1);
	edViewportSetBackgroundColor(pViewport, 0, 0, 0);
	pCVar5 = edViewportNew(&local_8, pVidModeDataA, pVidModeDataB, 0);
	pStaticMeshMaster = ed3DSceneCreate(pCamera, pViewport, 1);
	pStaticMeshMaster->bShadowScene = 1;
	pCVar6 = ed3DSceneGetConfig(pStaticMeshMaster);
	pCVar6->field_0x4 = (pTemplate->sceneConfig).pShadowConfig.field_0x4;
	pCVar6->field_0x18 = (pTemplate->sceneConfig).pShadowConfig.field_0x4;
	pCVar6->farClip = (pTemplate->sceneConfig).pShadowConfig.field_0xc;
	pCVar6->nearClip = (pTemplate->sceneConfig).pShadowConfig.field_0x8;
	pCVar6->field_0x14 = (pTemplate->sceneConfig).field_0x12c;
	pCVar6->field_0x10 = (pTemplate->sceneConfig).field_0x128;
	(pCVar6->pShadowConfig).pCamera_0x10 = pViewport;
	(pCVar6->pShadowConfig).pCamera_0x14 = pCVar5;
	(pCVar6->pShadowConfig).field_0x20 = (pTemplate->sceneConfig).pShadowConfig.field_0x20;
	(pCVar6->pShadowConfig).field_0x23 = (pTemplate->sceneConfig).pShadowConfig.field_0x23;
	(pCVar6->pShadowConfig).field_0x22 = (pTemplate->sceneConfig).pShadowConfig.field_0x22;
	(pCVar6->pShadowConfig).field_0x24 = (pTemplate->sceneConfig).pShadowConfig.field_0x24;
	(pCVar6->pShadowConfig).field_0x18 = (pTemplate->sceneConfig).pShadowConfig.field_0x18;
	(pCVar6->pShadowConfig).field_0x1c = (pTemplate->sceneConfig).pShadowConfig.field_0x1c;
	(pTemplate->sceneConfig).pShadowConfig.field_0x20 = 0;
	gShadowScene = pStaticMeshMaster;
	return pStaticMeshMaster;
}

ed_3D_Scene* ed3DSceneCastShadow(ed_3D_Scene* p3DScene, edFCamera* pCamera)
{
	ed_3D_Scene* pDisplayList;

	p3DScene->flags_0x4 = p3DScene->flags_0x4 | 2;
	pDisplayList = ed3DShadowCreateScene(p3DScene, pCamera);
	edListAddNode(&p3DScene->headerA, (DisplayListInternal*)pDisplayList);
	return pDisplayList;
}

Scene::Scene()
{
	void** errorPrinter;
	//LocalizationManager* pManager;
	//Manager_170* pMVar1;
	FrontendManager* pFrontendManager;
	//HelpManager* pHelpManager;
	PauseManager* pPVar2;
	//MapManager* pMVar3;
	CameraManager* pCameraViewmanager;
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
	edSurface* pVVar12;
	edSurface* pVVar13;
	ed_viewport* pCVar14;
	SceneConfig* pCVar15;
	ed_3D_Scene* pSVar16;
	undefined8 uVar17;
	//Manager_29b4** ppMVar18;
	Scene* puVar5;
	int iVar19;
	ed_3D_Scene** ppSVar20;
	edFCamera* pCVar21;
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
	this->pCameraObj28_0x4 = (ed_viewport*)0x0;
	this->pCameraObj28_0x8 = (ed_viewport*)0x0;
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
		LevelScheduleManager::gThis = g_ManagerSingletonArray_00451660.g_LevelScheduleManager_00451660;
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
	pCameraViewmanager = new CameraManager();
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
	local_8.posY = 0;
	local_8.posX = 0;
	local_8.screenWidth = peVar11->pActiveVidParams->params26.screenWidth;
	local_8.screenHeight = peVar11->pActiveVidParams->params26.screenHeight;
	pVVar12 = GetFrameBuffer_001ba9c0();
	pVVar13 = GetFrameBuffer_001ba9d0();
	pCVar14 = edViewportNew(&local_8, pVVar12, pVVar13, 3);
	this->pCameraObj28_0x4 = pCVar14;
	edViewportSetBackgroundColor(this->pCameraObj28_0x4, 0, 0, 0);
	Scene::_scene_handleA = ed3DSceneCreate(&CameraObj_130_0044a100, this->pCameraObj28_0x4, 1);
	Scene::_scene_handleA->ed3DSceneSetFlag(0x20);
	Scene::_scene_handleA->ed3DSceneSetFogProperty(1);
	Scene::_scene_handleA->SetFlag_002a5440(1);
	pCVar15 = ed3DSceneGetConfig(Scene::_scene_handleA);
	pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	(pCVar15->pShadowConfig).field_0x18 = 0x200;
	(pCVar15->pShadowConfig).field_0x1c = 0x100;
	pCVar15->field_0x12c = pCVar15->field_0x14 * 800;
	pCVar15->field_0x128 = 1;
	(pCVar15->pShadowConfig).field_0x0 = 0;
	(pCVar15->pShadowConfig).field_0x4 = 90.0;
	(pCVar15->pShadowConfig).field_0x8 = -0.01;
	(pCVar15->pShadowConfig).field_0xc = -150.0;
	(pCVar15->pShadowConfig).field_0x22 = 0x32;
	pVVar12 = GetFrameBuffer_001ba9c0();
	pVVar13 = GetFrameBuffer_001ba9d0();
	pCVar14 = edViewportNew(&local_8, pVVar12, pVVar13, 0);
	this->pCameraObj28_0x8 = pCVar14;
	edViewportSetBackgroundColor(this->pCameraObj28_0x8, 0, 0, 0);
	_scene_handleB = ed3DSceneCreate(&CameraObj_130_0044a100, this->pCameraObj28_0x8, 1);
	_scene_handleB->ed3DSceneSetFlag(0);
	_scene_handleB->ed3DSceneSetFogProperty(0);
	pCVar15 = ed3DSceneGetConfig(_scene_handleB);
	pCVar15->field_0x14 = (uint)pCVar15->field_0x14 >> 3;
	ed3DSceneGetConfig(Scene::_scene_handleA);
	iVar19 = 0;
	pCVar21 = CameraObj_130_ARRAY_0044a230;
	ppSVar20 = g_CameraPanStaticMasterArray_00451630;
	do {
		pSVar16 = ed3DSceneCastShadow(Scene::_scene_handleA, pCVar21);
		*ppSVar20 = pSVar16;
		(*ppSVar20)->ed3DSceneSetFlag(4);
		iVar19 = iVar19 + 1;
		pCVar21 = pCVar21 + 1;
		ppSVar20 = ppSVar20 + 1;
	} while (iVar19 < 10);
	this->count_0x120 = 0;
	this->field_0x11c = 0;
	Scene::_pinstance = this;
	return;
}


void Scene::CreateScene(void)
{
	new Scene;
}

void Game_Init(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->Game_Init();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	GlobalDList_Init();
	return;
}

void Scene::LevelLoading_Draw(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->LevelLoading_Draw();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	GlobalDList_AddToView();
	return;
}

int gCurTime = 0;

void ed3DResetTime(void)
{
	gCurTime = 0;
	return;
}


void Scene::LevelLoading_Begin(void)
{
	TimeController* pTimeController;
	Manager** loadLoopObject;
	int iVar1;

	pTimeController = GetTimer();
	pTimeController->ResetGameTimers();
	ed3DResetTime();
	iVar1 = 0;
	loadLoopObject = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*loadLoopObject != (Manager*)0x0) {
			(*loadLoopObject)->LevelLoading_Begin();
		}
		iVar1 = iVar1 + 1;
		loadLoopObject = loadLoopObject + 1;
	} while (iVar1 < 0x18);
	return;
}

bool Scene::LevelLoading_Manage()
{
	bool bVar1;
	Manager** loopFunc;
	int iVar2;
	bool bVar3;

	bVar3 = false;
	//HandleFogAndClippingSettings(param_1);
	iVar2 = 0;
	loopFunc = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if ((*loopFunc != (Manager*)0x0) && (bVar1 = (*loopFunc)->LevelLoading_Manage(), bVar1 != false)) {
			bVar3 = true;
		}
		iVar2 = iVar2 + 1;
		loopFunc = loopFunc + 1;
	} while (iVar2 < 0x18);
	return bVar3;
}

void Scene::LevelLoading_End(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->LevelLoading_End();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	return;
}

void Scene::Level_Install(void)
{
	Manager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		if (*ppMVar1 != (Manager*)0x0) {
			(*ppMVar1)->Level_Install();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	return;
}

void edViewportSetClearMask(ed_viewport* pViewport, uint clearMask)
{
	pViewport->clearMask = clearMask;
	return;
}

byte gbDoMipmap = 0;
uint gMipmapK = 0;
uint gMipmapL = 0;

void ed3DSetMipmapProp(bool bDoMipmap, uint mipMapL, uint mipMapK)
{
	gbDoMipmap = bDoMipmap;
	gMipmapK = mipMapK;
	gMipmapL = mipMapL;
	return;
}

void Scene::Level_Init()
{
	undefined4* puVar1;
	TimeController* pTimeController;
	Manager** loadFuncPtr;
	int loopCounter;

	if (this->curState != 0) {
		this->field_0x44 = 0.0;
		this->curState = 0;
	}
	this->field_0x44 = 0.0;
	if ((this->field_0xc[3] & 1) == 0) {
		this->field_0xcc = this->field_0xcc + 1;
		this->field_0xd8 = this->field_0xe8;
		this->field_0xdc = this->field_0xec;
		this->field_0xe0 = this->field_0xf0;
		this->field_0xe4 = this->field_0xf4;
		this->field_0xd0 = 0.0;
		this->field_0xd4 = 0.0;
		loopCounter = this->field_0xcc;
		(&this->field_0x0 + loopCounter * 2)[0x13] = (int)this->field_0xc;
		(&this->field_0x0 + loopCounter * 2)[0x14] = (int)0x0;
	}
	puVar1 = this->field_0xc;
	this->field_0xd8 = *puVar1;
	this->field_0xdc = puVar1[1];
	this->field_0xe0 = (undefined4)puVar1[2];
	this->field_0xe4 = puVar1[3];
	puVar1 = this->field_0xc;
	this->field_0xe8 = *puVar1;
	this->field_0xec = puVar1[1];
	this->field_0xf0 = (undefined4)puVar1[2];
	this->field_0xf4 = puVar1[3];
	this->field_0x48 = 0;
	ed3DSetMipmapProp(1, this->field_0x14, this->field_0x10);
	if ((g_ManagerSingletonArray_00451660.g_FileManager3D_00451664)->pMeshTransformParent == (MeshTransformParent*)0x0) {
		edViewportSetClearMask(this->pCameraObj28_0x4, 0);
	}
	else {
		edViewportSetClearMask(this->pCameraObj28_0x4, 0xffffffff);
	}
	edViewportSetClearMask(this->pCameraObj28_0x8, 0);
	pTimeController = GetTimer();
	pTimeController->ResetGameTimers();
	ed3DResetTime();
	this->field_0x38 = 1;
	//EffectsManager::Level_PreInit((EffectsManager*)g_ManagerSingletonArray_00451660[22]);
	loopCounter = 0;
	/* Init loop Initially points at 006db5b0 */
	loadFuncPtr = (Manager**)&g_ManagerSingletonArray_00451660;
	do {
		/* This will call load functions */
		if (*loadFuncPtr != (Manager*)0x0) {
			(*loadFuncPtr)->Level_Init();
		}
		loopCounter = loopCounter + 1;
		loadFuncPtr = loadFuncPtr + 1;
	} while (loopCounter < 0x18);
	this->field_0x38 = 1;
	//edColFreeTemporaryMemory();
	//(*(code*)LevelScheduleManager::gThis->pManagerFunctionData[1].field_0x0)();
	//WillLoadCinematic(g_CinematicManager_0048efc);
	//FUN_003f95f0((MapManager*)g_ManagerSingletonArray_00451660[11]);
	if (this->field_0xcc != -1) {
		//puVar1 = (undefined4*)(&param_1->field_0x4c)[param_1->field_0xcc * 2];
		//param_1->field_0xe8 = *puVar1;
		//param_1->field_0xec = puVar1[1];
		//param_1->field_0xf0 = (undefined4)puVar1[2];
		//param_1->field_0xf4 = puVar1[3];
		//param_1->field_0xd0 = param_1->field_0xd4;
	}
	return;
}

void Scene::HandleCurState()
{
	TimeController* pTVar1;
	ulong uVar2;

	//HandleFogAndClippingSettings(param_1);
	pTVar1 = GetTimer();
	this->field_0x44 = this->field_0x44 + pTVar1->lastFrameTime;
	switch (this->curState) {
	case 1:
		IMPLEMENTATION_GUARD();
		//FUN_001a0180(1.0, 2, 0);
		//if (param_1->field_0x40 != 2) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 2;
		//}
		break;
	case 2:
		IMPLEMENTATION_GUARD();
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
		IMPLEMENTATION_GUARD();
		//g_DebugCameraFlag_00448ea4 = g_DebugCameraFlag_00448ea4 & 0xfffffe7f;
		//LargeObject::OnSaveLoaded_001b8d40(param_1);
		//FUN_001a0180(1.0, 1, 0);
		//if (param_1->field_0x40 != 0) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 0;
		//}
		break;
	case 4:
		IMPLEMENTATION_GUARD();
		//FUN_001a0180(1.0, 2, 0);
		//if (param_1->field_0x40 != 5) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 5;
		//}
		break;
	case 5:
		IMPLEMENTATION_GUARD();
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
		IMPLEMENTATION_GUARD();
		//FUN_001a0180(1.0, 2, 0);
		//if (param_1->field_0x40 != 7) {
		//	param_1->field_0x44 = 0.0;
		//	param_1->field_0x40 = 7;
		//}
		break;
	case 7:
		IMPLEMENTATION_GUARD();
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

void Scene::Level_Manage()
{
	Manager** ppMVar1;
	int iVar2;

	if (this->field_0x48 == 0) {
		if ((GameFlags & 0x200) != 0) {
			Scene::_scene_handleA->RemoveFlag_002a53e0(4);
			_scene_handleB->RemoveFlag_002a53e0(4);
			GameFlags = GameFlags & 0xfffffdff;
		}
	}
	else {
		if ((GameFlags & 0x200) == 0) {
			GameFlags = GameFlags | 0x200;
			Scene::_scene_handleA->ed3DSceneSetFlag(4);
			_scene_handleB->ed3DSceneSetFlag(4);
		}
	}
	//EmptyFunction();
	if ((GameFlags & 0x20) == 0) {
		iVar2 = 0;
		ppMVar1 = (Manager**)&g_ManagerSingletonArray_00451660;
		do {
			if (*ppMVar1 != (Manager*)0x0) {
				(*ppMVar1)->Level_Manage();
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
				(*ppMVar1)->Level_ManagePaused();
			}
			iVar2 = iVar2 + 1;
			ppMVar1 = ppMVar1 + 1;
		} while (iVar2 < 0x18);
	}
	HandleCurState();
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
			(*ppMVar1)->Level_Draw();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	GlobalDList_AddToView();
	return;
}

char* s_Main_Camera_0042b460 = "Main Camera";

void Scene::Level_Setup(ByteCode* pMemoryStream)
{
	undefined4* puVar1;
	int* piVar2;
	uint uVar3;
	int iVar4;
	MainCamera* pCVar5;
	float fVar5;
	uint* local_4;

	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU64();
	pMemoryStream->GetS32();
	pMemoryStream->GetS32();
	uVar3 = pMemoryStream->GetU32();
	this->field_0x18 = uVar3;
	puVar1 = (undefined4*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(puVar1 + 4);
	this->field_0xc = puVar1;
	iVar4 = pMemoryStream->GetS32();
	this->field_0x14 = iVar4;
	iVar4 = pMemoryStream->GetS32();
	this->field_0x10 = iVar4;
	/* Main Camera */
	pCVar5 = (MainCamera*)g_ManagerSingletonArray_00451660.g_CameraManager_0045167c->AddCamera(CT_MainCamera, pMemoryStream, s_Main_Camera_0042b460);
	fVar5 = pMemoryStream->GetF32();
	this->field_0x118 = fVar5;
	//pCVar5->field_0x2dc = fVar5;
	uVar3 = pMemoryStream->GetU32();
	this->field_0x11c = uVar3;
	uVar3 = pMemoryStream->GetU32();
	this->count_0x120 = uVar3;
	LoadFunc_001b87b0();
	//MapManager::OnLoadLevelBnk_003f9a60(g_ManagerSingletonArray_00451660.g_MapManager_0045168c, pMemoryStream);
	iVar4 = pMemoryStream->GetS32();
	this->field_0x28 = iVar4;
	if (this->field_0x28 != -1) {
		IMPLEMENTATION_GUARD(FileManager3D::InstanciateG2D(g_ManagerSingletonArray_00451660.g_FileManager3D_00451664, this->field_0x28));
	}
	iVar4 = pMemoryStream->GetS32();
	this->defaultTextureIndex_0x2c = iVar4;
	//FUN_002cbc90((undefined4*)&this->field_0xfc);
	iVar4 = pMemoryStream->GetS32();
	this->field_0x30 = iVar4;
	//EffectsManager::OnLoadLevelBnk_001a1790(g_ManagerSingletonArray_00451660.g_EffectsManager_004516b8, pMemoryStream);
	local_4 = (uint*)0x0;
	//MemoryStreamReadFunc_00117330(&local_4, pMemoryStream);
	//FUN_00117210(&local_4);
	//LevelScheduleManager::OnLoadLevelBnk_002d9f80(LevelScheduleManager::gThis, pMemoryStream);
	iVar4 = pMemoryStream->GetS32();
	this->field_0x10c = iVar4;
	iVar4 = pMemoryStream->GetS32();
	this->field_0x110 = iVar4;
	piVar2 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 4;
	}
	//this->field_0x114 = piVar2;
	return;
}

bool Scene::CheckFunc_001b9300()
{
	return this->curState - 6U < 2;
}

void Func_002b6db0(ed_3D_Scene* pStaticMeshMaster, uint width, uint height)
{
	ed_viewport* peVar1;
	ed_viewport* peVar2;
	SceneConfig* pSVar3;
	uint uVar4;
	uint uVar5;
	int psVar7;
	short* psVar6;
	ushort uVar7;
	ushort uVar8;
	short local_8[4];
	int psVar3;

	pSVar3 = ed3DSceneGetConfig(pStaticMeshMaster);
	peVar1 = (pSVar3->pShadowConfig).pCamera_0x10;
	peVar2 = (pSVar3->pShadowConfig).pCamera_0x14;
	uVar4 = GetGreaterPower2Val(width);
	uVar5 = GetGreaterPower2Val(height);
	if (0x100 < (int)uVar5) {
		uVar5 = 0x100;
	}
	psVar6 = local_8;
	if (0x200 < (int)uVar4) {
		uVar4 = 0x200;
	}
	memset(local_8, 0, sizeof(local_8));
	uVar8 = (ushort)uVar4;
	uVar7 = (ushort)uVar5;
	(pSVar3->pShadowConfig).field_0x18 = uVar4;
	(pSVar3->pShadowConfig).field_0x1c = uVar5;
	peVar1->posX = local_8[0];
	peVar1->posY = local_8[1];
	peVar1->screenWidth = uVar8;
	peVar1->screenHeight = uVar7;
	peVar1->pColorBuffer->pSurfaceDesc->screenWidth = uVar8;
	peVar1->pColorBuffer->pSurfaceDesc->screenHeight = uVar7;
	peVar2->posX = local_8[0];
	peVar2->posY = local_8[1];
	peVar2->screenWidth = uVar8;
	peVar2->screenHeight = uVar7;
	return;
}

void Scene::LoadFunc_001b87b0()
{
	SceneConfig* pCVar1;
	ed_3D_Scene** ppSVar2;
	uint unaff_s1_lo;
	uint unaff_s2_lo;
	uint uVar3;

	if (this->field_0x11c == 0) {
		this->count_0x120 = 1;
	}
	if (10 < this->count_0x120) {
		this->count_0x120 = 10;
	}
	ed3DSceneGetConfig(Scene::_scene_handleA);
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
			(*ppSVar2)->ed3DSceneSetFlag(4);
		}
		Func_002b6db0(*ppSVar2, unaff_s2_lo, unaff_s1_lo);
		pCVar1 = ed3DSceneGetConfig(*ppSVar2);
		(pCVar1->pShadowConfig).field_0x20 = (short)(1 << (uVar3 & 0x1f));
		uVar3 = uVar3 + 1;
		ppSVar2 = ppSVar2 + 1;
		pCVar1->field_0x12c = pCVar1->field_0x14 * 800;
		pCVar1->field_0x128 = 1;
		(pCVar1->pShadowConfig).field_0x0 = 0;
		(pCVar1->pShadowConfig).field_0x4 = 90.0;
		(pCVar1->pShadowConfig).field_0x8 = -0.01;
		(pCVar1->pShadowConfig).field_0xc = -150.0;
		(pCVar1->pShadowConfig).field_0x22 = 0x32;
		(pCVar1->pShadowConfig).field_0x23 = 0;
		(pCVar1->pShadowConfig).field_0x24 = 0;
	} while ((int)uVar3 < 10);
	return;
}
