#include "ActorMiniGamesOrganizer.h"
#include "MemoryStream.h"
#include "ActorMiniGame.h"
#include "ActorMiniGamesManager.h"
#include "ActorHero.h"
#include "CinematicManager.h"
#include "CameraManager.h"
#include "CameraViewManager.h"
#include "CompatibilityHandlingPS2.h"
#include "DlistManager.h"
#include "EventManager.h"
#include "FileManager3D.h"
#include "Frontend.h"
#include "FrontEndDisp.h"
#include "LevelScheduler.h"
#include "Pause.h"
#include "InputManager.h"
#include "PoolAllocators.h"
#include "SectorManager.h"
#include "TimeController.h"
#include "BootData.h"
#include "TranslatedTextData.h"
#include "MathOps.h"
#include "edText.h"
#include "edVideo/VideoA.h"
#include "ed3D/ed3DG3D.h"
#include "kya.h"
#include "WayPoint.h"
#include "Rendering/edCTextFormat.h"

static void MoveMenuArrow(astruct_22* pArrow, bool bNext)
{
	pArrow->field_0x19c = !bNext;
	if (pArrow->field_0x198 == 0.0f) {
		pArrow->field_0x198 = pArrow->field_0x180;
	}
	float fVar1 = pArrow->field_0x180 / 2.0f;
	if (pArrow->field_0x198 <= fVar1) {
		pArrow->field_0x198 = fVar1;
	}
	return;
}

static void ScaleMenuMesh(float x, float y, float z, StaticMeshComponent* pMesh)
{
	edF32MATRIX4 local_40;
	edF32VECTOR4 local_50;
	if (pMesh->pMeshTransformData != (ed_3d_hierarchy_node*)0x0) {
		local_40 = pMesh->pMeshTransformData->base.transformA;
		local_50.x = x;
		local_50.y = y;
		local_50.z = z;
		local_50.w = 0.0f;
		edF32Matrix4ScaleHard(&local_40, &pMesh->perspectiveMatrix, &local_50);
		local_40.rowT = pMesh->pMeshTransformData->base.transformA.rowT;
		pMesh->pMeshTransformData->base.transformA = local_40;
	}
	return;
}

static void MenuFade(float duration, int direction)
{
	uint speed = (uint)(duration * (gVideoConfig.isNTSC == 1 ? 50.0f : 60.0f));
	if (speed == 0) {
		speed = 1;
	}
	edVideoSetFadeColor(0, 0, 0);
	if (direction == 1) {
		edVideoSetFade(1.0f);
		edVideoSetFadeIn(speed);
	}
	else {
		edVideoSetFade(0.0f);
		edVideoSetFadeOut(speed, 1);
	}
	return;
}

static void DrawDisconnectedController()
{
	edCTextStyle local_c0;
	edCTextStyle* pOldStyle;
	if (GuiDList_BeginCurrent()) {
		local_c0.SetShadow(0x100);
		local_c0.rgbaColour = 0xffffffff;
		local_c0.alpha = 0xff;
		local_c0.SetFont(BootDataFont, false);
		local_c0.SetHorizontalAlignment(2);
		local_c0.SetVerticalAlignment(8);
		local_c0.spaceSize = 10.0f;
		pOldStyle = edTextStyleSetCurrent(&local_c0);
		edTextDraw((float)gVideoConfig.screenWidth / 2.0f, (float)gVideoConfig.screenHeight / 2.0f,
			gMessageManager.get_message(0x52525f503700080c));
		edTextStyleSetCurrent(pOldStyle);
		GuiDList_EndCurrent();
	}
	return;
}

static void DrawMiniGameWheel(int curIndex, int nextIndex, S_MENU_WHEEL_DRAW* pDraw, void** pContext);

CMenuWheel::CMenuWheel()
{
	this->field_0xc0.field_0x1a8 = 0;
}

void CMenuWheel::Init(edDList_material* pMaterial, edDList_material* pArrow, edDList_material* pArrowHighlight)
{
	this->field_0x274 = 0;
	this->field_0xc0.centerX = 0.0f;
	this->field_0x270 = 0.0f;
	this->field_0x284 = 255.0f;
	this->field_0x280 = 255.0f;
	this->field_0x28c = 255.0f;
	this->field_0x27c = 0;
	this->field_0x278 = 1;
	this->pFunc = 0;
	this->field_0x298 = 0;
	this->field_0x290 = false;
	if (pMaterial != (edDList_material*)0x0) {
		this->field_0x0.Install(pMaterial);
		this->field_0x290 = true;
	}
	this->field_0xc0.FUN_002ef9b0(pArrow, pArrowHighlight);
	return;
}

void CMenuWheel::Reset()
{
	this->field_0x284 = 255.0f;
	this->field_0x280 = 255.0f;
	this->field_0x27c = 0;
	this->field_0xc0.field_0x198 = 0.0f;
	return;
}

void CMenuWheel::MoveWheel()
{
	this->field_0x284 = this->field_0x280;
	this->field_0xc0.field_0x198 = 0.0f;
	return;
}

void CMenuWheel::MoveWheel(bool bNext)
{
	int iVar1 = this->field_0x274;
	if (1 < iVar1) {
		if (!bNext) {
			this->field_0x27c = (this->field_0x27c + -1 + iVar1) % iVar1;
		}
		else {
			this->field_0x27c = (this->field_0x27c + 1) % iVar1;
		}
		MoveMenuArrow(&this->field_0xc0, bNext);
		this->field_0x280 = 255.0f;
		this->field_0x284 = 0.0f;
		this->field_0x288 = bNext;
	}
	return;
}

void CMenuWheel::Manage()
{
	float fVar3 = this->field_0x284;
	float fVar4 = this->field_0x280;
	if (fabsf(fVar3 - fVar4) < this->field_0x28c * GetTimer()->lastFrameTime) {
		this->field_0x284 = 255.0f;
	}
	this->field_0xc0.FUN_002ef890();
	if ((this->field_0x278 == 0) || (this->field_0x280 <= this->field_0x284)) {
		this->field_0x284 = this->field_0x280;
	}
	else {
		this->field_0x284 = this->field_0x284 + this->field_0x28c * GetTimer()->lastFrameTime;
	}
	return;
}

void CMenuWheel::FUN_002efa80(float x, float y, edF32VECTOR2* pRight, edF32VECTOR2* pLeft)
{
	this->field_0xc0.centerX = x;
	this->field_0x270 = y;
	if (!this->field_0x290) {
		if (pRight == (edF32VECTOR2*)0x0) return;
		this->field_0xc0.field_0x184 = pLeft->x;
		this->field_0xc0.field_0x188 = pLeft->y;
		this->field_0xc0.field_0x18c = pRight->x;
		this->field_0xc0.field_0x190 = pRight->y;
	}
	else {
		this->field_0xc0.field_0x184 = x - (float)this->field_0x0.iWidth / 2.0f;
		this->field_0xc0.field_0x188 = y;
		this->field_0xc0.field_0x18c = x + (float)this->field_0x0.iWidth / 2.0f;
		this->field_0xc0.field_0x190 = y;
	}
	return;
}

void CMenuWheel::Draw()
{
	S_MENU_WHEEL_DRAW local_10;
	int iVar1;
	if (this->field_0x290) {
		this->field_0x0.Draw(1.0f, this->field_0xc0.centerX, this->field_0x270, 0x12);
	}

	if (1 < this->field_0x274) {
		this->field_0xc0.FUN_002ef500();
	}

	local_10.x = this->field_0xc0.centerX;
	local_10.y = this->field_0x270;
	local_10.alpha = (byte)(int)this->field_0x284;
	if (!this->field_0x288) {
		iVar1 = (this->field_0x27c + 1) % this->field_0x274;
	}
	else {
		iVar1 = (this->field_0x27c + -1 + this->field_0x274) % this->field_0x274;
	}

	this->pFunc(this->field_0x27c, iVar1, &local_10, this->field_0x298);

	return;
}

CActorMiniGamesOrganizer::CActorMiniGamesOrganizer()
{
	this->field_0x76c.field_0x1a8 = 0;
	this->field_0x76c.pContext = 0;
	this->field_0x17c = (S_ACTOR_STREAM_REF*)0x0;
	this->field_0x9b8 = (edDList_material*)0x0;
}

CActorMiniGame* CActorMiniGamesOrganizer::GetMiniGame(int index)
{
	return static_cast<CActorMiniGame*>(this->field_0x17c->aEntries[index].Get());
}

void CActorMiniGamesOrganizer::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->field_0x168 = pByteCode->GetString();
	this->field_0x16c = pByteCode->GetS32();
	this->textureIndex_0x170 = pByteCode->GetS32();
	this->field_0x174 = pByteCode->GetS32();
	this->materialId_0x178 = pByteCode->GetS32();
	this->field_0x17c = S_ACTOR_STREAM_REF::Create(pByteCode);
	this->field_0x180 = pByteCode->GetU32();
	this->field_0x184 = pByteCode->GetU32();
	this->field_0x188 = pByteCode->GetS32();
	this->field_0x18c = pByteCode->GetS32();
	this->field_0x190 = pByteCode->GetS32();
	this->field_0x194 = pByteCode->GetF32();

	StaticMeshComponent* aMeshes[] = { &this->field_0x1d0, &this->field_0x230, &this->field_0x290,
		&this->field_0x2f0, &this->field_0x350, &this->field_0x3b0, &this->field_0x410, &this->field_0x470 };
	for (int i = 0; i < 8; i++) {
		aMeshes[i]->textureIndex = this->textureIndex_0x170;
		aMeshes[i]->meshIndex = this->field_0x174;
		aMeshes[i]->Reset();
	}

	return;
}

void CActorMiniGamesOrganizer::Init()
{
	uint uVar5;
	ed_hash_code* peVar3;

	CActor::Init();

	this->field_0x17c->Init();

	int iVar1 = 0;
	while (true) {
		int iVar2 = 0;
		if (this->field_0x17c != 0) iVar2 = this->field_0x17c->entryCount;
		if (iVar2 <= iVar1) break;
		GetMiniGame(iVar1)->field_0x1c0 = this;
		iVar1 = iVar1 + 1;
	}

	InitAlphabet();

	SV_InstallMaterialId(this->materialId_0x178);
	SV_InstallMaterialId(this->textureIndex_0x170);

	ed_g3d_manager* pMesh = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->field_0x174, this->textureIndex_0x170);

	peVar3 = ed3DG2DGetHashCode(&MenuBitmaps[0xb].textureManager, MenuBitmaps[11].materialInfo.pMaterial);
	if (peVar3 != (ed_hash_code*)0x0) {
		uVar5 = ed3DComputeHashCode("background");
		ed3DReplaceTexture(pMesh, &MenuBitmaps[0xb].textureManager, uVar5, peVar3->hash.number);
	}
	peVar3 = ed3DG2DGetHashCode(&MenuBitmaps[0xc].textureManager, MenuBitmaps[12].materialInfo.pMaterial);
	if (peVar3 != (ed_hash_code*)0x0) {
		uVar5 = ed3DComputeHashCode("tatoo_01");
		ed3DReplaceTexture(pMesh, &MenuBitmaps[0xc].textureManager, uVar5, peVar3->hash.number);
		uVar5 = ed3DComputeHashCode("tatoo_02");
		ed3DReplaceTexture(pMesh, &MenuBitmaps[0xc].textureManager, uVar5, peVar3->hash.number);
	}

	this->menuWheel.Init(0, &MenuBitmaps[9].materialInfo, &MenuBitmaps[10].materialInfo);
	this->menuWheel.field_0x278 = 1;
	this->menuWheel.field_0x274 = this->field_0x17c == 0 ? 0 : this->field_0x17c->entryCount;
	this->menuWheel.field_0x28c = 1020.0f;
	this->menuWheel.pFunc = DrawMiniGameWheel;
	this->menuWheel.field_0x298 = &this->field_0x76c.pContext;
	this->menuWheel.field_0xc0.field_0x0.color = 0x8000005d;
	this->menuWheel.field_0xc0.field_0xc0.color = 0x8000626a;
	this->menuWheel.field_0xc0.field_0x0.iWidth = (ushort)(int)((float)gVideoConfig.screenWidth * 0.06f);
	this->menuWheel.field_0xc0.field_0x0.iHeight = (ushort)(int)((float)gVideoConfig.screenHeight * 0.06f);
	this->menuWheel.field_0xc0.field_0xc0.iWidth = this->menuWheel.field_0xc0.field_0x0.iWidth;
	this->menuWheel.field_0xc0.field_0xc0.iHeight = this->menuWheel.field_0xc0.field_0x0.iHeight;

	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;
	local_10.x = (float)gVideoConfig.screenWidth * 0.5f - ((float)gVideoConfig.screenWidth * 0.58f) / 2.0f;
	local_10.y = (float)gVideoConfig.screenHeight * 0.13f;
	local_8.x = (float)gVideoConfig.screenWidth * 0.5f + ((float)gVideoConfig.screenWidth * 0.58f) / 2.0f;
	local_8.y = local_10.y;
	this->menuWheel.FUN_002efa80((float)(int)((float)gVideoConfig.screenWidth * 0.5f),
		(float)(int)local_10.y, &local_8, &local_10);

	this->field_0x91c = 0;
	this->field_0x76c.pContext = this;
	this->field_0x76c.FUN_002ef9b0(&MenuBitmaps[9].materialInfo, &MenuBitmaps[10].materialInfo);
	this->field_0x76c.field_0x0.color = 0x8000005d;
	this->field_0x76c.field_0xc0.color = 0x8000626a;

	memset(&this->field_0x198, 0, sizeof(this->field_0x198));

	(this->field_0x1b8).x = 0.0f;
	(this->field_0x1b8).y = 0.0f;
	(this->field_0x1b8).z = 0.0f;
	(this->field_0x1b8).w = 200.0f;

	this->field_0x1c8 = 150.0f;
	this->field_0x198.pBoundingSphere = &this->field_0x1b8;
	this->field_0x198.clipping_0x0 = &this->field_0x1c8;

	iVar1 = this->field_0x17c == 0 ? 0 : this->field_0x17c->entryCount;
	if (iVar1 != 0) {
		this->field_0x9b8 = NewPool_edDLIST_MATERIAL(iVar1);
	}

	ClearLocalData();

	return;
}

