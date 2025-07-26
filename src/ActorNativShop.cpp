#include "ActorNativShop.h"
#include "MemoryStream.h"
#include "LevelScheduleManager.h"
#include "InventoryInfo.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include "LocalizationManager.h"
#include "Rendering/edCTextStyle.h"
#include "TimeController.h"
#include "ed3D.h"
#include "Frontend.h"
#include "Rendering/DisplayList.h"
#include "Pause.h"
#include "FileManager3D.h"
#include "FrontEndDisp.h"
#include "FrontEndMoney.h"
#include "FrontEndEnemy.h"
#include "ActorHero.h"
#include "InputManager.h"
#include "kya.h"

StateConfig CActorNativShop::_gStateCfg_SHO[5]
{
	StateConfig(-1, 0),
	StateConfig(-1, 0),
	StateConfig(-1, 0),
	StateConfig(-1, 0),
	StateConfig(-1, 0),
};

void CActorNativShop::Create(ByteCode* pByteCode)
{
	int* piVar1;
	float fVar5;

	CActor::Create(pByteCode);

	fVar5 = pByteCode->GetF32();
	this->field_0x164 = fVar5;
	this->field_0x164 = fVar5 * fVar5;
	this->field_0x16c = pByteCode->GetString();

	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	this->pActorStream = reinterpret_cast<S_ACTOR_STREAM_REF*>(piVar1);

	this->field_0x170 = pByteCode->GetU32();
	this->field_0x174 = pByteCode->GetU32();
	this->actorRef.index = pByteCode->GetS32();
	this->field_0x17c = 0;

	return;
}

void CActorNativShop::Init()
{
	S_ACTOR_STREAM_REF* pActorRef;
	int iVar1;

	pActorRef = this->pActorStream;
	for (iVar1 = pActorRef->entryCount; iVar1 != 0; iVar1 = iVar1 + -1) {
		pActorRef->aEntries[iVar1 - 1].Init();
	}

	this->actorRef.Init();

	this->field_0x168 = 0;

	CActor::Init();

	return;
}

void CActorNativShop::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorNativShop::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorNativShop::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourNativShop;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig* CActorNativShop::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state + -5) < 5);
		pStateConfig = _gStateCfg_SHO + state + -5;
	}

	return pStateConfig;
}

CInventoryInfo* CActorNativShop::GetInventoryInfoForPurchase(int index)
{
	CBehaviour* pBehaviour;
	CInventoryInfo* pInventoryInfo;
	NativShopSubObj* pSubObj;

	pBehaviour = GetBehaviour(NATIV_SHOP_BEHAVIOUR_SELL);
	if (pBehaviour == (CBehaviourNativShopSell*)0x0) {
		pInventoryInfo = (CInventoryInfo*)0x0;
	}
	else {
		if (((index < 0) || (4 < index)) || (pSubObj = this->behaviourNativShop.aSubObjs + index, pSubObj->field_0x14 == 0)) {
			pInventoryInfo = (CInventoryInfo*)0x0;
		}
		else {
			pInventoryInfo = pSubObj->streamRefActor.Get()->GetInventoryInfo();
		}
	}

	return pInventoryInfo;
}

void CBehaviourNativShopSell::Create(ByteCode* pByteCode)
{
	bool bVar1;
	CAudioManager* pCVar2;
	CCamera* pCVar3;
	CActor* pCVar4;
	uint uVar5;
	CMusic* pCVar6;
	ed_zone_3d* peVar7;
	int iVar8;
	NativShopSubObj* pNVar9;
	float fVar10;

	pNVar9 = this->aSubObjs;
	
	this->streamRefCamera.index = pByteCode->GetS32();
	iVar8 = 0;
	this->field_0x94 = 0;
	do {
		pNVar9->streamRefActor.index = pByteCode->GetS32();

		pNVar9->field_0x14 = (int)(pNVar9->streamRefActor).index < 0 ^ 1;
		if (pNVar9->field_0x14 != 0) {
			this->field_0x94 = iVar8;
		}

		iVar8 = iVar8 + 1;
		pNVar9 = pNVar9 + 1;
	} while (iVar8 < 5);

	this->aSubObjs[0].field_0x4 = 2;
	this->aSubObjs[0].field_0x8 = 1;
	this->aSubObjs[0].field_0xc = 2;
	this->aSubObjs[0].field_0x10 = 1;

	this->aSubObjs[1].field_0x4 = 4;
	this->aSubObjs[1].field_0x8 = 0;
	this->aSubObjs[1].field_0xc = 4;
	this->aSubObjs[1].field_0x10 = 0;

	this->aSubObjs[2].field_0x4 = 0;
	this->aSubObjs[2].field_0x8 = 4;
	this->aSubObjs[2].field_0xc = 0;

	this->aSubObjs[2].field_0x10 = 3;
	this->aSubObjs[3].field_0x4 = 3;
	this->aSubObjs[3].field_0x8 = 2;
	this->aSubObjs[3].field_0xc = 3;
	this->aSubObjs[3].field_0x10 = 4;

	this->aSubObjs[4].field_0x4 = 1;
	this->aSubObjs[4].field_0x8 = 3;
	this->aSubObjs[4].field_0xc = 1;
	this->aSubObjs[4].field_0x10 = 2;

	pCVar2 = CScene::ptable.g_AudioManager_00451698;

	this->pathFollowReader.Create(pByteCode);
	this->materialId = pByteCode->GetS32();

	uVar5 = pByteCode->GetU32();
	IMPLEMENTATION_GUARD_AUDIO(
	if ((uVar5 == 0xffffffff) || (bVar1 = (uint)pCVar2->nbMusic <= uVar5, bVar1)) {
		pCVar6 = (CMusic*)0x0;
	}
	else {
		if (bVar1) {
			uVar5 = 0;
		}
		pCVar6 = pCVar2->aMusic + uVar5;
	})

	this->field_0xb8 = pCVar6;
	(this->streamRefZone).index = pByteCode->GetS32();
	this->field_0xc4 = pByteCode->GetF32();
	this->field_0xc8 = pByteCode->GetF32();
	this->field_0xcc = pByteCode->GetF32();
	this->field_0xc0 = -1;

	this->field_0x98 = 1;
	this->field_0x9c = 0;
	return;
}

void CBehaviourNativShopSell::Init(CActor* pOwner)
{
	CActor* pReceiver;
	int iVar1;
	edF32VECTOR4* flags;
	NativShopSubObj* pCVar2;
	int iVar3;

	CScene::GetManager(MO_Frontend);

	this->pOwner = static_cast<CActorNativShop*>(pOwner);

	this->pathFollowReader.Init();

	(this->objData).pActor = (CActor*)0x0;
	(this->objData).pNode = (edNODE*)0x0;
	(this->objData).pHierarchy = (ed_3d_hierarchy*)0x0;
	(this->objData).pTextureInfo = (ed_Chunck*)0x0;

	CActor::SV_InstallMaterialId(this->materialId);

	if (-1 < this->streamRefCamera.index) {
		this->streamRefCamera.Init();
	}

	this->streamRefZone.Init();

	iVar3 = 0;
	pCVar2 = this->aSubObjs;
	do {
		pCVar2->streamRefActor.Init();

		pReceiver = pCVar2->streamRefActor.Get();
		if (pReceiver != (CActor*)0x0) {
			pReceiver->GetInventoryInfo();
			iVar1 = this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x53, this->pathFollowReader.GetWayPoint(iVar3 + 2));
		}
		iVar3 = iVar3 + 1;
		pCVar2 = pCVar2 + 1;
	} while (iVar3 < 5);

	return;
}

