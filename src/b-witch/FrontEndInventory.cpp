#include "FrontEndInventory.h"
#include "camera.h"
#include "CameraViewManager.h"
#include "MathOps.h"
#include "BootData.h"
#include "FrontendBank.h"
#include "FrontendDisp.h"
#include "Frontend.h"
#include "ActorHero_Inventory.h"
#include "DlistManager.h"
#include "Kya.h"
#include "FileManager3D.h"
#include "InventoryInfo.h"
#include "TimeController.h"

edF32VECTOR2 INV_POS_DRAW_ON{ 0.8f, 0.05f };
edF32VECTOR2 INV_POS_DRAW_OFF{ 1.25f, -0.2f };
edF32VECTOR2 INV_SCALE{ 0.05f, 0.05f };

CFrontendInventory::CFrontendInventory()
{
	this->avatar.instance3d.pNode = (edNODE*)0x0;
	this->popupTime = 0.15f;
	this->field_0x50 = 0.15f;

	(this->slotOn).position = INV_POS_DRAW_ON;
	(this->slotOn).scale = INV_SCALE;

	(this->slotOff).position = INV_POS_DRAW_OFF;
	(this->slotOff).scale = INV_SCALE;
}

edF32VECTOR2 vCells_Offset = { 0.0f, 0.08f };
edF32VECTOR2 edF32VECTOR2_00437618 = { 0.04f, 0.03125f };
edF32VECTOR2 edF32VECTOR2_00437620 = { 0.0f, -0.12f };

void CFrontendInventory::Reset()
{
	float fVar1;

	CWidget::Reset();

	this->field_0x4c = 3;
	this->slotAlpha = 0.0f;

	this->widgetSlotC.position = (this->slotOn).position;
	this->widgetSlotC.scale = (this->slotOn).scale;
	this->widgetSlotB.position = (this->slotOn).position;
	this->widgetSlotB.scale = (this->slotOn).scale;

	this->bDisplayDirty = 1;

	this->aInvColumns[0].field_0x0 = 0;
	this->aInvColumns[0].field_0x4 = 0;
	this->aInvColumns[0].field_0x930.x = vCells_Offset.x;
	this->aInvColumns[0].field_0x930.y = vCells_Offset.y;
	this->aInvColumns[0].field_0x938.x = edF32VECTOR2_00437620.x;
	this->aInvColumns[0].field_0x938.y = edF32VECTOR2_00437620.y;

	this->aInvColumns[1].field_0x0 = 0;
	this->aInvColumns[1].field_0x4 = 0;
	this->aInvColumns[1].field_0x930.x = vCells_Offset.x;
	this->aInvColumns[1].field_0x930.y = vCells_Offset.y;
	this->aInvColumns[1].field_0x938.x = edF32VECTOR2_00437620.x;
	this->aInvColumns[1].field_0x938.y = edF32VECTOR2_00437620.y;

	this->field_0x1490 = 0;

	return;
}

void CFrontendInventory::Init()
{
	char* pcVar1;
	_FE_INV_ENTRY* pInvEntry;
	FE_INV_COLUMN* pInvColumn;
	int iVar4;
	int iVar5;
	CFrontendBank* pFrontendBank;

	CWidget::Init();

	pFrontendBank = CScene::ptable.g_FrontEndBank_00451674;
	iVar5 = 0;
	pInvColumn = this->aInvColumns;
	CSprite3D* pSprite = this->aSprites3d;
	do {
		pcVar1 = pFrontendBank->GetResource(MenuElementsBitmapNames[4], (edBANK_ENTRY_INFO*)0x0); // inv_select_01_p0
		pSprite->Install(pcVar1);
		pSprite->field_0xc4 = 12.5f;
		pInvColumn->field_0x870.SetFont(BootDataFont, false);
		pInvColumn->field_0x870.rgbaColour = 0xffffffff;
		pInvColumn->field_0x870.SetShadow(0x100);
		pInvColumn->field_0x870.SetShadowShift(2.0f, 2.0f);
		pInvColumn->field_0x870.SetHorizontalAlignment(1);
		pInvColumn->field_0x870.SetVerticalAlignment(4);
		iVar4 = 0;
		pInvEntry = pInvColumn->aEntries;
		do {
			pcVar1 = pFrontendBank->GetResource(MenuElementsBitmapNames[3], (edBANK_ENTRY_INFO*)0x0); // inv_case_01_p0
			pInvEntry->sprite.Install(pcVar1);
			iVar4 = iVar4 + 1;
			pInvEntry->sprite.field_0xc4 = 13.0f;
			pInvEntry = pInvEntry + 1;
		} while (iVar4 < 4);

		iVar5 = iVar5 + 1;
		pSprite = pSprite + 1;
		pInvColumn = pInvColumn + 1;
	} while (iVar5 < 2);

	return;
}