void CActorMiniGamesOrganizer::Term()
{
	if (this->field_0x9bc != 0) {
		for (int i = 0; i < this->field_0x9bc; i++) {
			edDListTermMaterial(this->field_0x9b8 + i);
		}

		this->field_0x9bc = 0;
		ed3DUnInstallG2D(&this->field_0x9c0);
	}

	CActor::Term();

	return;
}

void CActorMiniGamesOrganizer::Reset()
{
	SetBehaviour(-1, -1, -1);

	CActor::Reset();

	ClearLocalData();

	return;
}

void CActorMiniGamesOrganizer::CheckpointReset()
{
	CActor::CheckpointReset();

	int iVar1 = this->actorState;
	if (iVar1 == 0xe) {
		SetState(this->field_0x93c, -1);
	}
	else {
		if ((((iVar1 == 9) || (iVar1 == 8)) || (iVar1 == 7)) || (iVar1 == 6)) {
			Reset();
		}
	}

	this->field_0x940 = true;

	return;
}

CBehaviour* CActorMiniGamesOrganizer::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;
	if (behaviourType == MINI_GAMES_ORGANIZER_BEHAVIOUR_STAND) {
		pNewBehaviour = &this->behaviourStand;
	}
	else {
		pNewBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pNewBehaviour;
}

StateConfig CActorMiniGamesOrganizer::_gStateCfg_MGO[12] = {
	StateConfig(0, 0x100), StateConfig(0, 0x200), StateConfig(0, 0x200),
	StateConfig(0, 0x200), StateConfig(0, 0x200), StateConfig(0, 0),
	StateConfig(0, 0), StateConfig(0, 0x100), StateConfig(0, 0),
	StateConfig(0, 0x200), StateConfig(0, 0x200), StateConfig(0, 0x200)
};

StateConfig* CActorMiniGamesOrganizer::GetStateCfg(int state)
{
	StateConfig* pStateConfig;
	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 12);
		pStateConfig = _gStateCfg_MGO + state + -5;
	}
	return pStateConfig;
}

void CActorMiniGamesOrganizer::InitAlphabet()
{
	for (int i = 0; i < 26; i++) {
		this->field_0x958[i][0] = 'A' + i;
		this->field_0x958[i][1] = 0;
	}

	strcpy(this->field_0x958[26], "<");
	strcpy(this->field_0x958[27], "o");
	strcpy(this->field_0x958[28], "A");
	strcpy(this->field_0x958[29], "A");

	return;
}

void CActorMiniGamesOrganizer::ClearLocalData()
{
	this->field_0x17c->Reset();
	this->menuWheel.Reset();
	this->field_0x76c.field_0x198 = 0.0f;
	this->field_0x920 = 0;
	this->field_0x924 = 0;
	this->field_0x928 = 0;
	this->field_0x92c = 0;
	this->field_0x934 = 0;
	this->field_0x9f0 = 0;
	this->field_0x9f4 = 0;
	this->field_0x994 = 0;
	this->field_0x998 = 0;
	this->field_0x9bc = 0;

	for (int i = 0; i < 3; i++) {
		strcpy(this->field_0x9ac[i], "-");
	}

	this->field_0x9b4 = -1;
	this->field_0x940 = true;
	this->field_0x9fc = -1;
	this->field_0x948 = 1.0f;
	this->field_0x944 = 0.0f;
	this->field_0x941 = false;
	this->field_0x9f8 = -1;

	CCinematic* pCVar1 = g_CinematicManager_0048efc->GetCinematic(this->field_0x16c);
	if (pCVar1 != 0) {
		pCVar1->pActor = this;
	}

	this->field_0x1d0.Reset();
	this->field_0x230.Reset();
	this->field_0x290.Reset();
	this->field_0x2f0.Reset();
	this->field_0x350.Reset();
	this->field_0x3b0.Reset();
	this->field_0x410.Reset();
	this->field_0x470.Reset();

	return;
}


void CActorMiniGamesOrganizer::Draw()