void CBehaviourNativShopSell::Term()
{
	edNODE* pNode;

	pNode = (this->objData).pNode;
	if (pNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
	}

	(this->objData).pActor = (CActor*)0x0;
	(this->objData).pNode = (edNODE*)0x0;
	(this->objData).pHierarchy = (ed_3d_hierarchy*)0x0;
	(this->objData).pTextureInfo = (ed_Chunck*)0x0;

	return;
}

edF32VECTOR4 Back_Unit = { 0.0f, 0.42f, 0.5f, 0.42f };

void CBehaviourNativShopSell::Manage()
{
	CActor* pCVar1;
	CActorNativShop* pCVar2;
	ed_zone_3d* pZone;
	CCameraManager* pCVar3;
	CInventoryInfo* pCVar4;
	int iVar5;
	int** ppiVar6;
	float fVar7;

	pCVar3 = CScene::ptable.g_CameraManager_0045167c;
	iVar5 = this->pOwner->actorState;

	if (iVar5 == 9) {
		pCVar1 = this->aSubObjs[this->field_0x94].streamRefActor.Get();
		pCVar4 = pCVar1->GetInventoryInfo();
		iVar5 = this->field_0xac;
		fVar7 = edFIntervalUnitSrcLERP(this->pOwner->timeInAir, this->field_0xa8, this->field_0xa8 - (float)pCVar4->moneyCost);
		this->field_0xac = (int)fVar7;

		if (iVar5 != this->field_0xac) {
			CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 4, 0);
		}

		pCVar2 = this->pOwner;
		if (1.0f <= pCVar2->timeInAir) {
			pCVar2->SetState(NATIV_SHOP_STATE_DISPLAY, -1);
		}
	}
	else {
		if (iVar5 == NATIV_SHOP_STATE_LEAVING) {
			State_Leaving();
		}
		else {
			if (iVar5 == NATIV_SHOP_STATE_DISPLAY) {
				State_Display();
			}
			else {
				if (iVar5 == NATIV_SHOP_STATE_ENTERING) {
					State_Entering();
				}
			}
		}
	}

	IMPLEMENTATION_GUARD_AUDIO(
	pZone = (this->streamRefZone).Get();
	ppiVar6 = (int**)(CScene::ptable.g_AudioManager_00451698)->field_0x38;
	if (pZone != (ed_zone_3d*)0x0) {
		iVar5 = edEventComputeZoneAgainstVertex
		((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone,
			&(CActorHero::_gThis->character).characterBase.base.base.currentLocation, 0);
		if ((iVar5 == 1) && (this->field_0xc0 == -1)) {
			ppiVar6 = FUN_00187170((int*)this->field_0xc4, (int*)&DAT_3f800000, (int)this->field_0xc8, (int*)0x0, ppiVar6,
				(int*)this->field_0xb8, (int*)0x19);
			this->field_0xc0 = (int)ppiVar6;
		}
		else {
			if ((iVar5 == 2) && (this->field_0xc0 != -1)) {
				MusicStop(this->field_0xcc, 0, (int)ppiVar6, this->field_0xc0);
				this->field_0xc0 = -1;
			}
		}
	})

	return;
}

void CBehaviourNativShopSell::SectorChange(int oldSectorId, int newSectorId)
{
	if (newSectorId == this->pOwner->objectId) {
		int iVar4 = 0;
		NativShopSubObj* pCVar2 = this->aSubObjs;
		NativShopSubObj* pCVar3 = this->aSubObjs;
		do {
			CActor* pReceiver = pCVar3->streamRefActor.Get();
			if (pReceiver != (CActor*)0x0) {
				this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x53, this->pathFollowReader.GetWayPoint(iVar4 + 2));

				uint local_8 = 5;
				if (pCVar3->field_0x14 == 0) {
					local_8 = 6;
				}

				this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x5f, (void*)local_8);
			}

			if (pCVar3->field_0x14 == 0) {
				pCVar2->field_0x14 = 0;
			}

			iVar4 = iVar4 + 1;
			pCVar3 = pCVar3 + 1;
			pCVar2 = pCVar2 + 1;
		} while (iVar4 < 5);
	}

	return;
}

void CBehaviourNativShopSell::PauseChange(int bPaused)
{
	IMPLEMENTATION_GUARD();
}

float gSHO_Cursor1OffsetXTime = 0.0f;

float gSHO_Cursor1OffsetX;

_rgba gCursorColorA = { 0x80, 0x80, 0x80, 0x80 };
_rgba gCursorColorB = { 0x0, 0x0, 0x0, 0x40 };

edF32MATRIX4 gShopLightDirection;
edF32MATRIX4 gShopLightColor;

edF32VECTOR4 gShopLightAmbient = { 0.0f, 255.0f, 0.0f, 128.0f };

ed_3D_Light_Config gShopLightConfig = { &gShopLightAmbient , &gShopLightDirection, &gShopLightColor };

float FLOAT_00435c40 = 0.19f;
float FLOAT_00435c44 = 0.19f;
float FLOAT_00435c48 = 8.0f;

float FLOAT_00448b80 = 0.9f;

float FLOAT_00435c80 = 0.215f;
float FLOAT_00435c84 = 0.61f;
float FLOAT_00435c88 = 0.34f;
float FLOAT_00435c8c = 0.25f;

float FLOAT_00435c90 = 0.625f;
float FLOAT_00435c94 = 0.61f;
float FLOAT_00435c98 = 0.1f;
float FLOAT_00435c9c = 0.1f;
float FLOAT_00435ca0 = 0.875f;
float FLOAT_00435ca4 = 0.63f;

float FLOAT_00435ca8 = 0.375f;
float FLOAT_00435cac = 0.0625f;