void CFrontendInventory::Term()
{
	edNODE* peVar1;
	_FE_INV_ENTRY* pEntry;
	int iVar4;
	int iVar5;
	CSprite3D* pCurSprite;

	SetInterface((CInterface*)0x0);

	peVar1 = this->pNode;
	if (peVar1 != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, peVar1);
		this->pNode = (edNODE*)0x0;
		(this->avatar).field_0x0 = (CActor*)0x0;
	}

	iVar5 = 0;
	pCurSprite = this->aSprites3d;
	FE_INV_COLUMN* pInvColumn = this->aInvColumns;
	do {
		pCurSprite->Remove();
		pEntry = pInvColumn->aEntries;
		CFE_Avatar* pNode = pInvColumn->aNodes;
		iVar4 = 0;
		do {
			pEntry->sprite.Remove();

			peVar1 = pNode->instance3d.pNode;
			if (peVar1 != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, peVar1);
				pNode->instance3d.pNode = (edNODE*)0x0;
				pNode->field_0x0 = (CActor*)0x0;
			}

			iVar4 = iVar4 + 1;
			pEntry = pEntry + 1;
			pNode = pNode + 1;
		} while (iVar4 < 4);

		iVar5 = iVar5 + 1;
		pCurSprite = pCurSprite + 1;
		pInvColumn = pInvColumn + 1;
	} while (iVar5 < 2);

	return;
}

void CFrontendInventory::UpdatePos_StateWait(float time)
{
	FE_INV_COLUMN* pInvColumn;
	int iVar2;
	CFE_Avatar* pNode;
	int iVar4;
	float fVar5;

	this->slotAlpha = 1.0f;
	iVar4 = this->field_0x4c;
	if (iVar4 == 2) {
		iVar4 = 0;
		pInvColumn = this->aInvColumns;
		do {
			pNode = pInvColumn->aNodes;
			if (this->pInterface != (CMagicInterface*)0x0) {
				iVar2 = 0;
				do {
					if ((pNode->instance3d.pNode != (edNODE*)0x0) && (pNode->field_0x4 != 0)) {
						pNode->instance3d.SetDraw(false, CFrontend::_scene_handle);
						pNode->field_0x4 = 0;
					}
					iVar2 = iVar2 + 1;
					pNode = pNode + 1;

				} while (iVar2 < 4);
			}

			iVar4 = iVar4 + 1;
			pInvColumn = pInvColumn + 1;
		} while (iVar4 < 2);
		this->field_0x4c = 3;
		this->bVisible = 0;
	}
	else {
		if (iVar4 != 5) {
			if (iVar4 == 4) {
				iVar4 = 0;
				pInvColumn = this->aInvColumns;
				do {
					pNode = pInvColumn->aNodes;
					if (this->pInterface != (CMagicInterface*)0x0) {
						iVar2 = 0;
						do {
							if ((pNode->instance3d.pNode != (edNODE*)0x0) && (pNode->field_0x4 == 0)) {
								pNode->instance3d.SetDraw(true, CFrontend::_scene_handle);
								pNode->field_0x4 = 1;
							}
							iVar2 = iVar2 + 1;
							pNode = pNode + 1;
						} while (iVar2 < 4);
					}

					iVar4 = iVar4 + 1;
					pInvColumn = pInvColumn + 1;
				} while (iVar4 < 2);

				fVar5 = CFrontend::GetTime();
				if (this->field_0x50 < fVar5 - this->prevTime) {
					this->field_0x4c = 2;
					MoveToNext(&this->slotOff);
				}
			}
			else {
				if (iVar4 == 1) {
					this->field_0x4c = 5;
				}
				else {
					if (iVar4 == 0) {
						this->field_0x4c = 4;
					}
				}
			}
		}
	}

	return;
}

