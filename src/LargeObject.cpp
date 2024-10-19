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
#include "edVideo/Viewport.h"
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
#include "WayPoint.h"
#include "MapManager.h"
#include "Fx.h"
#include "Fx.h"
#include "PathManager.h"
#include "ActorManager.h"
#include "AnmManager.h"
#include "DlistManager.h"
#include "CollisionManager.h"
#include "LightManager.h"
#include "EventManager.h"
#include "EventTrack.h"
#include "CameraGame.h"
#include "kya.h"

#define SCENE_STATE_RESET 0x5

CScene* CScene::_pinstance = NULL;

ManagerContainer CScene::ptable = { 0 };

ed_3D_Scene* CScene::_scene_handleA = NULL;

ed_3D_Scene* CScene::_scene_handleB = NULL;

ed_3D_Scene* g_CameraPanStaticMasterArray_00451630[10] = { 0 };

uint GameFlags = 0;

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
	if (0x100 < (uint)(pTemplate->sceneConfig).pShadowConfig.texHeight) {
		(pTemplate->sceneConfig).pShadowConfig.texHeight = 0x100;
	}
	uVar2 = GetGreaterPower2Val((pTemplate->sceneConfig).pShadowConfig.texWidth);
	uVar3 = GetGreaterPower2Val((pTemplate->sceneConfig).pShadowConfig.texHeight);
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
	pCVar6->clipValue_0x4 = (pTemplate->sceneConfig).pShadowConfig.field_0x4;
	pCVar6->clipValue_0x18 = (pTemplate->sceneConfig).pShadowConfig.field_0x4;
	pCVar6->farClip = (pTemplate->sceneConfig).pShadowConfig.field_0xc;
	pCVar6->nearClip = (pTemplate->sceneConfig).pShadowConfig.field_0x8;
	pCVar6->projectionScaleFactorB = (pTemplate->sceneConfig).field_0x12c;
	pCVar6->projectionScaleFactorA = (pTemplate->sceneConfig).field_0x128;
	(pCVar6->pShadowConfig).pCamera_0x10 = pViewport;
	(pCVar6->pShadowConfig).pViewport = pCVar5;
	(pCVar6->pShadowConfig).renderMask = (pTemplate->sceneConfig).pShadowConfig.renderMask;
	(pCVar6->pShadowConfig).field_0x23 = (pTemplate->sceneConfig).pShadowConfig.field_0x23;
	(pCVar6->pShadowConfig).field_0x22 = (pTemplate->sceneConfig).pShadowConfig.field_0x22;
	(pCVar6->pShadowConfig).field_0x24 = (pTemplate->sceneConfig).pShadowConfig.field_0x24;
	(pCVar6->pShadowConfig).texWidth = (pTemplate->sceneConfig).pShadowConfig.texWidth;
	(pCVar6->pShadowConfig).texHeight = (pTemplate->sceneConfig).pShadowConfig.texHeight;
	(pTemplate->sceneConfig).pShadowConfig.renderMask = 0;
	gShadowScene = pStaticMeshMaster;
	return pStaticMeshMaster;
}

ed_3D_Scene* ed3DSceneCastShadow(ed_3D_Scene* p3DScene, edFCamera* pCamera)
{
	ed_3D_Scene* pDisplayList;

	p3DScene->flags = p3DScene->flags | SCENE_FLAG_CAST_SHADOW;
	pDisplayList = ed3DShadowCreateScene(p3DScene, pCamera);
	edListAddNode(&p3DScene->meshClusterShadowList, (DisplayListInternal*)pDisplayList);
	return pDisplayList;
}