void CBehaviourNativShopSell::Draw()
{
	CCamera* pCVar1;
	edNODE* pNode;
	ed_3d_hierarchy* peVar2;
	ed_3D_Scene* pScene;
	bool bVar3;
	edCTextStyle* pNewFont;
	CInventoryInfo* pCVar4;
	Timer* pTVar5;
	edDList_material* peVar6;
	edF32VECTOR4* pPosition;
	char* pcVar7;
	ulong uVar8;
	CActor* pCVar9;
	float fVar10;
	edF32VECTOR4 local_100;
	Rectangle local_f0;
	float local_e0;
	undefined4 uStack220;
	undefined4 uStack216;
	undefined4 uStack212;
	edCTextStyle textStyle;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;
	CLanguageManager* pLanguageManager;

	pCVar1 = (this->streamRefCamera).Get();
	pCVar9 = (this->pOwner->actorRef).Get();

	if ((((((GameFlags & 0x20) != 0) || ((GameFlags & 0x1c) != 0)) || (this->pOwner->actorState == 5)) ||
		(((pCVar9 != (CActor*)0x0 && ((pCVar9->flags & 0x800000) != 0)) &&
			((pCVar1 != (CCamera*)0x0 && (((CScene::ptable.g_CameraManager_0045167c)->cameraStack).pActiveCamera != pCVar1))
				)))) || (pCVar1 == (CCamera*)0x0)) {
		pNode = (this->objData).pNode;
		if (pNode != (edNODE*)0x0) {
			ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
		}

		(this->objData).pActor = (CActor*)0x0;
		(this->objData).pNode = (edNODE*)0x0;
		(this->objData).pHierarchy = (ed_3d_hierarchy*)0x0;
		(this->objData).pTextureInfo = (ed_Chunck*)0x0;
		return;
	}


	textStyle.Reset();
	pNewFont = edTextStyleSetCurrent(&textStyle);

	pLanguageManager = CScene::ptable.g_LocalizationManager_00451678;
	pCVar9 = this->aSubObjs[this->field_0x94].streamRefActor.Get();
	pCVar4 = (CInventoryInfo*)0x0;
	textStyle.SetFont(BootDataFont, false);
	textStyle.SetScale(1.2f, 1.2f);
	textStyle.SetShadow(0x100);
	textStyle.alpha = 0xc0;
	if (pCVar9 != (CActor*)0x0) {
		pCVar4 = pCVar9->GetInventoryInfo();
	}

	bVar3 = GameDList_BeginCurrent();
	if (bVar3 != false) {
		if (this->aSubObjs[this->field_0x94].field_0x14 == 0) {
			gSHO_Cursor1OffsetX = 1.05f;
		}
		else {
			gSHO_Cursor1OffsetX = sinf(gSHO_Cursor1OffsetXTime) * 0.05f + 1.05f;
			gSHO_Cursor1OffsetX = cosf(gSHO_Cursor1OffsetXTime) * 0.05f + 1.05f;
			gSHO_Cursor1OffsetXTime = edF32Between_2Pi(gSHO_Cursor1OffsetXTime + GetTimer()->cutsceneDeltaTime * 9.424778f);
		}
		Display_Cursor(-0.25, gCursorColorA);
		Display_Cursor(-0.05, gCursorColorB);
		GameDList_EndCurrent();
	}

	bVar3 = Frontend2DDList_BeginCurrent();
	if (bVar3 != false) {
		Display_BottomBackGround();

		if (this->aSubObjs[this->field_0x94].field_0x14 == 0) {
			pCVar9 = (CActor*)0x0;
		}
		
		this->objData.Display_Object(pCVar9);
		
		if ((this->objData).pNode != (edNODE*)0x0) {
			local_8.x = (this->field_0xa4 + FLOAT_00435c40) * 2.0f + -1.0f;
			local_8.y = FLOAT_00435c44 * 2.0f + -1.0f;
		
			(this->objData).pHierarchy = (ed_3d_hierarchy*)((this->objData).pNode)->pData;
			(((this->objData).pHierarchy)->transformA).aa = (((this->objData).pActor)->scale).x;
			(((this->objData).pHierarchy)->transformA).bb = (((this->objData).pActor)->scale).y;
			(((this->objData).pHierarchy)->transformA).cc = (((this->objData).pActor)->scale).z;
		
			pCVar9 = (this->objData).pActor;
			if (pCVar9->pAnimationController != (CAnimation*)0x0) {
				((this->objData).pHierarchy)->pAnimMatrix = (pCVar9->p3DHierNode->base).pAnimMatrix;
			}
		
			bVar3 = ed3DComputeScreenCoordinate(FLOAT_00435c48, &local_100, &local_8, CFrontend::_scene_handle);
			if (bVar3 != false) {
				peVar2 = (this->objData).pHierarchy;
				(peVar2->transformA).rowT = local_100;
			}
		
			((this->objData).pHierarchy)->pHierarchySetup->pLightData = &gShopLightConfig;
		}

		FrontendDList_EndCurrent();
	}

	bVar3 = GuiDList_BeginCurrent();
	if (bVar3 == false) goto LAB_0036e618;
	if ((this->aSubObjs[this->field_0x94].field_0x14 != 0) && (1 < pCVar4->field_0x2c)) {
		peVar6 = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 4);

		pScene = CScene::_scene_handleA;
		if ((peVar6 != (edDList_material*)0x0) && (this->aSubObjs[this->field_0x94].field_0x14 != 0)) {
			pPosition = this->pathFollowReader.GetWayPoint(this->field_0x94 + 2);

			bVar3 = ed3DComputeSceneCoordinate(&local_10, pPosition, pScene);
			if (bVar3 != false) {
				Rectangle spriteRect;
				spriteRect.height = 0.09375f;
				spriteRect.width = 0.125f;
				spriteRect.x = local_10.x * 0.5f + 0.5f + -0.01f;
				spriteRect.y = -local_10.y * 0.5f + 0.5f + -0.01f;
				Display_Sprite(peVar6, &spriteRect, _rgba(0x80, 0x80, 0x80, 0x80));
			}
		}

		Display_BubbleText(&textStyle);
	}

	if (this->aSubObjs[this->field_0x94].field_0x14 != 0) {
		if (this->field_0xb0 != 0) {
			IMPLEMENTATION_GUARD(
			local_e0 = (float)_DAT_004259c0;
			uStack220 = (undefined4)((ulong)_DAT_004259c0 >> 0x20);
			uStack216 = DAT_004259c8;
			uStack212 = DAT_004259cc;
			pcVar7 = get_message(&gMessageManager, pCVar4->field_0x10);
			FUN_0036bcc0(this, (long)(int)pcVar7, &local_e0);)
		}

		if (pCVar4->objectId == 0x20) {
			uVar8 = 1;
		}
		else {
			uVar8 = CInventoryInfo::IsObjectPurchased(pCVar4->objectId);
		}

		if (uVar8 == 0) {
			pcVar7 = gMessageManager.get_message(pCVar4->field_0x18);
			fVar10 = this->field_0xb4;
			textStyle.SetEolAutomatic(0);
			textStyle.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.375f);
			textStyle.SetVerticalSize((float)gVideoConfig.screenHeight * 0.21875f);
			textStyle.SetHorizontalAlignment(2);
			textStyle.SetVerticalAlignment(4);
			textStyle.rgbaColour = (int)((1.0f - fVar10) * 255.0f) & 0xffU | 0xffffff00;
			textStyle.alpha = (int)((1.0f - this->field_0xb4) * 192.0f) & 0xff;
			textStyle.altColour = textStyle.rgbaColour;
			textStyle.SetScale(FLOAT_00448b80, FLOAT_00448b80);
			Display_Text(0.5f, 0.575f, &textStyle, pcVar7);
			textStyle.SetScale(1.2f, 1.2f);
			textStyle.alpha = 0xc0;
		}
		else {
			if (pCVar4->field_0x10 != 0) {
				fVar10 = this->field_0xb4;
				textStyle.SetScale(1.0f, 1.0f);
				textStyle.SetEolAutomatic(0);
				textStyle.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.375f);
				textStyle.SetVerticalSize((float)gVideoConfig.screenHeight * 0.21875f);
				textStyle.SetHorizontalAlignment(0);
				textStyle.SetVerticalAlignment(4);
				textStyle.rgbaColour = (int)((1.0f - fVar10) * 255.0f) & 0xffU | 0xffffff00;
				textStyle.alpha = (int)((1.0f - this->field_0xb4) * 192.0f) & 0xff;
				textStyle.altColour = textStyle.rgbaColour;
				pcVar7 = pLanguageManager->GetHelpString(0x414143524b5d550c);
				Display_Text(0.075f, 0.575f, &textStyle, pcVar7);
				textStyle.SetScale(1.2f, 1.2f);
				textStyle.alpha = 0xc0;
			}
		}
	}

	textStyle.SetHorizontalSize(FLOAT_00435ca8* (float)gVideoConfig.screenWidth);
	textStyle.SetVerticalSize(FLOAT_00435cac * (float)gVideoConfig.screenHeight);
	textStyle.rgbaColour = -0xff01;
	Display_Mts(-this->field_0xa4 + FLOAT_00435ca0, FLOAT_00435ca4, &textStyle, this->field_0xac);

	peVar6 = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 5);
	local_f0.x = -this->field_0xa4 + FLOAT_00435c90;
	local_f0.y = FLOAT_00435c94;
	local_f0.width = FLOAT_00435c98;
	local_f0.height = FLOAT_00435c9c;
	Display_Sprite(peVar6, &local_f0, _rgba(0x80, 0x80, 0x80, 0x80));
	textStyle.SetEolAutomatic(0);
	textStyle.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.375f);
	textStyle.SetVerticalSize((float)gVideoConfig.screenHeight * 0.21875f);
	textStyle.SetHorizontalAlignment(0);
	textStyle.SetVerticalAlignment(0);
	textStyle.altColour = 0xffffffff;
	textStyle.rgbaColour = 0xffff00ff;

	pcVar7 = pLanguageManager->GetHelpString(0xc0d17190b475549);
	Display_Text(0.675f - this->field_0xa4, 0.875f, &textStyle, pcVar7);

	if (this->aSubObjs[this->field_0x94].field_0x14 == 0) {
	LAB_0036e3f0:
		textStyle.rgbaColour = 0x808080ff;
		textStyle.altColour = 0x808080ff;
	}
	else {
		if (pCVar4->objectId == 0x20) {
			uVar8 = 1;
		}
		else {
			uVar8 = CInventoryInfo::IsObjectPurchased(pCVar4->objectId);
		}

		if (uVar8 == 0) goto LAB_0036e3f0;
	}

	pcVar7 = pLanguageManager->GetHelpString(0xc0a1a095f475549);
	Display_Text(0.675f - this->field_0xa4, 0.775f, &textStyle, pcVar7);
	if (this->aSubObjs[this->field_0x94].field_0x14 != 0) {
		textStyle.SetHorizontalSize(FLOAT_00435c88 * (float)gVideoConfig.screenWidth);
		textStyle.SetVerticalSize(FLOAT_00435c8c * (float)gVideoConfig.screenHeight);
		textStyle.SetHorizontalAlignment(2);
		textStyle.rgbaColour = -0xff01;
		pcVar7 = gMessageManager.get_message(pCVar4->field_0x8);
		Display_Text(this->field_0xa4 + FLOAT_00435c80, FLOAT_00435c84, &textStyle, pcVar7);
		textStyle.SetHorizontalAlignment(0);
		textStyle.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.34f);
		textStyle.SetVerticalSize((float)gVideoConfig.screenHeight * 0.09375f);
		textStyle.rgbaColour = -0xff01;
		pcVar7 = pLanguageManager->GetHelpString(0x45425e151218051b);
		Display_Text(this->field_0xa4 + 0.05f, 0.89f, &textStyle, pcVar7);

		if (CLevelScheduler::_gGameNfo.nbMoney < pCVar4->moneyCost) {
			textStyle.rgbaColour = 0xff4040ff;
		}
		else {
			textStyle.rgbaColour = 0x40ff40ff;
		}

		Display_Mts(this->field_0xa4 + 0.29f, 0.89f, &textStyle, pCVar4->moneyCost);
	}

	GuiDList_EndCurrent();