{
	ed_3d_hierarchy_node *peVar1;
	bool bVar2;
	edF32VECTOR4 local_3b0;
	edF32VECTOR4 local_3a0;
	edF32VECTOR4 local_390;
	edF32VECTOR4 local_380;
	edF32VECTOR4 local_370;
	edF32VECTOR4 local_360;
	edF32VECTOR4 local_350;
	edF32VECTOR4 local_340;
	edF32VECTOR4 local_330;
	edF32VECTOR4 local_320;
	edF32VECTOR4 local_310;
	edF32VECTOR4 local_300;
	edF32VECTOR4 local_2f0;
	edF32VECTOR4 local_2e0;
	edF32VECTOR4 local_2d0;
	edF32VECTOR4 local_2c0;
	edF32VECTOR4 local_2b0;
	edF32VECTOR4 local_2a0;
	edF32VECTOR4 local_290;
	edF32VECTOR4 local_280;
	edF32VECTOR4 local_270;
	edF32VECTOR4 local_260;
	edF32VECTOR4 local_250;
	edF32VECTOR4 local_240;
	edF32VECTOR4 local_230;
	edF32VECTOR4 local_220;
	edF32VECTOR4 local_210;
	edF32VECTOR4 local_200;
	edF32VECTOR4 local_1f0;
	edF32VECTOR4 local_1e0;
	edF32VECTOR4 local_1d0;
	edF32VECTOR4 local_1c0;
	edF32VECTOR4 local_1b0;
	edF32VECTOR4 local_1a0;
	edF32VECTOR4 local_190;
	edF32VECTOR4 local_180;
	edF32VECTOR4 local_170;
	edF32VECTOR4 local_160;
	edF32VECTOR4 local_150;
	edF32VECTOR2 local_138;
	edF32VECTOR2 local_130;
	edF32VECTOR2 local_128;
	edF32VECTOR2 local_120;
	edF32VECTOR2 local_118;
	edF32VECTOR2 local_110;
	edF32VECTOR2 local_108;
	edF32VECTOR2 local_100;
	edF32VECTOR2 local_f8;
	edF32VECTOR2 local_f0;
	edF32VECTOR2 local_e8;
	edF32VECTOR2 local_e0;
	edF32VECTOR2 local_d8;
	edF32VECTOR2 local_d0;
	edF32VECTOR2 local_c8;
	edF32VECTOR2 local_c0;
	edF32VECTOR2 local_b8;
	edF32VECTOR2 local_b0;
	edF32VECTOR2 local_a8;
	edF32VECTOR2 local_a0;
	edF32VECTOR2 local_98;
	edF32VECTOR2 local_90;
	edF32VECTOR2 local_88;
	edF32VECTOR2 local_80;
	edF32VECTOR2 local_78;
	edF32VECTOR2 local_70;
	edF32VECTOR2 local_68;
	edF32VECTOR2 local_60;
	edF32VECTOR2 local_58;
	edF32VECTOR2 local_50;
	edF32VECTOR2 local_48;
	edF32VECTOR2 local_40;
	edF32VECTOR2 local_38;
	edF32VECTOR2 local_30;
	edF32VECTOR2 local_28;
	edF32VECTOR2 local_20;
	edF32VECTOR2 local_18;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;

	if (this->actorState == 0x10) {
		DrawDisconnectedController();
		bVar2 = Frontend2DDList_BeginCurrent();
		if (bVar2 != false) {
			local_8.x = 0.0f;
			local_8.y = 0.0f;
			ed3DComputeScreenCoordinate(104.0f,&local_150,&local_8,CFrontend::_scene_handle);
			ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
			peVar1 = (this->field_0x1d0).pMeshTransformData;
			if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
				(peVar1->base).transformA.rowT.x = local_150.x;
				(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_150.y;
				(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_150.z;
				(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_150.w;
			}

			FrontendDList_EndCurrent();
		}
	}
	else {
		CActor::Draw();
		if ((GameFlags & 0x1c) == 0) {
			switch(this->actorState) {
			case 6:
				bVar2 = Frontend2DDList_BeginCurrent();
				if (bVar2 != false) {
					local_10.x = 0.0f;
					local_10.y = 0.0f;
					ed3DComputeScreenCoordinate(104.0f,&local_160,&local_10,CFrontend::_scene_handle);
					ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
					peVar1 = (this->field_0x1d0).pMeshTransformData;
					if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
						(peVar1->base).transformA.rowT.x = local_160.x;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_160.y;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_160.z;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_160.w;
					}
					local_18.x = 0.31f;
					local_18.y = -0.25f;
					ed3DComputeScreenCoordinate(102.0f,&local_170,&local_18,CFrontend::_scene_handle);
					bVar2 = this->field_0x230.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x230);
						peVar1 = (this->field_0x230).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_170.x;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.y = local_170.y;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.z = local_170.z;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.w = local_170.w;
						}
					}
					local_20.x = -0.44f;
					local_20.y = -0.22f;
					ed3DComputeScreenCoordinate(100.0f,&local_180,&local_20,CFrontend::_scene_handle);
					bVar2 = this->field_0x290.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x290);
						peVar1 = (this->field_0x290).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_180.x;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.y = local_180.y;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.z = local_180.z;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.w = local_180.w;
						}
					}
					local_28.y = 0.745f;
					local_28.x = 0.0f;
					ed3DComputeScreenCoordinate(103.0f,&local_190,&local_28,CFrontend::_scene_handle);
					bVar2 = this->field_0x2f0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x2f0);
						peVar1 = (this->field_0x2f0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_190.x;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.y = local_190.y;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.z = local_190.z;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.w = local_190.w;
						}
					}
					local_30.x = 0.0f;
					local_30.y = 0.0f;
					ed3DComputeScreenCoordinate(99.0f,&local_1a0,&local_30,CFrontend::_scene_handle);
					bVar2 = this->field_0x470.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x470);
						peVar1 = (this->field_0x470).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_1a0.x;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.y = local_1a0.y;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.z = local_1a0.z;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.w = local_1a0.w;
						}
					}
					FrontendDList_EndCurrent();
				}
				DrawMenuChooseText();
				break;
			case 7:
				DrawMenuBetText();
				bVar2 = Frontend2DDList_BeginCurrent();
				if (bVar2 != false) {
					local_38.x = 0.0f;
					local_38.y = 0.0f;
					ed3DComputeScreenCoordinate(104.0f,&local_1b0,&local_38,CFrontend::_scene_handle);
					ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
					peVar1 = (this->field_0x1d0).pMeshTransformData;
					if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
						(peVar1->base).transformA.rowT.x = local_1b0.x;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_1b0.y;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_1b0.z;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_1b0.w;
					}
					local_40.x = -0.55f;
					local_40.y = 0.68f;
					ed3DComputeScreenCoordinate(100.0f,&local_1c0,&local_40,CFrontend::_scene_handle);
					bVar2 = this->field_0x2f0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x2f0);
						peVar1 = (this->field_0x2f0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_1c0.x;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.y = local_1c0.y;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.z = local_1c0.z;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.w = local_1c0.w;
						}
					}
					local_48.x = 0.09f;
					local_48.y = -0.58f;
					ed3DComputeScreenCoordinate(103.0f,&local_1d0,&local_48,CFrontend::_scene_handle);
					bVar2 = this->field_0x350.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x350);
						peVar1 = (this->field_0x350).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_1d0.x;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.y = local_1d0.y;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.z = local_1d0.z;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.w = local_1d0.w;
						}
					}
					local_50.x = 0.39f;
					local_50.y = 0.59f;
					ed3DComputeScreenCoordinate(103.0f,&local_1e0,&local_50,CFrontend::_scene_handle);
					bVar2 = this->field_0x230.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x230);
						peVar1 = (this->field_0x230).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_1e0.x;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.y = local_1e0.y;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.z = local_1e0.z;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.w = local_1e0.w;
						}
					}
					local_58.x = 0.0f;
					local_58.y = 0.0f;
					ed3DComputeScreenCoordinate(102.0f,&local_1f0,&local_58,CFrontend::_scene_handle);
					bVar2 = this->field_0x290.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x290);
						peVar1 = (this->field_0x290).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_1f0.x;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.y = local_1f0.y;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.z = local_1f0.z;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.w = local_1f0.w;
						}
					}
					local_60.x = 0.0f;
					local_60.y = 0.0f;
					ed3DComputeScreenCoordinate(99.0f,&local_200,&local_60,CFrontend::_scene_handle);
					bVar2 = this->field_0x470.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x470);
						peVar1 = (this->field_0x470).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_200.x;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.y = local_200.y;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.z = local_200.z;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.w = local_200.w;
						}
					}
					FrontendDList_EndCurrent();
				}
				break;
			case 8:
				DrawMenuTrainText();
				bVar2 = Frontend2DDList_BeginCurrent();
				if (bVar2 != false) {
					local_68.x = 0.0f;
					local_68.y = 0.0f;
					ed3DComputeScreenCoordinate(104.0f,&local_210,&local_68,CFrontend::_scene_handle);
					ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
					peVar1 = (this->field_0x1d0).pMeshTransformData;
					if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
						(peVar1->base).transformA.rowT.x = local_210.x;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_210.y;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_210.z;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_210.w;
					}
					local_70.x = 0.25f;
					local_70.y = 0.15f;
					ed3DComputeScreenCoordinate(103.0f,&local_220,&local_70,CFrontend::_scene_handle);
					bVar2 = this->field_0x230.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x230);
						peVar1 = (this->field_0x230).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_220.x;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.y = local_220.y;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.z = local_220.z;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.w = local_220.w;
						}
					}
					local_78.y = 0.745f;
					local_78.x = 0.0f;
					ed3DComputeScreenCoordinate(103.0f,&local_230,&local_78,CFrontend::_scene_handle);
					bVar2 = this->field_0x290.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x290);
						peVar1 = (this->field_0x290).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_230.x;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.y = local_230.y;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.z = local_230.z;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.w = local_230.w;
						}
					}
					local_80.x = -0.44f;
					local_80.y = 0.1f;
					ed3DComputeScreenCoordinate(103.0f,&local_240,&local_80,CFrontend::_scene_handle);
					bVar2 = this->field_0x2f0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x2f0);
						peVar1 = (this->field_0x2f0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_240.x;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.y = local_240.y;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.z = local_240.z;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.w = local_240.w;
						}
					}
					local_88.x = 0.09f;
					local_88.y = -0.58f;
					ed3DComputeScreenCoordinate(103.0f,&local_250,&local_88,CFrontend::_scene_handle);
					bVar2 = this->field_0x350.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x350);
						peVar1 = (this->field_0x350).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_250.x;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.y = local_250.y;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.z = local_250.z;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.w = local_250.w;
						}
					}
					local_90.x = 0.0f;
					local_90.y = 0.0f;
					ed3DComputeScreenCoordinate(99.0f,&local_260,&local_90,CFrontend::_scene_handle);
					bVar2 = this->field_0x470.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x470);
						peVar1 = (this->field_0x470).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_260.x;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.y = local_260.y;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.z = local_260.z;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.w = local_260.w;
						}
					}
					FrontendDList_EndCurrent();
				}
				break;
			case 9:
				DrawMenuMultiText();
				bVar2 = Frontend2DDList_BeginCurrent();
				if (bVar2 != false) {
					local_98.x = 0.0f;
					local_98.y = 0.0f;
					ed3DComputeScreenCoordinate(104.0f,&local_270,&local_98,CFrontend::_scene_handle);
					ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
					peVar1 = (this->field_0x1d0).pMeshTransformData;
					if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
						(peVar1->base).transformA.rowT.x = local_270.x;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_270.y;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_270.z;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_270.w;
					}
					local_a0.y = 0.71f;
					local_a0.x = 0.0f;
					ed3DComputeScreenCoordinate(103.0f,&local_280,&local_a0,CFrontend::_scene_handle);
					bVar2 = this->field_0x230.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x230);
						peVar1 = (this->field_0x230).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_280.x;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.y = local_280.y;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.z = local_280.z;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.w = local_280.w;
						}
					}
					local_a8.x = -0.37f;
					local_a8.y = 0.11f;
					ed3DComputeScreenCoordinate(103.0f,&local_290,&local_a8,CFrontend::_scene_handle);
					bVar2 = this->field_0x290.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x290);
						peVar1 = (this->field_0x290).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_290.x;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.y = local_290.y;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.z = local_290.z;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.w = local_290.w;
						}
					}
					local_b0.x = 0.56f;
					local_b0.y = 0.22f;
					ed3DComputeScreenCoordinate(103.0f,&local_2a0,&local_b0,CFrontend::_scene_handle);
					bVar2 = this->field_0x2f0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x2f0);
						peVar1 = (this->field_0x2f0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_2a0.x;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.y = local_2a0.y;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.z = local_2a0.z;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.w = local_2a0.w;
						}
					}
					local_b8.x = 0.09f;
					local_b8.y = -0.58f;
					ed3DComputeScreenCoordinate(103.0f,&local_2b0,&local_b8,CFrontend::_scene_handle);
					bVar2 = this->field_0x350.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x350);
						peVar1 = (this->field_0x350).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_2b0.x;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.y = local_2b0.y;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.z = local_2b0.z;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.w = local_2b0.w;
						}
					}
					local_c0.x = 0.0f;
					local_c0.y = 0.0f;
					ed3DComputeScreenCoordinate(99.0f,&local_2c0,&local_c0,CFrontend::_scene_handle);
					bVar2 = this->field_0x470.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x470);
						peVar1 = (this->field_0x470).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_2c0.x;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.y = local_2c0.y;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.z = local_2c0.z;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.w = local_2c0.w;
						}
					}
					FrontendDList_EndCurrent();
				}
				break;
			case 0xe:
				DrawMenuResultText();
				bVar2 = Frontend2DDList_BeginCurrent();
				if (bVar2 != false) {
					local_c8.x = 0.0f;
					local_c8.y = 0.0f;
					ed3DComputeScreenCoordinate(104.0f,&local_2d0,&local_c8,CFrontend::_scene_handle);
					ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
					peVar1 = (this->field_0x1d0).pMeshTransformData;
					if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
						(peVar1->base).transformA.rowT.x = local_2d0.x;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_2d0.y;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_2d0.z;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_2d0.w;
					}
					local_d0.y = 0.745f;
					local_d0.x = 0.0f;
					ed3DComputeScreenCoordinate(103.0f,&local_2e0,&local_d0,CFrontend::_scene_handle);
					bVar2 = this->field_0x230.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x230);
						peVar1 = (this->field_0x230).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_2e0.x;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.y = local_2e0.y;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.z = local_2e0.z;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.w = local_2e0.w;
						}
					}
					local_d8.x = 0.44f;
					local_d8.y = -0.38f;
					ed3DComputeScreenCoordinate(103.0f,&local_2f0,&local_d8,CFrontend::_scene_handle);
					bVar2 = this->field_0x290.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x290);
						peVar1 = (this->field_0x290).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_2f0.x;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.y = local_2f0.y;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.z = local_2f0.z;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.w = local_2f0.w;
						}
					}
					local_e0.x = -0.31f;
					local_e0.y = 0.31f;
					ed3DComputeScreenCoordinate(103.0f,&local_300,&local_e0,CFrontend::_scene_handle);
					bVar2 = this->field_0x2f0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x2f0);
						peVar1 = (this->field_0x2f0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_300.x;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.y = local_300.y;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.z = local_300.z;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.w = local_300.w;
						}
					}
					local_e8.x = -0.46f;
					local_e8.y = -0.06f;
					ed3DComputeScreenCoordinate(103.0f,&local_310,&local_e8,CFrontend::_scene_handle);
					bVar2 = this->field_0x350.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x350);
						peVar1 = (this->field_0x350).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_310.x;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.y = local_310.y;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.z = local_310.z;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.w = local_310.w;
						}
					}
					local_f0.x = 0.0f;
					local_f0.y = 0.0f;
					ed3DComputeScreenCoordinate(99.0f,&local_320,&local_f0,CFrontend::_scene_handle);
					bVar2 = this->field_0x470.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x470);
						peVar1 = (this->field_0x470).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_320.x;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.y = local_320.y;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.z = local_320.z;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.w = local_320.w;
						}
					}
					bVar2 = this->field_0x3b0.HasMesh();
					if (bVar2 != false) {
						local_f8.x = -0.31f;
						local_f8.y = 0.31f;
						ed3DComputeScreenCoordinate(103.0f,&local_330,&local_f8,CFrontend::_scene_handle);
						bVar2 = this->field_0x3b0.HasMesh();
						if (bVar2 != false) {
							ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
													 1.0f,1.0f,&this->field_0x3b0);
							peVar1 = (this->field_0x3b0).pMeshTransformData;
							if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
								(peVar1->base).transformA.rowT.x = local_330.x;
								(((this->field_0x3b0).pMeshTransformData)->base).transformA.rowT.y = local_330.y;
								(((this->field_0x3b0).pMeshTransformData)->base).transformA.rowT.z = local_330.z;
								(((this->field_0x3b0).pMeshTransformData)->base).transformA.rowT.w = local_330.w;
							}
						}
					}
					FrontendDList_EndCurrent();
				}
				break;
			case 0xf:
				DrawMenuEnterNameText();
				bVar2 = Frontend2DDList_BeginCurrent();
				if (bVar2 != false) {
					local_100.x = 0.0f;
					local_100.y = 0.0f;
					ed3DComputeScreenCoordinate(104.0f,&local_340,&local_100,CFrontend::_scene_handle);
					ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f, &this->field_0x1d0);
					peVar1 = (this->field_0x1d0).pMeshTransformData;
					if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
						(peVar1->base).transformA.rowT.x = local_340.x;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.y = local_340.y;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.z = local_340.z;
						(((this->field_0x1d0).pMeshTransformData)->base).transformA.rowT.w = local_340.w;
					}
					local_108.y = 0.745f;
					local_108.x = 0.0f;
					ed3DComputeScreenCoordinate(103.0f,&local_350,&local_108,CFrontend::_scene_handle);
					bVar2 = this->field_0x230.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x230);
						peVar1 = (this->field_0x230).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_350.x;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.y = local_350.y;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.z = local_350.z;
							(((this->field_0x230).pMeshTransformData)->base).transformA.rowT.w = local_350.w;
						}
					}
					local_110.x = 0.44f;
					local_110.y = -0.38f;
					ed3DComputeScreenCoordinate(103.0f,&local_360,&local_110,CFrontend::_scene_handle);
					bVar2 = this->field_0x290.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x290);
						peVar1 = (this->field_0x290).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_360.x;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.y = local_360.y;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.z = local_360.z;
							(((this->field_0x290).pMeshTransformData)->base).transformA.rowT.w = local_360.w;
						}
					}
					local_118.x = -0.31f;
					local_118.y = 0.31f;
					ed3DComputeScreenCoordinate(103.0f,&local_370,&local_118,CFrontend::_scene_handle);
					bVar2 = this->field_0x2f0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x2f0);
						peVar1 = (this->field_0x2f0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_370.x;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.y = local_370.y;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.z = local_370.z;
							(((this->field_0x2f0).pMeshTransformData)->base).transformA.rowT.w = local_370.w;
						}
					}
					local_120.x = -0.46f;
					local_120.y = -0.06f;
					ed3DComputeScreenCoordinate(103.0f,&local_380,&local_120,CFrontend::_scene_handle);
					bVar2 = this->field_0x350.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x350);
						peVar1 = (this->field_0x350).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_380.x;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.y = local_380.y;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.z = local_380.z;
							(((this->field_0x350).pMeshTransformData)->base).transformA.rowT.w = local_380.w;
						}
					}
					local_128.x = -0.36f;
					local_128.y = -0.61f;
					ed3DComputeScreenCoordinate(103.0f,&local_390,&local_128,CFrontend::_scene_handle);
					bVar2 = this->field_0x3b0.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x3b0);
						peVar1 = (this->field_0x3b0).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_390.x;
							(((this->field_0x3b0).pMeshTransformData)->base).transformA.rowT.y = local_390.y;
							(((this->field_0x3b0).pMeshTransformData)->base).transformA.rowT.z = local_390.z;
							(((this->field_0x3b0).pMeshTransformData)->base).transformA.rowT.w = local_390.w;
						}
					}
					local_130.x = 0.0f;
					local_130.y = 0.0f;
					ed3DComputeScreenCoordinate(99.0f,&local_3a0,&local_130,CFrontend::_scene_handle);
					bVar2 = this->field_0x470.HasMesh();
					if (bVar2 != false) {
						ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f,&this->field_0x470);
						peVar1 = (this->field_0x470).pMeshTransformData;
						if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
							(peVar1->base).transformA.rowT.x = local_3a0.x;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.y = local_3a0.y;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.z = local_3a0.z;
							(((this->field_0x470).pMeshTransformData)->base).transformA.rowT.w = local_3a0.w;
						}
					}
					bVar2 = this->field_0x410.HasMesh();
					if (bVar2 != false) {
						local_138.x = -0.31f;
						local_138.y = 0.31f;
						ed3DComputeScreenCoordinate(103.0f,&local_3b0,&local_138,CFrontend::_scene_handle);
						bVar2 = this->field_0x410.HasMesh();
						if (bVar2 != false) {
							ScaleMenuMesh((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
													 1.0f,1.0f,&this->field_0x410);
							peVar1 = (this->field_0x410).pMeshTransformData;
							if (peVar1 != (ed_3d_hierarchy_node *)0x0) {
								(peVar1->base).transformA.rowT.x = local_3b0.x;
								(((this->field_0x410).pMeshTransformData)->base).transformA.rowT.y = local_3b0.y;
								(((this->field_0x410).pMeshTransformData)->base).transformA.rowT.z = local_3b0.z;
								(((this->field_0x410).pMeshTransformData)->base).transformA.rowT.w = local_3b0.w;
							}
						}
					}
					FrontendDList_EndCurrent();
				}
			}
		}
	}
	return;
}



void CActorMiniGamesOrganizer::PlayMenuSound(int soundId)
{
	this->field_0xa00.pSound = CScene::ptable.g_AudioManager_00451698->GetSound(soundId);
	if (NoAudio == 0) {
		this->field_0xa00.pSound3dData = 0;
		if (this->field_0xa00.pSound != 0) {
			this->field_0xa00.field_0x20 = -1;
			this->field_0xa00.soundId = this->field_0xa00.pSound->Play(this->field_0xa00.soundId,
				this->field_0xa00.field_0x20, 0, &this->field_0xa00, 0, &this->field_0xa00.soundId);
		}
	}
	return;
}

void CActorMiniGamesOrganizer::BehaviourMiniGamesOrganizerStand_Manage()
{
	ManageFade();

	ManageMusic(this->actorState);

	bool bVar2 = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);
	if (!bVar2 || ((GameFlags & 0x1c) != 0)) {
		if (this->actorState == 0x10) SetState(this->prevActorState, -1);
	}
	else {
		if ((GetStateFlags(this->actorState) & 0x200) != 0) SetState(0x10, -1);
	}
	if (this->field_0x940) {
		switch (this->actorState) {
		case 6: ManageMenuChoose(); break;
		case 7: ManageMenuMulti(); break;
		case 8: {
			CPlayerInput* pCVar4 = this->field_0x9f0->GetInputManager(0, 0);
			if (pCVar4 != 0) {
				if ((pCVar4->pressedBitfield & 0x1000000) != 0) {
					DoMessage(GetMiniGame(this->field_0x920), (ACTOR_MESSAGE)0x56, (void*)3);
					DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
					SetState(10, -1);
					PlayMenuSound(this->field_0x188);
				}
				if ((pCVar4->pressedBitfield & 0x4000000) != 0) {
					PlayMenuSound(this->field_0x18c);
					SetState(6, -1);
				}
			}
			break;
		}
		case 9: ManageMenuTrain(); break;
		case 10: break;
		case 0xd: if (0.5f < this->timeInAir) SetState(0xe, -1); break;
		case 0xe: ManageMenuResult(); break;
		case 0xf: ManageMenuEnterName(); break;
		}
		ManageZone();
	}
	return;
}