CScene::CScene()
{
	void** errorPrinter;
	//LocalizationManager* pManager;
	//Manager_170* pMVar1;
	FrontendManager* pFrontendManager;
	//HelpManager* pHelpManager;
	CPauseManager* pPVar2;
	//MapManager* pMVar3;
	CCameraManager* pCameraViewmanager;
	CSectorManager* pSectorManager;
	//LightManager* pLightManager;
	C3DFileManager* p3DFileManager;
	//Manager_100* pMVar4;
	//Manager_208* pMVar5;
	//CAudioManager* inAllocatedMemory;
	//Manager_C* pMVar6;
	//Manager_10* pMVar7;
	//ActorManager* pActorManager;
	CCinematicManager* pNewCinematicObject;
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
	CScene* puVar5;
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
	this->fogClipSettingStackSize = -1;
	this->field_0xd0 = 0.0;
	//ppMVar18 = &g_Manager29B4_004516bc;
	this->field_0xd4 = 0.0;
	this->clipValue_0xe8 = 0;
	this->field_0xec = 0;
	this->fogRGBA = _rgba(0, 0, 0, 0);
	this->fogFlags = 0;
	this->field_0xd8 = this->clipValue_0xe8;
	this->field_0xdc = this->field_0xec;
	this->prevFogRGBA = this->fogRGBA;
	this->prevFogFlags = this->fogFlags;
	this->pViewportA = (ed_viewport*)0x0;
	this->pViewportB = (ed_viewport*)0x0;
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
	CScene::ptable.g_LevelScheduleManager_00451660 = new CLevelScheduler();
	/* Initial load point at 006db5b0 */
	if (CScene::ptable.g_LevelScheduleManager_00451660 != (CLevelScheduler*)0x0) {
		CLevelScheduler::gThis = CScene::ptable.g_LevelScheduleManager_00451660;
	}
	pManager = new LocalizationManager();
	CScene::ptable.g_LocalizationManager_00451678 = pManager;
	//g_Manager1e4_00451678 = pManager;
	//pMVar1 = (Manager_170*)Allocate(0x170);
	//if (pMVar1 != (Manager_170*)0x0) {
	//	uVar17 = ManagerConstructor_001dba90(pMVar1);
	//	pMVar1 = (Manager_170*)uVar17;
	//}
	//g_Manager170_00451674 = pMVar1;
	pFrontendManager = new FrontendManager();
	CScene::ptable.g_FrontendManager_00451680 = pFrontendManager;
	//pHelpManager = (HelpManager*)Allocate(0x1e4);
	//if (pHelpManager != (HelpManager*)0x0) {
	//	uVar17 = SetupHelpManager_0037bc70(pHelpManager);
	//	pHelpManager = (HelpManager*)uVar17;
	//}
	//g_HelpManager_00451684 = pHelpManager;
	pPVar2 = new CPauseManager();
	CScene::ptable.g_PauseManager_00451688 = pPVar2;
	CScene::ptable.g_MapManager_0045168c = new MapManager;
	pCameraViewmanager = new CCameraManager;
	CScene::ptable.g_CameraManager_0045167c = pCameraViewmanager;
	pSectorManager = new CSectorManager;
	CScene::ptable.g_SectorManager_00451670 = pSectorManager;
	CScene::ptable.g_LightManager_004516b0 = new CLightManager;
	p3DFileManager = new C3DFileManager();
	CScene::ptable.g_C3DFileManager_00451664 = p3DFileManager;
	CScene::ptable.g_CollisionManager_00451690 = new CCollisionManager;
	//g_Manager100_00451690 = pMVar4;
	//pMVar5 = (Manager_208*)Allocate(0x208);
	//if (pMVar5 != (Manager_208*)0x0) {
	//	uVar17 = SetupManager208_001ad8f0(pMVar5);
	//	pMVar5 = (Manager_208*)uVar17;
	//}
	//g_Manager208_00451694 = pMVar5;
	//inAllocatedMemory = (CAudioManager*)Allocate(0x2e8);
	//if (inAllocatedMemory != (CAudioManager*)0x0) {
	//	inAllocatedMemory = (CAudioManager*)GlobalSoundConstructor(inAllocatedMemory);
	//}
	//g_GlobalSoundPtr_00451698 = inAllocatedMemory;
	CScene::ptable.g_WayPointManager_0045169c = new CWayPointManager;
	CScene::ptable.g_PathManager_004516a0 = new CPathManager;
	CScene::ptable.g_ActorManager_004516a4 = new CActorManager;
	CScene::ptable.g_CinematicManagerB_0045166c = new CCinematicManagerB;
	pNewCinematicObject = new CCinematicManager;
	CScene::ptable.g_CinematicManagerPtr_004516ac = pNewCinematicObject;

	CScene::ptable.g_AnimManager_00451668 = new CAnimationManager;
	CScene::ptable.g_TrackManager_004516b4 = new CTrackManager;
	CScene::ptable.g_EffectsManager_004516b8 = new CFxManager;
	CScene::ptable.g_EventManager_006f5080 = new CEventManager;
	CScene::ptable.g_GlobalDListManager_004516bc = new CGlobalDListManager;
	peVar11 = edVideoGetInfo();
	local_8.posY = 0;
	local_8.posX = 0;
	local_8.screenWidth = peVar11->pActiveVidParams->params26.screenWidth;
	local_8.screenHeight = peVar11->pActiveVidParams->params26.screenHeight;
	pVVar12 = VideoGetDisplaybuffer();
	pVVar13 = VideoGetZbuffer();
	pCVar14 = edViewportNew(&local_8, pVVar12, pVVar13, 3);
	this->pViewportA = pCVar14;
	edViewportSetBackgroundColor(this->pViewportA, 0, 0, 0);
	CScene::_scene_handleA = ed3DSceneCreate(&_gDisplayCamera, this->pViewportA, 1);
	CScene::_scene_handleA->ed3DSceneSetFlag(0x20);
	CScene::_scene_handleA->ed3DSceneSetFogProperty(1);
	CScene::_scene_handleA->SetFlag_002a5440(1);
	pCVar15 = ed3DSceneGetConfig(CScene::_scene_handleA);
	pCVar15->projectionScaleFactorB = (uint)pCVar15->projectionScaleFactorB >> 3;
	(pCVar15->pShadowConfig).texWidth = 0x200;
	(pCVar15->pShadowConfig).texHeight = 0x100;
	pCVar15->field_0x12c = pCVar15->projectionScaleFactorB * 800;
	pCVar15->field_0x128 = 1;
	(pCVar15->pShadowConfig).field_0x0 = 0;
	(pCVar15->pShadowConfig).field_0x4 = 90.0;
	(pCVar15->pShadowConfig).field_0x8 = -0.01;
	(pCVar15->pShadowConfig).field_0xc = -150.0;
	(pCVar15->pShadowConfig).field_0x22 = 0x32;
	pVVar12 = VideoGetDisplaybuffer();
	pVVar13 = VideoGetZbuffer();
	pCVar14 = edViewportNew(&local_8, pVVar12, pVVar13, 0);
	this->pViewportB = pCVar14;
	edViewportSetBackgroundColor(this->pViewportB, 0, 0, 0);
	_scene_handleB = ed3DSceneCreate(&_gDisplayCamera, this->pViewportB, 1);
	_scene_handleB->ed3DSceneSetFlag(0);
	_scene_handleB->ed3DSceneSetFogProperty(0);
	pCVar15 = ed3DSceneGetConfig(_scene_handleB);
	pCVar15->projectionScaleFactorB = (uint)pCVar15->projectionScaleFactorB >> 3;
	ed3DSceneGetConfig(CScene::_scene_handleA);
	iVar19 = 0;
	pCVar21 = gSceneCameras;
	ppSVar20 = g_CameraPanStaticMasterArray_00451630;
	do {
		pSVar16 = ed3DSceneCastShadow(CScene::_scene_handleA, pCVar21);
		*ppSVar20 = pSVar16;
		(*ppSVar20)->ed3DSceneSetFlag(4);
		iVar19 = iVar19 + 1;
		pCVar21 = pCVar21 + 1;
		ppSVar20 = ppSVar20 + 1;
	} while (iVar19 < 10);
	this->count_0x120 = 0;
	this->field_0x11c = 0;
	CScene::_pinstance = this;
	return;
}