LAB_0036e618:
	edTextStyleSetCurrent(pNewFont);

	return;
}

void CBehaviourNativShopSell::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActor* pReceiver;
	uint uVar1;
	NativShopSubObj* pCVar2;
	NativShopSubObj* pCVar3;
	int iVar4;
	undefined4 local_20[5];
	undefined4* local_c;
	uint local_8;
	edF32VECTOR4* local_4;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	uVar1 = CLevelScheduler::ScenVar_Get(6);
	if (uVar1 < this->pOwner->field_0x170) {
		this->field_0x9c = 1;
	}

	iVar4 = 0;
	pCVar2 = this->aSubObjs;
	pCVar3 = this->aSubObjs;
	do {
		pReceiver = pCVar3->streamRefActor.Get();
		if (pReceiver != (CActor*)0x0) {
			this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x53, this->pathFollowReader.GetWayPoint(iVar4 + 2));

			local_8 = 5;
			if (pCVar3->field_0x14 == 0) {
				local_8 = 6;
			}

			this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x5f, (void*)local_8);
		}

		if (pCVar3->field_0x14 == 0) {
			pCVar2->field_0x14 = 0;
		}

		iVar4 = iVar4 + 1;
		pCVar3 = pCVar3 + 1;
		pCVar2 = pCVar2 + 1;
	} while (iVar4 < 5);

	local_20[0] = 4;
	this->pOwner->DoMessage((this->pOwner->actorRef).Get(), (ACTOR_MESSAGE)0x60, &local_20);

	UpdateValidity();

	this->field_0xc0 = -1;
	this->field_0xb0 = 0;
	this->field_0xb4 = 0.0f;

	return;
}

void CBehaviourNativShopSell::InitState(int newState)
{
	CCamera* pCameraView;
	undefined4 local_20[6];
	undefined4* local_8;
	undefined4 local_4;

	if (newState == 9) {
		this->field_0xac = CLevelScheduler::_gGameNfo.nbMoney;
		this->field_0xa8 = this->field_0xac;
	}
	else {
		if (newState == NATIV_SHOP_STATE_DISPLAY) {
			State_Display_Init();
		}
		else {
			if (newState == NATIV_SHOP_STATE_LEAVING) {
					State_Leaving_Init();
				}
			else {
				if (newState == NATIV_SHOP_STATE_ENTERING) {
					State_Entering_Init();
				}
			}
		}
	}
	return;
}

void CBehaviourNativShopSell::TermState(int oldState, int newState)
{
	edNODE* pNode;
	CActor* pCVar1;
	CLevelScheduler* pCVar2;
	CFrontendDisplay* pCVar3;
	CInventoryInfo* pCVar4;
	undefined4 local_20;
	int local_1c;
	undefined4 local_c;
	undefined4 local_8;
	undefined4* local_4;

	pCVar3 = CScene::ptable.g_FrontendManager_00451680;
	if (oldState == 9) {
		pCVar1 = this->aSubObjs[this->field_0x94].streamRefActor.Get();
		pCVar4 = pCVar1->GetInventoryInfo();
		local_4 = &local_20;
		local_20 = 3;
		local_1c = pCVar4->purchaseId;
		this->pOwner->DoMessage((this->pOwner->actorRef).Get(), (ACTOR_MESSAGE)0x60, &local_20);
		pCVar2 = CLevelScheduler::gThis;
		pCVar1 = this->aSubObjs[this->field_0x94].streamRefActor.Get();
		pCVar4 = pCVar1->GetInventoryInfo();
		pCVar2->Money_GiveToShop(pCVar4->moneyCost);
		local_8 = 6;
		this->pOwner->DoMessage(pCVar1, (ACTOR_MESSAGE)0x5f, (void*)6);
		local_c = 7;
		this->pOwner->DoMessage(pCVar1, (ACTOR_MESSAGE)0x5f, (void*)7);
		this->aSubObjs[this->field_0x94].field_0x14 = 0;

		UpdateValidity();
	}
	else {
		if ((oldState != NATIV_SHOP_STATE_DISPLAY) && (oldState == 8)) {
			(CScene::ptable.g_FrontendManager_00451680)->pMagicOrbs->Magic_SetDisplay(true);
			pCVar3->pMoney->field_0x74 = 1;
			pCVar3->pFreedWolfun->field_0x70 = 1;
			pNode = (this->objData).pNode;
			if (pNode != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
			}
			(this->objData).pActor = (CActor*)0x0;
			(this->objData).pNode = (edNODE*)0x0;
			(this->objData).pHierarchy = (ed_3d_hierarchy*)0x0;
			(this->objData).pTextureInfo = (ed_Chunck*)0x0;

			GameFlags = GameFlags & 0xffff7eff;
		}
	}
	return;
}