bool CFrontendInventory::UpdateDisp(float time)
{
	int activeItemIndex;
	bool bVar2;
	CInventoryInterface* pInventoryInterface;

	bVar2 = false;
	if ((this->bDisplayDirty != 0) && (pInventoryInterface = static_cast<CInventoryInterface*>(GetInterface()), pInventoryInterface != (CInventoryInterface*)0x0)) {
		pInventoryInterface = (CInventoryInterface*)this->pInterface;
		activeItemIndex = pInventoryInterface->aHeaderInfo[0].activeItemIndex;
		this->aInvColumns[0].field_0x4 = pInventoryInterface->aHeaderInfo[0].nbUsedSlots;
		if (4 < (int)this->aInvColumns[0].field_0x4) {
			this->aInvColumns[0].field_0x4 = 4;
		}

		this->aInvColumns[0].field_0x0 = activeItemIndex;
		pInventoryInterface = (CInventoryInterface*)this->pInterface;
		activeItemIndex = pInventoryInterface->aHeaderInfo[1].activeItemIndex;
		this->aInvColumns[1].field_0x4 = pInventoryInterface->aHeaderInfo[1].nbUsedSlots;
		if (4 < (int)this->aInvColumns[1].field_0x4) {
			this->aInvColumns[1].field_0x4 = 4;
		}

		this->aInvColumns[1].field_0x0 = activeItemIndex;
		this->bDisplayDirty = 0;
		bVar2 = true;
	}

	return bVar2;
}

void CFrontendInventory::Update(float time)
{
	FUN_003ca620();

	CWidget::Update(time);

	return;
}

edF32VECTOR2 LeftOffset$10739 = { -0.105f, 0.14f };
edF32VECTOR2 RightOffset$10740 = { 0.1f, 0.14f };
edF32VECTOR2 vSelection_Offset = { 0.0f, 0.0f };
edF32VECTOR2 vSelection_Scale = { 0.04f, 0.03125f };

void CFrontendInventory::Draw()
{
	bool bVar1;
	edCTextStyle* pNewFont;
	int iVar2;
	edF32VECTOR2* v1;
	CSprite3D* pCVar3;
	FE_INV_COLUMN* pCVar4;
	CFE_Avatar* pInvNode;
	edCTextStyle textStyle;
	edF32VECTOR2 local_10;
	edF32VECTOR2 local_8;

	ManageTransit();

	if (((this->bVisible == 0) || (this->field_0x4c == 3)) || ((GameFlags & 0x3c) != 0)) {
		pInvNode = this->aInvColumns[0].aNodes;
		if (this->pInterface != (CInterface*)0x0) {
			iVar2 = 0;
			do {
				if ((pInvNode->instance3d.pNode != (edNODE*)0x0) && (pInvNode->field_0x4 != 0)) {
					pInvNode->instance3d.SetDraw(false, CFrontend::_scene_handle);
					pInvNode->field_0x4 = 0;
				}
				iVar2 = iVar2 + 1;
				pInvNode = pInvNode + 1;
			} while (iVar2 < 4);
		}

		pInvNode = this->aInvColumns[1].aNodes;
		if (this->pInterface != (CInterface*)0x0) {
			iVar2 = 0;
			do {
				if ((pInvNode->instance3d.pNode != (edNODE*)0x0) && (pInvNode->field_0x4 != 0)) {
					pInvNode->instance3d.SetDraw(false, CFrontend::_scene_handle);
					pInvNode->field_0x4 = 0;
				}
				iVar2 = iVar2 + 1;
				pInvNode = pInvNode + 1;
			} while (iVar2 < 4);
		}
	}
	else {
		bVar1 = GuiDList_BeginCurrent();
		if (bVar1 != false) {
			textStyle.Reset();
			textStyle.SetFont(BootDataFont, false);
			textStyle.SetHorizontalAlignment(2);
			textStyle.SetVerticalAlignment(8);
			pNewFont = edTextStyleSetCurrent(&textStyle);
			edCTextFormat textFormat;
			edF32Vector2Add(&local_8, &this->widgetSlotA.position, &LeftOffset$10739);
			textFormat.FormatString("%[ACTION]b");
			textFormat.Display(local_8.x * static_cast<float>(gVideoConfig.screenWidth), local_8.y * static_cast<float>(gVideoConfig.screenHeight));
			edF32Vector2Add(&local_8, &this->widgetSlotA.position, &RightOffset$10740);
			textFormat.FormatString("%[CATCH]b");
			textFormat.Display(local_8.x * static_cast<float>(gVideoConfig.screenWidth), local_8.y * static_cast<float>(gVideoConfig.screenHeight));
			edTextStyleSetCurrent(pNewFont);

			iVar2 = 0;
			do {
				Column_DisplayText(iVar2);
				iVar2 = iVar2 + 1;
			} while (iVar2 < 2);

			GuiDList_EndCurrent();
		}

		bVar1 = Frontend2DDList_BeginCurrent();
		v1 = &this->widgetSlotA.scale;
		if (bVar1 != false) {
			edF32Vector2Add(&this->aInvColumns[0].field_0x930, v1, &vCells_Offset);
			this->aInvColumns[0].field_0x930.x = -this->aInvColumns[0].field_0x930.x;
			edF32Vector2Add(&this->aInvColumns[1].field_0x930, v1, &vCells_Offset);
			pCVar3 = this->aSprites3d;
			pCVar4 = this->aInvColumns;
			for (iVar2 = 0; iVar2 < 2; iVar2 = iVar2 + 1) {
				Column_Display(iVar2);

				edF32Vector2Add(&local_10, &this->widgetSlotA.position, &pCVar4->field_0x930);
				edF32Vector2Add(&local_10, &local_10, &vSelection_Offset);
				pCVar3->bValid = false;
				if (iVar2 == 0) {
					pCVar3->flags = pCVar3->flags | 0x1000;
				}
				else {
					pCVar3->flags = pCVar3->flags & 0xffffefff;
				}
				pCVar3->bValid = false;
				pCVar3->field_0x5c.scale.x = vSelection_Scale.x;
				pCVar3->field_0x5c.scale.y = vSelection_Scale.y;
				pCVar3->bValid = false;
				pCVar3->field_0x5c.position.x = local_10.x;
				pCVar3->field_0x5c.position.y = local_10.y;
				pCVar3->Draw(true);
				pCVar4 = pCVar4 + 1;
				pCVar3 = pCVar3 + 1;
			}

			FrontendDList_EndCurrent();
		}
	}

	return;
}