static void SetMenuArrowSize(astruct_22* pArrow, float size)
{
	pArrow->field_0x198 = 0.0f;
	pArrow->field_0x0.iWidth = (ushort)(int)((float)gVideoConfig.screenWidth * size);
	pArrow->field_0x0.iHeight = (ushort)(int)((float)gVideoConfig.screenHeight * size);
	pArrow->field_0xc0.iWidth = pArrow->field_0x0.iWidth;
	pArrow->field_0xc0.iHeight = pArrow->field_0x0.iHeight;
}

void CActorMiniGamesOrganizer::BehaviourMiniGamesOrganizerStand_InitState(int newState)
{
	InitMenuMeshes(newState);
	if (((GetStateFlags(this->actorState) & 0x200) != 0) &&
		!((this->prevActorState == 0xe) && (this->actorState == 0xf)) &&
		!((this->prevActorState == 0xf) && (this->actorState == 0xe)) && !this->field_0x941) {
		MenuFade(0.1f, 1);
		this->field_0x944 = 1.0f;
	}
	if ((GetStateFlags(this->actorState) & 0x200) != 0) {
		CallPauseChange(1);
		GameFlags = GameFlags | 0x4080;
		CScene::ptable.g_FrontendManager_00451680->SetActive(false);
	}
	switch (newState) {
	case 6: {
		ComputeCurPlayMode();
		int local_20[4] = { 0, 0, 1, 1 };
		DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x23, local_20);
		SetMenuArrowSize(&this->field_0x76c, 0.06f);
		break;
	}
	case 7: InitMenuMulti(); break;
	case 8: this->field_0x924 = 3; break;
	case 9:
		SetMenuArrowSize(&this->field_0x76c, 0.045f);
		this->field_0x924 = 2;
		break;
	case 10:
	case 0xb:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		break;
	case 0xd:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		if (!this->field_0x941) {
			MenuFade(0.5f, 2);
			this->field_0x944 = 1.0f;
		}
		DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x25, 0);
		CScene::ptable.g_CameraManager_0045167c->PushCamera(CActorHero::_gThis->pDeathCamera, 0);
		break;
	case 0xe:
	case 0xf:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags | 0x80;
		this->flags = this->flags & 0xffffffdf;
		EvaluateDisplayState();
		this->flags = this->flags | 0x400;
		if (newState == 0xf) {
			this->field_0x9b4 = 0;
			this->field_0x998 = 0;
			this->field_0x994 = 0;
		}
		break;
	case 0x10:
		this->flags = this->flags | 0x80;
		this->flags = this->flags & 0xffffffdf;
		EvaluateDisplayState();
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags | 0x400;
		break;
	}
	return;
}

void CActorMiniGamesOrganizer::BehaviourMiniGamesOrganizerStand_TermState(int oldState)
{
	if ((oldState == 0x10) || (oldState == 6) || (oldState == 8) || (oldState == 9) ||
		(oldState == 7) || (oldState == 0xe) || (oldState == 0xf)) TermMenuMeshes();
	if ((GetStateFlags(this->actorState) & 0x200) != 0) {
		CallPauseChange(0);
		GameFlags = GameFlags & 0xffffbf7f;
		CScene::ptable.g_FrontendManager_00451680->SetActive(true);
	}
	switch (oldState) {
	case 6:
		this->menuWheel.MoveWheel();
		this->field_0x76c.field_0x198 = 0.0f;
		break;
	case 0xd:
		this->flags = this->flags & 0xfffffffc;
		DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x26, 0);
		CScene::ptable.g_CameraManager_0045167c->PopCamera(CActorHero::_gThis->pDeathCamera);
		break;
	case 0xe:
	case 0xf:
		if (oldState == 0xf) {
			char local_4[4] = { this->field_0x9ac[0][0], this->field_0x9ac[1][0], this->field_0x9ac[2][0], 0 };
			GetMiniGame(this->field_0x920)->SetScoreName(local_4);
		}
		this->flags = this->flags & 0xfffffffc;
		this->flags = this->flags & 0xffffff5f;
		EvaluateDisplayState();
		this->flags = this->flags & 0xfffffbff;
		if (oldState == 0xf) this->field_0x9b4 = -1;
		break;
	case 0x10:
		this->flags = this->flags & 0xffffff5f;
		EvaluateDisplayState();
		this->flags = this->flags & 0xfffffffc;
		this->flags = this->flags & 0xfffffbff;
		break;
	}
	return;
}

void CActorMiniGamesOrganizer::InitMenuMeshes(int state)
{
	StaticMeshComponent* aMeshes[] = { &this->field_0x1d0, &this->field_0x230, &this->field_0x290,
		&this->field_0x2f0, &this->field_0x350, &this->field_0x3b0, &this->field_0x410, &this->field_0x470 };
	if ((state != 6) && (state != 7) && (state != 8) && (state != 9) &&
		(state != 0xe) && (state != 0xf) && (state != 0x10)) return;
	for (int i = 0; i < 8; i++) {
		if ((aMeshes[i]->meshIndex == -1) || (aMeshes[i]->textureIndex == -1)) return;
	}
	ed_g3d_manager* pMesh = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->field_0x174, this->textureIndex_0x170);
	this->field_0x1d0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_back_01");
	switch (state) {
	case 6:
		this->field_0x230.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_01_pan_01");
		this->field_0x290.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_01_pan_02");
		this->field_0x2f0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_03_pan_02");
		this->field_0x470.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_Symbole_01");
		this->field_0x76c.pContext = this;
		break;
	case 7:
		this->field_0x230.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_04_pan_01");
		this->field_0x290.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_04_pan_02");
		this->field_0x2f0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_04_pan_03");
		this->field_0x350.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_02_pan_04");
		this->field_0x470.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_Symbole_01");
		break;
	case 8:
		this->field_0x230.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_03_pan_01");
		this->field_0x290.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_03_pan_02");
		this->field_0x2f0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_03_pan_03");
		this->field_0x350.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_02_pan_04");
		this->field_0x470.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_Symbole_01");
		break;
	case 9:
		this->field_0x230.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_02_pan_01");
		this->field_0x290.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_02_pan_02");
		this->field_0x2f0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_02_pan_03");
		this->field_0x350.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_02_pan_04");
		this->field_0x470.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_Symbole_02");
		break;
	case 0xe:
	case 0xf:
		this->field_0x230.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_03_pan_02");
		this->field_0x290.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_05_pan_01");
		this->field_0x2f0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_05_pan_02");
		this->field_0x350.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_05_pan_03");
		this->field_0x470.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_Symbole_01");
		if (state == 0xf) this->field_0x3b0.Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_05_pan_04");
		if ((GetMiniGame(this->field_0x920)->curBehaviourId == 3) && (GetMiniGame(this->field_0x920)->field_0x1b0 == 0)) {
			(state == 0xe ? this->field_0x3b0 : this->field_0x410).Init(CFrontend::_scene_handle, pMesh, &this->field_0x198, "Gam_05_pan_02_5");
		}
		break;
	}
	return;
}

void CActorMiniGamesOrganizer::TermMenuMeshes()
{
	StaticMeshComponent* aMeshes[] = { &this->field_0x1d0, &this->field_0x230, &this->field_0x290,
		&this->field_0x2f0, &this->field_0x350, &this->field_0x3b0, &this->field_0x410, &this->field_0x470 };
	for (int i = 0; i < 8; i++) {
		if (aMeshes[i]->HasMesh()) aMeshes[i]->Term(CFrontend::_scene_handle);
	}
	return;
}

void CActorMiniGamesOrganizer::ManageMenuChoose()
{
	CPlayerInput* pCVar2 = this->field_0x9f0->GetInputManager(0, 0);
	if (pCVar2 != 0) {
		this->menuWheel.Manage();
		this->field_0x76c.FUN_002ef890();
		uint uVar4 = pCVar2->pressedBitfield;
		if ((uVar4 & 0x100004) != 0) {
			MoveMenuArrow(&this->field_0x76c, false);
			PlayMenuSound(this->field_0x190);
			PrevPlayMode();
		}
		else if ((uVar4 & 0x200008) != 0) {
			MoveMenuArrow(&this->field_0x76c, true);
			PlayMenuSound(this->field_0x190);
			NextPlayMode();
		}
		else if ((uVar4 & 0x400001) != 0) {
			this->field_0x920 = (this->field_0x920 + this->field_0x17c->entryCount - 1) % this->field_0x17c->entryCount;
			ComputeCurPlayMode();
			ComputeCurPlayMode();
			this->menuWheel.MoveWheel(false);
			PlayMenuSound(this->field_0x190);
		}
		else if ((uVar4 & 0x800002) != 0) {
			this->field_0x920 = (this->field_0x920 + 1) % this->field_0x17c->entryCount;
			ComputeCurPlayMode();
			ComputeCurPlayMode();
			this->menuWheel.MoveWheel(true);
			PlayMenuSound(this->field_0x190);
		}
		else if ((uVar4 & 0x1000000) != 0) {
			if (this->field_0x928 == 2) SetState(9, -1);
			else if (this->field_0x928 == 3) {
				PlayMenuSound(this->field_0x188);
				SetState(8, -1);
			}
			else if (this->field_0x928 == 1) SetState(7, -1);
		}
		else if ((uVar4 & 0x4000000) != 0) {
			PlayMenuSound(this->field_0x18c);
			DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
			SetState(5, -1);
		}
	}
	return;
}

static bool IsMenuBetAvailable(S_MINI_GAME_BET* pBet)
{
	CLevelScheduler* pLevel = CScene::ptable.g_LevelScheduleManager_00451660;
	int iVar1 = pBet->cost + CLevelScheduler::_gGameNfo.bet;
	int iVar2 = pLevel->GetEpisode(CLevelScheduler::ScenVar_Get(SCN_GAME_CURRENT_EPISODE))->bet;
	if (iVar2 < iVar1) iVar1 = iVar2;
	return pBet->reward <= iVar1;
}

void CActorMiniGamesOrganizer::InitMenuMulti()
{
	SetMenuArrowSize(&this->field_0x76c, 0.045f);
	CBehaviourMiniGameMulti* pBehaviour = GetMiniGame(this->field_0x920)->GetMultiBehaviour();
	this->field_0x938 = 0;
	this->field_0x924 = 0;
	for (int i = 0; i < pBehaviour->nbBets; i++) {
		pBehaviour->aBets[i].bAvailable = IsMenuBetAvailable(pBehaviour->aBets + i);
		if (pBehaviour->aBets[i].bAvailable) {
			if (this->field_0x938 == 0) this->field_0x924 = i;
			this->field_0x938++;
		}
	}
	return;
}

void CActorMiniGamesOrganizer::ManageMenuMulti()
{
	CPlayerInput* pInput = this->field_0x9f0->GetInputManager(0, 0);
	if (pInput != 0) {
		this->field_0x76c.FUN_002ef890();
		CActorMiniGame* pMiniGame = GetMiniGame(this->field_0x920);
		CBehaviourMiniGameMulti* pBehaviour = pMiniGame->GetMultiBehaviour();
		this->field_0x938 = 0;
		for (int i = 0; i < pBehaviour->nbBets; i++) {
			pBehaviour->aBets[i].bAvailable = IsMenuBetAvailable(pBehaviour->aBets + i);
			if (pBehaviour->aBets[i].bAvailable) this->field_0x938++;
		}
		if ((0 < pBehaviour->nbBets) && (0 < this->field_0x938) && ((pInput->pressedBitfield & 0x100004) != 0)) {
			PlayMenuSound(this->field_0x190);
			MoveMenuArrow(&this->field_0x76c, false);
			if (0 < this->field_0x938) {
				do { this->field_0x924 = (this->field_0x924 + 1) % pBehaviour->nbBets; }
				while (!pBehaviour->aBets[this->field_0x924].bAvailable);
			}
		}
		if ((0 < pBehaviour->nbBets) && (0 < this->field_0x938) && ((pInput->pressedBitfield & 0x200008) != 0)) {
			PlayMenuSound(this->field_0x190);
			MoveMenuArrow(&this->field_0x76c, true);
			if (0 < this->field_0x938) {
				do { this->field_0x924 = (this->field_0x924 + pBehaviour->nbBets - 1) % pBehaviour->nbBets; }
				while (!pBehaviour->aBets[this->field_0x924].bAvailable);
			}
		}
		if ((pInput->pressedBitfield & 0x1000000) != 0) {
			if (0 < this->field_0x938) {
				pBehaviour->curBet = this->field_0x924;
				int cost = pBehaviour->aBets[pBehaviour->curBet].cost;
				if (cost <= CLevelScheduler::_gGameNfo.nbMoney) {
					CLevelScheduler::gThis->Money_GiveToBet(cost);
					DoMessage(pMiniGame, (ACTOR_MESSAGE)0x56, (void*)1);
					DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
					SetState(10, -1);
				}
			}
		}
		if ((pInput->pressedBitfield & 0x4000000) != 0) SetState(6, -1);
	}
	return;
}

void CActorMiniGamesOrganizer::ManageMenuTrain()
{
	CPlayerInput* pInput = this->field_0x9f0->GetInputManager(0, 0);
	if (pInput != 0) {
		this->field_0x76c.FUN_002ef890();
		CActorMiniGame* pMiniGame = GetMiniGame(this->field_0x920);
		if ((pInput->pressedBitfield & 0x100004) != 0) {
			PlayMenuSound(this->field_0x190);
			MoveMenuArrow(&this->field_0x76c, false);
			if (pMiniGame->GetSoloBehaviour()->nbPlayers < 6) pMiniGame->GetSoloBehaviour()->nbPlayers++;
		}
		if ((pInput->pressedBitfield & 0x200008) != 0) {
			PlayMenuSound(this->field_0x190);
			MoveMenuArrow(&this->field_0x76c, true);
			if (2 < pMiniGame->GetSoloBehaviour()->nbPlayers) pMiniGame->GetSoloBehaviour()->nbPlayers--;
		}
		if ((pInput->pressedBitfield & 0x1000000) != 0) {
			DoMessage(pMiniGame, (ACTOR_MESSAGE)0x56, (void*)2);
			DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
			SetState(10, -1);
		}
		if ((pInput->pressedBitfield & 0x4000000) != 0) SetState(6, -1);
	}
	return;
}