int CBehaviourNativShopSell::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	float fVar1;
	float fVar2;
	int iVar3;

	if (this->field_0x9c == 0) {
		if (msg == 0x15) {
			this->pOwner->SetState(5, -1);
			iVar3 = 1;
		}
		else {
			if (msg == 0x14) {
				if (reinterpret_cast<int>(pMsgParam) == 1) {
					this->pOwner->SetState(NATIV_SHOP_STATE_LEAVING, -1);
				}
				else {
					this->pOwner->SetState(NATIV_SHOP_STATE_ENTERING, -1);
					this->pOwner->field_0x17c = 1;
				}

				iVar3 = 1;
			}
			else {
				if (msg == 0x13) {
					iVar3 = 1;
				}
				else {
					if (((msg != 0x12) ||
						(fVar1 = (pSender->currentLocation).x - this->pOwner->currentLocation.x, fVar2 = (pSender->currentLocation).z - this->pOwner->currentLocation.z,
							this->pOwner->field_0x164 <= fVar1 * fVar1 + fVar2 * fVar2)) || (iVar3 = 0xe, 3.0f <= fabs(this->pOwner->currentLocation.y - (pSender->currentLocation).y))) {
						iVar3 = 0;
					}
				}
			}
		}
	}
	else {
		iVar3 = 0;
	}

	return iVar3;
}

void CBehaviourNativShopSell::UpdateValidity()
{
	CActor* pCVar1;
	S_ACTOR_STREAM_REF* pSVar2;
	bool bVar3;
	CInventoryInfo* pCVar4;
	int puVar5;
	int iVar6;
	NativShopSubObj* pCVar7;
	int iVar8;
	int iVar9;
	undefined4 local_20[5];
	undefined4* local_c;
	undefined4 local_8;
	undefined4 local_4;
	CActorNativShop* pShop;

	bVar3 = false;
	iVar9 = 0;
	this->field_0x98 = 0;
	pCVar7 = this->aSubObjs;
	do {
		this->field_0x98 = this->field_0x98 | pCVar7->field_0x14;
		if ((pCVar7->field_0x14 != 0) && (pCVar1 = pCVar7->streamRefActor.Get(), pCVar4 = pCVar1->GetInventoryInfo(), pCVar4->purchaseId < 0x15)) {
			bVar3 = true;
		}

		iVar9 = iVar9 + 1;
		pCVar7 = pCVar7 + 1;
	} while (iVar9 < 5);

	if ((this->field_0x9c == 0) && (iVar9 = 0, bVar3)) {
		while (true) {
			pShop = this->pOwner;

			if (pShop->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
				puVar5 = 0;
			}
			else {
				puVar5 = pShop->pActorStream->entryCount;
			}

			if (puVar5 <= iVar9) break;

			local_4 = 0;
			pShop->DoMessage(pShop->pActorStream->aEntries[iVar9].Get(), MESSAGE_LEAVE_SHOP, 0);
			local_8 = 0;
			this->pOwner->DoMessage(this->pOwner->pActorStream->aEntries[iVar9].Get(), (ACTOR_MESSAGE)0x51, 0);
			iVar9 = iVar9 + 1;
		}
	}
	else {
		iVar8 = 0;
		while (true) {
			pSVar2 = this->pOwner->pActorStream;
			if (pSVar2 == (S_ACTOR_STREAM_REF*)0x0) {
				iVar6 = 0;
			}
			else {
				iVar6 = pSVar2->entryCount;
			}

			if (iVar6 <= iVar8) break;

			local_20[0] = 1;
			local_c = local_20;
			this->pOwner->DoMessage(this->pOwner->pActorStream->aEntries[iVar8].Get(), (ACTOR_MESSAGE)0x23, &local_20);
			iVar8 = iVar8 + 1;
		}
	}
	return;
}

void CBehaviourNativShopSell::State_Entering_Init()
{
	edNODE* pNode;
	CCamera* pCamera;
	CFrontendDisplay* pCVar1;
	_msg_enter_shop local_20;

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		local_20.field_0x0 = 0;
		local_20.field_0x4 = 0;
		local_20.field_0x8 = 1;
		local_20.field_0xc = 1;
		this->pOwner->DoMessage(CActorHero::_gThis, MESSAGE_ENTER_SHOP, &local_20);
	}

	pNode = (this->objData).pNode;
	if (pNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
	}

	(this->objData).pActor = (CActor*)0x0;
	(this->objData).pNode = (edNODE*)0x0;
	(this->objData).pHierarchy = (ed_3d_hierarchy*)0x0;
	(this->objData).pTextureInfo = (ed_Chunck*)0x0;

	pCamera = (this->streamRefCamera).Get();
	if (pCamera != (CCamera*)0x0) {
		CScene::ptable.g_CameraManager_0045167c->PushCamera(pCamera, 1);
	}

	pCVar1 = CScene::ptable.g_FrontendManager_00451680;
	(CScene::ptable.g_FrontendManager_00451680)->pMagicOrbs->Magic_SetDisplay(false);
	pCVar1->pMoney->field_0x74 = 0;
	pCVar1->pFreedWolfun->field_0x70 = 0.0f;
	GameFlags = GameFlags | 0x8100;
	this->field_0xac = CLevelScheduler::_gGameNfo.nbMoney;

	return;
}


void CBehaviourNativShopSell::State_Entering()
{
	CCameraManager* pCVar1;
	edF32VECTOR4 eStack16;
	CActorHero* pHero;

	pCVar1 = CScene::ptable.g_CameraManager_0045167c;
	this->field_0xa4 = 0.0f;

	pHero = CActorHero::_gThis;
	if (CActorHero::_gThis != (CActorHero*)0x0) {
		edF32Vector4SubHard(&eStack16, &gF32Vector4Zero, &this->pOwner->rotationQuat);
		pHero->SV_UpdateOrientation2D(3.926991f, &eStack16, 0);
		edF32Vector4SubHard(&eStack16, &this->pOwner->currentLocation, &pHero->currentLocation);
		edF32Vector4NormalizeHard(&eStack16, &eStack16);
		edF32Vector4ScaleHard(GetTimer()->cutsceneDeltaTime, &eStack16, &eStack16);
		edF32Vector4AddHard(&eStack16, &pHero->currentLocation, &eStack16);
		pHero->UpdatePosition(&eStack16, true);
	}

	if ((pCVar1->flags & 0x4000000) == 0) {
		this->pOwner->SetState(NATIV_SHOP_STATE_DISPLAY, -1);
	}
	else {
		this->field_0xa4 = -Back_Unit.z * (1.0f - pCVar1->activeCamManager.GetInternalViewAlpha());
	}

	return;
}