void CFrontendInventory::Level_Init()
{
	this->field_0x4c = 3;
	this->slotAlpha = 0.0f;

	this->widgetSlotC.position = (this->slotOn).position;
	this->widgetSlotC.scale = (this->slotOn).scale;

	this->widgetSlotB.position = (this->slotOn).position;
	this->widgetSlotB.scale = (this->slotOn).scale;

	this->bDisplayDirty = 1;

	this->aInvColumns[0].field_0x0 = 0;
	this->aInvColumns[0].field_0x4 = 0;
	this->aInvColumns[0].field_0x930.x = vCells_Offset.x;
	this->aInvColumns[0].field_0x930.y = vCells_Offset.y;
	this->aInvColumns[0].field_0x938.x = edF32VECTOR2_00437620.x;
	this->aInvColumns[0].field_0x938.y = edF32VECTOR2_00437620.y;

	this->aInvColumns[1].field_0x0 = 0;
	this->aInvColumns[1].field_0x4 = 0;
	this->aInvColumns[1].field_0x930.x = vCells_Offset.x;
	this->aInvColumns[1].field_0x930.y = vCells_Offset.y;
	this->aInvColumns[1].field_0x938.x = edF32VECTOR2_00437620.x;
	this->aInvColumns[1].field_0x938.y = edF32VECTOR2_00437620.y;

	this->field_0x1490 = 0;

	return;
}

void CFrontendInventory::Level_Term()
{
	edNODE* pNode;
	int iVar1;
	int iVar2;
	FE_INV_COLUMN* pInvColumn;

	iVar2 = 0;
	pInvColumn = this->aInvColumns;
	do {
		iVar1 = 0;
		CFE_Avatar* pInvNode = pInvColumn->aNodes;
		do {
			pNode = pInvNode->instance3d.pNode;
			if (pNode != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
				pInvNode->instance3d.pNode = (edNODE*)0x0;
				pInvNode->field_0x0 = (CActor*)0x0;
			}

			iVar1 = iVar1 + 1;
			pInvNode = pInvNode + 1;
		} while (iVar1 < 4);

		iVar2 = iVar2 + 1;
		pInvColumn = pInvColumn + 1;
	} while (iVar2 < 2);

	return;
}