void CActorMiniGamesOrganizer::ManageMenuResult()
{
	CActorMiniGame* pMiniGame = GetMiniGame(this->field_0x920);
	this->field_0x76c.FUN_002ef890();
	if ((gPlayerInput.pressedBitfield & 0x200008) != 0) {
		PlayMenuSound(this->field_0x190);
		MoveMenuArrow(&this->field_0x76c, true);
		pMiniGame->NextFinalAction();
	}
	if ((gPlayerInput.pressedBitfield & 0x100004) != 0) {
		PlayMenuSound(this->field_0x190);
		MoveMenuArrow(&this->field_0x76c, false);
		pMiniGame->PrevFinalAction();
	}
	if ((gPlayerInput.pressedBitfield & 0x1000000) != 0) {
		int iVar1 = pMiniGame->field_0x1cc;
		if (iVar1 == 0) {
			for (int i = 0; i < 3; i++) strcpy(this->field_0x9ac[i], "-");
			PlayMenuSound(this->field_0x188);
			SetState(0xf, -1);
		}
		else if (iVar1 == 2) {
			PlayMenuSound(this->field_0x188);
			pMiniGame->DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x58, 0);
			char local_8[4] = { this->field_0x9ac[0][0], this->field_0x9ac[1][0], this->field_0x9ac[2][0], 0 };
			pMiniGame->SetScoreName(local_8);
			DoMessage(pMiniGame, (ACTOR_MESSAGE)0x56, 0);
			this->field_0x93c = 5;
			this->field_0x940 = false;
		}
		else if (iVar1 == 1) {
			PlayMenuSound(this->field_0x188);
			_msg_mini_game_restart local_28;
			local_28.pLocation = &pMiniGame->field_0x18c.Get()->location;
			local_28.pRotation = &pMiniGame->field_0x18c.Get()->rotation;
			local_28.sectorId = pMiniGame->field_0x190;
			if (local_28.sectorId == -1) local_28.sectorId = CScene::ptable.g_SectorManager_00451670->baseSector.desiredSectorID;
			char local_4[4] = { this->field_0x9ac[0][0], this->field_0x9ac[1][0], this->field_0x9ac[2][0], 0 };
			pMiniGame->SetScoreName(local_4);
			pMiniGame->DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x5b, &local_28);
			DoMessage(pMiniGame, (ACTOR_MESSAGE)0x5b, 0);
			this->field_0x93c = 10;
			this->field_0x940 = false;
		}
	}
	return;
}

void CActorMiniGamesOrganizer::ManageMenuEnterName()
{
	uint uVar2 = gPlayerInput.pressedBitfield;
	if ((uVar2 & 0x400001) != 0) {
		PlayMenuSound(this->field_0x190);
		int iVar6 = this->field_0x99c[this->field_0x998];
		this->field_0x994 = (this->field_0x994 - 1 + iVar6) % iVar6;
	}
	if ((uVar2 & 0x800002) != 0) {
		PlayMenuSound(this->field_0x190);
		this->field_0x994 = (this->field_0x994 + 1) % this->field_0x99c[this->field_0x998];
	}
	if ((uVar2 & 0x100004) != 0) {
		PlayMenuSound(this->field_0x190);
		this->field_0x998 = (this->field_0x998 + 3) % 4;
		this->field_0x994 %= this->field_0x99c[this->field_0x998];
	}
	if ((uVar2 & 0x200008) != 0) {
		PlayMenuSound(this->field_0x190);
		this->field_0x998 = (this->field_0x998 + 1) % 4;
		this->field_0x994 %= this->field_0x99c[this->field_0x998];
	}
	if ((uVar2 & 0x1000000) != 0) {
		PlayMenuSound(this->field_0x188);
		int iVar6 = this->field_0x994;
		for (int i = 0; i < this->field_0x998; i++) iVar6 += this->field_0x99c[i];
		if (iVar6 == 0x1a) {
			this->field_0x9b4--;
			if (this->field_0x9b4 < 0) this->field_0x9b4 = 0;
			this->field_0x9ac[this->field_0x9b4][0] = '-';
			return;
		}
		if (iVar6 == 0x1b) {
			this->field_0x9b4 = 3;
			SetState(0xe, -1);
			return;
		}
		if (this->field_0x9b4 < 3) {
			this->field_0x9ac[this->field_0x9b4][0] = this->field_0x958[iVar6][0];
			if (this->field_0x9b4 == 2) {
				this->field_0x998 = 3;
				this->field_0x994 = 0;
			}
			this->field_0x9b4++;
		}
	}
	if ((uVar2 & 0x4000000) != 0) {
		PlayMenuSound(this->field_0x18c);
		// The PS2 writes the next name byte even when all three letters have been entered.
		if (this->field_0x9b4 < 3) this->field_0x9ac[this->field_0x9b4][0] = '-';
	}
	return;
}

void CActorMiniGamesOrganizer::ManageFade()
{
	if (!CScene::_pinstance->FUN_001b92f0()) {
		if (0.0f < this->field_0x944) this->field_0x944 -= GetTimer()->lastFrameTime / 0.2f;
	}
	else {
		this->field_0x944 += GetTimer()->lastFrameTime;
	}
	if (this->field_0x948 < this->field_0x944) this->field_0x944 = this->field_0x948;
	else if (this->field_0x944 < 0.0f) this->field_0x944 = 0.0f;
	float fVar4 = (this->field_0x948 - this->field_0x944) / this->field_0x948;
	float fVar3 = 1.0f;
	if (fVar4 <= 1.0f) {
		fVar3 = fVar4;
		if (fVar4 < 0.0f) fVar3 = 0.0f;
	}

	this->field_0x94c = (byte)(int)(fVar3 * 255.0f);

	return;
}

void CActorMiniGamesOrganizer::ManageMusic(int state)
{
	CAudioManager* pAudio = CScene::ptable.g_AudioManager_00451698;
	CMusic* pMusic = ((this->field_0x184 == -1) || ((uint)pAudio->nbMusic <= this->field_0x184)) ? 0 : pAudio->aMusic + this->field_0x184;
	if (((GetStateFlags(this->prevActorState) & 0x200) == 0) && ((GetStateFlags(state) & 0x200) != 0)) {
		if ((pMusic != 0) && (this->field_0x9f8 == -1)) this->field_0x9f8 = pAudio->field_0x38->Start(5.0f, 1.0f, 1.3f, 0.0f, pMusic, 0x19);
	}
	if (((GetStateFlags(state) & 0x200) == 0) && ((GetStateFlags(this->prevActorState) & 0x200) != 0)) {
		if ((this->field_0x9f8 != -1) && pAudio->field_0x38->IsMusic(this->field_0x9f8, pMusic)) pAudio->field_0x38->Stop(0.0f, 0.0f, this->field_0x9f8);
		this->field_0x9f8 = -1;
	}
	return;
}

void CActorMiniGamesOrganizer::ManageZone()
{
	bool bVar2 = false;
	for (int i = 0; i < this->field_0x17c->entryCount; i++) {
		CActorMiniGame* pMiniGame = GetMiniGame(i);
		if (pMiniGame->field_0x1d4 && (pMiniGame->field_0x1d8 == 0)) bVar2 = true;
	}
	if (bVar2) {
		ed_zone_3d* pZone = 0;
		CEventManager* pEvent = CScene::ptable.g_EventManager_006f5080;
		if (this->field_0x180 != -1) pZone = edEventGetChunkZone(pEvent->activeChunkId, this->field_0x180);
		if (pZone != 0) {
			int iVar5 = edEventComputeZoneAgainstVertex(pEvent->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
			int local_20[3] = { iVar5 == 1 ? 0x10 : 0x12, 0, this->field_0x9fc };
			this->field_0x9f4->DoMessage(this->field_0x9f4->actorRef.Get(), (ACTOR_MESSAGE)0x4e, local_20);
			this->field_0x9fc = local_20[2];
		}
	}
	return;
}

void CActorMiniGamesOrganizer::ComputeCurPlayMode()
{
	CActor *pActor;
	CBehaviour *pCVar2;
	int iVar3;

	pActor = GetMiniGame(this->field_0x920);
	if (this->field_0x17c == 0) {
		iVar3 = 0;
	}
	else {
		iVar3 = this->field_0x17c->entryCount;
	}
	if (0 < iVar3) {
		iVar3 = this->field_0x928;
		if (iVar3 == 3) {
			pCVar2 = pActor->GetBehaviour(2);
			if (pCVar2 == (CBehaviour *)0x0) {
				this->field_0x928 = 2;
				pCVar2 = pActor->GetBehaviour(4);
				if (pCVar2 == (CBehaviour *)0x0) {
					this->field_0x928 = 1;
				}
			}
		}
		else {
			if (iVar3 == 2) {
				pCVar2 = pActor->GetBehaviour(4);
				if (pCVar2 == (CBehaviour *)0x0) {
					this->field_0x928 = 1;
					pCVar2 = pActor->GetBehaviour(3);
					if (pCVar2 == (CBehaviour *)0x0) {
						this->field_0x928 = 3;
					}
				}
			}
			else {
				if (iVar3 == 1) {
					pCVar2 = pActor->GetBehaviour(3);
					if (pCVar2 == (CBehaviour *)0x0) {
						this->field_0x928 = 2;
						pCVar2 = pActor->GetBehaviour(4);
						if (pCVar2 == (CBehaviour *)0x0) {
							this->field_0x928 = 3;
						}
					}
				}
				else {
					if (iVar3 == 0) {
						this->field_0x928 = 1;
						pCVar2 = pActor->GetBehaviour(3);
						if (pCVar2 == (CBehaviour *)0x0) {
							this->field_0x928 = 2;
							pCVar2 = pActor->GetBehaviour(4);
							if (pCVar2 == (CBehaviour *)0x0) {
								this->field_0x928 = 3;
							}
						}
					}
				}
			}
		}
	}
	return;
}


void CActorMiniGamesOrganizer::PrevPlayMode()
{
	int iVar1;
	CActor *pActor;
	CBehaviour *pCVar2;

	iVar1 = this->field_0x928;
	pActor = GetMiniGame(this->field_0x920);
	if (iVar1 == 3) {
		this->field_0x928 = 2;
		pCVar2 = pActor->GetBehaviour(4);
		if (pCVar2 == (CBehaviour *)0x0) {
			this->field_0x928 = 1;
			pCVar2 = pActor->GetBehaviour(3);
			if (pCVar2 == (CBehaviour *)0x0) {
				this->field_0x928 = 3;
			}
		}
	}
	else {
		if (iVar1 == 2) {
			this->field_0x928 = 1;
			pCVar2 = pActor->GetBehaviour(3);
			if (pCVar2 == (CBehaviour *)0x0) {
				this->field_0x928 = 3;
				pCVar2 = pActor->GetBehaviour(2);
				if (pCVar2 == (CBehaviour *)0x0) {
					this->field_0x928 = 2;
				}
			}
		}
		else {
			if ((iVar1 == 1) || (iVar1 == 0)) {
				this->field_0x928 = 3;
				pCVar2 = pActor->GetBehaviour(2);
				if (pCVar2 == (CBehaviour *)0x0) {
					this->field_0x928 = 2;
					pCVar2 = pActor->GetBehaviour(4);
					if (pCVar2 == (CBehaviour *)0x0) {
						this->field_0x928 = 1;
					}
				}
			}
		}
	}
	return;
}


void CActorMiniGamesOrganizer::NextPlayMode()
{
	int iVar1;
	CActor *pActor;
	CBehaviour *pCVar2;

	iVar1 = this->field_0x928;
	pActor = GetMiniGame(this->field_0x920);
	if (iVar1 == 3) {
		this->field_0x928 = 1;
		pCVar2 = pActor->GetBehaviour(3);
		if (pCVar2 == (CBehaviour *)0x0) {
			this->field_0x928 = 2;
			pCVar2 = pActor->GetBehaviour(4);
			if (pCVar2 == (CBehaviour *)0x0) {
				this->field_0x928 = 3;
			}
		}
	}
	else {
		if (iVar1 == 2) {
			this->field_0x928 = 3;
			pCVar2 = pActor->GetBehaviour(2);
			if (pCVar2 == (CBehaviour *)0x0) {
				this->field_0x928 = 1;
				pCVar2 = pActor->GetBehaviour(3);
				if (pCVar2 == (CBehaviour *)0x0) {
					this->field_0x928 = 2;
				}
			}
		}
		else {
			if ((iVar1 == 1) || (iVar1 == 0)) {
				this->field_0x928 = 2;
				pCVar2 = pActor->GetBehaviour(4);
				if (pCVar2 == (CBehaviour *)0x0) {
					this->field_0x928 = 3;
					pCVar2 = pActor->GetBehaviour(2);
					if (pCVar2 == (CBehaviour *)0x0) {
						this->field_0x928 = 1;
					}
				}
			}
		}
	}
	return;
}



int CActorMiniGamesOrganizer::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x7c) {
		_msg_cinematic_install_param* pParam = (_msg_cinematic_install_param*)pMsgParam;
		CCinematic* pCVar6 = g_CinematicManager_0048efc->GetCinematic(this->field_0x16c);
		if ((this->field_0x9b8 != 0) && (pCVar6 == pParam->pCinematic)) {
			if (pParam->action == 1) {
				if (this->field_0x9bc == 0) return 0;
				for (int i = 0; i < this->field_0x9bc; i++) edDListTermMaterial(this->field_0x9b8 + i);
				this->field_0x9bc = 0;
				ed3DUnInstallG2D(&this->field_0x9c0);
				return 1;
			}
			if (pParam->action == 0) {
				edBANK_ENTRY_INFO eStack32;
				int iStack4;
				if (!pCVar6->LoadEntryByFile(&eStack32, "G2D", 0)) return 0;
				ed3DInstallG2D((char*)eStack32.fileBufferStart, eStack32.size, &iStack4, &this->field_0x9c0, 1);
				this->field_0x9bc = ed3DG2DGetG2DNbMaterials(&this->field_0x9c0);
				int iVar8 = this->field_0x17c == 0 ? 0 : this->field_0x17c->entryCount;
				if (iVar8 < this->field_0x9bc) this->field_0x9bc = iVar8;
				for (int i = 0; i < this->field_0x9bc; i++)
					edDListCreatMaterialFromIndex(this->field_0x9b8 + i, i, &this->field_0x9c0, 2);
				return 1;
			}
		}
		return 0;
	}
	if (msg == 0x24) {
		SetState(5, -1);
		return 0;
	}
	if (msg == 0x55) {
		if (pMsgParam == 0) SetState(0xb, -1);
		if (pMsgParam != (void*)1) return 0;
		CActorMiniGame* pMiniGame = static_cast<CActorMiniGame*>(pSender);
		if ((pMiniGame->curBehaviourId == 3) && (pMiniGame->field_0x1b0 == 0)) {
			CBehaviourMiniGameMulti* pBehaviour = pMiniGame->GetMultiBehaviour();
			if (this->field_0x924 < pBehaviour->nbBets)
				CScene::ptable.g_LevelScheduleManager_00451660->Money_TakeFromBet(pBehaviour->aBets[pBehaviour->curBet].reward);
		}
		SetState(0xd, -1);
		return 0;
	}
	if (msg == 0x14) {
		if (this->actorState != 5) return 0;
		this->field_0x9f0 = pSender;
		SetState(6, -1);
		return 1;
	}
	if (msg == 0x12) {
		float fVar1 = pSender->currentLocation.x - this->currentLocation.x;
		float fVar2 = pSender->currentLocation.z - this->currentLocation.z;
		if ((sqrtf(fVar1 * fVar1 + fVar2 * fVar2) < this->field_0x194) &&
			((GetStateFlags(this->actorState) & 0x100) != 0) &&
			(this->field_0x17c != 0) && (0 < this->field_0x17c->entryCount)) return 0xe;
		return 0;
	}
	return CActor::InterpretMessage(pSender, msg, pMsgParam);
}