void CScene::CreateScene(void)
{
	new CScene;
}

void Game_Init(void)
{
	CObjectManager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = CScene::ptable.aManagers;
	do {
		if (*ppMVar1 != (CObjectManager*)0x0) {
			(*ppMVar1)->Game_Init();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	GlobalDList_Init();
	return;
}

void CScene::LevelLoading_Draw(void)
{
	CObjectManager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = CScene::ptable.aManagers;
	do {
		if (*ppMVar1 != (CObjectManager*)0x0) {
			(*ppMVar1)->LevelLoading_Draw();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	GlobalDList_AddToView();
	return;
}

void CScene::LevelLoading_Begin(void)
{
	Timer* pTimeController;
	CObjectManager** loadLoopObject;
	int iVar1;

	pTimeController = GetTimer();
	pTimeController->ResetGameTimers();
	ed3DResetTime();
	iVar1 = 0;
	loadLoopObject = CScene::ptable.aManagers;
	do {
		if (*loadLoopObject != (CObjectManager*)0x0) {
			(*loadLoopObject)->LevelLoading_Begin();
		}
		iVar1 = iVar1 + 1;
		loadLoopObject = loadLoopObject + 1;
	} while (iVar1 < 0x18);
	return;
}

bool CScene::LevelLoading_Manage()
{
	bool bVar1;
	CObjectManager** loopFunc;
	int iVar2;
	bool bVar3;

	bVar3 = false;
	HandleFogAndClippingSettings();
	iVar2 = 0;
	loopFunc = CScene::ptable.aManagers;
	do {
		if ((*loopFunc != (CObjectManager*)0x0) && (bVar1 = (*loopFunc)->LevelLoading_Manage(), bVar1 != false)) {
			bVar3 = true;
		}
		iVar2 = iVar2 + 1;
		loopFunc = loopFunc + 1;
	} while (iVar2 < 0x18);
	return bVar3;
}

void CScene::LevelLoading_End(void)
{
	CObjectManager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = CScene::ptable.aManagers;
	do {
		if (*ppMVar1 != (CObjectManager*)0x0) {
			(*ppMVar1)->LevelLoading_End();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	return;
}

void CScene::Level_Install(void)
{
	CObjectManager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = CScene::ptable.aManagers;
	do {
		if (*ppMVar1 != (CObjectManager*)0x0) {
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

void CScene::Level_Init()
{
	undefined4* puVar1;
	Timer* pTimeController;
	CObjectManager** loadFuncPtr;
	int loopCounter;

	if (this->curState != 0) {
		this->timeInState = 0.0;
		this->curState = 0;
	}
	this->timeInState = 0.0;
	if ((this->pFogClipStream->flags & 1) == 0) {
		this->fogClipSettingStackSize = this->fogClipSettingStackSize + 1;
		this->field_0xd8 = this->clipValue_0xe8;
		this->field_0xdc = this->field_0xec;
		this->prevFogRGBA = this->fogRGBA;
		this->prevFogFlags = this->fogFlags;
		this->field_0xd0 = 0.0;
		this->field_0xd4 = 0.0;
		loopCounter = this->fogClipSettingStackSize;
		this->aFogClipStack[loopCounter].pStreamDef = this->pFogClipStream;
		this->aFogClipStack[loopCounter].field_0x4 = 0.0f;
	}
	S_STREAM_FOG_DEF* pSVar1 = this->pFogClipStream;
	this->field_0xd8 = pSVar1->clipValue_0x0;
	this->field_0xdc = pSVar1->field_0x4;
	this->prevFogRGBA = pSVar1->fogRGBA;
	this->prevFogFlags = pSVar1->flags;
	pSVar1 = this->pFogClipStream;
	this->clipValue_0xe8 = pSVar1->clipValue_0x0;
	this->field_0xec = pSVar1->field_0x4;
	this->fogRGBA = pSVar1->fogRGBA;
	this->fogFlags = pSVar1->flags;
	this->field_0x48 = 0;
	ed3DSetMipmapProp(true, this->mipmapL, this->mipmapK);
	if ((CScene::ptable.g_C3DFileManager_00451664)->pBackgroundNode == (edNODE*)0x0) {
		edViewportSetClearMask(this->pViewportA, 0);
	}
	else {
		edViewportSetClearMask(this->pViewportA, 0xffffffff);
	}
	edViewportSetClearMask(this->pViewportB, 0);
	pTimeController = GetTimer();
	pTimeController->ResetGameTimers();
	ed3DResetTime();
	this->field_0x38 = 1;
	//EffectsManager::Level_PreInit((EffectsManager*)Scene::ptable[22]);
	loopCounter = 0;
	/* Init loop Initially points at 006db5b0 */
	loadFuncPtr = CScene::ptable.aManagers;
	do {
		/* This will call load functions */
		if (*loadFuncPtr != (CObjectManager*)0x0) {
			(*loadFuncPtr)->Level_Init();
		}
		loopCounter = loopCounter + 1;
		loadFuncPtr = loadFuncPtr + 1;
	} while (loopCounter < 0x18);
	this->field_0x38 = 1;
	//edColFreeTemporaryMemory();
	//(*(code*)LevelScheduleManager::gThis->pManagerFunctionData[1].field_0x0)();
	g_CinematicManager_0048efc->WillLoadCinematic();
	//FUN_003f95f0((MapManager*)Scene::ptable[11]);
	if (this->fogClipSettingStackSize != -1) {
		//puVar1 = (undefined4*)(&param_1->field_0x4c)[param_1->field_0xcc * 2];
		//param_1->field_0xe8 = *puVar1;
		//param_1->field_0xec = puVar1[1];
		//param_1->field_0xf0 = (undefined4)puVar1[2];
		//param_1->field_0xf4 = puVar1[3];
		//param_1->field_0xd0 = param_1->field_0xd4;
	}
	return;
}

static void Fade(float param_1, int param_2, int param_3)
{
	ulong uVar1;
	float fVar2;
	uint uVar3;

	if (gVideoConfig.isNTSC == 1) {
		fVar2 = param_1 * 50.0f;
		if (fVar2 < 2.147484e+09f) {
			uVar3 = (uint)fVar2;
		}
		else {
			uVar3 = (int)(fVar2 - 2.147484e+09f) | 0x80000000;
		}
	}
	else {
		fVar2 = param_1 * 60.0f;
		if (fVar2 < 2.147484e+09f) {
			uVar3 = (uint)fVar2;
		}
		else {
			uVar3 = (int)(fVar2 - 2.147484e+09f) | 0x80000000;
		}
	}

	if (uVar3 == 0) {
		uVar3 = 1;
	}

	edVideoSetFadeColor(0, 0, 0);
	if (param_2 == 1) {
		edVideoSetFadeColor(0, 0, 0);
		edVideoSetFade(1.0);
		edVideoSetFadeIn(uVar3);
	}
	else {
		edVideoSetFadeColor(0, 0, 0);
		edVideoSetFade(0.0);
		edVideoSetFadeOut(uVar3, 1);
	}
	if (param_3 != 0) {
		uVar1 = edVideoIsFadeActive();
		while (uVar1 != 0) {
			edVideoFlip();
			uVar1 = edVideoIsFadeActive();
		}
	}
	return;
}

void CScene::HandleCurState()
{
	Timer* pTVar1;
	ulong uVar2;

	HandleFogAndClippingSettings();

	pTVar1 = GetTimer();
	this->timeInState = this->timeInState + pTVar1->lastFrameTime;

	switch (this->curState) {
	case 1:
		Fade(1.0f, 2, 0);

		if (this->curState != 2) {
			this->timeInState = 0.0f;
			this->curState = 2;
		}
		break;
	case 2:
		uVar2 = edVideoIsFadeActive();
		if (uVar2 == 0) {
			GameFlags = GameFlags & 0xfffffe7f;

			Level_CheckpointReset();

			Fade(1.0f, 1, 0);

			if (this->curState != 0) {
				this->timeInState = 0.0f;
				this->curState = 0;
			}
		}
		break;
	case 3:
		GameFlags = GameFlags & 0xfffffe7f;

		Level_CheckpointReset();

		Fade(1.0f, 1, 0);

		if (this->curState != 0) {
			this->timeInState = 0.0f;
			this->curState = 0;
		}
		break;
	case 4:
		Fade(1.0f, 2, 0);

		if (this->curState != SCENE_STATE_RESET) {
			this->timeInState = 0.0f;
			this->curState = SCENE_STATE_RESET;
		}
		break;
	case SCENE_STATE_RESET:
		uVar2 = edVideoIsFadeActive();
		if (uVar2 == 0) {
			GameFlags = GameFlags & 0xfffffe7f;

			Level_Reset();

			Fade(1.0f, 1, 0);

			if (this->curState != 0) {
				this->timeInState = 0.0f;
				this->curState = 0;
			}
		}
		break;
	case 6:
		Fade(1.0f, 2, 0);

		if (this->curState != 7) {
			this->timeInState = 0.0f;
			this->curState = 7;
		}
		break;
	case 7:
		uVar2 = edVideoIsFadeActive();
		if (uVar2 == 0) {
			Fade(1.0f, 1, 0);

			GameFlags = GameFlags | 2;

			if (this->curState != 0) {
				this->timeInState = 0.0f;
				this->curState = 0;
			}
		}
	}
	return;
}

void CScene::Level_CheckpointReset(void)
{
	S_STREAM_FOG_DEF* pSVar1;
	CObjectManager** pMVar2;
	int iVar3;

	iVar3 = 0;
	pMVar2 = CScene::ptable.aManagers;
	do {
		if (*pMVar2 != (CObjectManager*)0x0) {
			(*pMVar2)->Level_PreCheckpointReset();
		}
		iVar3 = iVar3 + 1;
		pMVar2 = pMVar2 + 1;
	} while (iVar3 < 0x18);

	iVar3 = 0;
	pMVar2 = CScene::ptable.aManagers;
	do {
		if (*pMVar2 != (CObjectManager*)0x0) {
			(*pMVar2)->Level_CheckpointReset();
		}
		iVar3 = iVar3 + 1;
		pMVar2 = pMVar2 + 1;
	} while (iVar3 < 0x18);

	if (this->fogClipSettingStackSize != -1) {
		pSVar1 = this->aFogClipStack[this->fogClipSettingStackSize].pStreamDef;
		this->clipValue_0xe8 = pSVar1->clipValue_0x0;
		this->field_0xec = pSVar1->field_0x4;
		this->fogRGBA = pSVar1->fogRGBA;
		this->fogFlags = pSVar1->flags;
		this->field_0xd0 = this->field_0xd4;
	}

	return;
}

void CScene::InitiateCheckpointReset(int param_2)
{
	if (this->curState == 0) {
		if (param_2 == 0) {
			this->timeInState = 0.0f;
			this->curState = 3;
		}
		else {
			this->timeInState = 0.0f;
			this->curState = 1;
		}

		GameFlags = GameFlags | 0x80;
	}

	return;
}

void CScene::Level_Reset()
{
	IMPLEMENTATION_GUARD();
}

void CScene::Level_Manage()
{
	CObjectManager** ppMVar1;
	int iVar2;

	if (this->field_0x48 == 0) {
		if ((GameFlags & 0x200) != 0) {
			CScene::_scene_handleA->ed3DSceneRemoveFlag(4);
			_scene_handleB->ed3DSceneRemoveFlag(4);
			GameFlags = GameFlags & 0xfffffdff;
		}
	}
	else {
		if ((GameFlags & 0x200) == 0) {
			GameFlags = GameFlags | 0x200;
			CScene::_scene_handleA->ed3DSceneSetFlag(4);
			_scene_handleB->ed3DSceneSetFlag(4);
		}
	}
	//EmptyFunction();
	if ((GameFlags & GAME_STATE_PAUSED) == 0) {
		iVar2 = 0;
		ppMVar1 = CScene::ptable.aManagers;
		do {
			if (*ppMVar1 != (CObjectManager*)0x0) {
				(*ppMVar1)->Level_Manage();
			}
			iVar2 = iVar2 + 1;
			ppMVar1 = ppMVar1 + 1;
		} while (iVar2 < 0x18);
	}
	else {
		iVar2 = 0;
		ppMVar1 = CScene::ptable.aManagers;
		do {
			if (*ppMVar1 != (CObjectManager*)0x0) {
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
	CObjectManager** ppMVar1;
	int iVar2;

	iVar2 = 0;
	ppMVar1 = CScene::ptable.aManagers;
	do {
		if (*ppMVar1 != (CObjectManager*)0x0) {
			(*ppMVar1)->Level_Draw();
		}
		iVar2 = iVar2 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar2 < 0x18);
	GlobalDList_AddToView();
	return;
}

char* s_Main_Camera_0042b460 = "Main Camera";

void CScene::Level_Setup(ByteCode* pMemoryStream)
{
	undefined4* puVar1;
	int* piVar2;
	uint uVar3;
	int iVar4;
	CCameraGame* pCVar5;
	float fVar5;
	ConditionedOperationArray local_4;

	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU64();
	pMemoryStream->GetS32();
	pMemoryStream->GetS32();
	uVar3 = pMemoryStream->GetU32();
	this->field_0x18 = uVar3;
	S_STREAM_FOG_DEF* pSVar1 = (S_STREAM_FOG_DEF*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(pSVar1 + 1);
	this->pFogClipStream = pSVar1;
	iVar4 = pMemoryStream->GetS32();
	this->mipmapL = iVar4;
	iVar4 = pMemoryStream->GetS32();
	this->mipmapK = iVar4;
	/* Main Camera */
	pCVar5 = (CCameraGame*)CScene::ptable.g_CameraManager_0045167c->AddCamera(CT_MainCamera, pMemoryStream, s_Main_Camera_0042b460);
	fVar5 = pMemoryStream->GetF32();
	this->field_0x118 = fVar5;
	pCVar5->field_0x2dc = fVar5;
	uVar3 = pMemoryStream->GetU32();
	this->field_0x11c = uVar3;
	uVar3 = pMemoryStream->GetU32();
	this->count_0x120 = uVar3;
	LoadFunc_001b87b0();
	CScene::ptable.g_MapManager_0045168c->OnLoadLevelBnk_003f9a60(pMemoryStream);
	iVar4 = pMemoryStream->GetS32();
	this->field_0x28 = iVar4;
	if (this->field_0x28 != -1) {
		CScene::ptable.g_C3DFileManager_00451664->InstanciateG2D(this->field_0x28);
	}
	iVar4 = pMemoryStream->GetS32();
	this->defaultTextureIndex_0x2c = iVar4;
	//FUN_002cbc90((undefined4*)&this->field_0xfc);
	iVar4 = pMemoryStream->GetS32();
	this->defaultMaterialIndex = iVar4;
	CScene::ptable.g_EffectsManager_004516b8->AddPool(pMemoryStream);
	//local_4 = { };
	local_4.Create(pMemoryStream);
	local_4.Perform();
	CLevelScheduler::gThis->Level_LoadObjectives(pMemoryStream);
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

bool CScene::CheckFunc_001b9300()
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
	peVar2 = (pSVar3->pShadowConfig).pViewport;
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
	(pSVar3->pShadowConfig).texWidth = uVar4;
	(pSVar3->pShadowConfig).texHeight = uVar5;
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

void CScene::LoadFunc_001b87b0()
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
	ed3DSceneGetConfig(CScene::_scene_handleA);
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
			(*ppSVar2)->ed3DSceneRemoveFlag(4);
		}
		else {
			(*ppSVar2)->ed3DSceneSetFlag(4);
		}

		Func_002b6db0(*ppSVar2, unaff_s2_lo, unaff_s1_lo);
		pCVar1 = ed3DSceneGetConfig(*ppSVar2);
		(pCVar1->pShadowConfig).renderMask = (short)(1 << (uVar3 & 0x1f));
		uVar3 = uVar3 + 1;
		ppSVar2 = ppSVar2 + 1;
		pCVar1->field_0x12c = pCVar1->projectionScaleFactorB * 800;
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

void _rgba::LerpRGBA(float alpha, _rgba param_3, _rgba param_4)
{
	byte bStack7;
	byte bStack6;
	byte bStack5;

	bStack7 = param_3.b;
	bStack6 = param_3.g;
	bStack5 = param_3.a;
	this->r = (byte)(int)((float)(param_3.r) + alpha * (float)((param_4.r) - (param_3.r)));
	this->g = (byte)(int)((float)(uint)bStack7 + alpha * (float)((param_4.g) - (uint)bStack7));
	this->b = (byte)(int)((float)(uint)bStack6 + alpha * (float)((param_4.b) - (uint)bStack6));
	this->a = (byte)(int)((float)(uint)bStack5 + alpha * (float)((param_4.a) - (uint)bStack5));
	return;
}

void CScene::HandleFogAndClippingSettings()
{
	S_STREAM_FOG_DEF* pSVar1;
	Timer* pTVar2;
	SceneConfig* pSVar3;
	FxFogProp* puVar4;
	float fVar4;
	uint uVar5;
	S_STREAM_FOG_DEF* pfVar1;

	if ((GameFlags & 0x10) == 0) {
		pTVar2 = Timer::GetTimer();
		fVar4 = pTVar2->cutsceneDeltaTime * 1000.0f;
		if (fVar4 < 2.147484e+09f) {
			uVar5 = (uint)fVar4;
		}
		else {
			uVar5 = (int)(fVar4 - 2.147484e+09f) | 0x80000000;
		}
		ed3DSetDeltaTime(uVar5);
	}
	else {
		pTVar2 = Timer::GetTimer();
		fVar4 = pTVar2->lastFrameTime * 1000.0f;
		if (fVar4 < 2.147484e+09f) {
			uVar5 = (uint)fVar4;
		}
		else {
			uVar5 = (int)(fVar4 - 2.147484e+09f) | 0x80000000;
		}
		ed3DSetDeltaTime(uVar5);
	}
	if ((this->fogClipSettingStackSize != -1) && (this->field_0xd0 <= this->field_0xd4)) {
		pSVar1 = this->aFogClipStack[this->fogClipSettingStackSize].pStreamDef;
		pTVar2 = Timer::GetTimer();
		fVar4 = this->field_0xd0 + pTVar2->cutsceneDeltaTime;
		this->field_0xd0 = fVar4;
		if (this->field_0xd4 < fVar4) {
			this->clipValue_0xe8 = pSVar1->clipValue_0x0;
			this->field_0xec = pSVar1->field_0x4;
			this->fogRGBA = pSVar1->fogRGBA;
			this->fogFlags = pSVar1->flags;
		}
		else {
			fVar4 = fVar4 / this->field_0xd4;
			this->clipValue_0xe8 = this->field_0xd8 + fVar4 * (pSVar1->clipValue_0x0 - this->field_0xd8);
			this->field_0xec = this->field_0xdc + fVar4 * (pSVar1->field_0x4 - this->field_0xdc);
			this->fogRGBA.LerpRGBA(fVar4, this->prevFogRGBA, pSVar1->fogRGBA);
		}
		pSVar3 = ed3DSceneGetConfig(_scene_handleA);
		pSVar3->clipValue_0x4 = this->clipValue_0xe8;
		puVar4 = ed3DGetFxFogProp();
		puVar4->field_0x4 = (int)this->field_0xec;
		if ((int)puVar4->field_0x4 < 0x80) {
			puVar4->field_0x0 = puVar4->field_0x0 | 1;
		}
		else {
			puVar4->field_0x0 = puVar4->field_0x0 & 0xfffffffe;
		}
		if ((this->fogFlags & 2) == 0) {
			puVar4->field_0x0 = puVar4->field_0x0 & 0xfffffffd;
		}
		else {
			puVar4->field_0x0 = puVar4->field_0x0 | 2;
		}
		puVar4->field_0x8 = (int)(uint)(this->fogRGBA).r >> 1;
		puVar4->field_0xc = (int)(uint)(this->fogRGBA).g >> 1;
		puVar4->field_0x10 = (int)(uint)(this->fogRGBA).b >> 1;
		puVar4->field_0x14 = 0x80;
	}
	return;
}


void CScene::PopFogAndClippingSettings(S_STREAM_FOG_DEF* pFogStream)
{
	S_STREAM_FOG_DEF* pSVar1;
	float* pfVar2;
	FogClipEntry* pFVar3;
	FogClipEntry* pFVar4;
	float fVar5;

	if ((pFogStream->flags & 1) == 0) {
		pfVar2 = &this->aFogClipStack[this->fogClipSettingStackSize + -10].field_0x4;
		pFVar3 = (FogClipEntry*)(pfVar2 + 0x13);

		for (pFVar4 = this->aFogClipStack; (pFVar4 <= pFVar3 && (pFVar4->pStreamDef != pFogStream)); pFVar4 = pFVar4 + 1) {
		}

		if (pFVar4 <= pFVar3) {
			if (pFVar4 == pFVar3) {
				pSVar1 = pFVar3->pStreamDef;
				this->field_0xd8 = pSVar1->clipValue_0x0;
				this->field_0xdc = pSVar1->field_0x4;
				this->prevFogRGBA = pSVar1->fogRGBA;
				this->prevFogFlags = pSVar1->flags;
				fVar5 = pfVar2[0x14];
				this->field_0xd4 = fVar5;
				fVar5 = fVar5 - this->field_0xd0;
				this->field_0xd0 = fVar5;

				if (fVar5 < 0.0f) {
					this->field_0xd0 = 0.0f;
				}
			}
			else {
				for (; pFVar4 < pFVar3; pFVar4 = pFVar4 + 1) {
					pFVar4->pStreamDef = pFVar4[1].pStreamDef;
					pFVar4->field_0x4 = pFVar4[1].field_0x4;
				}
			}

			this->fogClipSettingStackSize = this->fogClipSettingStackSize + -1;
		}
	}
	return;
}

void CScene::PushFogAndClippingSettings(float other, S_STREAM_FOG_DEF* pFogStream)
{
	FogClipEntry* pfVar1;

	if ((pFogStream->flags & 1) == 0) {
		this->fogClipSettingStackSize = this->fogClipSettingStackSize + 1;
		this->field_0xd8 = this->clipValue_0xe8;
		this->field_0xdc = this->field_0xec;
		this->prevFogRGBA = this->fogRGBA;
		this->prevFogFlags = this->fogFlags;
		this->field_0xd0 = 0.0;
		if (this->fogClipSettingStackSize == 0) {
			this->field_0xd4 = 0.0;
		}
		else {
			this->field_0xd4 = other;
		}
		pfVar1 = &this->aFogClipStack[this->fogClipSettingStackSize];
		pfVar1->pStreamDef = pFogStream;
		pfVar1->field_0x4 = other;
	}
	return;
}

void CScene::Level_SectorChange(int oldSectorId, int newSectorId)
{
	CObjectManager* pLVar1;
	CObjectManager** pMVar2;
	int iVar3;

	iVar3 = 0;
	pMVar2 = (CObjectManager**)&ptable;
	do {
		pLVar1 = *pMVar2;
		if (pLVar1 != (CObjectManager*)0x0) {
			pLVar1->Level_SectorChange(oldSectorId, newSectorId);
		}
		iVar3 = iVar3 + 1;
		pMVar2 = pMVar2 + 1;
	} while (iVar3 < 0x18);
	return;
}

void* CScene::GetManager(MANAGER_TYPE type)
{
	CObjectManager* pManager = ptable.aManagers[type];
	return (void*)pManager;
}

ed_3D_Scene* GetStaticMeshMasterA_001031b0(void)
{
	return CScene::_scene_handleA;
}