bool CFrontendInventory::ComputeGameScreenCoordinate(edF32VECTOR2* pOutCoordinate, edF32VECTOR4* pPosition, float* pOutDepth, CCamera* pCamera)
{
	CCameraManager* pCameraManager;
	float fov;

	// Position in camera space.
	edF32VECTOR4 pCameraPosition;

	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	if (pCamera == (CCamera*)0x0) {
		fov = pCameraManager->fov_0xa34;
		edF32Matrix4MulF32Vector4Hard(&pCameraPosition, &pCameraManager->worldToCamera_0x3d0, pPosition);
	}
	else {
		edF32MATRIX4 flippedTransform;
		fov = pCamera->fov;
		edF32Matrix4FlipXZAxes(&flippedTransform, &pCamera->transformationMatrix);
		edF32Matrix4GetInverseOrthoHard(&flippedTransform, &flippedTransform);
		edF32Matrix4MulF32Vector4Hard(&pCameraPosition, &flippedTransform, pPosition);
	}

	pOutCoordinate->x = (-pCameraPosition.x / (pCameraPosition.z * fov)) * 0.5f + 0.5f;
	pOutCoordinate->y = (pCameraPosition.y / (pCameraPosition.z * fov)) * 0.5f + 0.5f;
	*pOutDepth = -pCameraPosition.z * (fov / pCameraManager->pFrontendCamera_0x4e4->fov);

	return true;
}

bool CFrontendInventory::FUN_003c9b00(CActor* pInventoryOwner, int param_3, edF32VECTOR4* param_4, CActor* param_5)
{
	IMPLEMENTATION_GUARD();
	return true;
}

void CFrontendInventory::FUN_003ca620()
{
	_FE_INV_ENTRY* p_Var1;
	bool bVar2;
	float puVar3;
	long lVar4;
	int iVar5;
	CFE_Avatar* pFVar6;
	FE_INV_COLUMN* pCVar7;
	int iVar8;
	uint uVar9;
	uint uVar10;
	float fVar11;
	float puVar11;
	float puVar12;

	if (this->pInterface != (CInterface*)0x0) {
		lVar4 = this->pInterface->IsActive();
		if (((lVar4 != 0) || (bVar2 = ((CInventoryInterface*)this->pInterface)->HasChanged(), bVar2 != false)) || (this->field_0x1490 != 0)) {
			iVar8 = this->field_0x4c;
			if (iVar8 != 2) {
				if (iVar8 == 3) {
					iVar8 = 0;
					do {
						pFVar6 = this->aInvColumns[iVar8].aNodes;
						if (this->pInterface != (CInterface*)0x0) {
							iVar5 = 0;
							do {
								if ((pFVar6->instance3d.pNode != (edNODE*)0x0) && (pFVar6->field_0x4 == 0)) {
									pFVar6->instance3d.SetDraw(true, CFrontend::_scene_handle);
									pFVar6->field_0x4 = 1;
								}
								iVar5 = iVar5 + 1;
								pFVar6 = pFVar6 + 1;
							} while (iVar5 < 4);
						}

						iVar8 = iVar8 + 1;
					} while (iVar8 < 2);

					MoveToNext(&this->slotOn);
					this->field_0x4c = 0;
					this->bVisible = 1;
				}
				else {
					if (iVar8 == 4) {
						fVar11 = CFrontend::GetTime();
						this->prevTime = fVar11;
					}
				}
			}

			this->bDisplayDirty = 1;
		}

		uVar10 = 0;
		pCVar7 = this->aInvColumns;
		CSprite3D* pSprite = this->aSprites3d;
		do {
			GetTimer();

			puVar11 = -1.0f;
			if (0.0f <= static_cast<float>((uVar10 & 1) - 1)) {
				puVar11 = 1.0f;
			}

			fVar11 = edF32Between_2Pi(pSprite->field_0xc0 + puVar11 * 0.0f);
			pSprite->field_0xc0 = fVar11;
			uVar9 = 0;
			pFVar6 = pCVar7->aNodes;
			_FE_INV_ENTRY* pEntry = pCVar7->aEntries;
			do {
				GetTimer();

				puVar12 = -1.0f;
				if (0.0f <= static_cast<float>((uVar9 & 1) - 1)) {
					puVar12 = 1.0f;
				}

				puVar3 = -1.0f;
				if (0.0f <= (float)-uVar10) {
					puVar3 = 1.0f;
				}

				fVar11 = edF32Between_2Pi(pEntry->sprite.field_0xc0 + puVar3 * puVar12 * 0.0f);
				uVar9 = uVar9 + 1;
				pFVar6 = pFVar6 + 1;
				pEntry = pEntry + 1;
				pEntry->sprite.field_0xc0 = fVar11;
			} while (static_cast<int>(uVar9) < 4);

			uVar10 = uVar10 + 1;
			pSprite = pSprite + 1;
			pCVar7 = pCVar7 + 1;
		} while (static_cast<int>(uVar10) < 2);
	}

	return;
}


