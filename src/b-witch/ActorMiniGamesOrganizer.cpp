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
#include "edVideo/VideoD.h"
#include "ed3D/ed3DG3D.h"
#include "kya.h"
#include "WayPoint.h"
#include "Rendering/edCTextFormat.h"

char* CActorMiniGamesOrganizer::FUN_003b2a00()
{
	int iVar1;
	char* pcVar2;

	iVar1 = this->field_0x9b4;
	if (iVar1 == 2) {
		pcVar2 = "%s%s%[RED]k%s%[YELLOW]k     ";
	}
	else {
		if (iVar1 == 1) {
			pcVar2 = "%s%[RED]k%s%[YELLOW]k%s     ";
		}
		else {
			if (iVar1 == 0) {
				pcVar2 = "%[RED]k%s%[YELLOW]k%s%s     ";
			}
			else {
				pcVar2 = "%s%s%s     ";
			}
		}
	}
	return pcVar2;
}

char* CActorMiniGamesOrganizer::GetCurNameLetter(int index)
{
	return this->field_0x9ac[index];
}

void astruct_22::MoveMenuArrow(bool bNext)
{
	float fVar1;

	if (bNext == true) {
		this->field_0x19c = false;
	}
	else {
		this->field_0x19c = true;
	}

	if (this->field_0x198 == 0.0f) {
		this->field_0x198 = this->field_0x180;
	}

	fVar1 = this->field_0x180 / 2.0f;
	if (this->field_0x198 <= fVar1) {
		this->field_0x198 = fVar1;
	}

	return;
}

void astruct_22::Reset()
{
	this->field_0x198 = 0.0f;
}

void StaticMeshComponent::SetScale(float x, float y, float z)
{
	edF32MATRIX4 local_40;
	edF32VECTOR4 local_50;
	if (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0) {
		local_40 = this->pMeshTransformData->base.transformA;
		local_50.x = x;
		local_50.y = y;
		local_50.z = z;
		local_50.w = 0.0f;
		edF32Matrix4ScaleHard(&local_40, &this->perspectiveMatrix, &local_50);
		local_40.rowT = this->pMeshTransformData->base.transformA.rowT;
		this->pMeshTransformData->base.transformA = local_40;
	}

	return;
}