static uint MiniGamePlayerColour(int player)
{
	switch (player) {
	case 0: return 0xffff00ff;
	case 1: return 0xff0000ff;
	case 2: return 0xff00ffff;
	case 3: return 0x00ff00ff;
	case 4: return 0x00ffffff;
	case 5: return 0xff8000ff;
	}
	return 0;
}

static void FormatMiniGameScore(float score, CActorMiniGame* pMiniGame, edCTextFormat* pText, char* pName)
{
	int iVar2 = pMiniGame->GetScoreType();
	if (iVar2 == 3) {
		if (score == -1.0f) pText->FormatString("%s%s--", pName, "     ");
		else pText->FormatString("%s%s%d ", pName, "     ", (int)score);
	}
	else if (iVar2 == 2) {
		const char* pUnits = gVideoConfig.omode == 2 ? "Y." : "m";
		int precision = 2;
		if ((999.99f <= score) && (score <= 9999.99f)) precision = 1;
		if ((9999.99f <= score) && (score <= 99999.99f)) precision = 0;
		if (score == -1.0f) pText->FormatString("%s%s-- %s", pName, "   ", pUnits);
		else {
			char local_80[128];
			snprintf(local_80, sizeof(local_80), "%.*f", precision, score);
			pText->FormatString("%s%s%s  %s", pName, "   ", local_80, pUnits);
		}
	}
	else if (iVar2 == 1) {
		if (score == -1.0f) pText->FormatString("%s%s--'--\"--", pName, "     ");
		else {
			int iVar5 = (int)score / 100;
			int iVar4 = iVar5 / 60;
			int iVar3 = iVar5 - iVar4 * 60;
			int iVar1 = (int)score - iVar5 * 100;
			pText->FormatString("%s%s%d%d'%d%d\"%d%d", pName, "     ",
				iVar4 / 10, iVar4 % 10, iVar3 / 10, iVar3 % 10, iVar1 / 10, iVar1 % 10);
		}
	}
	return;
}

static void FormatMiniGameResultScore(float score, CActorMiniGame* pMiniGame, edCTextFormat* pText)
{
	CActorMiniGamesOrganizer* pOrganizer = pMiniGame->field_0x1c0;
	char* pcVar2;
	if (pOrganizer->field_0x9b4 == 2) pcVar2 = "%s%s%[RED]k%s%[YELLOW]k     ";
	else if (pOrganizer->field_0x9b4 == 1) pcVar2 = "%s%[RED]k%s%[YELLOW]k%s     ";
	else if (pOrganizer->field_0x9b4 == 0) pcVar2 = "%[RED]k%s%[YELLOW]k%s%s     ";
	else pcVar2 = "%s%s%s     ";
	char acStack128[128];
	strcpy(acStack128, pcVar2);
	int scoreType = pMiniGame->GetScoreType();
	if (scoreType == 3) {
		if (score == -1.0f) {
			strcat(acStack128, " --");
			pText->FormatString(acStack128, pOrganizer->field_0x9ac[0], pOrganizer->field_0x9ac[1], pOrganizer->field_0x9ac[2]);
		}
		else {
			strcat(acStack128, " %d");
			pText->FormatString(acStack128, pOrganizer->field_0x9ac[0], pOrganizer->field_0x9ac[1], pOrganizer->field_0x9ac[2], (int)score);
		}
	}
	else if (scoreType == 2) {
		const char* pUnits = gVideoConfig.omode == 2 ? "Y." : "m";
		int precision = 2;
		if ((999.99f <= score) && (score <= 9999.99f)) precision = 1;
		if ((9999.99f <= score) && (score <= 99999.99f)) precision = 0;
		if (score == -1.0f) {
			strcat(acStack128, " --  %s");
			pText->FormatString(acStack128, pOrganizer->field_0x9ac[0], pOrganizer->field_0x9ac[1], pOrganizer->field_0x9ac[2], pUnits);
		}
		else {
			char local_80[128];
			snprintf(local_80, sizeof(local_80), "%.*f", precision, score);
			strcat(acStack128, " %s  %s");
			pText->FormatString(acStack128, pOrganizer->field_0x9ac[0], pOrganizer->field_0x9ac[1], pOrganizer->field_0x9ac[2], local_80, pUnits);
		}
	}
	else if (scoreType == 1) {
		if (score == -1.0f) {
			strcat(acStack128, " --'--\"--");
			pText->FormatString(acStack128, pOrganizer->field_0x9ac[0], pOrganizer->field_0x9ac[1], pOrganizer->field_0x9ac[2]);
		}
		else {
			int iVar10 = (int)score / 100;
			int iVar9 = iVar10 / 60;
			int iVar8 = iVar10 - iVar9 * 60;
			int iVar7 = (int)score - iVar10 * 100;
			strcat(acStack128, "%d%d'%d%d\"%d%d");
			pText->FormatString(acStack128, pOrganizer->field_0x9ac[0], pOrganizer->field_0x9ac[1], pOrganizer->field_0x9ac[2],
				iVar9 / 10, iVar9 % 10, iVar8 / 10, iVar8 % 10, iVar7 / 10, iVar7 % 10);
		}
	}
	return;
}

static void DrawMiniGameHighScores(float x, float y, float width, float lineScale, CActorMiniGame* pMiniGame, CBehaviourMiniGameTrain* pBehaviour)
{
	for (int iVar3 = 0; iVar3 < pBehaviour->nbScores; iVar3++) {
		edCTextFormat auStack10784;
		FormatMiniGameScore(pBehaviour->aScores[iVar3].score, pMiniGame, &auStack10784, pBehaviour->aScores[iVar3].name);
		auStack10784.Display(x, y);
		y = y + lineScale * auStack10784.field_0xc;
	}
	return;
}

static void DrawMiniGameWheel(int curIndex, int nextIndex, S_MENU_WHEEL_DRAW* pDraw, void** pContext)
{
	CActorMiniGamesOrganizer* pOrganizer = static_cast<CActorMiniGamesOrganizer*>(*pContext);
	edCTextStyle eStack192;
	eStack192.alpha = pDraw->alpha;
	uint uVar4 = pOrganizer->field_0x94c;
	if ((uint)eStack192.alpha < uVar4) uVar4 = eStack192.alpha;
	eStack192.rgbaColour = uVar4 | 0xffff0000;
	eStack192.SetScale(1.5f, 1.5f);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.SetShadow(0x100);
	eStack192.SetFont(BootDataFont, false);
	edCTextStyle* peVar6 = edTextStyleSetCurrent(&eStack192);
	if (*pContext != 0) {
		edCTextFormat auStack5584;
		CActorMiniGame* pCur = pOrganizer->GetMiniGame(curIndex);
		CActorMiniGame* pNext = pOrganizer->GetMiniGame(nextIndex);
		if (pCur != 0) {
			auStack5584.FormatString(gMessageManager.get_message(pCur->field_0x160));
			auStack5584.Display(pDraw->x, pDraw->y);
			if (curIndex < pOrganizer->field_0x9bc) {
				CSprite local_1690;
				local_1690.Install(pOrganizer->field_0x9b8 + curIndex);
				local_1690.color = ((uint)(pDraw->alpha / 2) << 24) | 0x808080;
				local_1690.iWidth = (ushort)(int)((float)gVideoConfig.screenWidth * 0.33f);
				local_1690.iHeight = (ushort)(int)((float)gVideoConfig.screenHeight * 0.33f);
				local_1690.Draw(1.0f, (float)gVideoConfig.screenWidth * 0.28f, (float)gVideoConfig.screenHeight * 0.59f, 0x12);
			}
		}
		if (pNext != 0) {
			eStack192.alpha = pOrganizer->field_0x94c;
			byte alpha = (byte)(int)((255.0f - (float)pDraw->alpha) / 2.0f);
			uVar4 = eStack192.alpha;
			if (alpha < uVar4) uVar4 = alpha;
			eStack192.rgbaColour = (uVar4 & 0xff) | 0xffff0000;
			if (alpha < eStack192.alpha) eStack192.alpha = alpha;
			auStack5584.FormatString(gMessageManager.get_message(pNext->field_0x160));
			auStack5584.Display(pDraw->x, pDraw->y);
			if (nextIndex < pOrganizer->field_0x9bc) {
				CSprite local_1750;
				local_1750.Install(pOrganizer->field_0x9b8 + nextIndex);
				local_1750.color = ((uint)alpha << 24) | 0x808080;
				local_1750.iWidth = (ushort)(int)((float)gVideoConfig.screenWidth * 0.33f);
				local_1750.iHeight = (ushort)(int)((float)gVideoConfig.screenHeight * 0.33f);
				local_1750.Draw(1.0f, (float)gVideoConfig.screenWidth * 0.28f, (float)gVideoConfig.screenHeight * 0.59f, 0x12);
			}
		}
	}
	edTextStyleSetCurrent(peVar6);
	return;
}

void CActorMiniGamesOrganizer::DrawMenuChooseText()
{
	CActor *pActor;
	bool bVar1;
	CBehaviour *pCVar2;
	edCTextStyle *pNewFont;
	char *pcVar3;
	char cVar4;
	float fVar5;
	float fVar6;
	float angle;
	float y;
	edCTextFormat auStack21760;
	edCTextFormat auStack16368;
	edCTextFormat auStack10976;
	edCTextFormat auStack5584;
	edCTextStyle eStack192;

	eStack192.Reset();
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetEolAutomatic(0x80);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
	eStack192.alpha = this->field_0x94c;
	eStack192.altColour = this->field_0x94c | 0xffffff00;
	eStack192.SetShadow(0x100);
	pActor = GetMiniGame(this->field_0x920);
	y = (float)gVideoConfig.screenHeight * 0.48f;
	fVar6 = (float)gVideoConfig.screenHeight * 0.12f;
	pCVar2 = pActor->GetBehaviour(3);
	cVar4 = pCVar2 != (CBehaviour *)0x0;
	pCVar2 = pActor->GetBehaviour(4);
	if (pCVar2 != (CBehaviour *)0x0) {
		cVar4 = cVar4 + '\x01';
	}
	pCVar2 = pActor->GetBehaviour(2);
	if (pCVar2 != (CBehaviour *)0x0) {
		cVar4 = cVar4 + '\x01';
	}
	bVar1 = GuiDList_BeginCurrent();
	if (bVar1 != false) {
		pNewFont = edTextStyleSetCurrent(&eStack192);
		this->menuWheel.Draw();
		fVar5 = (float)(uint)this->field_0x94c / 2.0f;
		if (fVar5 < 2.147484e+09f) {
			this->field_0x76c.field_0x1a4 = (char)(int)fVar5;
		}
		else {
			this->field_0x76c.field_0x1a4 = (char)(int)(fVar5 - 2.147484e+09f);
		}
		this->field_0x76c.FUN_002ef4e0((float)gVideoConfig.screenWidth * 0.71f, (float)gVideoConfig.screenHeight * 0.39f, (float)gVideoConfig.screenWidth * 0.71f, (float)gVideoConfig.screenHeight * 0.8f, 0);
		fVar5 = 0.0f;
		if (cVar4 == '\x01') {
			y = y + fVar6;
		}
		else {
			if (cVar4 == '\x02') {
				fVar5 = 0.08f;
				y = y + fVar6 / 2.0f;
			}
			else {
				if (cVar4 == '\x03') {
					fVar5 = 0.08f;
					y = (float)gVideoConfig.screenHeight * 0.48f;
				}
			}
		}
		angle = -fVar5;
		pCVar2 = pActor->GetBehaviour(3);
		if (pCVar2 != (CBehaviour *)0x0) {
			if (this->field_0x928 == 1) {
				eStack192.SetScale(1.25f, 1.25f);
				eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
			}
			else {
				eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
				eStack192.SetScale(1.0f, 1.0f);
			}
			eStack192.SetRotation(angle);
			pcVar3 = gMessageManager.get_message(0x1e160c0c13414d45);

			auStack5584.FormatString(pcVar3);
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.71f, y);
			y = y + fVar6;
			angle = angle + fVar5;

		}
		pCVar2 = pActor->GetBehaviour(4);
		if (pCVar2 != (CBehaviour *)0x0) {
			if (this->field_0x928 == 2) {
				eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
				eStack192.SetScale(1.25f, 1.25f);
			}
			else {
				eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
				eStack192.SetScale(1.0f, 1.0f);
			}
			eStack192.SetRotation(angle);
			pcVar3 = gMessageManager.get_message(0x52575a5959150415);
			edTextDraw((float)gVideoConfig.screenWidth * 0.71f,y,pcVar3);
			y = y + fVar6;
			angle = angle + fVar5;
		}
		pCVar2 = pActor->GetBehaviour(2);
		if (pCVar2 != (CBehaviour *)0x0) {
			if (this->field_0x928 == 3) {
				eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
				eStack192.SetScale(1.25f, 1.25f);
			}
			else {
				eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
				eStack192.SetScale(1.0f, 1.0f);
			}
			eStack192.SetRotation(angle);
			pcVar3 = gMessageManager.get_message(0x50511a1b0608030c);

			auStack10976.FormatString(pcVar3);
			auStack10976.Display((float)gVideoConfig.screenWidth * 0.71f, y)
			;

		}


		eStack192.SetRotation(0);
		eStack192.SetScale(0.8f, 0.8f);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		eStack192.SetHorizontalAlignment(2);
		pcVar3 = gMessageManager.get_message(0x5c434c5c4446091a);
		auStack16368.FormatString(pcVar3);
		auStack16368.Display((float)gVideoConfig.screenWidth * 0.13f, (float)gVideoConfig.screenHeight * 0.92f);
		pcVar3 = gMessageManager.get_message(0x4b425f5e40151207);
		auStack21760.FormatString(pcVar3);
		auStack21760.Display((float)gVideoConfig.screenWidth * 0.88f, (float)gVideoConfig.screenHeight * 0.92f);
		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();


	}
	return;
}