void CBehaviourNativShopSell::State_Display_Init()
{
	uint local_20;
	local_20 = 0;
	this->pOwner->DoMessage((this->pOwner->actorRef).Get(), (ACTOR_MESSAGE)0x60, &local_20);
	this->field_0xb0 = 0;
	this->field_0xb4 = 0.0f;

	return;
}

void CBehaviourNativShopSell::State_Display()
{
	CActor* pReceiver;
	CPlayerInput* pCVar1;
	int iVar2;
	ulong uVar4;
	NativShopSubObj* puVar5;
	undefined4 local_30[4];
	undefined4 local_20[6];
	undefined4* local_8;
	undefined4* local_4;

	pCVar1 = CActorHero::_gThis->GetInputManager(0, 0);
	if (pCVar1 == (CPlayerInput*)0x0) {
		this->pOwner->field_0x168 = 0;
		this->pOwner->SetState(NATIV_SHOP_STATE_LEAVING, -1);
	}

	if (this->field_0xb0 == 0) {
		if ((((pCVar1->pressedBitfield & 0x100000) != 0) || ((pCVar1->pressedBitfield & 4) != 0)) &&
			(iVar2 = this->aSubObjs[this->field_0x94].field_0x4, this->field_0x94 != iVar2)) {
			this->field_0x94 = iVar2;
			CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 1, 0);
		}

		if ((((pCVar1->pressedBitfield & 0x200000) != 0) || ((pCVar1->pressedBitfield & 8) != 0)) &&
			(iVar2 = this->aSubObjs[this->field_0x94].field_0xc, this->field_0x94 != iVar2)) {
			this->field_0x94 = iVar2;
			CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 1, 0);
		}

		if ((((pCVar1->pressedBitfield & 0x400000) != 0) || ((pCVar1->pressedBitfield & 1) != 0)) &&
			(iVar2 = this->aSubObjs[this->field_0x94].field_0x8, this->field_0x94 != iVar2)) {
			this->field_0x94 = iVar2;
			CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 1, 0);
		}

		if ((((pCVar1->pressedBitfield & 0x800000) != 0) || ((pCVar1->pressedBitfield & 2) != 0)) &&
			(iVar2 = this->aSubObjs[this->field_0x94].field_0x10, this->field_0x94 != iVar2)) {
			this->field_0x94 = iVar2;
			CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 1, 0);
		}
	}

	if ((this->field_0x98 == 0) || (puVar5 = this->aSubObjs + this->field_0x94, puVar5->field_0x14 == 0))
		goto LAB_0036d578;

	CInventoryInfo* pInventoryInfo = puVar5->streamRefActor.Get()->GetInventoryInfo();

	pReceiver = (this->pOwner->actorRef).Get();
	if ((pCVar1->pressedBitfield & 0x1000000) != 0) {
		if (pInventoryInfo->moneyCost <= CLevelScheduler::_gGameNfo.nbMoney) {
			uVar4 = 1;
			if (pInventoryInfo->objectId != 0x20) {
				uVar4 = CInventoryInfo::IsObjectPurchased(pInventoryInfo->objectId);
			}
			if (uVar4 != 0) {
				this->pOwner->SetState(9, -1);
				CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 0, 0);
				goto LAB_0036d528;
			}
		}
		local_4 = local_20;
		local_20[0] = 2;
		this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x60, &local_20);
		this->pOwner->field_0x168 = GetTimer()->scaledTotalTime;

		CFrontendSamplePlayer::PlaySample(1.0f, (float*)(CScene::ptable.g_FrontendManager_00451680)->field_0x78, 3, 0);
	}

LAB_0036d528:
	if (((pCVar1->pressedBitfield & 0x80000) != 0) && (pInventoryInfo->field_0x10 != 0)) {
		if ((this->field_0xb0 == 0) || (this->field_0xb0 == 3)) {
			this->field_0xb0 = 1;
		}
		else {
			this->field_0xb0 = 3;
		}
	}

LAB_0036d578:
	if ((pCVar1->pressedBitfield & PAD_BITMASK_TRIANGLE) != 0) {
		local_8 = local_30;
		local_30[0] = 1;
		this->pOwner->DoMessage((this->pOwner->actorRef).Get(), (ACTOR_MESSAGE)0x60, &local_30);
		this->pOwner->field_0x168 = 0;
		this->pOwner->SetState(NATIV_SHOP_STATE_LEAVING, -1);
	}

	return;
}

void CBehaviourNativShopSell::State_Leaving_Init()
{
	if (CActorHero::_gThis != (CActorHero*)0x0) {
		uint local_4 = 0;
		this->pOwner->DoMessage(CActorHero::_gThis, (ACTOR_MESSAGE)0x24, 0);
	}

	CCamera* pCameraView = (this->streamRefCamera).Get();
	if (pCameraView != (CCamera*)0x0) {
		CScene::ptable.g_CameraManager_0045167c->PopCamera(pCameraView);
	}

	this->field_0xb0 = 0;
	this->field_0xb4 = 0.0f;

	return;
}

void CBehaviourNativShopSell::State_Leaving()
{
	CCameraManager* pCVar3 = CScene::ptable.g_CameraManager_0045167c;

	this->field_0xa4 = 1.0f;
	if ((pCVar3->flags & 0x4000000) == 0) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->field_0xa4 = -Back_Unit.z * pCVar3->activeCamManager.GetInternalViewAlpha();
	}

	return;
}