void DrawDisconnectedController(int param_1)
{
	bool bVar1;
	edCTextStyle* pNewFont;
	char* text;
	float fVar2;
	float x;
	edCTextStyle eStack192;

	bVar1 = GuiDList_BeginCurrent();
	if (bVar1 != false) {
		if (param_1 != 0) {
			edDListUseMaterial((edDList_material*)0x0);
			edDListColor4u8(0, 0, 0, 0x80);
			edDListLoadIdentity();
			edDListBegin(1.0f, 1.0f, 1.0f, 6, 2);
			edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
			edDListVertex4f(static_cast<float>(gVideoConfig.screenWidth), static_cast<float>(gVideoConfig.screenHeight), 0.0f, 0.0f);
			edDListEnd();
		}

		x = static_cast<float>(gVideoConfig.screenWidth) / 2.0f;
		fVar2 = static_cast<float>(gVideoConfig.screenHeight);
		eStack192.Reset();
		eStack192.SetShadow(0x100);
		eStack192.rgbaColour = 0xffffffff;
		eStack192.alpha = 0xff;
		eStack192.SetFont(BootDataFont, false);
		eStack192.SetHorizontalAlignment(2);
		eStack192.SetVerticalAlignment(8);
		eStack192.spaceSize = 10.0f;
		pNewFont = edTextStyleSetCurrent(&eStack192);
		text = gMessageManager.get_message(0x52525f503700080c);
		edTextDraw(x, fVar2 / 2.0f, text);
		edTextStyleSetCurrent(pNewFont);
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
	int iVar1;
	float fVar2;

	iVar1 = this->field_0x274;
	if (1 < iVar1) {
		if (!bNext) {
			this->field_0x27c = (this->field_0x27c + -1 + iVar1) % iVar1;
		}
		else {
			this->field_0x27c = (this->field_0x27c + 1) % iVar1;
		}

		if (bNext == true) {
			this->field_0xc0.field_0x19c = false;
		}
		else {
			this->field_0xc0.field_0x19c = true;
		}

		if (this->field_0xc0.field_0x198 == 0.0f) {
			this->field_0xc0.field_0x198 = this->field_0xc0.field_0x180;
		}

		fVar2 = this->field_0xc0.field_0x180 / 2.0f;
		if (this->field_0xc0.field_0x198 <= fVar2) {
			this->field_0xc0.field_0x198 = fVar2;
		}

		this->field_0x280 = 255.0f;
		this->field_0x284 = 0.0f;
		this->field_0x288 = bNext;
	}

	return;
}

bool CMenuWheel::Manage()
{
	Timer *pTVar1;
	bool bVar2;
	float fVar3;
	float fVar4;

	fVar3 = this->field_0x284;
	fVar4 = this->field_0x280;
	pTVar1 = GetTimer();

	if (fabsf(fVar3 - fVar4) < this->field_0x28c * pTVar1->lastFrameTime) {
		this->field_0x284 = 255.0f;
	}

	if (0.0f < this->field_0xc0.field_0x198) {
		pTVar1 = GetTimer();
		fVar3 = this->field_0xc0.field_0x198 - pTVar1->lastFrameTime;
		this->field_0xc0.field_0x198 = fVar3;
		fVar4 = this->field_0xc0.field_0x180;
		if (fVar4 < fVar3) {
			this->field_0xc0.field_0x198 = fVar4;
		}
		else {
			if (fVar3 < 0.0f) {
				this->field_0xc0.field_0x198 = 0.0f;
			}
		}
	}

	if ((this->field_0x278 == 0) || (this->field_0x280 <= this->field_0x284)) {
		bVar2 = false;
		this->field_0x284 = this->field_0x280;
	}
	else {
		bVar2 = true;
		pTVar1 = GetTimer();
		this->field_0x284 = this->field_0x284 + this->field_0x28c * pTVar1->lastFrameTime;
	}

	return bVar2;
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

	return;
}

CActorMiniGame* CActorMiniGamesOrganizer::GetMiniGame(int index)
{
	return static_cast<CActorMiniGame*>(this->pMiniGameStreamRefs->aEntries[index].Get());
}

void CActorMiniGamesOrganizer::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->field_0x168 = pByteCode->GetString();
	this->field_0x16c = pByteCode->GetS32();
	this->textureIndex_0x170 = pByteCode->GetS32();
	this->field_0x174 = pByteCode->GetS32();
	this->materialId_0x178 = pByteCode->GetS32();
	this->pMiniGameStreamRefs = S_ACTOR_STREAM_REF::Create(pByteCode);
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

	this->pMiniGameStreamRefs->Init();

	int iVar1 = 0;
	while (true) {
		int iVar2 = 0;
		if (this->pMiniGameStreamRefs != 0) iVar2 = this->pMiniGameStreamRefs->entryCount;
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
	this->menuWheel.field_0x274 = this->pMiniGameStreamRefs == 0 ? 0 : this->pMiniGameStreamRefs->entryCount;
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

	iVar1 = this->pMiniGameStreamRefs == 0 ? 0 : this->pMiniGameStreamRefs->entryCount;
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

StateConfig CActorMiniGamesOrganizer::_gStateCfg_ORG[12] = {
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
		pStateConfig = _gStateCfg_ORG + state + -5;
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
	this->pMiniGameStreamRefs->Reset();
	this->menuWheel.Reset();
	this->field_0x76c.Reset();
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

	CCinematic* pCinematic = g_CinematicManager_0048efc->GetCinematic(this->field_0x16c);
	if (pCinematic != 0) {
		pCinematic->pActor = this;
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
		DrawDisconnectedController(0);

		bVar2 = Frontend2DDList_BeginCurrent();
		if (bVar2 != false) {
			local_8.x = 0.0f;
			local_8.y = 0.0f;
			ed3DComputeScreenCoordinate(104.0f,&local_150,&local_8,CFrontend::_scene_handle);
			this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
					this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
						this->field_0x230.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x290.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x2f0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x470.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
					this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
						this->field_0x2f0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x350.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x230.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x290.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x470.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
					this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
						this->field_0x230.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x290.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x2f0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x350.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x470.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
					this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
						this->field_0x230.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x290.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x2f0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x350.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x470.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
					this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
						this->field_0x230.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x290.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x2f0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x350.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x470.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
							this->field_0x3b0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
													 1.0f,1.0f);
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
					this->field_0x1d0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f, 1.0f);
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
						this->field_0x230.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x290.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x2f0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x350.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x3b0.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
						this->field_0x470.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f,
												 1.0f,1.0f);
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
							this->field_0x410.SetScale((CScene::ptable.g_CameraManager_0045167c)->aspectRatio / 1.333333f, 1.0f,1.0f);
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

void CActorMiniGamesOrganizer::BehaviourMiniGamesOrganizerStand_Manage()
{
	int iVar1;
	bool bVar2;
	StateConfig *pSVar3;
	CPlayerInput *pCVar4;
	CSoundSample *pCVar5;
	uint uVar6;

	ManageFade();

	ManageMusic(this->actorState);

	bVar2 = gCompatibilityHandlingPtr->HandleDisconnectedDevices(0);
	if ((bVar2 == false) || ((GameFlags & 0x1c) != 0)) {
		if (this->actorState == 0x10) {
			SetState(this->prevActorState, -1);
		}
	}
	else {
		iVar1 = this->actorState;
		if (iVar1 == -1) {
			uVar6 = 0;
		}
		else {
			pSVar3 = GetStateCfg(iVar1);
			uVar6 = pSVar3->flags_0x4;
		}

		if ((uVar6 & 0x200) != 0) {
			SetState(0x10, -1);
		}
	}

	if (this->field_0x940 != false) {
		switch(this->actorState) {
		case 6:
			ManageMenuChoose();
			break;
		case 7:
			ManageMenuBet();
			break;
		case 8:
			pCVar4 = this->field_0x9f0->GetInputManager(0, 0);
			if (pCVar4 != (CPlayerInput *)0x0) {
				if ((pCVar4->pressedBitfield & 0x1000000) != 0) {
					DoMessage(GetMiniGame(this->field_0x920), (ACTOR_MESSAGE)0x56, (void*)3);
					DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
					SetState(10, -1);

					pCVar5 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x188);
					this->field_0xa00.pSound = pCVar5;
					if ((NoAudio == 0) &&
						(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0))
					{
						this->field_0xa00.field_0x20 = 0xffffffff;
						uVar6 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
							(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
						this->field_0xa00.soundId = uVar6;
					}
				}

				if ((pCVar4->pressedBitfield & 0x4000000) != 0) {
					pCVar5 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x18c);
					this->field_0xa00.pSound = pCVar5;
					if ((NoAudio == 0) &&
						(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0))
					{
						this->field_0xa00.field_0x20 = 0xffffffff;
						uVar6 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
							(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
						this->field_0xa00.soundId = uVar6;
					}

					SetState(6, -1);
				}
			}
			break;
		case 9:
			ManageMenuMulti();
			break;
		case 10:
			break;
		case 0xd:
			if (0.5f < this->timeInAir) {
				SetState(0xe, -1);
			}
			break;
		case 0xe:
			ManageMenuResult();
			break;
		case 0xf:
			ManageMenuEnterName();
		}

		ManageZone();
	}

	return;
}


void CActorMiniGamesOrganizer::BehaviourMiniGamesOrganizerStand_InitState(int newState)
{
	int iVar1;
	StateConfig *pSVar3;
	CCameraManager* pCameraManager;
	uint uVar5;
	float fVar6;
	_msg_enter_shop local_20;
	void* local_8;

	InitMenuMeshes(newState);

	if (((((GetStateFlags(this->actorState) & 0x200) != 0) && ((iVar1 = this->prevActorState, iVar1 != 0xe || (this->actorState != 0xf)))) &&
			((iVar1 != 0xf || (this->actorState != 0xe)))) &&
		(this->field_0x941 == false)) {
		Fade(0.1f, 1, 0);
		this->field_0x944 = 1.0f;
	}

	if ((GetStateFlags(this->actorState) & 0x200) != 0) {
		CallPauseChange(1);
		GameFlags = GameFlags | 0x4080;
		CScene::ptable.g_FrontendManager_00451680->SetActive(false);
	}

	switch(newState) {
	case 6:
		ComputeCurPlayMode();

		local_20.field_0x0 = 0;
		local_20.field_0x4 = 0;
		local_20.field_0x8 = 1;
		local_20.field_0xc = 1;
		DoMessage(this->field_0x9f0, MESSAGE_ENTER_SHOP, &local_20);

		this->field_0x76c.Reset();
		fVar6 = (float)gVideoConfig.screenWidth * 0.06f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0x0.iWidth = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0x0.iWidth =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		fVar6 = (float)gVideoConfig.screenHeight * 0.06f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0x0.iHeight = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0x0.iHeight =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		fVar6 = (float)gVideoConfig.screenWidth * 0.06f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0xc0.iWidth = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0xc0.iWidth =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		fVar6 = (float)gVideoConfig.screenHeight * 0.06f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0xc0.iHeight = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0xc0.iHeight =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}
		break;
	case 7:
		InitMenuBet();
		break;
	case 8:
		this->field_0x924 = 3;
		break;
	case 9:
		this->field_0x76c.Reset();
		fVar6 = (float)gVideoConfig.screenWidth * 0.045f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0x0.iWidth = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0x0.iWidth =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		fVar6 = (float)gVideoConfig.screenHeight * 0.045f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0x0.iHeight = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0x0.iHeight =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		fVar6 = (float)gVideoConfig.screenWidth * 0.045f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0xc0.iWidth = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0xc0.iWidth =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		fVar6 = (float)gVideoConfig.screenHeight * 0.045f;
		if (fVar6 < 2.147484e+09f) {
			this->field_0x76c.field_0xc0.iHeight = (ushort)(int)fVar6;
		}
		else {
			this->field_0x76c.field_0xc0.iHeight =
					(ushort)(int)(fVar6 - 2.147484e+09f);
		}

		this->field_0x924 = 2;
		break;
	case 10:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		break;
	case 0xb:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		break;
	case 0xd:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		if (this->field_0x941 == false) {
			Fade(0.5f, 2, 0);
			this->field_0x944 = 1.0f;
		}

		local_8 = 0;
		DoMessage(this->field_0x9f0, MESSAGE_DISABLE_INPUT, local_8);
		pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
		pCameraManager->PushCamera(CActorHero::_gThis->pDeathCamera, 0);
		break;
	case 0xe:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags | 0x80;
		this->flags = this->flags & 0xffffffdf;
		EvaluateDisplayState();
		this->flags = this->flags | 0x400;
		break;
	case 0xf:
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags | 0x80;
		this->flags = this->flags & 0xffffffdf;
		EvaluateDisplayState();
		this->flags = this->flags | 0x400;
		this->field_0x9b4 = 0;
		this->field_0x998 = 0;
		this->field_0x994 = 0;
		break;
	case 0x10:
		this->flags = this->flags | 0x80;
		this->flags = this->flags & 0xffffffdf;
		EvaluateDisplayState();
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags | 0x400;
	}

	return;
}

void CActorMiniGamesOrganizer::BehaviourMiniGamesOrganizerStand_TermState(int oldState)
{
	StateConfig* pSVar1;
	uint uVar2;
	char local_4[4];

	if ((((oldState == 0x10) || (oldState == 6)) || (oldState == 8)) ||
		(((oldState == 9 || (oldState == 7)) || ((oldState == 0xe || (oldState == 0xf)))))) {
		TermMenuMeshes();
	}

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
		this->flags = this->flags & 0xfffffffc;
		this->flags = this->flags & 0xffffff5f;
		EvaluateDisplayState();
		this->flags = this->flags & 0xfffffbff;
		break;
	case 0xf:
		local_4[0] = this->field_0x9ac[0][0];
		local_4[1] = this->field_0x9ac[1][0];
		local_4[2] = this->field_0x9ac[2][0];
		local_4[3] = 0;
		GetMiniGame(this->field_0x920)->UpdateCurHighScoreName(local_4);
		this->flags = this->flags & 0xfffffffc;
		this->flags = this->flags & 0xffffff5f;
		EvaluateDisplayState();
		this->flags = this->flags & 0xfffffbff;
		this->field_0x9b4 = -1;
		break;
	case 0x10:
		this->flags = this->flags & 0xffffff5f;
		EvaluateDisplayState();
		this->flags = this->flags & 0xfffffffc;
		this->flags = this->flags & 0xfffffbff;
	}
	return;
}

