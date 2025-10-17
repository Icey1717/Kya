#include "LargeObject.h"
#include <stdlib.h>
#include "LevelScheduleManager.h"
#include "Pause.h"
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
#include "profiling.h"
#endif
#include "edVideo/VideoC.h"
#include "TimeController.h"
#include "MemoryStream.h"
#include "FrontendDisp.h"
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
#include "PoolAllocators.h"
#include "FrontEndBank.h"
#include "ActorFactory.h"

#define SCENE_STATE_NONE 0x0
#define SCENE_STATE_CHECKPOINT_FADE_OUT 0x1
#define SCENE_STATE_CHECKPOINT_FADE_IN 0x2
#define SCENE_STATE_CHECKPOINT_FADE_IN_IMMEDIATE 0x3
#define SCENE_STATE_RESET 0x5
#define SCENE_STATE_EXIT_FADE_OUT 0x6
#define SCENE_STATE_EXIT_FADE_IN 0x7

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
	edListAddNode(&p3DScene->meshClusterShadowList, (DisplayList*)pDisplayList);
	return pDisplayList;
}

CScene::CScene()
{
	void** errorPrinter;
	//LocalizationManager* pManager;
	//Manager_170* pMVar1;
	CFrontendDisplay* pFrontendManager;
	//HelpManager* pHelpManager;
	CPauseManager* pPVar2;
	//CMapManager* pMVar3;
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
	CLanguageManager* pManager;

	this->field_0x38 = 1;
	this->pActorFactory = new CActorFactory();
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
	pManager = new CLanguageManager();
	CScene::ptable.g_LocalizationManager_00451678 = pManager;
	CScene::ptable.g_FrontEndBank_00451674 = new CFrontendBank();
	CScene::ptable.g_FrontendManager_00451680 = new CFrontendDisplay();
	//pHelpManager = (HelpManager*)Allocate(0x1e4);
	//if (pHelpManager != (HelpManager*)0x0) {
	//	uVar17 = SetupHelpManager_0037bc70(pHelpManager);
	//	pHelpManager = (HelpManager*)uVar17;
	//}
	//g_HelpManager_00451684 = pHelpManager;
	pPVar2 = new CPauseManager();
	CScene::ptable.g_PauseManager_00451688 = pPVar2;
	CScene::ptable.g_MapManager_0045168c = new CMapManager;
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
	local_8.screenWidth = peVar11->pActiveVidParams->screenWidth;
	local_8.screenHeight = peVar11->pActiveVidParams->screenHeight;
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


void S_ELEVATOR_CUTSCENE_LIST::Create(S_ELEVATOR_CUTSCENE_LIST** ppList, ByteCode* pByteCode)
{
	S_ELEVATOR_CUTSCENE_LIST* pList = reinterpret_cast<S_ELEVATOR_CUTSCENE_LIST*>(pByteCode->currentSeekPos);

	pByteCode->currentSeekPos = (char*)(pList->aEntries);
	if (pList->nbEntries != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pList->nbEntries * sizeof(int);
	}

	*ppList = pList;

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


void CScene::Level_PauseChange(int bPaused)
{
	CObjectManager** ppMVar1;
	int iVar4;

	Timer::GetTimer()->SetTimeScale_001ba6f0(bPaused);

	iVar4 = 0;
	ppMVar1 = CScene::ptable.aManagers;
	do {
		if (*ppMVar1 != (CObjectManager*)0x0) {
			(*ppMVar1)->Level_PauseChange(bPaused);
		}

		iVar4 = iVar4 + 1;
		ppMVar1 = ppMVar1 + 1;
	} while (iVar4 < 0x18);

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
	CObjectManager** ppManager;
	int managerIndex;
	
	SetState(SCENE_STATE_NONE);
	this->timeInState = 0.0f;

	if ((this->pFogClipStream->flags & 1) == 0) {
		this->fogClipSettingStackSize = this->fogClipSettingStackSize + 1;
		this->field_0xd8 = this->clipValue_0xe8;
		this->field_0xdc = this->field_0xec;
		this->prevFogRGBA = this->fogRGBA;
		this->prevFogFlags = this->fogFlags;
		this->field_0xd0 = 0.0f;
		this->field_0xd4 = 0.0f;
		managerIndex = this->fogClipSettingStackSize;
		this->aFogClipStack[managerIndex].pStreamDef = this->pFogClipStream;
		this->aFogClipStack[managerIndex].field_0x4 = 0.0f;
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
	ptable.g_EffectsManager_004516b8->Level_PreInit();

	// Level_Init
	managerIndex = 0;
	ppManager = CScene::ptable.aManagers;
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_Init();
		}

		managerIndex = managerIndex + 1;
		ppManager = ppManager + 1;
	} while (managerIndex < 0x18);

	this->field_0x38 = 1;
	edColFreeTemporaryMemory();
	CLevelScheduler::gThis->Level_PostInit();
	g_CinematicManager_0048efc->WillLoadCinematic();
	//FUN_003f95f0((CMapManager*)Scene::ptable[11]);
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

static void Fade(float speed, int bFadeIn, int bWaitForFade)
{
	ulong uVar1;
	float fVar2;
	uint fadeSpeed;

	if (gVideoConfig.isNTSC == 1) {
		fVar2 = speed * 50.0f;
		if (fVar2 < 2.147484e+09f) {
			fadeSpeed = (uint)fVar2;
		}
		else {
			fadeSpeed = (int)(fVar2 - 2.147484e+09f) | 0x80000000;
		}
	}
	else {
		fVar2 = speed * 60.0f;
		if (fVar2 < 2.147484e+09f) {
			fadeSpeed = (uint)fVar2;
		}
		else {
			fadeSpeed = (int)(fVar2 - 2.147484e+09f) | 0x80000000;
		}
	}

	if (fadeSpeed == 0) {
		fadeSpeed = 1;
	}

	edVideoSetFadeColor(0, 0, 0);

	if (bFadeIn == 1) {
		edVideoSetFadeColor(0, 0, 0);
		edVideoSetFade(1.0f);
		edVideoSetFadeIn(fadeSpeed);
	}
	else {
		edVideoSetFadeColor(0, 0, 0);
		edVideoSetFade(0.0f);
		edVideoSetFadeOut(fadeSpeed, 1);
	}

	if (bWaitForFade != 0) {
		uVar1 = edVideoIsFadeActive();
		while (uVar1 != 0) {
			edVideoFlip();
			uVar1 = edVideoIsFadeActive();
		}
	}

	return;
}

void CScene::SetGlobalPaused_001b8c30(int param_2)
{
	this->field_0x48 = param_2;

	return;
}

#define FADE_DIRECTION_IN 1
#define FADE_DIRECTION_OUT 2

void CScene::SetState(int state)
{
	if (this->curState != state) {
		this->timeInState = 0.0f;
		this->curState = state;
	}
}

void CScene::HandleCurState()
{
	HandleFogAndClippingSettings();

	this->timeInState = this->timeInState + GetTimer()->lastFrameTime;

	switch (this->curState) {
	case SCENE_STATE_CHECKPOINT_FADE_OUT:
		Fade(1.0f, FADE_DIRECTION_OUT, 0);
		SetState(2);
		break;
	case SCENE_STATE_CHECKPOINT_FADE_IN:
		if (edVideoIsFadeActive() == 0) {
			GameFlags = GameFlags & 0xfffffe7f;
			Level_CheckpointReset();
			Fade(1.0f, FADE_DIRECTION_IN, 0);
			SetState(SCENE_STATE_NONE);
		}
		break;
	case SCENE_STATE_CHECKPOINT_FADE_IN_IMMEDIATE:
		GameFlags = GameFlags & 0xfffffe7f;
		Level_CheckpointReset();
		Fade(1.0f, FADE_DIRECTION_IN, 0);
		SetState(SCENE_STATE_NONE);
		break;
	case 4:
		Fade(1.0f, FADE_DIRECTION_OUT, 0);
		SetState(SCENE_STATE_RESET);
		break;
	case SCENE_STATE_RESET:
		if (edVideoIsFadeActive() == 0) {
			GameFlags = GameFlags & 0xfffffe7f;
			Level_Reset();
			Fade(1.0f, FADE_DIRECTION_IN, 0);
			SetState(SCENE_STATE_NONE);
		}
		break;
	case SCENE_STATE_EXIT_FADE_OUT:
		Fade(1.0f, FADE_DIRECTION_OUT, 0);
		SetState(SCENE_STATE_EXIT_FADE_IN);
		break;
	case SCENE_STATE_EXIT_FADE_IN:
		if (edVideoIsFadeActive() == 0) {
			Fade(1.0f, FADE_DIRECTION_IN, 0);
			GameFlags = GameFlags | GAME_REQUEST_TERM;
			SetState(SCENE_STATE_NONE);
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

void CScene::InitiateCheckpointReset(int bFadeOut)
{
	if (this->curState == SCENE_STATE_NONE) {
		if (bFadeOut == 0) {
			this->timeInState = 0.0f;
			this->curState = SCENE_STATE_CHECKPOINT_FADE_IN_IMMEDIATE;
		}
		else {
			this->timeInState = 0.0f;
			this->curState = SCENE_STATE_CHECKPOINT_FADE_OUT;
		}

		GameFlags = GameFlags | GAME_CUTSCENE_80;
	}

	return;
}

void CScene::Level_SaveContext()
{
	CObjectManager** ppManager;
	int curManagerIndex;

	curManagerIndex = 0;
	ppManager = CScene::ptable.aManagers;
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_SaveContext();
		}

		curManagerIndex = curManagerIndex + 1;
		ppManager = ppManager + 1;
	} while (curManagerIndex < 0x18);

	return;
}

void CScene::Level_LoadContext()
{
	CObjectManager** ppManager;
	int curManagerIndex;
	curManagerIndex = 0;
	ppManager = CScene::ptable.aManagers;
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_LoadContext();
		}

		curManagerIndex = curManagerIndex + 1;
		ppManager = ppManager + 1;
	} while (curManagerIndex < 0x18);

	return;
}

void CScene::Level_Term(void)
{
	FxFogProp* pFVar2;
	int curIndex;


	g_CinematicManager_0048efc->StopAllCutscenes();

	CLevelScheduler::gThis->Level_PreTerm();

	curIndex = 0;
	CObjectManager** ppManager = CScene::ptable.IterateBackwards();
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_Term();
		}
		curIndex = curIndex + 1;
		ppManager = ppManager - 1;
	} while (curIndex < 0x18);

	PopFogAndClippingSettings(this->pFogClipStream);

	this->pFogClipStream = (S_STREAM_FOG_DEF*)0x0;
	this->field_0x48 = 0;

	edViewportSetClearMask(this->pViewportA, 0);
	edViewportSetClearMask(this->pViewportB, 0);

	pFVar2 = ed3DGetFxFogProp();
	pFVar2->field_0x0 = pFVar2->field_0x0 & 0xfffffffe;
	if (g_CinematicManager_0048efc->pCinematic == (CCinematic*)0x0) {
		this->fogClipSettingStackSize = -1;
	}

	edDListDeleteFrameBufferMaterial(&this->frameBufferMaterial);

	GlobalDList_AddToView();