void CActorMiniGamesOrganizer::DrawMenuTrainText()
{
	CActorMiniGame* piVar1;
	bool bVar2;
	edCTextStyle *pNewFont;
	char *pcVar3;
	int iVar4;
	edCTextFormat auStack5584;
	edCTextStyle eStack192;

	eStack192.Reset();
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetEolAutomatic(0x80);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
	eStack192.alpha = this->field_0x94c;
	eStack192.altColour = this->field_0x94c | 0xffffff00;
	eStack192.SetShadow(0x100);
	piVar1 = GetMiniGame(this->field_0x920);
	bVar2 = GuiDList_BeginCurrent();
	if (bVar2 != false) {
		pNewFont = edTextStyleSetCurrent(&eStack192);

		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.8f, 1.8f);
		pcVar3 = gMessageManager.get_message(0x50511a1b0608030c);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.5f, (float)gVideoConfig.screenHeight * 0.13f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.24f);
		eStack192.SetRotation(0.17f);
		eStack192.SetScale(1.0f, 1.0f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		pcVar3 = gMessageManager.get_message(0x5d59544553091216);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.23f, (float)gVideoConfig.screenHeight * 0.42f);
		eStack192.SetRotation(0);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetEolAutomatic(0);
		eStack192.SetScale(1.0f, 1.0f);
		DrawMiniGameHighScores((float)gVideoConfig.screenWidth * 0.62f, (float)gVideoConfig.screenHeight * 0.3f,
			(float)gVideoConfig.screenWidth * 0.25f, 0.9714286f, piVar1, piVar1->GetTrainBehaviour());
		eStack192.SetEolAutomatic(0x80);
		eStack192.SetHorizontalJustification(0x10);
		eStack192.SetVerticalAlignment(8);
		eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
		eStack192.SetScale(0.95f, 0.95f);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		pcVar3 = gMessageManager.get_message(piVar1->field_0x1a8);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.49f, (float)gVideoConfig.screenHeight * 0.74f);
		eStack192.SetHorizontalJustification(0);
		eStack192.SetVerticalAlignment(8);
		eStack192.SetScale(0.8f, 0.8f);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		pcVar3 = gMessageManager.get_message(0x5c434c5c4446091a);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.13f, (float)gVideoConfig.screenHeight * 0.92f);
		pcVar3 = gMessageManager.get_message(0x4b4258474a0a1207);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.88f, (float)gVideoConfig.screenHeight * 0.92f);
		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();

	}
	return;
}


void CActorMiniGamesOrganizer::DrawMenuBetText()
{
	CActorMiniGame* piVar1;
	int iVar2;
	bool bVar3;
	edCTextStyle *pNewFont;
	CBehaviourMiniGameMulti* iVar4;
	char *pcVar5;
	uint uVar6;
	S_MINI_GAME_BET* piVar7;
	float fVar8;
	float fVar10;
	edCTextFormat auStack5584;
	edCTextStyle eStack192;

	eStack192.Reset();
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetEolAutomatic(0x80);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
	eStack192.alpha = this->field_0x94c;
	eStack192.altColour = this->field_0x94c | 0xffffff00;
	eStack192.SetShadow(0x100);
	piVar1 = GetMiniGame(this->field_0x920);
	bVar3 = GuiDList_BeginCurrent();
	if (bVar3 != false) {
		pNewFont = edTextStyleSetCurrent(&eStack192);

		fVar8 = (float)(uint)this->field_0x94c / 2.0f;
		if (fVar8 < 2.147484e+09f) {
			this->field_0x76c.field_0x1a4 = (char)(int)fVar8;
		}
		else {
			this->field_0x76c.field_0x1a4 = (char)(int)(fVar8 - 2.147484e+09f);
		}
		fVar8 = (float)gVideoConfig.screenWidth * 0.36f;
		fVar10 = (float)gVideoConfig.screenHeight * 0.245f;
		this->field_0x76c.FUN_002ef4e0(fVar8, (float)gVideoConfig.screenHeight * 0.13f, fVar8, fVar10, 0);
		uVar6 = this->field_0x94c | 0x3b3b0000;
		iVar4 = piVar1->GetMultiBehaviour();
		iVar2 = this->field_0x924;
		if (iVar2 < iVar4->nbBets) {
			iVar4 = piVar1->GetMultiBehaviour();
			piVar7 = iVar4->aBets + iVar2;
			eStack192.rgbaColour = uVar6;
			eStack192.altColour = uVar6;
			if (piVar7->cost <= CLevelScheduler::_gGameNfo.nbMoney) {
				eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
				eStack192.altColour = this->field_0x94c | 0xffff0000;
			}
			eStack192.SetScale(1.5f, 1.5f);
			eStack192.SetRotation(0.14f);
			gMessageManager.get_message(0x1e16030609041445);
			auStack5584.FormatString("%d", piVar7->cost);
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.33f, (float)gVideoConfig.screenHeight * 0.18f);
			eStack192.SetScale(0.9f, 0.9f);
			auStack5584.FormatString("%[MONEY]b");
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.385f, (float)gVideoConfig.screenHeight * 0.2f);
			eStack192.SetScale(1.5f, 1.5f);
			eStack192.SetRotation(-0.12f);
			auStack5584.FormatString("%d", piVar7->reward);
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.845f, (float)gVideoConfig.screenHeight * 0.185f);
			eStack192.SetScale(0.9f, 0.9f);
			auStack5584.FormatString("%[MONEY]b");
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.91f, (float)gVideoConfig.screenHeight * 0.185f);
		}
		eStack192.SetScale(1.13f, 1.13f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.altColour = this->field_0x94c | 0xffffff00;
		eStack192.SetRotation(0.14f);
		pcVar5 = gMessageManager.get_message(0x1e160c0c13414d45);
		auStack5584.FormatString(pcVar5);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.17f, (float)gVideoConfig.screenHeight * 0.16f);
		eStack192.SetScale(1.13f, 1.13f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetRotation(-0.12f);
		pcVar5 = gMessageManager.get_message(0x1e16190009414d45);
		auStack5584.FormatString(pcVar5);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.66f, (float)gVideoConfig.screenHeight * 0.21f);
		eStack192.SetRotation(0);
		eStack192.SetScale(1.5f, 1.5f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetRotation(0);
		pcVar5 = gMessageManager.get_message(0x1e161c0c040e1f01);
		auStack5584.FormatString(pcVar5);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.47f, (float)gVideoConfig.screenHeight * 0.42f);
		if (0 < piVar1->field_0x17c) {
			eStack192.SetScale(1.5f, 1.5f);
			eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
			eStack192.SetRotation(0);
			FormatMiniGameScore(piVar1->field_0x184, piVar1, &auStack5584, "");
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.45f, (float)gVideoConfig.screenHeight * 0.51f);
		}
		eStack192.SetRotation(0);
		eStack192.SetHorizontalJustification(0x10);
		eStack192.SetVerticalAlignment(8);
		eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
		eStack192.SetScale(0.95f, 0.95f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		pcVar5 = gMessageManager.get_message(piVar1->field_0x1a8);
		auStack5584.FormatString(pcVar5);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.49f, (float)gVideoConfig.screenHeight * 0.74f);
		eStack192.SetHorizontalJustification(0);
		eStack192.SetVerticalAlignment(8);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		eStack192.SetScale(0.8f, 0.8f);
		pcVar5 = gMessageManager.get_message(0x5c434c5c4446091a);
		auStack5584.FormatString(pcVar5);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.13f, (float)gVideoConfig.screenHeight * 0.92f);
		pcVar5 = gMessageManager.get_message(0x4b4258474a0a1207);
		auStack5584.FormatString(pcVar5);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.88f, (float)gVideoConfig.screenHeight * 0.92f);
		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();

	}
	return;
}


void CActorMiniGamesOrganizer::DrawMenuMultiText()
{
	CActorMiniGame* piVar1;
	bool bVar2;
	edCTextStyle *pNewFont;
	char *pcVar3;
	CBehaviourMiniGameSolo* iVar4;
	ulong uVar6;
	uint uVar7;
	float fVar8;
	float fVar10;
	edCTextFormat auStack5584;
	edCTextStyle eStack192;

	eStack192.Reset();
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetEolAutomatic(0x80);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
	eStack192.alpha = this->field_0x94c;
	eStack192.altColour = this->field_0x94c | 0xffffff00;
	eStack192.SetShadow(0x100);
	piVar1 = GetMiniGame(this->field_0x920);
	bVar2 = GuiDList_BeginCurrent();
	if (bVar2 != false) {
		fVar8 = (float)(uint)this->field_0x94c / 2.0f;
		if (fVar8 < 2.147484e+09f) {
			(this->field_0x76c).field_0x1a4 = (byte)(int)fVar8;
		}
		else {
			(this->field_0x76c).field_0x1a4 = (byte)(int)(fVar8 - 2.147484e+09f);
		}
		fVar8 = (float)gVideoConfig.screenWidth * 0.77f;
		fVar10 = (float)gVideoConfig.screenHeight * 0.19f;
		this->field_0x76c.FUN_002ef4e0(fVar8, (float)gVideoConfig.screenHeight * 0.08f, fVar8, fVar10, 0);
		pNewFont = edTextStyleSetCurrent(&eStack192);

		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.13f, 1.13f);
		pcVar3 = gMessageManager.get_message(0x4753525818110104);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.47f, (float)gVideoConfig.screenHeight * 0.14f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.13f, 1.13f);
		iVar4 = piVar1->GetSoloBehaviour();
		auStack5584.FormatString("%d", iVar4->nbPlayers);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.77f, (float)gVideoConfig.screenHeight * 0.14f);
		eStack192.SetScale(1.2f, 1.2f);
		eStack192.SetRotation(-0.11f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		pcVar3 = gMessageManager.get_message(0x1e161c0c040e1f01);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.81f, (float)gVideoConfig.screenHeight * 0.35f);
		iVar4 = piVar1->GetSoloBehaviour();
		if (0 < iVar4->nbScores) {
			eStack192.SetScale(1.2f, 1.2f);
			eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
			eStack192.SetRotation(-0.11f);
			iVar4 = piVar1->GetSoloBehaviour();
			FormatMiniGameScore(iVar4->aScores[0].score, piVar1, &auStack5584, "");
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.785f, (float)gVideoConfig.screenHeight * 0.43f);
		}
		eStack192.SetHorizontalJustification(0x10);
		eStack192.SetVerticalAlignment(8);
		eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
		eStack192.SetScale(0.95f, 0.95f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetRotation(0);
		pcVar3 = gMessageManager.get_message(piVar1->field_0x1a8);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.49f, (float)gVideoConfig.screenHeight * 0.74f);
		eStack192.SetHorizontalJustification(0);
		eStack192.SetVerticalAlignment(8);
		fVar8 = (float)gVideoConfig.screenWidth * 0.185f;
		fVar10 = (float)gVideoConfig.screenHeight * 0.34f;
		pcVar3 = gMessageManager.get_message(0x1e161e0506180817);
		uVar6 = 0;
		do {
			uVar7 = MiniGamePlayerColour((int)uVar6);
			iVar4 = piVar1->GetSoloBehaviour();
			if ((long)uVar6 < (long)iVar4->nbPlayers) {
				eStack192.SetScale(0.98f, 0.98f);
			}
			else {
				uVar7 = uVar7 & 0xff |
								((uVar7 & 0xff00) >> 8) * 0x28 & 0xffffff00 |
								((uVar7 >> 0x18) * 0x28 >> 8) << 0x18 |
								(((uVar7 & 0xff0000) >> 0x10) * 0x28 >> 8) << 0x10;
				eStack192.SetScale(0.78f, 0.78f);
			}
			eStack192.rgbaColour = uVar7;
			auStack5584.FormatString(pcVar3, (int)uVar6 + 1);
			auStack5584.Display(fVar8, fVar10);
			if ((uVar6 & 1) == 0) {
				fVar8 = fVar8 + (float)gVideoConfig.screenWidth * 0.28f;
			}
			else {
				fVar8 = (float)gVideoConfig.screenWidth * 0.185f;
				fVar10 = fVar10 + (float)gVideoConfig.screenHeight * 0.095f;
			}
			uVar6 = (int)uVar6 + 1;
		} while ((long)uVar6 < 6);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		eStack192.SetHorizontalAlignment(2);
		eStack192.SetScale(0.8f, 0.8f);
		pcVar3 = gMessageManager.get_message(0x5c434c5c4446091a);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.13f, (float)gVideoConfig.screenHeight * 0.92f);
		pcVar3 = gMessageManager.get_message(0x4b4258474a0a1207);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.88f, (float)gVideoConfig.screenHeight * 0.92f);
		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();

	}
	return;
}