void CActorMiniGamesOrganizer::InitMenuMeshes(int state)
{
	CActorMiniGame* pMiniGame;
	bool bVar2;

	if (state != 0x10) {
		if (state != 6) {
			if (state != 9) {
				if (state != 8) {
					if (state != 7) {
						if (state != 0xf) {
							if (state != 0xe) {
								return;
							}
							bVar2 = this->field_0x1d0.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x1d0.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x230.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x230.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = this->field_0x290.textureIndex != -1;
									if (bVar2) {
										bVar2 = this->field_0x290.meshIndex != -1;
									}
									if (bVar2) {
										bVar2 = this->field_0x2f0.textureIndex != -1;
										if (bVar2) {
											bVar2 = this->field_0x2f0.meshIndex != -1;
										}
										if (bVar2) {
											bVar2 = this->field_0x350.textureIndex != -1;
											if (bVar2) {
												bVar2 = this->field_0x350.meshIndex != -1;
											}
											if (bVar2) {
												bVar2 = this->field_0x3b0.textureIndex != -1;
												if (bVar2) {
													bVar2 = this->field_0x3b0.meshIndex != -1;
												}
												if (bVar2) {
													bVar2 = this->field_0x410.textureIndex != -1;
													if (bVar2) {
														bVar2 = this->field_0x410.meshIndex != -1;
													}
													if (bVar2) {
														bVar2 = this->field_0x470.textureIndex != -1;
														if (bVar2) {
															bVar2 = this->field_0x470.meshIndex != -1;
														}
														if (bVar2) {
															bVar2 = true;
															goto LAB_003b1240;
														}
													}
												}
											}
										}
									}
								}
							}
							bVar2 = false;
LAB_003b1240:
							if (!bVar2) {
								return;
							}
							this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
							this->field_0x230.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_03_pan_02");
							this->field_0x290.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_01");
							this->field_0x2f0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_02");
							this->field_0x350.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_03");
							this->field_0x470.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_Symbole_01");
							pMiniGame = GetMiniGame(this->field_0x920);
							if (pMiniGame->curBehaviourId != 3) {
								return;
							}
							if (pMiniGame->field_0x1b0 != 0) {
								return;
							}
							this->field_0x3b0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_02_5");
							return;
						}
						bVar2 = this->field_0x1d0.textureIndex != -1;
						if (bVar2) {
							bVar2 = this->field_0x1d0.meshIndex != -1;
						}
						if (bVar2) {
							bVar2 = this->field_0x230.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x230.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x290.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x290.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = this->field_0x2f0.textureIndex != -1;
									if (bVar2) {
										bVar2 = this->field_0x2f0.meshIndex != -1;
									}
									if (bVar2) {
										bVar2 = this->field_0x350.textureIndex != -1;
										if (bVar2) {
											bVar2 = this->field_0x350.meshIndex != -1;
										}
										if (bVar2) {
											bVar2 = this->field_0x3b0.textureIndex != -1;
											if (bVar2) {
												bVar2 = this->field_0x3b0.meshIndex != -1;
											}
											if (bVar2) {
												bVar2 = this->field_0x410.textureIndex != -1;
												if (bVar2) {
													bVar2 = this->field_0x410.meshIndex != -1;
												}
												if (bVar2) {
													bVar2 = this->field_0x470.textureIndex != -1;
													if (bVar2) {
														bVar2 = this->field_0x470.meshIndex != -1;
													}
													if (bVar2) {
														bVar2 = true;
														goto LAB_003b14b8;
													}
												}
											}
										}
									}
								}
							}
						}
						bVar2 = false;
LAB_003b14b8:
						if (!bVar2) {
							return;
						}
						this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
						this->field_0x230.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_03_pan_02");
						this->field_0x290.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_01");
						this->field_0x2f0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_02");
						this->field_0x350.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_03");
						this->field_0x3b0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_04");
						this->field_0x470.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_Symbole_01");
						pMiniGame = GetMiniGame(this->field_0x920);
						if (pMiniGame->curBehaviourId != 3) {
							return;
						}
						if (pMiniGame->field_0x1b0 != 0) {
							return;
						}
						this->field_0x410.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_05_pan_02_5");
						return;
					}
					bVar2 = this->field_0x1d0.textureIndex != -1;
					if (bVar2) {
						bVar2 = this->field_0x1d0.meshIndex != -1;
					}
					if (bVar2) {
						bVar2 = this->field_0x230.textureIndex != -1;
						if (bVar2) {
							bVar2 = this->field_0x230.meshIndex != -1;
						}
						if (bVar2) {
							bVar2 = this->field_0x290.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x290.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x2f0.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x2f0.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = this->field_0x350.textureIndex != -1;
									if (bVar2) {
										bVar2 = this->field_0x350.meshIndex != -1;
									}
									if (bVar2) {
										bVar2 = this->field_0x3b0.textureIndex != -1;
										if (bVar2) {
											bVar2 = this->field_0x3b0.meshIndex != -1;
										}
										if (bVar2) {
											bVar2 = this->field_0x410.textureIndex != -1;
											if (bVar2) {
												bVar2 = this->field_0x410.meshIndex != -1;
											}
											if (bVar2) {
												bVar2 = this->field_0x470.textureIndex != -1;
												if (bVar2) {
													bVar2 = this->field_0x470.meshIndex != -1;
												}
												if (bVar2) {
													bVar2 = true;
													goto LAB_003b1748;
												}
											}
										}
									}
								}
							}
						}
					}
					bVar2 = false;
LAB_003b1748:
					if (!bVar2) {
						return;
					}
					this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
					this->field_0x230.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_02_pan_01");
					this->field_0x290.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_02_pan_02");
					this->field_0x2f0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_02_pan_03");
					this->field_0x350.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_02_pan_04");
					this->field_0x470.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_Symbole_02");
					return;
				}
				bVar2 = this->field_0x1d0.textureIndex != -1;
				if (bVar2) {
					bVar2 = this->field_0x1d0.meshIndex != -1;
				}
				if (bVar2) {
					bVar2 = this->field_0x230.textureIndex != -1;
					if (bVar2) {
						bVar2 = this->field_0x230.meshIndex != -1;
					}
					if (bVar2) {
						bVar2 = this->field_0x290.textureIndex != -1;
						if (bVar2) {
							bVar2 = this->field_0x290.meshIndex != -1;
						}
						if (bVar2) {
							bVar2 = this->field_0x2f0.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x2f0.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x350.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x350.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = this->field_0x3b0.textureIndex != -1;
									if (bVar2) {
										bVar2 = this->field_0x3b0.meshIndex != -1;
									}
									if (bVar2) {
										bVar2 = this->field_0x410.textureIndex != -1;
										if (bVar2) {
											bVar2 = this->field_0x410.meshIndex != -1;
										}
										if (bVar2) {
											bVar2 = this->field_0x470.textureIndex != -1;
											if (bVar2) {
												bVar2 = this->field_0x470.meshIndex != -1;
											}
											if (bVar2) {
												bVar2 = true;
												goto LAB_003b1970;
											}
										}
									}
								}
							}
						}
					}
				}
				bVar2 = false;
LAB_003b1970:
				if (!bVar2) {
					return;
				}
				this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
				this->field_0x230.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_03_pan_01");
				this->field_0x290.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_03_pan_02");
				this->field_0x2f0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_03_pan_03");
				this->field_0x350.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_02_pan_04");
				this->field_0x470.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_Symbole_01");
				return;
			}
			bVar2 = this->field_0x1d0.textureIndex != -1;
			if (bVar2) {
				bVar2 = this->field_0x1d0.meshIndex != -1;
			}
			if (bVar2) {
				bVar2 = this->field_0x230.textureIndex != -1;
				if (bVar2) {
					bVar2 = this->field_0x230.meshIndex != -1;
				}
				if (bVar2) {
					bVar2 = this->field_0x290.textureIndex != -1;
					if (bVar2) {
						bVar2 = this->field_0x290.meshIndex != -1;
					}
					if (bVar2) {
						bVar2 = this->field_0x2f0.textureIndex != -1;
						if (bVar2) {
							bVar2 = this->field_0x2f0.meshIndex != -1;
						}
						if (bVar2) {
							bVar2 = this->field_0x350.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x350.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x3b0.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x3b0.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = this->field_0x410.textureIndex != -1;
									if (bVar2) {
										bVar2 = this->field_0x410.meshIndex != -1;
									}
									if (bVar2) {
										bVar2 = this->field_0x470.textureIndex != -1;
										if (bVar2) {
											bVar2 = this->field_0x470.meshIndex != -1;
										}
										if (bVar2) {
											bVar2 = true;
											goto LAB_003b1b98;
										}
									}
								}
							}
						}
					}
				}
			}
			bVar2 = false;