#ifndef PLATFORM_WIN
	edVideoFlip();
#endif

	curIndex = 0;
	ppManager = CScene::ptable.IterateBackwards();
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			(*ppManager)->Level_ClearAll();
		}
		curIndex = curIndex + 1;
		ppManager = ppManager - 1;
	} while (curIndex < 0x18);

	FreeAllAllocators();

	this->field_0x10c = -1;
	this->field_0x110 = -1;
	this->pElevatorCutsceneList = 0;

	return;
}

void CScene::Level_Reset()
{
	IMPLEMENTATION_GUARD();
}

void CScene::Level_Manage()
{
	CObjectManager** ppManager;
	int curManagerIndex;

	ZONE_SCOPED;

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
		ZONE_SCOPED_NAME("Level_Manage");
		curManagerIndex = 0;
		ppManager = CScene::ptable.aManagers;
		do {
			if (*ppManager != (CObjectManager*)0x0) {
				ZONE_SCOPED_NAME("Level_Manage");
				ZONE_NAME((*ppManager)->ProfileGetName(), strlen((*ppManager)->ProfileGetName()));
				(*ppManager)->Level_Manage();
			}
			curManagerIndex = curManagerIndex + 1;
			ppManager = ppManager + 1;
		} while (curManagerIndex < 0x18);
	}
	else {
		ZONE_SCOPED_NAME("Level_ManagePaused");
		curManagerIndex = 0;
		ppManager = CScene::ptable.aManagers;
		do {
			if (*ppManager != (CObjectManager*)0x0) {
				ZONE_SCOPED_NAME("Level_ManagePaused");
				ZONE_NAME((*ppManager)->ProfileGetName(), strlen((*ppManager)->ProfileGetName()));
				(*ppManager)->Level_ManagePaused();
			}
			curManagerIndex = curManagerIndex + 1;
			ppManager = ppManager + 1;
		} while (curManagerIndex < 0x18);
	}

	HandleCurState();

	return;
}