void CFrontendInventory::ManageTransit()
{
	edNODE* pNode;
	CFrontendDisplay* pFrontendDisplay;
	bool bVar1;
	CCameraManager* iVar2;
	CFrontendInventory* iVar3;
	float fVar3;
	float t;
	float fVar4;
	edF32VECTOR4 local_80;
	_msg_5e_param local_70;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR2 local_18;
	undefined4 local_10;
	edF32VECTOR4* local_c;
	undefined4 local_8;
	_msg_5e_param* local_4;

	pFrontendDisplay = CScene::ptable.g_FrontendManager_00451680;
	if (this->field_0x1490 != 0) {
		t = this->field_0x1498 / 0.3f;
		if (this->field_0x14b8 != (CActor*)0x0) {
			local_4 = &local_70;
			local_70.field_0x0 = 2;
			this->field_0x149c->DoMessage(this->field_0x14b8, (ACTOR_MESSAGE)0x5e, local_4);
			iVar2 = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
			fVar4 = iVar2->fov_0xa34;
			edF32Matrix4MulF32Vector4Hard(&local_80, &iVar2->worldToCamera_0x3d0, &local_50);
			fVar3 = local_80.z * fVar4;
			local_18.x = (-local_80.x / fVar3) * 0.5f + 0.5f;
			local_18.y = (local_80.y / fVar3) * 0.5f + 0.5f;

			bVar1 = pFrontendDisplay->ComputeSceneCoordinate(-local_80.z * (fVar4 / iVar2->pFrontendCamera_0x4e4->fov), &this->field_0x14a0, &local_18);
			if (bVar1 == false) {
				return;
			}
		}
		if (1.0f <= t) {
			pNode = this->avatar.instance3d.pNode;
			if (pNode != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
				this->avatar.instance3d.pNode = (edNODE*)0x0;
				this->avatar.field_0x0 = (CActor*)0x0;
			}

			if (this->field_0x1494 == 0) {
				this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x5f, (MSG_PARAM)2);
			}
			else {
				local_c = &local_50;
				this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x53, local_c);
				this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x5f, (MSG_PARAM)4);
			}

			this->field_0x1490 = 0;
		}
		else {
			if (this->field_0x1494 == 1) {
				t = 1.0f - t;
			}

			IMPLEMENTATION_GUARD(
			iVar3 = static_cast<CFrontendInventory*>((int)this->aSprites3d + (this->field_0x14b0).headerSlot * 0x940 + -0x74);
			edF32Vector2Add(&local_18, (edF32VECTOR2*)&this->slotOn, &iVar3->aInvColumns[0].field_0x930);
			edF32Vector2Add(&local_18, &local_18, &iVar3->aInvColumns[0].field_0x938);
			local_18.y = local_18.y + static_cast<float>((this->field_0x14b0).itemSlot + 1) * iVar3->aInvColumns[0].field_0x930.y;
			pFrontendDisplay->ComputeSceneCoordinate(12.0f, &eStack64, &local_18);
			edF32Vector4LERPHard(t, &local_30, (edF32VECTOR4*)&this->field_0x14a0, &eStack64);
			local_30.w = 1.0f;

			if (this->avatar.instance3d.pNode != (edNODE*)0x0) {
				this->avatar.instance3d.position = local_30;
				this->avatar.instance3d.position.w = 1.0f;
				(this->avatar).instance3d.ComputeObjectMatrix();
				if (this->avatar.field_0x0->pAnimationController != (CAnimation*)0x0) {
					(this->avatar).instance3d.ManageAnimation(GetTimer()->lastFrameTime);
				}
			})

			this->field_0x1498 = this->field_0x1498 + GetTimer()->cutsceneDeltaTime;
		}
	}

	return;
}