LAB_003b1b98:
			if (!bVar2) {
				return;
			}
			this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
			this->field_0x230.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_04_pan_01");
			this->field_0x290.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_04_pan_02");
			this->field_0x2f0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_04_pan_03");
			this->field_0x350.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_02_pan_04");
			this->field_0x470.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_Symbole_01");
			return;
		}
		bVar2 = this->field_0x1d0.textureIndex != -1;
		if (bVar2) {
			bVar2 = this->field_0x1d0.meshIndex != -1;
		}
		if (bVar2) {
			bVar2 = this->field_0x230.textureIndex != -1;
			if (bVar2) {
				bVar2 = this->field_0x230.meshIndex != -1;
			}
			if (bVar2) {
				bVar2 = this->field_0x290.textureIndex != -1;
				if (bVar2) {
					bVar2 = this->field_0x290.meshIndex != -1;
				}
				if (bVar2) {
					bVar2 = this->field_0x2f0.textureIndex != -1;
					if (bVar2) {
						bVar2 = this->field_0x2f0.meshIndex != -1;
					}
					if (bVar2) {
						bVar2 = this->field_0x350.textureIndex != -1;
						if (bVar2) {
							bVar2 = this->field_0x350.meshIndex != -1;
						}
						if (bVar2) {
							bVar2 = this->field_0x3b0.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x3b0.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x410.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x410.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = this->field_0x470.textureIndex != -1;
									if (bVar2) {
										bVar2 = this->field_0x470.meshIndex != -1;
									}
									if (bVar2) {
										bVar2 = true;
										goto LAB_003b1dc0;
									}
								}
							}
						}
					}
				}
			}
		}
		bVar2 = false;
LAB_003b1dc0:
		if (bVar2) {
			this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
			this->field_0x230.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_01_pan_01");
			this->field_0x290.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_01_pan_02");
			this->field_0x2f0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_03_pan_02");
			this->field_0x470.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_Symbole_01");
		}
		this->field_0x76c.pContext = this;
		return;
	}
	bVar2 = this->field_0x1d0.textureIndex != -1;
	if (bVar2) {
		bVar2 = this->field_0x1d0.meshIndex != -1;
	}
	if (bVar2) {
		bVar2 = this->field_0x230.textureIndex != -1;
		if (bVar2) {
			bVar2 = this->field_0x230.meshIndex != -1;
		}
		if (bVar2) {
			bVar2 = this->field_0x290.textureIndex != -1;
			if (bVar2) {
				bVar2 = this->field_0x290.meshIndex != -1;
			}
			if (bVar2) {
				bVar2 = this->field_0x2f0.textureIndex != -1;
				if (bVar2) {
					bVar2 = this->field_0x2f0.meshIndex != -1;
				}
				if (bVar2) {
					bVar2 = this->field_0x350.textureIndex != -1;
					if (bVar2) {
						bVar2 = this->field_0x350.meshIndex != -1;
					}
					if (bVar2) {
						bVar2 = this->field_0x3b0.textureIndex != -1;
						if (bVar2) {
							bVar2 = this->field_0x3b0.meshIndex != -1;
						}
						if (bVar2) {
							bVar2 = this->field_0x410.textureIndex != -1;
							if (bVar2) {
								bVar2 = this->field_0x410.meshIndex != -1;
							}
							if (bVar2) {
								bVar2 = this->field_0x470.textureIndex != -1;
								if (bVar2) {
									bVar2 = this->field_0x470.meshIndex != -1;
								}
								if (bVar2) {
									bVar2 = true;
									goto LAB_003b1fd0;
								}
							}
						}
					}
				}
			}
		}
	}
	bVar2 = false;
LAB_003b1fd0:
	if (bVar2) {
		this->field_0x1d0.Init(CFrontend::_scene_handle, (ed_g3d_manager*)0x0, &this->field_0x198, "Gam_back_01");
	}
	return;
}