void CActorMiniGamesOrganizer::DrawMenuResultText()
{
	CActorMiniGame* piVar1;
	bool bVar2;
	edCTextStyle *pNewFont;
	char *pcVar3;
	int iVar4;
	uint uVar5;
	float fVar12;
	float in_f21;
	float y;
	float fVar13;
	float x;
	edCTextFormat auStack5584;
	edCTextStyle eStack192;

	eStack192.Reset();
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
	eStack192.alpha = this->field_0x94c;
	eStack192.altColour = this->field_0x94c | 0xffffff00;
	eStack192.SetShadow(0x100);
	piVar1 = GetMiniGame(this->field_0x920);
	x = (float)gVideoConfig.screenWidth * 0.74f;
	y = (float)gVideoConfig.screenHeight * 0.53f;
	fVar13 = (float)gVideoConfig.screenHeight * 0.13f;
	bVar2 = GuiDList_BeginCurrent();
	if (bVar2 != false) {
		pNewFont = edTextStyleSetCurrent(&eStack192);

		fVar12 = (float)(uint)this->field_0x94c / 2.0f;
		if (fVar12 < 2.147484e+09f) {
			this->field_0x76c.field_0x1a4 = (char)(int)fVar12;
		}
		else {
			this->field_0x76c.field_0x1a4 = (char)(int)(fVar12 - 2.147484e+09f);
		}
		this->field_0x76c.FUN_002ef4e0((float)gVideoConfig.screenWidth * 0.73f, (float)gVideoConfig.screenHeight * 0.43f, (float)gVideoConfig.screenWidth * 0.73f, (float)gVideoConfig.screenHeight * 0.88f, 0);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.5f, 1.5f);
		pcVar3 = gMessageManager.get_message(piVar1->field_0x160);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.5f, (float)gVideoConfig.screenHeight * 0.13f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.1f, 1.1f);
		eStack192.SetRotation(0.00f);
		if (piVar1->field_0x1b0 == 0) {
			pcVar3 = gMessageManager.get_message(0x1e16190009414d45);
		}
		else {
			if ((piVar1->curBehaviourId == 3) || (piVar1->field_0x1b0 != 1)) {
				pcVar3 = gMessageManager.get_message(0x1e16020608120845);
			}
			else {
				pcVar3 = gMessageManager.get_message(0x5d59544553091216);
			}
		}
		auStack5584.FormatString(pcVar3);
		if ((piVar1->curBehaviourId == 3) && (piVar1->field_0x1b0 == 0)) {
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.19f, (float)gVideoConfig.screenHeight * 0.34f);
		}
		else {
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.3f, (float)gVideoConfig.screenHeight * 0.34f);
		}
		if ((piVar1->curBehaviourId == 3) && (piVar1->field_0x1b0 == 0)) {
			eStack192.SetScale(1.26f, 1.26f);
			CBehaviourMiniGameMulti* pBehaviour = piVar1->GetMultiBehaviour();
			auStack5584.FormatString("%d", pBehaviour->aBets[pBehaviour->curBet].reward);
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.39f, (float)gVideoConfig.screenHeight * 0.34f);
			eStack192.SetScale(0.9f, 0.9f);
			auStack5584.FormatString("%[MONEY]b");
			auStack5584.Display((float)gVideoConfig.screenWidth * 0.45f, (float)gVideoConfig.screenHeight * 0.35f);
		}
		eStack192.SetScale(1.22f, 1.22f);
		eStack192.SetRotation(0.03f);
		pcVar3 = gMessageManager.get_message(0x5d59454312131216);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.28f, (float)gVideoConfig.screenHeight * 0.45f);
		eStack192.SetRotation(0.06f);
		eStack192.SetScale(1.0f, 1.0f);
		FormatMiniGameResultScore(piVar1->field_0x1d0, piVar1, &auStack5584);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.27f, (float)gVideoConfig.screenHeight * 0.55f);
		iVar4 = 1;
		if ((piVar1->field_0x1b0 == 0) || (piVar1->field_0x1b0 == 1)) {
			iVar4 = 2;
		}
		if (piVar1->curBehaviourId != 3) {
			iVar4 = iVar4 + 1;
		}
		fVar12 = -0.0f;
		if (iVar4 == 1) {
			fVar12 = 0.0f;
			y = y + fVar13;
			in_f21 = 0.0f;
		}
		else {
			if (iVar4 == 2) {
				fVar12 = 0.08f;
				in_f21 = -0.04f;
				y = y + fVar13 / 2.0f;
			}
			else {
				if (iVar4 == 3) {
					fVar12 = 0.08f;
					y = (float)gVideoConfig.screenHeight * 0.53f;
					in_f21 = -0.08f;
				}
			}
		}
		eStack192.SetEolAutomatic(0x80);
		eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.35f);
		if ((piVar1->field_0x1b0 == 0) || (piVar1->field_0x1b0 == 1)) {
			if (piVar1->field_0x1cc == 0) {
				eStack192.SetScale(1.25f, 1.25f);
				eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
			}
			else {
				eStack192.SetScale(1.0f, 1.0f);
				eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
			}
			eStack192.SetRotation(in_f21);
			pcVar3 = gMessageManager.get_message(0x5057464213041f1a);
			auStack5584.FormatString(pcVar3);
			auStack5584.Display(x, y);
			y = y + fVar13;
			in_f21 = in_f21 + fVar12;
		}
		if (piVar1->curBehaviourId != 3) {
			eStack192.SetRotation(in_f21);
			if (piVar1->curBehaviourId == 2) {
				if (piVar1->field_0x1cc == 1) {
					eStack192.SetScale(1.25f, 1.25f);
					eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
				}
				else {
					eStack192.SetScale(1.0f, 1.0f);
					eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
				}
				pcVar3 = gMessageManager.get_message(0x4a161c0c14150c17);
				auStack5584.FormatString(pcVar3);
			}
			else {
				if (piVar1->field_0x1cc == 1) {
					eStack192.SetScale(1.25f, 1.25f);
					uVar5 = MiniGamePlayerColour(piVar1->GetSoloBehaviour()->winner);
				}
				else {
					eStack192.SetScale(1.0f, 1.0f);
					uint colour = MiniGamePlayerColour(piVar1->GetSoloBehaviour()->winner);
					uVar5 = ((((colour & 0xff00) >> 8) * 0x78 >> 8) << 8) |
						(((colour >> 24) * 0x78 >> 8) << 24) |
						((((colour & 0xff0000) >> 16) * 0x78 >> 8) << 16);
				}
				eStack192.rgbaColour = uVar5 & 0xffffff00 | (uint)this->field_0x94c;
				pcVar3 = gMessageManager.get_message(0x1e161e0506180817);
				auStack5584.FormatString(pcVar3, piVar1->GetSoloBehaviour()->winner + 1);
			}
			auStack5584.Display(x, y);
			y = y + fVar13;
			in_f21 = in_f21 + fVar12;
		}
		if (piVar1->field_0x1cc == 2) {
			eStack192.SetScale(1.25f, 1.25f);
			eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
		}
		else {
			eStack192.SetScale(1.0f, 1.0f);
			eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
		}
		eStack192.SetRotation(in_f21);
		pcVar3 = gMessageManager.get_message(0x1e160b110e154d45);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display(x, y);
		eStack192.SetEolAutomatic(0);
		eStack192.SetScale(0.8f, 0.8f);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		eStack192.SetRotation(0.00f);
		pcVar3 = gMessageManager.get_message(0x5c434c5c4446091a);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.13f, (float)gVideoConfig.screenHeight * 0.92f);
		edTextStyleSetCurrent(pNewFont);
		GuiDList_EndCurrent();

	}
	return;
}

void CActorMiniGamesOrganizer::DrawMenuEnterNameText()
{
	bool bVar1;
	edCTextStyle *pNewFont;
	edCTextFormat auStack5584;
	edCTextStyle eStack192;

	DrawMenuResultText();
	eStack192.Reset();
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetEolAutomatic(0x80);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.SetShadow(0x100);
	pNewFont = edTextStyleSetCurrent(&eStack192);

	bVar1 = GuiDList_BeginCurrent();
	if (bVar1 != false) {
		DrawAllLetters();
		GuiDList_EndCurrent();
	}
	edTextStyleSetCurrent(pNewFont);

	return;
}

void CActorMiniGamesOrganizer::DrawLetterCursor(float x, float y, float halfWidth, float halfHeight)
{
	edDList_material* pMaterialInfo = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId_0x178, 0);
	edDListUseMaterial(pMaterialInfo);
	edDListLoadIdentity();
	edDListBegin(0.0f, 0.0f, 0.0f, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST, 4);
	edDListColor4u8(0x80, 0x80, 0x80, 0x80);
	edDListTexCoo2f(0.0f, 0.0f);
	edDListVertex4f(x - halfWidth, y - halfHeight, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 0.0f);
	edDListVertex4f(x + halfWidth, y - halfHeight, 0.0f, 0.0f);
	edDListTexCoo2f(0.0f, 1.0f);
	edDListVertex4f(x - halfWidth, y + halfHeight, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	edDListVertex4f(x + halfWidth, y + halfHeight, 0.0f, 0.0f);
	edDListEnd();

	return;
}

void CActorMiniGamesOrganizer::DrawAllLetters()
{
	edCTextStyle eStack192;
	eStack192.SetFont(BootDataFont, false);
	eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
	eStack192.SetEolAutomatic(0x80);
	eStack192.SetHorizontalAlignment(2);
	eStack192.SetVerticalAlignment(8);
	eStack192.SetShadow(0x100);
	float fVar7 = (float)gVideoConfig.screenWidth * 0.14f;
	float y = (float)gVideoConfig.screenHeight * 0.67f;
	edCTextFormat auStack5584;
	edCTextStyle* pNewFont = edTextStyleSetCurrent(&eStack192);
	int iVar5 = 0;
	int iVar4 = 0;
	float x = fVar7;
	for (int iVar6 = 0; iVar6 < 0x1a; iVar6++) {
		if ((this->field_0x994 == iVar4) && (this->field_0x998 == iVar5))
			eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
		else eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
		auStack5584.FormatString(this->field_0x958[iVar6]);
		auStack5584.Display(x, y);
		if ((this->field_0x994 == iVar4) && (this->field_0x998 == iVar5)) {
			this->field_0x950 = x;
			this->field_0x954 = y;
			DrawLetterCursor(x, y, 5.0f + auStack5584.field_0x8 / 2.0f, auStack5584.field_0xc / 2.0f);
		}
		float fVar1 = 10.0f;
		if (10.0f <= auStack5584.field_0x8) fVar1 = auStack5584.field_0x8;
		x = x + fVar1 + 4.0f;
		iVar4 = iVar4 + 1;
		if ((float)gVideoConfig.screenWidth * 0.275f < x - fVar7) {
			this->field_0x99c[iVar5] = iVar4;
			iVar5++;
			iVar4 = 0;
			y = y + (float)gVideoConfig.screenHeight * 0.06f;
			x = fVar7;
		}
	}
	if ((this->field_0x994 == iVar4) && (this->field_0x998 == iVar5))
		eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
	else eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
	auStack5584.FormatString("<-");
	auStack5584.Display(x, y);
	if ((this->field_0x994 == iVar4) && (this->field_0x998 == iVar5)) {
		this->field_0x950 = x;
		this->field_0x954 = y;
		DrawLetterCursor(x, y, 5.0f + auStack5584.field_0x8 / 2.0f, auStack5584.field_0xc / 2.0f);
	}
	this->field_0x99c[iVar5] = iVar4 + 1;
	if ((this->field_0x994 == 0) && (this->field_0x998 == iVar5 + 1))
		eStack192.rgbaColour = this->field_0x94c | 0xfc990000;
	else eStack192.rgbaColour = this->field_0x94c | 0xffffd200;
	auStack5584.FormatString("OK");
	x = (float)gVideoConfig.screenWidth * 0.27f;
	y = (float)gVideoConfig.screenHeight * 0.85f;
	auStack5584.Display(x, y);
	if ((this->field_0x994 == 0) && (this->field_0x998 == iVar5 + 1)) {
		this->field_0x950 = x;
		this->field_0x954 = y;
		DrawLetterCursor(x, y, 5.0f + auStack5584.field_0x8 / 2.0f, auStack5584.field_0xc / 2.0f);
	}
	this->field_0x99c[iVar5 + 1] = 1;
	edTextStyleSetCurrent(pNewFont);
	return;
}

void CBehaviourMiniGamesOrganizer::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGamesOrganizer::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorMiniGamesOrganizer*>(pOwner);

	return;
}

int CBehaviourMiniGamesOrganizer::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourMiniGamesOrganizer::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourMiniGamesOrganizerStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGamesOrganizerStand::Manage()
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_Manage();

	return;
}

void CBehaviourMiniGamesOrganizerStand::ManageFrozen()
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_Manage();

	return;
}

void CBehaviourMiniGamesOrganizerStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourMiniGamesOrganizer::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMiniGamesOrganizerStand::InitState(int newState)
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_InitState(newState);

	return;
}

void CBehaviourMiniGamesOrganizerStand::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_TermState(oldState);

	return;
}

int CBehaviourMiniGamesOrganizerStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