void CScene::Level_Draw(void)
{
	CObjectManager** ppManager;
	int curManagerIndex;

	ZONE_SCOPED;

	curManagerIndex = 0;
	ppManager = CScene::ptable.aManagers;
	do {
		if (*ppManager != (CObjectManager*)0x0) {
			ZONE_SCOPED_NAME("Level_Draw");
			ZONE_NAME((*ppManager)->ProfileGetName(), strlen((*ppManager)->ProfileGetName()));
			(*ppManager)->Level_Draw();
		}
		curManagerIndex = curManagerIndex + 1;
		ppManager = ppManager + 1;
	} while (curManagerIndex < 0x18);

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

	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU8();
	pMemoryStream->GetU64();
	pMemoryStream->GetS32();
	pMemoryStream->GetS32();

	this->field_0x18 = pMemoryStream->GetU32();

	S_STREAM_FOG_DEF* pSVar1 = (S_STREAM_FOG_DEF*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(pSVar1 + 1);
	this->pFogClipStream = pSVar1;
	this->mipmapL = pMemoryStream->GetS32();
	this->mipmapK = pMemoryStream->GetS32();

	/* Main Camera */
	pCVar5 = (CCameraGame*)CScene::ptable.g_CameraManager_0045167c->AddCamera(CT_MainCamera, pMemoryStream, s_Main_Camera_0042b460);

	fVar5 = pMemoryStream->GetF32();
	this->field_0x118 = fVar5;
	pCVar5->field_0x2dc = fVar5;

	this->field_0x11c = pMemoryStream->GetU32();
	this->count_0x120 = pMemoryStream->GetU32();

	LoadFunc_001b87b0();
	CScene::ptable.g_MapManager_0045168c->OnLoadLevelBnk_003f9a60(pMemoryStream);
	this->defaultTextureIndex_0x28 = pMemoryStream->GetS32();
	if (this->defaultTextureIndex_0x28 != -1) {
		CScene::ptable.g_C3DFileManager_00451664->InstanciateG2D(this->defaultTextureIndex_0x28);
	}
	this->defaultTextureIndex_0x2c = pMemoryStream->GetS32();
	edDListCreateFrameBufferMaterial(&this->frameBufferMaterial);
	this->defaultMaterialIndex = pMemoryStream->GetS32();
	CScene::ptable.g_EffectsManager_004516b8->AddPool(pMemoryStream);

	ConditionedOperationArray local_4;
	local_4.Create(pMemoryStream);
	local_4.Perform();

	CLevelScheduler::gThis->Level_LoadObjectives(pMemoryStream);
	this->field_0x10c = pMemoryStream->GetS32();
	this->field_0x110 = pMemoryStream->GetS32();

	S_ELEVATOR_CUTSCENE_LIST::Create(&this->pElevatorCutsceneList, pMemoryStream);
	return;
}

bool CScene::IsFadeTermActive()
{
	return (this->curState - 6U) < 2;
}

bool CScene::IsCutsceneFadeActive(void)
{
	return (this->curState - 1U) < 3;
}

bool CScene::IsResetFadeActive(void)
{
	return (this->curState - 4U) < 2;
}

CScene::~CScene()
{
	CActorFactory* pCVar2;
	CObjectManager** ppManager;
	int iVar4;

	if (this != (CScene*)0x0) {
		iVar4 = 0x17;
		ppManager = ptable.IterateBackwards();
		do {
			if (*ppManager != (CObjectManager*)0x0) {
				if (*ppManager != (CObjectManager*)0x0) {
					delete* ppManager;
				}

				*ppManager = (CGlobalDListManager*)0x0;
			}

			iVar4 = iVar4 + -1;
			ppManager = ppManager + -1;
		} while (-1 < iVar4);

		ed3DSceneTerm(_scene_handleA);
		_scene_handleA = (ed_3D_Scene*)0xffffffff;

		ed3DSceneTerm(_scene_handleB);
		_scene_handleB = (ed_3D_Scene*)0xffffffff;

		edViewportDel(this->pViewportA, false);
		this->pViewportA = (ed_viewport*)0x0;

		edViewportDel(this->pViewportB, false);
		this->pViewportB = (ed_viewport*)0x0;

		_pinstance = (CScene*)0x0;

		if (this->curState != 0) {
			this->timeInState = 0.0f;
			this->curState = 0;
		}

		delete this->pActorFactory;
	}

	return;
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

void _rgba::LerpRGBA(float alpha, uint src, uint dst)
{
	byte bStack11;
	byte bStack10;
	byte bStack9;

	this->r = (byte)(int)((float)(src & 0xff) + alpha * (float)((dst & 0xff) - (src & 0xff)));
	bStack11 = (byte)(src >> 8);
	this->g = (byte)(int)((float)(uint)bStack11 + alpha * (float)((dst >> 8 & 0xff) - (uint)bStack11));
	bStack10 = (byte)(src >> 0x10);
	this->b = (byte)(int)((float)(uint)bStack10 + alpha * (float)((dst >> 0x10 & 0xff) - (uint)bStack10));
	bStack9 = (byte)(src >> 0x18);
	this->a = (byte)(int)((float)(uint)bStack9 + alpha * (float)((dst >> 0x18) - (uint)bStack9));
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
			this->fogRGBA.LerpRGBA(fVar4, this->prevFogRGBA.rgba, pSVar1->fogRGBA.rgba);
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
	S_STREAM_FOG_DEF* pStreamFogDef;
	FogClipEntry* pEndEntries;
	FogClipEntry* pCurEntry;
	float fVar5;

	if ((pFogStream->flags & 1) == 0) {
		pEndEntries = this->aFogClipStack + this->fogClipSettingStackSize;

		for (pCurEntry = this->aFogClipStack; (pCurEntry <= pEndEntries && (pCurEntry->pStreamDef != pFogStream)); pCurEntry = pCurEntry + 1) {
		}

		if (pCurEntry <= pEndEntries) {
			if (pCurEntry == pEndEntries) {
				pStreamFogDef = pEndEntries->pStreamDef;

				this->field_0xd8 = pStreamFogDef->clipValue_0x0;
				this->field_0xdc = pStreamFogDef->field_0x4;
				this->prevFogRGBA = pStreamFogDef->fogRGBA;
				this->prevFogFlags = pStreamFogDef->flags;
				fVar5 = pEndEntries->field_0x4;
				this->field_0xd4 = fVar5;
				fVar5 = fVar5 - this->field_0xd0;
				this->field_0xd0 = fVar5;

				if (fVar5 < 0.0f) {
					this->field_0xd0 = 0.0f;
				}
			}
			else {
				for (; pCurEntry < pEndEntries; pCurEntry = pCurEntry + 1) {
					pCurEntry->pStreamDef = pCurEntry[1].pStreamDef;
					pCurEntry->field_0x4 = pCurEntry[1].field_0x4;
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

void CScene::SetFadeStateTerm(bool bFadeOut)
{
	if (1 < this->curState - 6U) {
		if (bFadeOut == false) {
			if (this->curState != SCENE_STATE_EXIT_FADE_IN) {
				this->timeInState = 0.0f;
				this->curState = SCENE_STATE_EXIT_FADE_IN;
			}
		}
		else {
			if (this->curState != SCENE_STATE_EXIT_FADE_OUT) {
				this->timeInState = 0.0f;
				this->curState = SCENE_STATE_EXIT_FADE_OUT;
			}
		}

		GameFlags = GameFlags | 0x180;
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

uint CScene::Rand()
{
	const uint value = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
	CScene::_pinstance->field_0x38 = value;
	return (value >> 0x10) & 0x7fff; // Return the upper 15 bits
}

ed_3D_Scene* GetStaticMeshMasterA_001031b0(void)
{
	return CScene::_scene_handleA;
}

void CAudioManager::PlayCombatMusic()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

void CAudioManager::StopCombatMusic()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

char* CObjectManager::ProfileGetName()
{
	return "unnamed";
}