void CActorMiniGamesOrganizer::TermMenuMeshes()
{
	bool bVar1;

	bVar1 = this->field_0x1d0.HasMesh();
	if (bVar1 != 0) {
		this->field_0x1d0.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x230.HasMesh();
	if (bVar1 != 0) {
		this->field_0x230.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x290.HasMesh();
	if (bVar1 != 0) {
		this->field_0x290.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x2f0.HasMesh();
	if (bVar1 != 0) {
		this->field_0x2f0.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x350.HasMesh();
	if (bVar1 != 0) {
		this->field_0x350.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x3b0.HasMesh();
	if (bVar1 != 0) {
		this->field_0x3b0.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x410.HasMesh();
	if (bVar1 != 0) {
		this->field_0x410.Term(CFrontend::_scene_handle);
	}

	bVar1 = this->field_0x470.HasMesh();
	if (bVar1 != 0) {
		this->field_0x470.Term(CFrontend::_scene_handle);
	}

	return;
}

void CActorMiniGamesOrganizer::ManageMenuChoose()
{
	CPlayerInput *pInputManager;
	CSoundSample *pSoundSample;
	uint uVar4;
	int iVar5;
	int iVar7;

	pInputManager = this->field_0x9f0->GetInputManager(0, 0);
	if (pInputManager != (CPlayerInput*)0x0) {
		this->menuWheel.Manage();
		this->field_0x76c.FUN_002ef890();

		uVar4 = pInputManager->pressedBitfield;
		if (((uVar4 & 0x100000) == 0) && ((uVar4 & 4) == 0)) {
			if (((uVar4 & 0x200000) == 0) && ((uVar4 & 8) == 0)) {
				if (((uVar4 & 0x400000) == 0) && ((uVar4 & 1) == 0)) {
					if (((uVar4 & 0x800000) == 0) && ((uVar4 & 2) == 0)) {
						if ((uVar4 & 0x1000000) == 0) {
							if ((uVar4 & 0x4000000) != 0) {
								pSoundSample = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x18c);
								this->field_0xa00.pSound = pSoundSample;

								if ((NoAudio == 0) &&
									(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)
									) {
									this->field_0xa00.field_0x20 = 0xffffffff;
									uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
										(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
									this->field_0xa00.soundId = uVar4;
								}

								DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
								SetState(5, -1);
							}
						}
						else {
							iVar7 = this->field_0x928;
							if (iVar7 == 2) {
								SetState(9, -1);
							}
							else {
								if (iVar7 == 3) {
									pSoundSample = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x188);
									this->field_0xa00.pSound = pSoundSample;

									if ((NoAudio == 0) &&
										(this->field_0xa00.pSound3dData = 0,
										this->field_0xa00.pSound != (CSoundSample *)0x0)) {
										this->field_0xa00.field_0x20 = 0xffffffff;
										uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
											(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
										this->field_0xa00.soundId = uVar4;
									}

									SetState(8, -1);
								}
								else {
									if (iVar7 == 1) {
										SetState(7, -1);
									}
								}
							}
						}
					}
					else {
						iVar7 = 0;
						if (this->pMiniGameStreamRefs != (S_ACTOR_STREAM_REF*)0x0) {
							iVar7 = this->pMiniGameStreamRefs->entryCount;
						}
						assert(iVar7 != 0);
						this->field_0x920 = (this->field_0x920 + 1) % iVar7;
						ComputeCurPlayMode();
						ComputeCurPlayMode();
						this->menuWheel.MoveWheel(true);
						pSoundSample = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
						this->field_0xa00.pSound = pSoundSample;
						if ((NoAudio == 0) &&
							(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
							this->field_0xa00.field_0x20 = 0xffffffff;
							uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
								(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
							this->field_0xa00.soundId = uVar4;
						}
					}
				}
				else {
					if (this->pMiniGameStreamRefs == (S_ACTOR_STREAM_REF*)0x0) {
						iVar7 = 0;
					}
					else {
						iVar7 = this->pMiniGameStreamRefs->entryCount;
					}

					if (this->pMiniGameStreamRefs == (S_ACTOR_STREAM_REF*)0x0) {
						iVar5 = 0;
					}
					else {
						iVar5 = this->pMiniGameStreamRefs->entryCount;
					}

					assert(iVar5 != 0);
					this->field_0x920 = (this->field_0x920 + iVar7 + -1) % iVar5;
					ComputeCurPlayMode();
					ComputeCurPlayMode();

					this->menuWheel.MoveWheel(false);
					pSoundSample = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
					this->field_0xa00.pSound = pSoundSample;

					if ((NoAudio == 0) &&
						(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
						this->field_0xa00.field_0x20 = 0xffffffff;
						uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
							(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
						this->field_0xa00.soundId = uVar4;
					}
				}
			}
			else {
				this->field_0x76c.MoveMenuArrow(true);

				pSoundSample = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
				this->field_0xa00.pSound = pSoundSample;

				if ((NoAudio == 0) && (this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
					this->field_0xa00.field_0x20 = 0xffffffff;

					uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
						(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
					this->field_0xa00.soundId = uVar4;
				}

				NextPlayMode();
			}
		}
		else {
			this->field_0x76c.MoveMenuArrow(false);

			pSoundSample = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
			this->field_0xa00.pSound = pSoundSample;

			if ((NoAudio == 0) &&
				(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
				this->field_0xa00.field_0x20 = 0xffffffff;
				uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
					(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
				this->field_0xa00.soundId = uVar4;
			}

			PrevPlayMode();
		}
	}

	return;
}


void CActorMiniGamesOrganizer::InitMenuBet()
{
	CActorMiniGame* pMiniGame;
	CBehaviourMiniGameBetting* pBehaviour;
	S_MINI_GAME_BET* pBet;
	int iVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float fVar7;

	this->field_0x76c.field_0x198 = 0.0f;

	fVar7 = (float)gVideoConfig.screenWidth * 0.045f;
	if (fVar7 < 2.147484e+09f) {
		this->field_0x76c.field_0x0.iWidth = (ushort)(int)fVar7;
	}
	else {
		this->field_0x76c.field_0x0.iWidth = (ushort)(int)(fVar7 - 2.147484e+09f);
	}

	fVar7 = (float)gVideoConfig.screenHeight * 0.045f;
	if (fVar7 < 2.147484e+09f) {
		this->field_0x76c.field_0x0.iHeight = (ushort)(int)fVar7;
	}
	else {
		this->field_0x76c.field_0x0.iHeight = (ushort)(int)(fVar7 - 2.147484e+09f);
	}

	fVar7 = (float)gVideoConfig.screenWidth * 0.045f;
	if (fVar7 < 2.147484e+09f) {
		this->field_0x76c.field_0xc0.iWidth = (ushort)(int)fVar7;
	}
	else {
		this->field_0x76c.field_0xc0.iWidth = (ushort)(int)(fVar7 - 2.147484e+09f);
	}

	fVar7 = (float)gVideoConfig.screenHeight * 0.045f;
	if (fVar7 < 2.147484e+09f) {
		this->field_0x76c.field_0xc0.iHeight = (ushort)(int)fVar7;
	}
	else {
		this->field_0x76c.field_0xc0.iHeight = (ushort)(int)(fVar7 - 2.147484e+09f);
	}

	iVar5 = 0;
	iVar4 = 0;
	pMiniGame = GetMiniGame(this->field_0x920);
	pBehaviour = pMiniGame->GetBhvBetting();
	iVar3 = 0;
	if (0 < pBehaviour->nbBets) {
		do {
			pBehaviour = pMiniGame->GetBhvBetting();
			pBet = pBehaviour->aBets + iVar3;
			iVar2 = this->field_0x9f4->IsBetAvailable(pBet->cost, pBet->reward);
			if (iVar2 == 0) {
				pBet->bAvailable = 0;
			}
			else {
				iVar5 = iVar5 + 1;
				pBet->bAvailable = 1;
			}

			iVar3 = iVar3 + 1;
			iVar4 = iVar4 + 1;
			pBehaviour = pMiniGame->GetBhvBetting();
		} while (iVar4 < pBehaviour->nbBets);
	}

	this->field_0x938 = iVar5;
	this->field_0x924 = 0;
	pBehaviour = pMiniGame->GetBhvBetting();
	if (((0 < pBehaviour->nbBets) &&
		(pBehaviour = pMiniGame->GetBhvBetting(), pBehaviour->aBets[0].bAvailable == 0)) &&
		(pMiniGame = GetMiniGame(this->field_0x920), this->field_0x938 != 0)) {
		pBehaviour = pMiniGame->GetBhvBetting();
		assert(pBehaviour->nbBets != 0);
		iVar2 = (this->field_0x924 + 1) % pBehaviour->nbBets;
		do {
			pBehaviour = pMiniGame->GetBhvBetting();
			if (pBehaviour->nbBets <= iVar2) {
				return;
			}

			pBehaviour = pMiniGame->GetBhvBetting();
			if (pBehaviour->aBets[iVar2].bAvailable != 0) break;
			pBehaviour = pMiniGame->GetBhvBetting();
			assert(pBehaviour->nbBets != 0);
			iVar2 = (iVar2 + 1) % pBehaviour->nbBets;
		} while (iVar2 != this->field_0x924);

		this->field_0x924 = iVar2;
	}

	return;
}

void CActorMiniGamesOrganizer::ManageMenuBet()
{
	CActorMiniGame* pReceiver;
	CActorMiniGame* pMiniGame;
	CPlayerInput* pCVar1;
	int iVar2;
	CSoundSample* pCVar3;
	uint uVar4;
	CBehaviourMiniGameBetting* pBehaviour;
	S_MINI_GAME_BET* pBet;
	int iVar11;
	int iVar12;
	int iVar13;

	pCVar1 = this->field_0x9f0->GetInputManager(0, 0);
	if (pCVar1 != (CPlayerInput*)0x0) {
		iVar12 = 0;
		iVar11 = 0;
		pReceiver = GetMiniGame(this->field_0x920);
		pBehaviour = pReceiver->GetBhvBetting();
		iVar13 = 0;
		if (0 < pBehaviour->nbBets) {
			do {
				pBehaviour = pReceiver->GetBhvBetting();
				pBet = pBehaviour->aBets + iVar13;
				iVar2 = this->field_0x9f4->IsBetAvailable(pBet->cost, pBet->reward);
				if (iVar2 == 0) {
					pBet->bAvailable = 0;
				}
				else {
					iVar12 = iVar12 + 1;
					pBet->bAvailable = 1;
				}

				iVar13 = iVar13 + 1;
				iVar11 = iVar11 + 1;
				pBehaviour = pReceiver->GetBhvBetting();
			} while (iVar11 < pBehaviour->nbBets);
		}

		this->field_0x938 = iVar12;
		this->field_0x76c.FUN_002ef890();

		pBehaviour = pReceiver->GetBhvBetting();
		if ((0 < pBehaviour->nbBets) && (0 < this->field_0x938)) {
			if (((pCVar1->pressedBitfield & 0x100000) != 0) || ((pCVar1->pressedBitfield & 4) != 0)) {
				pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
				this->field_0xa00.pSound = pCVar3;
				if ((NoAudio == 0) &&
					(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample*)0x0)) {
					this->field_0xa00.field_0x20 = 0xffffffff;
					uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
						(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
					this->field_0xa00.soundId = uVar4;
				}

				this->field_0x76c.MoveMenuArrow(false);

				pMiniGame = GetMiniGame(this->field_0x920);
				if (this->field_0x938 != 0) {
					pBehaviour = pMiniGame->GetBhvBetting();
					assert(pBehaviour->nbBets != 0);
					iVar2 = (this->field_0x924 + 1) % pBehaviour->nbBets;
					do {
						pBehaviour = pMiniGame->GetBhvBetting();
						if (pBehaviour->nbBets <= iVar2) goto LAB_003b6910;
						pBehaviour = pMiniGame->GetBhvBetting();
						if (pBehaviour->aBets[iVar2].bAvailable != 0) break;
						pBehaviour = pMiniGame->GetBhvBetting();
						assert(pBehaviour->nbBets != 0);
						iVar2 = (iVar2 + 1) % pBehaviour->nbBets;
					} while (iVar2 != this->field_0x924);
					this->field_0x924 = iVar2;
				}
			}
		LAB_003b6910:
			if (((pCVar1->pressedBitfield & 0x200000) != 0) || ((pCVar1->pressedBitfield & 8) != 0)) {
				pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
				this->field_0xa00.pSound = pCVar3;
				if ((NoAudio == 0) &&
					(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample*)0x0)) {
					this->field_0xa00.field_0x20 = 0xffffffff;
					uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
						(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
					this->field_0xa00.soundId = uVar4;
				}

				this->field_0x76c.MoveMenuArrow(true);
				pMiniGame = GetMiniGame(this->field_0x920);
				pBehaviour = pMiniGame->GetBhvBetting();
				iVar11 = pBehaviour->nbBets;
				pBehaviour = pMiniGame->GetBhvBetting();
				assert(iVar11 != 0);
				iVar2 = (this->field_0x924 + pBehaviour->nbBets + -1) % iVar11;
				do {
					pBehaviour = pMiniGame->GetBhvBetting();
					if (pBehaviour->nbBets <= iVar2) goto LAB_003b6aa0;
					pBehaviour = pMiniGame->GetBhvBetting();
					if (pBehaviour->aBets[iVar2].bAvailable != 0) break;
					pBehaviour = pMiniGame->GetBhvBetting();
					iVar11 = pBehaviour->nbBets;
					pBehaviour = pMiniGame->GetBhvBetting();
					assert(iVar11 != 0);
					iVar2 = (iVar2 + pBehaviour->nbBets + -1) % iVar11;
				} while (iVar2 != this->field_0x924);

				this->field_0x924 = iVar2;
			}
		LAB_003b6aa0:
			if ((pCVar1->pressedBitfield & 0x1000000) != 0) {
				CActorMiniGamesManager* pManager = this->field_0x9f4;
				iVar11 = this->field_0x924;
				pBehaviour = pReceiver->GetBhvBetting();
				pBehaviour->curBet = iVar11;
				pBehaviour = pReceiver->GetBhvBetting();
				iVar2 = pManager->PlaceBet(pBehaviour->aBets[pBehaviour->curBet].cost);
				if (iVar2 != 0) {
					DoMessage(pReceiver, (ACTOR_MESSAGE)0x56, (void*)1);
					DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
					SetState(10, -1);
				}
			}
		}

		if ((pCVar1->pressedBitfield & 0x4000000) != 0) {
			SetState(6, -1);
		}
	}

	return;
}

void CActorMiniGamesOrganizer::ManageMenuMulti()
{
	CActorMiniGame* pReceiver;
	CPlayerInput *pCVar1;
	CSoundSample *pCVar2;
	uint uVar3;
	CBehaviourMiniGameMulti *pCVar4;

	pCVar1 = this->field_0x9f0->GetInputManager(0, 0);
	if (pCVar1 != (CPlayerInput *)0x0) {
		this->field_0x76c.FUN_002ef890();
		pReceiver = GetMiniGame(this->field_0x920);
		if (((pCVar1->pressedBitfield & 0x100000) != 0) || ((pCVar1->pressedBitfield & 4) != 0)) {
			pCVar2 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
			this->field_0xa00.pSound = pCVar2;
			if ((NoAudio == 0) &&
				(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
				this->field_0xa00.field_0x20 = 0xffffffff;
				uVar3 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
					(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
				this->field_0xa00.soundId = uVar3;
			}

			this->field_0x76c.MoveMenuArrow(false);
			pCVar4 = pReceiver->GetBhvMulti();
			pCVar4->AddOnePlayer();
		}

		if (((pCVar1->pressedBitfield & 0x200000) != 0) || ((pCVar1->pressedBitfield & 8) != 0)) {
			pCVar2 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
			this->field_0xa00.pSound = pCVar2;
			if ((NoAudio == 0) &&
				(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
				this->field_0xa00.field_0x20 = 0xffffffff;
				uVar3 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
					(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
				this->field_0xa00.soundId = uVar3;
			}

			this->field_0x76c.MoveMenuArrow(true);
			pCVar4 = pReceiver->GetBhvMulti();
			pCVar4->SubOnePlayer();
		}

		if ((pCVar1->pressedBitfield & 0x1000000) != 0) {
			DoMessage(pReceiver, (ACTOR_MESSAGE)0x56, (void*)2);
			DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x24, 0);
			SetState(10, -1);
		}

		if ((pCVar1->pressedBitfield & 0x4000000) != 0) {
			SetState(6, -1);
		}
	}

	return;
}

void CActorMiniGamesOrganizer::ManageMenuResult()
{
	CActorMiniGame* pMiniGame;
	int iVar1;
	CActorMiniGame* pAVar2;
	CSoundSample *pCVar3;
	uint uVar4;
	_msg_mini_game_restart local_28;
	void* local_18;
	void* local_14;
	_msg_mini_game_restart* local_10;
	void* local_c;
	char local_8[4];
	char local_4[4];

	pMiniGame = GetMiniGame(this->field_0x920);
	this->field_0x76c.FUN_002ef890();

	if (((gPlayerInput.pressedBitfield & 0x200000) != 0) || ((gPlayerInput.pressedBitfield & 8) != 0))
	{
		pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
		this->field_0xa00.pSound = pCVar3;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar4;
		}
		this->field_0x76c.MoveMenuArrow(true);
		pMiniGame->NextFinalAction();
	}

	if (((gPlayerInput.pressedBitfield & 0x100000) != 0) || ((gPlayerInput.pressedBitfield & 4) != 0))
	{
		pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
		this->field_0xa00.pSound = pCVar3;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar4;
		}

		this->field_0x76c.MoveMenuArrow(false);
		pMiniGame->PrevFinalAction();
	}

	if ((gPlayerInput.pressedBitfield & 0x1000000) != 0) {
		iVar1 = pMiniGame->field_0x1cc;
		if (iVar1 == 0) {
			strcpy(this->field_0x9ac[0], "-");
			strcpy(this->field_0x9ac[1], "-");
			strcpy(this->field_0x9ac[2], "-");
			pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x188);
			this->field_0xa00.pSound = pCVar3;
			if ((NoAudio == 0) &&
				(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
				this->field_0xa00.field_0x20 = 0xffffffff;
				uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
					(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
				this->field_0xa00.soundId = uVar4;
			}
			SetState(0xf, -1);
		}
		else {
			if (iVar1 == 2) {
				pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x188);
				this->field_0xa00.pSound = pCVar3;
				if ((NoAudio == 0) &&
					(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
					this->field_0xa00.field_0x20 = 0xffffffff;
					uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
						(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
					this->field_0xa00.soundId = uVar4;
				}
				local_18 = 0;
				pMiniGame->DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x58, local_18);
				local_8[0] = this->field_0x9ac[0][0];
				local_8[1] = this->field_0x9ac[1][0];
				local_8[2] = this->field_0x9ac[2][0];
				local_8[3] = 0;
				pAVar2 = GetMiniGame(this->field_0x920);
				pAVar2->UpdateCurHighScoreName(local_8);
				local_c = 0;
				DoMessage(pAVar2, (ACTOR_MESSAGE)0x56, local_c);
				this->field_0x93c = 5;
				this->field_0x940 = false;
			}
			else {
				if (iVar1 == 1) {
					pCVar3 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x188);
					this->field_0xa00.pSound = pCVar3;
					if ((NoAudio == 0) &&
						(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
						this->field_0xa00.field_0x20 = 0xffffffff;
						uVar4 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
							(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
						this->field_0xa00.soundId = uVar4;
					}
					local_28.pLocation = &pMiniGame->wayPointRef.Get()->location;
					local_28.pRotation = &pMiniGame->wayPointRef.Get()->rotation;
					local_28.sectorId = pMiniGame->field_0x190;
					if (local_28.sectorId == -1) {
						local_28.sectorId = ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID;
					}
					local_4[0] = this->field_0x9ac[0][0];
					local_4[1] = this->field_0x9ac[1][0];
					local_4[2] = this->field_0x9ac[2][0];
					local_4[3] = 0;
					pAVar2 = GetMiniGame(this->field_0x920);
					pAVar2->UpdateCurHighScoreName(local_4);
					local_10 = &local_28;
					pAVar2->DoMessage(this->field_0x9f0, (ACTOR_MESSAGE)0x5b, local_10);
					local_14 = 0;
					DoMessage(pAVar2, (ACTOR_MESSAGE)0x5b, local_14);
					this->field_0x93c = 10;
					this->field_0x940 = false;
				}
			}
		}
	}
	return;
}

void CActorMiniGamesOrganizer::ManageMenuEnterName()
{
	CSoundSample *pCVar1;
	uint uVar2;
	uint uVar3;
	int* piVar4;
	int iVar5;
	int iVar6;

	if (((gPlayerInput.pressedBitfield & 0x400000) != 0) || ((gPlayerInput.pressedBitfield & 1) != 0)) {
		pCVar1 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
		this->field_0xa00.pSound = pCVar1;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar2 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar2;
		}
		iVar6 = this->field_0x99c[this->field_0x998];
		assert(iVar6 != 0);
		this->field_0x994 = (this->field_0x994 + -1 + iVar6) % iVar6;
	}

	if (((gPlayerInput.pressedBitfield & 0x800000) != 0) || ((gPlayerInput.pressedBitfield & 2) != 0)) {
		pCVar1 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
		this->field_0xa00.pSound = pCVar1;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar2 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar2;
		}
		assert(this->field_0x99c[this->field_0x998] != 0);
		this->field_0x994 =
				(this->field_0x994 + 1) %
				this->field_0x99c[this->field_0x998];
	}

	if (((gPlayerInput.pressedBitfield & 0x100000) != 0) || ((gPlayerInput.pressedBitfield & 4) != 0)) {
		pCVar1 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
		this->field_0xa00.pSound = pCVar1;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar2 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar2;
		}

		uVar3 = this->field_0x998 + 3;
		uVar2 = uVar3 & 3;
		if (((int)uVar3 < 0) && (uVar2 != 0)) {
			uVar2 = uVar2 - 4;
		}

		this->field_0x998 = uVar2;
		assert(this->field_0x99c[this->field_0x998] != 0);
		this->field_0x994 =
				this->field_0x994 %
				this->field_0x99c[this->field_0x998];
	}

	if (((gPlayerInput.pressedBitfield & 0x200000) != 0) || ((gPlayerInput.pressedBitfield & 8) != 0)) {
		pCVar1 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x190);
		this->field_0xa00.pSound = pCVar1;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar2 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar2;
		}

		uVar3 = this->field_0x998 + 1;
		uVar2 = uVar3 & 3;
		if (((int)uVar3 < 0) && (uVar2 != 0)) {
			uVar2 = uVar2 - 4;
		}

		this->field_0x998 = uVar2;
		assert(this->field_0x99c[this->field_0x998] != 0);
		this->field_0x994 =
				this->field_0x994 %
				this->field_0x99c[this->field_0x998];
	}

	if ((gPlayerInput.pressedBitfield & 0x1000000) != 0) {
		pCVar1 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x188);
		this->field_0xa00.pSound = pCVar1;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar2 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar2;
		}

		iVar6 = this->field_0x994;
		iVar5 = 0;
		piVar4 = this->field_0x99c;
		if (this->field_0x998 != 0) {
			do {
				iVar5 = iVar5 + 1;
				iVar6 = iVar6 + *piVar4;
				piVar4 = piVar4 + 1;
			} while (iVar5 != this->field_0x998);
		}

		if (iVar6 == 0x1a) {
			this->field_0x9b4 = this->field_0x9b4 + -1;
			if (this->field_0x9b4 < 0) {
				this->field_0x9b4 = 0;
			}
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
			this->field_0x9b4 = this->field_0x9b4 + 1;
		}
	}

	if ((gPlayerInput.pressedBitfield & 0x4000000) != 0) {
		pCVar1 = CScene::ptable.g_AudioManager_00451698->GetSound(this->field_0x18c);
		this->field_0xa00.pSound = pCVar1;
		if ((NoAudio == 0) &&
			(this->field_0xa00.pSound3dData = 0, this->field_0xa00.pSound != (CSoundSample *)0x0)) {
			this->field_0xa00.field_0x20 = 0xffffffff;
			uVar2 = this->field_0xa00.pSound->Play(this->field_0xa00.soundId, this->field_0xa00.field_0x20,
				(edsound_3d_data*)0x0, &this->field_0xa00, (uint*)0x0, &this->field_0xa00.soundId);
			this->field_0xa00.soundId = uVar2;
		}

		// Keep the port's guard: index 3 would overwrite field_0x9b4 on PS2.
		if (this->field_0x9b4 < 3) {
			this->field_0x9ac[this->field_0x9b4][0] = '-';
		}
	}

	return;
}

void CActorMiniGamesOrganizer::ManageFade()
{
	bool bVar1;
	Timer *pTVar2;
	float fVar3;
	float fVar4;

	bVar1 = CScene::_pinstance->FUN_001b92f0();
	if (bVar1 == false) {
		if (0.0f < this->field_0x944) {
			pTVar2 = GetTimer();
			this->field_0x944 = this->field_0x944 - pTVar2->lastFrameTime / 0.2f;
			if (this->field_0x948 < this->field_0x944) {
				this->field_0x944 = this->field_0x948;
			}
			else {
				if (this->field_0x944 < 0.0f) {
					this->field_0x944 = 0.0f;
				}
			}
		}
	}
	else {
		pTVar2 = GetTimer();
		this->field_0x944 = this->field_0x944 + pTVar2->lastFrameTime;
		if (this->field_0x948 < this->field_0x944) {
			this->field_0x944 = this->field_0x948;
		}
		else {
			if (this->field_0x944 < 0.0f) {
				this->field_0x944 = 0.0f;
			}
		}
	}

	fVar4 = (this->field_0x948 - this->field_0x944) / this->field_0x948;
	fVar3 = 1.0f;
	if ((fVar4 <= 1.0f) && (fVar3 = fVar4, fVar4 < 0.0f)) {
		fVar3 = 0.0f;
	}

	fVar3 = fVar3 * 255.0f;
	if (fVar3 < 2.147484e+09f) {
		this->field_0x94c = (byte)(int)fVar3;
	}
	else {
		this->field_0x94c = (byte)(int)(fVar3 - 2.147484e+09f);
	}

	return;
}

void CActorMiniGamesOrganizer::ManageMusic(int state)
{
	bool bVar1;
	int iVar2;
	StateConfig* pSVar3;
	uint uVar4;
	CMusic* pCVar5;
	CMusicManager* pMusicManager;

	iVar2 = this->prevActorState;
	if (iVar2 == -1) {
		uVar4 = 0;
	}
	else {
		pSVar3 = GetStateCfg(iVar2);
		uVar4 = pSVar3->flags_0x4;
	}
	if ((uVar4 & 0x200) == 0) {
		if (state == -1) {
			uVar4 = 0;
		}
		else {
			pSVar3 = GetStateCfg(state);
			uVar4 = pSVar3->flags_0x4 & 0x200;
		}
		if (uVar4 != 0) {
			uVar4 = this->field_0x184;
			if ((uVar4 == 0xffffffff) ||
				(bVar1 = (uint)CScene::ptable.g_AudioManager_00451698->nbMusic <= uVar4, bVar1)) {
				pCVar5 = (CMusic*)0x0;
			}
			else {
				if (bVar1) {
					uVar4 = 0;
				}
				pCVar5 = CScene::ptable.g_AudioManager_00451698->aMusic + uVar4;
			}
			if ((pCVar5 != (CMusic*)0x0) && (this->field_0x9f8 == -1)) {
				iVar2 = CScene::ptable.g_AudioManager_00451698->field_0x38->Start(5.0f, 1.0f, 1.3f, 0.0f, pCVar5, 0x19);
				this->field_0x9f8 = iVar2;
			}
		}
	}
	if (state == -1) {
		uVar4 = 0;
	}
	else {
		pSVar3 = GetStateCfg(state);
		uVar4 = pSVar3->flags_0x4 & 0x200;
	}
	if (uVar4 == 0) {
		iVar2 = this->prevActorState;
		if (iVar2 == -1) {
			uVar4 = 0;
		}
		else {
			pSVar3 = GetStateCfg(iVar2);
			uVar4 = pSVar3->flags_0x4 & 0x200;
		}
		if (uVar4 != 0) {
			pMusicManager = CScene::ptable.g_AudioManager_00451698->field_0x38;
			if (this->field_0x9f8 != -1) {
				uVar4 = this->field_0x184;
				if ((uVar4 == 0xffffffff) ||
					(bVar1 = (uint)CScene::ptable.g_AudioManager_00451698->nbMusic <= uVar4, bVar1)) {
					pCVar5 = (CMusic*)0x0;
				}
				else {
					if (bVar1) {
						uVar4 = 0;
					}
					pCVar5 = CScene::ptable.g_AudioManager_00451698->aMusic + uVar4;
				}
				bVar1 = pMusicManager->IsMusic(this->field_0x9f8, pCVar5);
				if (bVar1 != false) {
					pMusicManager->Stop(0.0f, 0.0f, this->field_0x9f8);
				}
			}
			this->field_0x9f8 = -1;
		}
	}

	return;
}

void CActorMiniGamesOrganizer::ManageZone()
{
	S_ACTOR_STREAM_REF* pStream;
	bool bVar2;
	CActorHero* pCVar3;
	CEventManager* pCVar4;
	ed_zone_3d* pZone;
	CActorMiniGame* pMiniGame;
	int iVar5;
	int iVar6;
	int iVar7;
	int local_20[3];
	int* local_4;

	pCVar4 = CScene::ptable.g_EventManager_006f5080;
	pCVar3 = CActorHero::_gThis;
	bVar2 = false;
	iVar7 = 0;
	pStream = this->pMiniGameStreamRefs;
	iVar5 = 0;
	while (true) {
		iVar6 = 0;
		if (pStream != (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = pStream->entryCount;
		}
		if (iVar6 <= iVar7) break;
		pMiniGame = static_cast<CActorMiniGame*>(pStream->aEntries[iVar5].Get());
		if ((pMiniGame->field_0x1d4 != 0) && (pMiniGame->field_0x1d8 == 0)) {
			bVar2 = true;
		}
		iVar5 = iVar5 + 1;
		iVar7 = iVar7 + 1;
	}
	if (bVar2) {
		pZone = (ed_zone_3d*)0x0;
		if (this->field_0x180 != 0xffffffff) {
			pZone = edEventGetChunkZone(CScene::ptable.g_EventManager_006f5080->activeChunkId, this->field_0x180);
		}
		if (pZone != (ed_zone_3d*)0x0) {
			iVar5 = edEventComputeZoneAgainstVertex(pCVar4->activeChunkId, pZone, &pCVar3->currentLocation, 0);
			local_20[2] = this->field_0x9fc;
			if (iVar5 == 1) {
				local_20[0] = 0x10;
			}
			else {
				local_20[0] = 0x12;
			}
			// The PS2 leaves this unused message word uninitialized.
			local_20[1] = 0;
			local_4 = local_20;
			this->field_0x9f4->DoMessage(this->field_0x9f4->actorRef.Get(), (ACTOR_MESSAGE)0x4e, local_4);
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
	if (this->pMiniGameStreamRefs == 0) {
		iVar3 = 0;
	}
	else {
		iVar3 = this->pMiniGameStreamRefs->entryCount;
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
				if (this->field_0x9bc == 0) {
					return 0;
				}

				for (int i = 0; i < this->field_0x9bc; i++) {
					edDListTermMaterial(this->field_0x9b8 + i);
				}

				this->field_0x9bc = 0;
				ed3DUnInstallG2D(&this->field_0x9c0);
				return 1;
			}

			if (pParam->action == 0) {
				edBANK_ENTRY_INFO eStack32;
				int iStack4;

				NAME_NEXT_OBJECT("MiniGameG2D");

				if (!pCVar6->LoadEntryByFile(&eStack32, "G2D", 0)) {
					return 0;
				}

				ed3DInstallG2D(eStack32.fileBufferStart, eStack32.size, &iStack4, &this->field_0x9c0, 1);

				this->field_0x9bc = ed3DG2DGetG2DNbMaterials(&this->field_0x9c0);

				int iVar8;
				if (this->pMiniGameStreamRefs == (S_ACTOR_STREAM_REF*)0x0) {
					iVar8 = 0;
				}
				else {
					iVar8 = this->pMiniGameStreamRefs->entryCount;
				}

				if (iVar8 < this->field_0x9bc) {
					if (this->pMiniGameStreamRefs == (S_ACTOR_STREAM_REF*)0x0) {
						iVar8 = 0;
					}
					else {
						iVar8 = this->pMiniGameStreamRefs->entryCount;
					}
				}

				this->field_0x9bc = iVar8;

				for (int i = 0; i < this->field_0x9bc; i++) {
					edDListCreatMaterialFromIndex(this->field_0x9b8 + i, i, &this->field_0x9c0, 2);
				}

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
		if (pMsgParam == 0) {
			SetState(0xb, -1);
		}
		if (pMsgParam != (void*)1) {
			return 0;
		}

		CActorMiniGame* pMiniGame = static_cast<CActorMiniGame*>(pSender);
		if ((pMiniGame->curBehaviourId == 3) && (pMiniGame->field_0x1b0 == 0)) {
			CBehaviourMiniGameBetting* pBehaviour = pMiniGame->GetBhvBetting();
			if (this->field_0x924 < pBehaviour->nbBets) {
				CScene::ptable.g_LevelScheduleManager_00451660->Money_TakeFromBet(pBehaviour->aBets[pBehaviour->curBet].reward);
			}
		}

		SetState(0xd, -1);
		return 0;
	}

	if (msg == 0x14) {
		if (this->actorState != 5) {
			return 0;
		}

		this->field_0x9f0 = pSender;
		SetState(6, -1);
		return 1;
	}

	if (msg == 0x12) {
		float fVar1 = pSender->currentLocation.x - this->currentLocation.x;
		float fVar2 = pSender->currentLocation.z - this->currentLocation.z;
		if ((sqrtf(fVar1 * fVar1 + fVar2 * fVar2) < this->field_0x194) &&
			((GetStateFlags(this->actorState) & 0x100) != 0) &&
			(this->pMiniGameStreamRefs != 0) && (0 < this->pMiniGameStreamRefs->entryCount)) {
			return 0xe;
		}

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

		edCTextFormat auStack21760;
		edCTextFormat auStack16368;
		edCTextFormat auStack10976;
		edCTextFormat auStack5584;

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
	CActorMiniGame* pMiniGame;
	bool bVar2;
	edCTextStyle *pNewFont;
	char *pcVar3;
	int iVar4;
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
	pMiniGame = GetMiniGame(this->field_0x920);
	bVar2 = GuiDList_BeginCurrent();
	if (bVar2 != false) {
		pNewFont = edTextStyleSetCurrent(&eStack192);
		edCTextFormat auStack5584;

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
		pMiniGame->DrawHighScoreArray((float)gVideoConfig.screenWidth * 0.62f, (float)gVideoConfig.screenHeight * 0.3f,
			(float)gVideoConfig.screenWidth * 0.25f, 0.9714286f, &pMiniGame->GetBhvTraining()->scoreList);
		eStack192.SetEolAutomatic(0x80);
		eStack192.SetHorizontalJustification(0x10);
		eStack192.SetVerticalAlignment(8);
		eStack192.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.8421053f);
		eStack192.SetScale(0.95f, 0.95f);
		eStack192.rgbaColour = this->field_0x94c | 0xffffff00;
		pcVar3 = gMessageManager.get_message(pMiniGame->field_0x1a8);
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
	CBehaviourMiniGameBetting* iVar4;
	char *pcVar5;
	uint uVar6;
	S_MINI_GAME_BET* piVar7;
	float fVar8;
	float fVar10;
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
		edCTextFormat auStack5584;

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
		iVar4 = piVar1->GetBhvBetting();
		iVar2 = this->field_0x924;
		if (iVar2 < iVar4->nbBets) {
			iVar4 = piVar1->GetBhvBetting();
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
		if (0 < piVar1->nbScores) {
			eStack192.SetScale(1.5f, 1.5f);
			eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
			eStack192.SetRotation(0);
			piVar1->FormatScore(piVar1->defaultScore.score, &auStack5584, "", 0, 1);
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
	CBehaviourMiniGameMulti* iVar4;
	ulong uVar6;
	uint uVar7;
	float fVar8;
	float fVar10;
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
		edCTextFormat auStack5584;
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.13f, 1.13f);
		pcVar3 = gMessageManager.get_message(0x4753525818110104);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.47f, (float)gVideoConfig.screenHeight * 0.14f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		eStack192.SetScale(1.13f, 1.13f);
		iVar4 = piVar1->GetBhvMulti();
		auStack5584.FormatString("%d", iVar4->nbPlayers);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.77f, (float)gVideoConfig.screenHeight * 0.14f);
		eStack192.SetScale(1.2f, 1.2f);
		eStack192.SetRotation(-0.11f);
		eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
		pcVar3 = gMessageManager.get_message(0x1e161c0c040e1f01);
		auStack5584.FormatString(pcVar3);
		auStack5584.Display((float)gVideoConfig.screenWidth * 0.81f, (float)gVideoConfig.screenHeight * 0.35f);
		iVar4 = piVar1->GetBhvMulti();
		if (0 < iVar4->scoreList.nbScores) {
			eStack192.SetScale(1.2f, 1.2f);
			eStack192.rgbaColour = this->field_0x94c | 0xffff0000;
			eStack192.SetRotation(-0.11f);
			iVar4 = piVar1->GetBhvMulti();
			piVar1->FormatScore(iVar4->scoreList.aScores[0].score, &auStack5584, "", 0, 1);
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
			iVar4 = piVar1->GetBhvMulti();
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
		edCTextFormat auStack5584;
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
		pcVar3 = piVar1->FUN_003ace10();
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
			CBehaviourMiniGameBetting* pBehaviour = piVar1->GetBhvBetting();
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
		piVar1->FormatScore(piVar1->field_0x1d0, &auStack5584, 0);
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
					uVar5 = MiniGamePlayerColour(piVar1->GetBhvMulti()->winner);
				}
				else {
					eStack192.SetScale(1.0f, 1.0f);
					uint colour = MiniGamePlayerColour(piVar1->GetBhvMulti()->winner);
					uVar5 = ((((colour & 0xff00) >> 8) * 0x78 >> 8) << 8) |
						(((colour >> 24) * 0x78 >> 8) << 24) |
						((((colour & 0xff0000) >> 16) * 0x78 >> 8) << 16);
				}

				eStack192.rgbaColour = uVar5 & 0xffffff00 | (uint)this->field_0x94c;
				pcVar3 = gMessageManager.get_message(0x1e161e0506180817);
				auStack5584.FormatString(pcVar3, piVar1->GetBhvMulti()->winner + 1);
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

	edCTextFormat auStack5584;

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
	edCTextStyle* pNewFont = edTextStyleSetCurrent(&eStack192);
	edCTextFormat auStack5584;
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