void CBehaviourNativShopSell::Display_Cursor(float scale, _rgba color)
{
	CCameraManager* pCameraManager;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	edDList_material* peVar4;
	edF32VECTOR4* pRowY;
	float fVar5;
	float fVar6;
	float y;
	float fVar7;
	byte local_54;
	byte bStack83;
	byte bStack82;
	byte bStack81;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 auStack64;
	C3DFileManager* pFileManager;

	pCameraManager = CScene::ptable.g_CameraManager_0045167c;
	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	pRowY = &auStack64.rowY;
	fVar7 = gSHO_Cursor1OffsetX * 0.75f;

	peVar2 = this->pathFollowReader.GetWayPoint(2);
	peVar3 = this->pathFollowReader.GetWayPoint(4);
	edF32Vector4SubHard(pRowY, peVar2, peVar3);
	edF32Vector4NormalizeHard(pRowY, pRowY);
	peVar2 = this->pathFollowReader.GetWayPoint(3);
	peVar3 = this->pathFollowReader.GetWayPoint(2);
	edF32Vector4SubHard(&auStack64.rowX, peVar2, peVar3);
	edF32Vector4NormalizeHard(&auStack64.rowX, &auStack64.rowX);
	edF32Vector4CrossProductHard(&auStack64.rowZ, pRowY, &auStack64.rowX);
	edF32Vector4ScaleHard(scale, &eStack80, &auStack64.rowZ);
	peVar2 = this->pathFollowReader.GetWayPoint(this->field_0x94 + 2);
	edF32Vector4AddHard(&auStack64.rowT, peVar2, &eStack80);
	edDListLoadMatrix(&auStack64);
	peVar4 = pFileManager->GetMaterialFromId(this->materialId, 3);

	local_54 = color.r;
	bStack83 = color.b;
	bStack82 = color.g;
	bStack81 = color.a;

	if (peVar4 != (edDList_material*)0x0) {
		edDListUseMaterial(peVar4);
		edDListBegin(0.0f, 0.0f, 0.0f, 8, 0x10);
		edDListColor4u8(local_54, bStack83, bStack82, bStack81);
		edDListTexCoo2f(1.0f, 0.0f);
		fVar5 = -fVar7;
		fVar6 = fVar5 + 0.225f;
		edDListVertex4f(-0.3f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(0.3f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(-0.3f, fVar5 - 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(0.3f, fVar5 - 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(-0.3f, fVar7 + 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(0.3f, fVar7 + 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(-0.3f, fVar7 - 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(0.3f, fVar7 - 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(-gSHO_Cursor1OffsetX - 0.3f, 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(0.3f - gSHO_Cursor1OffsetX, 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(-gSHO_Cursor1OffsetX - 0.3f, -0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(0.3f - gSHO_Cursor1OffsetX, -0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(gSHO_Cursor1OffsetX - 0.3f, 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(gSHO_Cursor1OffsetX + 0.3f, 0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(gSHO_Cursor1OffsetX - 0.3f, -0.225f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(gSHO_Cursor1OffsetX + 0.3f, -0.225f, 0.0f, 0.0f);
		edDListEnd();
	}

	fVar5 = 0.8f / pCameraManager->aspectRatio;
	fVar7 = 0.3f / pCameraManager->aspectRatio;

	peVar4 = pFileManager->GetMaterialFromId(this->materialId, 2);
	if (peVar4 != (edDList_material*)0x0) {
		edDListUseMaterial(peVar4);
		edDListBegin(0.0f, 0.0f, 0.0f, 8, 0x10);
		edDListColor4u8(local_54, bStack83, bStack82, bStack81);
		edDListTexCoo2f(0.0f, 0.0f);
		fVar6 = fVar5 + fVar7;
		edDListVertex4f(-1.1f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(-0.5f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		y = fVar5 - fVar7;
		edDListVertex4f(-1.1f, y, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(-0.5f, y, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(0.5f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(1.1f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(0.5f, y, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(1.1f, y, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		fVar6 = -fVar5 + fVar7;
		edDListVertex4f(-1.1f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(-0.5f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		fVar7 = -fVar5 - fVar7;
		edDListVertex4f(-1.1f, fVar7, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(-0.5f, fVar7, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f(0.5f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f(1.1f, fVar6, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f(0.5f, fVar7, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f(1.1f, fVar7, 0.0f, 0.0f);
		edDListEnd();
	}

	return;
}

float zCoord = 20.0f;
_rgba BG_Color = { 0x80, 0x80, 0x80, 0x80 };
edF32VECTOR4 SepL_Unit = { 0.0f, 0.325f, 0.4f, 0.04f };
edF32VECTOR4 SepR_Unit = { 0.0f, 0.27f, 0.37f, 0.04f };

void CBehaviourNativShopSell::Display_BottomBackGround()
{
	bool bVar1;
	bool bVar2;
	edDList_material* pMaterial;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->materialId, 1);

	if (pMaterial != (edDList_material*)0x0) {
		edDListLoadIdentity();
		edDListUseMaterial(pMaterial);

		local_8.x = (this->field_0xa4 + Back_Unit.x) * 2.0f + -1.0f;
		local_8.y = Back_Unit.y * 2.0f + -1.0f;
		bVar1 = ed3DComputeScreenCoordinate(zCoord, &local_20, &local_8, CFrontend::_scene_handle);

		local_10.x = local_8.x + Back_Unit.z * 2.0f;
		local_10.y = local_8.y - Back_Unit.w * 2.0f;
		bVar2 = ed3DComputeScreenCoordinate(zCoord, &local_30, &local_10, CFrontend::_scene_handle);

		if ((bVar1 & bVar2) != 0) {
			edDListBegin(0.0f, 0.0f, 0.0f, 8, 8);
			edDListColor4u8(BG_Color.r, BG_Color.g, BG_Color.b, BG_Color.a);
			edDListTexCoo2f(0.0f, 0.0f);
			edDListVertex4f(local_20.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(1.0f, 0.0f);
			edDListVertex4f(local_30.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(0.0f, 1.0f);
			edDListVertex4f(local_20.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(1.0f, 1.0f);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			local_8.x = -local_8.x;
			ed3DComputeScreenCoordinate(zCoord, &local_20, &local_8, CFrontend::_scene_handle);
			local_10.x = -local_10.x;
			ed3DComputeScreenCoordinate(zCoord, &local_30, &local_10, CFrontend::_scene_handle);
			edDListTexCoo2f(0.0f, 0.0f);
			edDListVertex4f(local_20.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(1.0f, 0.0f);
			edDListVertex4f(local_30.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(0.0f, 1.0f);
			edDListVertex4f(local_20.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(1.0f, 1.0f);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			edDListEnd();
		}
	}

	pMaterial = pFileManager->GetMaterialFromId(this->materialId, 0);
	if (pMaterial != (edDList_material*)0x0) {
		edDListLoadIdentity();
		edDListUseMaterial(pMaterial);

		local_8.x = (this->field_0xa4 + SepL_Unit.x) * 2.0f + -1.0f;
		local_8.y = SepL_Unit.y * 2.0f + -1.0f;
		bVar1 = ed3DComputeScreenCoordinate(zCoord - 0.5f, &local_20, &local_8, CFrontend::_scene_handle);

		local_10.x = local_8.x + SepL_Unit.z * 2.0f;
		local_10.y = local_8.y - SepL_Unit.w * 2.0f;
		bVar2 = ed3DComputeScreenCoordinate(zCoord - 0.5f, &local_30, &local_10, CFrontend::_scene_handle);

		if ((bVar1 & bVar2) != 0) {
			edDListBegin(0.0f, 0.0f, 0.0f, 8, 8);
			edDListColor4u8(BG_Color.r, BG_Color.g, BG_Color.b, BG_Color.a);
			edDListTexCoo2f(0.0f, 0.0f);
			edDListVertex4f(local_20.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(1.0f, 0.0f);
			edDListVertex4f(local_30.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(0.0f, 1.0f);
			edDListVertex4f(local_20.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(1.0f, 1.0f);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			local_8.y = SepR_Unit.y * 2.0f + -1.0f;
			local_8.x = -local_8.x;
			ed3DComputeScreenCoordinate(zCoord - 0.5f, &local_20, &local_8, CFrontend::_scene_handle);
			local_10.x = local_8.x - SepR_Unit.z * 2.0f;
			local_10.y = local_8.y - SepR_Unit.w * 2.0f;
			ed3DComputeScreenCoordinate(zCoord - 0.5f, &local_30, &local_10, CFrontend::_scene_handle);
			edDListTexCoo2f(0.0f, 0.0f);
			edDListVertex4f(local_20.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(1.0f, 0.0f);
			edDListVertex4f(local_30.x, local_20.y, local_20.z, 0.0f);
			edDListTexCoo2f(0.0f, 1.0f);
			edDListVertex4f(local_20.x, local_30.y, local_30.z, 0.0f);
			edDListTexCoo2f(1.0f, 1.0f);
			edDListVertex4f(local_30.x, local_30.y, local_30.z, 0.0f);
			edDListEnd();
		}
	}

	return;
}

void CBehaviourNativShopSell::Display_Sprite(edDList_material* pMaterial, Rectangle* pRect, _rgba color)
{
	undefined4* puVar1;
	undefined4* puVar2;
	undefined4* puVar3;
	float y;
	float y_00;
	float x;
	float x_00;
	_rgba local_4 = {};

	local_4 = color;
	edDListLoadIdentity();
	edDListUseMaterial(pMaterial);

	x = pRect->x * (float)gVideoConfig.screenWidth;
	y = pRect->y * (float)gVideoConfig.screenHeight;
	x_00 = x + pRect->width * (float)gVideoConfig.screenWidth;
	y_00 = y + pRect->height * (float)gVideoConfig.screenHeight;

	edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
	edDListColor4u8(local_4.r, local_4.g, local_4.b, local_4.a);
	edDListTexCoo2f(0.0f, 0.0f);
	edDListVertex4f(x, y, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 0.0f);
	edDListVertex4f(x_00, y, 0.0f, 0.0f);
	edDListTexCoo2f(0.0f, 1.0f);
	edDListVertex4f(x, y_00, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	edDListVertex4f(x_00, y_00, 0.0f, 0.0f);
	edDListEnd();

	return;
}

void CBehaviourNativShopSell::Display_BubbleText(edCTextStyle* pStyle)
{
	CActor* pCVar1;
	bool bVar2;
	edF32VECTOR4* pPosition;
	edF32VECTOR2 local_8;

	pCVar1 = this->aSubObjs[this->field_0x94].streamRefActor.Get();
	CInventoryInfo* pInventoryInfo = pCVar1->GetInventoryInfo();

	pPosition = this->pathFollowReader.GetWayPoint(this->field_0x94 + 2);

	bVar2 = ed3DComputeSceneCoordinate(&local_8, pPosition, CScene::_scene_handleA);
	if (bVar2 != false) {
		pStyle->SetEolAutomatic(0);
		pStyle->SetHorizontalSize(FLOAT_00435c88 * (float)gVideoConfig.screenWidth);
		pStyle->SetVerticalSize(FLOAT_00435c8c * (float)gVideoConfig.screenHeight);
		pStyle->SetHorizontalAlignment(2);
		pStyle->SetVerticalAlignment(0);
		pStyle->rgbaColour = -0xff01;
		edCTextFormat auStack5408;
		local_8.x = (float)gVideoConfig.screenWidth * (local_8.x * 0.5f + 0.5f + 0.05f);
		local_8.y = (float)gVideoConfig.screenHeight * (-local_8.y * 0.5f + 0.5f + 0.0025f);
		auStack5408.FormatString("x%u", pInventoryInfo->field_0x2c);
		auStack5408.Display(local_8.x, local_8.y);
	}

	return;
}

void CBehaviourNativShopSell::Display_Text(float param_1, float param_2, edCTextStyle* pStyle, char* pText)
{
	edCTextFormat textFormat;
	textFormat.FormatString(pText);
	textFormat.Display(param_1 * (float)gVideoConfig.screenWidth, param_2 * (float)gVideoConfig.screenHeight);
	return;
}


void CBehaviourNativShopSell::Display_Mts(float param_1, float param_2, edCTextStyle* pStyle, uint param_5)
{
	edDList_material* pMaterial;
	Rectangle local_1530;
	Rectangle local_1520;
	edCTextFormat auStack5392;

	pMaterial = &((CScene::ptable.g_FrontendManager_00451680)->pMoney->sprite).materialInfo;
	local_1520.width = 0.06f;
	local_1520.height = 0.06f;
	local_1520.x = param_1;
	local_1520.y = param_2;
	pStyle->SetEolAutomatic(0);
	pStyle->SetVerticalAlignment(0);
	pStyle->SetHorizontalAlignment(1);
	auStack5392.FormatString("%u ", param_5);
	auStack5392.Display(param_1 * (float)gVideoConfig.screenWidth, param_2 * (float)gVideoConfig.screenHeight);
	local_1520.y = local_1520.y + 0.005f;
	local_1530.x = local_1520.x + 0.005f;
	local_1530.width = local_1520.width;
	local_1530.height = local_1520.height;
	local_1530.y = local_1520.y + 0.005f;

	Display_Sprite(pMaterial, &local_1530, _rgba(0x80, 0x80, 0x80, 0x80));
	Display_Sprite(pMaterial, &local_1520, _rgba(0x80, 0x80, 0x80, 0x80));

	return;
}



void ShopObjData::Display_Object(CActor* pActor)
{
	ed_Chunck* pcVar1;
	CinNamedObject30* pCVar2;
	ed_g3d_manager* pG3D;
	edNODE* peVar3;

	if (pActor == (CActor*)0x0) {
		if (this->pNode != (edNODE*)0x0) {
			ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, this->pNode);
		}

		this->pActor = (CActor*)0x0;
		this->pNode = (edNODE*)0x0;
		this->pHierarchy = (ed_3d_hierarchy*)0x0;
		this->pTextureInfo = (ed_Chunck*)0x0;
	}
	else {
		pcVar1 = (pActor->p3DHierNode->base).pTextureInfo;

		if (pcVar1 != this->pTextureInfo) {
			pCVar2 = pActor->pCinData;
			if (pCVar2->textureIndex == -1) {
				pG3D = CScene::ptable.g_C3DFileManager_00451664->GetCommonLevelMeshInfo(pCVar2->meshIndex);
				peVar3 = this->pNode;
			}
			else {
				pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(pCVar2->meshIndex, pCVar2->textureIndex);
				peVar3 = this->pNode;
			}

			if (peVar3 != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, peVar3);
			}

			this->pActor = (CActor*)0x0;
			this->pNode = (edNODE*)0x0;
			this->pHierarchy = (ed_3d_hierarchy*)0x0;
			this->pTextureInfo = (ed_Chunck*)0x0;
			this->pActor = pActor;
			this->pTextureInfo = pcVar1;
			peVar3 = ed3DHierarchyAddToScene(CFrontend::_scene_handle, pG3D, (char*)0x0);
			this->pNode = peVar3;
			this->pHierarchy = (ed_3d_hierarchy*)this->pNode->pData;
			ed3DHierarchySetSetup(this->pHierarchy, &pActor->hierarchySetup);
			this->pHierarchy->pTextureInfo = this->pTextureInfo;

			gShopLightDirection.aa = 0.0f;
			gShopLightDirection.ab = 0.0f;
			gShopLightDirection.ac = 1.0f;
			gShopLightDirection.ad = 0.0f;

			gShopLightDirection.ba = 0.0f;
			gShopLightDirection.bb = 1.0f;
			gShopLightDirection.bc = 0.0f;
			gShopLightDirection.bd = 0.0f;

			gShopLightDirection.ca = 1.0f;
			gShopLightDirection.cb = 0.0f;
			gShopLightDirection.cc = 0.0f;
			gShopLightDirection.cd = 0.0f;

			gShopLightDirection.da = 0.0f;
			gShopLightDirection.db = 1.0f;
			gShopLightDirection.dc = 1.0f;
			gShopLightDirection.dd = 0.0f;

			gShopLightColor.aa = 255.0f;
			gShopLightColor.ab = 255.0f;
			gShopLightColor.ac = 255.0f;
			gShopLightColor.ad = 255.0f;

			gShopLightColor.ba = 0.0f;
			gShopLightColor.bb = 0.0f;
			gShopLightColor.bc = 255.0f;
			gShopLightColor.bd = 255.0f;

			gShopLightColor.ca = 0.0f;
			gShopLightColor.cb = 255.0f;
			gShopLightColor.cc = 0.0f;
			gShopLightColor.cd = 255.0f;

			gShopLightColor.da = 255.0f;
			gShopLightColor.dd = 255.0f;
			gShopLightColor.db = 0.0f;
			gShopLightColor.dc = 0.0f;
		}
	}

	return;
}