void CFrontendInventory::Column_DisplayText(int index)
{
	int iVar1;
	FE_INV_COLUMN* iVar2;
	edCTextStyle* pTextStyle;
	int iVar3;
	edF32VECTOR2 local_8;

	iVar2 = this->aInvColumns + index;
	local_8.x = this->widgetSlotA.scale.x * 0.8f;
	local_8.y = this->widgetSlotA.scale.y * 1.3f;
	edF32Vector2Add(&local_8, &local_8, &iVar2->field_0x930);
	edF32Vector2Add(&local_8, &local_8, &this->widgetSlotA.position);
	pTextStyle = edTextStyleSetCurrent(&iVar2->field_0x870);
	iVar3 = 0;
	if (0 < iVar2->field_0x4) {
		do {
			iVar1 = static_cast<CInventoryInterface*>(this->pInterface)->aHeaderInfo[index].nbUsedSlots;
			if (iVar1 == 0) {
				trap(7);
			}
			if (1 < static_cast<CInventoryInterface*>(this->pInterface)->aSlots[index][(iVar2->field_0x0 + iVar3) % iVar1].nbItems) {
				edCTextFormat textFormat;
				textFormat.FormatString("x%d");
				textFormat.Display(local_8.x * static_cast<float>(gVideoConfig.screenWidth), local_8.y * static_cast<float>(gVideoConfig.screenHeight));
			}

			iVar3 = iVar3 + 1;
			local_8.y = local_8.y + iVar2->field_0x930.y;
		} while (iVar3 < iVar2->field_0x4);
	}

	edTextStyleSetCurrent(pTextStyle);

	return;
}

void CFrontendInventory::Column_Display(int index)
{
	//CInterfaceVTable* pCVar1;
	edNODE* pNode;
	bool bVar2;
	int iVar3;
	CActor* pActor;
	CInventoryInfo* pCVar4;
	int iVar5;
	FE_INV_COLUMN* pInvColumn;
	CFE_Avatar* pFVar7;
	CFE_Avatar* iVar13;
	CFE_Avatar* pInvNode;
	int iVar9;
	FE_INV_COLUMN* pFVar10;
	FE_INV_COLUMN* pFVar11;
	int iVar12;
	int iVar14;
	edF32VECTOR4 local_30;
	int local_20;
	int local_1c;
	FE_Position local_18;
	edF32VECTOR2 eStack16;
	edF32VECTOR2 local_8;

	pInvColumn = this->aInvColumns + index;
	//pFVar10 = pInvColumn->aInvColumns;
	iVar12 = 0;
	iVar14 = 0;
	pInvNode = pInvColumn->aNodes;
	do {
		if (pInvNode->instance3d.pNode != (edNODE*)0x0) {
			IMPLEMENTATION_GUARD(
			iVar9 = static_cast<int>(pFVar10)->aEntries + iVar14 + -8;
			local_20 = index;
			if (*static_cast<int*>(iVar9 + 0x340) != 0) {
				iVar5 = 0;
				do {
					pCVar1 = this->pInterface[index * 3 + 5].pVTable;
					local_1c = (pFVar10->field_0x0 + iVar5) % static_cast<int>(pCVar1);
					if (pCVar1 == (CInterfaceVTable*)0x0) {
						trap(7);
					}
					pCVar1 = this->pInterface[local_20 * 0x30 + local_1c * 3 + 10].pVTable;
					iVar3 = (**static_cast<code**>(**(int**)(iVar9 + 0x330) + 0xe8))();
					if (*static_cast<CInterfaceVTable**>(iVar3 + 0x20) == pCVar1) goto LAB_003ca278;
					iVar5 = iVar5 + 1;
				} while (iVar5 < 4);
			})

			iVar5 = -1;
		LAB_003ca278:
			if (iVar5 == -1) {
				pNode = pInvNode->instance3d.pNode;
				if (pNode != (edNODE*)0x0) {
					ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
					pInvNode->instance3d.pNode = (edNODE*)0x0;
					pInvNode->field_0x0 = (CActor*)0x0;
				}
			}
		}
		iVar12 = iVar12 + 1;
		pInvNode = pInvNode + 1;
		iVar14 = iVar14 + 0x150;
	} while (iVar12 < 4);

	edF32Vector2Add(&local_8, (edF32VECTOR2*)&this->widgetSlotA, &pInvColumn->field_0x930);

	iVar12 = 0;
	if (0 < pInvColumn->field_0x4) {
		_FE_INV_ENTRY* pEntry = pInvColumn->aEntries;
		local_18.headerSlot = index;
		do {
			int nbUsedSlots = static_cast<CInventoryInterface*>(this->pInterface)->aHeaderInfo[index].nbUsedSlots;
			local_18.itemSlot = (pFVar10->field_0x0 + iVar12) % nbUsedSlots;
			if (nbUsedSlots == 0x0) {
				trap(7);
			}

			pEntry->sprite.bValid = false;
			if (index == 0) {
				pEntry->sprite.flags = pEntry->sprite.flags | 0x1000;
			}
			else {
				pEntry->sprite.flags = pEntry->sprite.flags & 0xffffefff;
			}

			pEntry->sprite.bValid = false;
			pEntry->sprite.field_0x5c.scale = edF32VECTOR2_00437618;
			pEntry->sprite.bValid = false;
			pEntry->sprite.field_0x5c.position = local_8;
			pEntry->sprite.rgba[0] = 0x80;
			pEntry->sprite.rgba[1] = 0x80;
			pEntry->sprite.rgba[2] = 0x80;
			pEntry->sprite.rgba[3] = 0x60;
			pEntry->sprite.Draw(true);

			local_8.y = local_8.y + pInvColumn->field_0x930.y;
			edF32Vector2Add(&eStack16, &local_8, &pInvColumn->field_0x938);
			pFVar11 = this->aInvColumns + local_18.headerSlot;
			pActor = ((CInventoryInterface*)this->pInterface)->FindActorForCell(&local_18);
			if (pActor != (CActor*)0x0) {
				pCVar4 = pActor->GetInventoryInfo();
				iVar14 = pCVar4->purchaseId;
				iVar9 = 0;
				pFVar7 = pFVar11->aNodes;
				do {
					CInventoryInfo* pInvInfo;
					if ((pFVar7->instance3d.pNode != (edNODE*)0x0) && (pInvInfo = pFVar7->field_0x0->GetInventoryInfo(), iVar14 == pInvInfo->purchaseId)) goto LAB_003ca480;
					iVar9 = iVar9 + 1;
					pFVar7 = pFVar7 + 1;
				} while (iVar9 < 4);

				iVar9 = -1;
			LAB_003ca480:
				if (iVar9 == -1) {
					iVar9 = 0;
					pFVar7 = pFVar11->aNodes;
					do {
						if (pFVar7->instance3d.pNode == (edNODE*)0x0) goto LAB_003ca4c8;
						iVar9 = iVar9 + 1;
						pFVar7 = pFVar7 + 1;
					} while (iVar9 < 4);
					iVar9 = -1;
				LAB_003ca4c8:
					pFVar11->aNodes[iVar9].Create(pActor);
				}

				if ((iVar9 != -1) && (bVar2 = CScene::ptable.g_FrontendManager_00451680->ComputeSceneCoordinate(12.0f, &local_30, &eStack16), bVar2 != false)) {
					iVar13 = pInvColumn->aNodes;
					if (iVar13->instance3d.pNode != (edNODE*)0x0) {
						iVar13->instance3d.position = local_30;
						iVar13->instance3d.ComputeObjectMatrix();
						if (iVar13->field_0x0->pAnimationController != (CAnimation*)0x0) {
							iVar13->instance3d.ManageAnimation(GetTimer()->lastFrameTime);
						}
					}
				}
			}

			iVar12 = iVar12 + 1;
			pEntry = pEntry + 1;
		} while (iVar12 < pInvColumn->field_0x4);
	}

	return;
}

void CFE_Avatar::Create(CActor* pActor)
{
	CinNamedObject30* pCVar1;
	CAnimation* this_00;
	ed_g3d_manager* pG3D;
	edNODE* peVar2;
	int iVar3;

	this->field_0x0 = pActor;
	this->field_0x4 = 1;
	pCVar1 = pActor->pCinData;
	if (pCVar1->textureIndex == -1) {
		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetCommonLevelMeshInfo(pCVar1->meshIndex);
	}
	else {
		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(pCVar1->meshIndex, pCVar1->textureIndex);
	}

	peVar2 = ed3DHierarchyAddToScene(CFrontend::_scene_handle, pG3D, (char*)0x0);
	(this->instance3d).pNode = peVar2;

	reinterpret_cast<ed_g3d_hierarchy*>(this->instance3d.pNode->pData)->pTextureInfo = STORE_POINTER((pActor->p3DHierNode->base).pTextureInfo);

	this->instance3d.ClearLocalData();
	if (pActor->pAnimationController != (CAnimation*)0x0) {
		this->instance3d.SetAnimationDatas(CScene::ptable.g_AnimManager_00451668->pAnimKeyTable);
		this_00 = pActor->pAnimationController;
		iVar3 = pActor->GetIdMacroAnim(5);
		iVar3 = this_00->GetPhysicalAnimIndex(iVar3);
		this->instance3d.SetAnimation(iVar3, 1);
	}
	return;
}