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
	char* pResource;
	_FE_INV_ENTRY* pInvEntry;
	FE_INV_COLUMN* pInvColumn;
	int entryIndex;
	int columnIndex;
	CFrontendBank* pFrontendBank;

	CWidget::Init();

	pFrontendBank = CScene::ptable.g_FrontEndBank_00451674;
	columnIndex = 0;
	pInvColumn = this->aInvColumns;
	CSprite3D* pSprite3d = this->aSprites3d;
	do {
		pResource = pFrontendBank->GetResource(MenuElementsBitmapNames[4], (edBANK_ENTRY_INFO*)0x0); // inv_select_01_p0
		pSprite3d->Install(pResource);
		pSprite3d->field_0xc4 = 12.5f;
		pInvColumn->field_0x870.SetFont(BootDataFont, false);
		pInvColumn->field_0x870.rgbaColour = 0xffffffff;
		pInvColumn->field_0x870.SetShadow(0x100);
		pInvColumn->field_0x870.SetShadowShift(2.0f, 2.0f);
		pInvColumn->field_0x870.SetHorizontalAlignment(1);
		pInvColumn->field_0x870.SetVerticalAlignment(4);
		entryIndex = 0;
		pInvEntry = pInvColumn->aEntries;
		do {
			pResource = pFrontendBank->GetResource(MenuElementsBitmapNames[3], (edBANK_ENTRY_INFO*)0x0); // inv_case_01_p0
			pInvEntry->sprite.Install(pResource);
			entryIndex = entryIndex + 1;
			pInvEntry->sprite.field_0xc4 = 13.0f;
			pInvEntry = pInvEntry + 1;
		} while (entryIndex < 4);

		columnIndex = columnIndex + 1;
		pSprite3d = pSprite3d + 1;
		pInvColumn = pInvColumn + 1;
	} while (columnIndex < 2);

	return;
}

void CFrontendInventory::Term()
{
	edNODE* pNodeToRemove;
	_FE_INV_ENTRY* pEntry;
	int entryIndex;
	int columnIndex;
	CSprite3D* pCurSprite;

	SetInterface((CInterface*)0x0);

	pNodeToRemove = this->pNode;
	if (pNodeToRemove != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNodeToRemove);
		this->pNode = (edNODE*)0x0;
		(this->avatar).field_0x0 = (CActor*)0x0;
	}

	columnIndex = 0;
	pCurSprite = this->aSprites3d;
	FE_INV_COLUMN* pInvColumn = this->aInvColumns;
	do {
		pCurSprite->Remove();
		pEntry = pInvColumn->aEntries;
		CFE_Avatar* pAvatarNode = pInvColumn->aNodes;
		entryIndex = 0;
		do {
			pEntry->sprite.Remove();

			pNodeToRemove = pAvatarNode->instance3d.pNode;
			if (pNodeToRemove != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNodeToRemove);
				pAvatarNode->instance3d.pNode = (edNODE*)0x0;
				pAvatarNode->field_0x0 = (CActor*)0x0;
			}

			entryIndex = entryIndex + 1;
			pEntry = pEntry + 1;
			pAvatarNode = pAvatarNode + 1;
		} while (entryIndex < 4);

		columnIndex = columnIndex + 1;
		pCurSprite = pCurSprite + 1;
		pInvColumn = pInvColumn + 1;
	} while (columnIndex < 2);

	return;
}

void CFrontendInventory::UpdatePos_StateWait(float time)
{
	FE_INV_COLUMN* pInvColumn;
	int nodeIndex;
	CFE_Avatar* pNode;
	int columnIndex;
	float currentTime;

	this->slotAlpha = 1.0f;
	columnIndex = this->field_0x4c;
	if (columnIndex == 2) {
		columnIndex = 0;
		pInvColumn = this->aInvColumns;
		do {
			pNode = pInvColumn->aNodes;
			if (this->pInterface != (CMagicInterface*)0x0) {
				nodeIndex = 0;
				do {
					if ((pNode->instance3d.pNode != (edNODE*)0x0) && (pNode->field_0x4 != 0)) {
						pNode->instance3d.SetDraw(false, CFrontend::_scene_handle);
						pNode->field_0x4 = 0;
					}
					nodeIndex = nodeIndex + 1;
					pNode = pNode + 1;

				} while (nodeIndex < 4);
			}

			columnIndex = columnIndex + 1;
			pInvColumn = pInvColumn + 1;
		} while (columnIndex < 2);
		this->field_0x4c = 3;
		this->bVisible = 0;
	}
	else {
		if (columnIndex != 5) {
			if (columnIndex == 4) {
				columnIndex = 0;
				pInvColumn = this->aInvColumns;
				do {
					pNode = pInvColumn->aNodes;
					if (this->pInterface != (CMagicInterface*)0x0) {
						nodeIndex = 0;
						do {
							if ((pNode->instance3d.pNode != (edNODE*)0x0) && (pNode->field_0x4 == 0)) {
								pNode->instance3d.SetDraw(true, CFrontend::_scene_handle);
								pNode->field_0x4 = 1;
							}
							nodeIndex = nodeIndex + 1;
							pNode = pNode + 1;
						} while (nodeIndex < 4);
					}

					columnIndex = columnIndex + 1;
					pInvColumn = pInvColumn + 1;
				} while (columnIndex < 2);

				currentTime = CFrontend::GetTime();
				if (this->field_0x50 < currentTime - this->prevTime) {
					this->field_0x4c = 2;
					MoveToNext(&this->slotOff);
				}
			}
			else {
				if (columnIndex == 1) {
					this->field_0x4c = 5;
				}
				else {
					if (columnIndex == 0) {
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
	bool bSuccess;
	CInventoryInterface* pInventoryInterface;

	bSuccess = false;
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
		bSuccess = true;
	}

	return bSuccess;
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
	bool bIsDrawing;
	edCTextStyle* pPreviousStyle;
	int columnIndex;
	edF32VECTOR2* pWidgetScale;
	CSprite3D* pCurrentSprite;
	FE_INV_COLUMN* pCurrentColumn;
	CFE_Avatar* pInvNode;
	edCTextStyle textStyle;
	edF32VECTOR2 selectionPosition;
	edF32VECTOR2 textPosition;

	ManageTransit();

	if (((this->bVisible == 0) || (this->field_0x4c == 3)) || ((GameFlags & 0x3c) != 0)) {
		pInvNode = this->aInvColumns[0].aNodes;
		if (this->pInterface != (CInterface*)0x0) {
			columnIndex = 0;
			do {
				if ((pInvNode->instance3d.pNode != (edNODE*)0x0) && (pInvNode->field_0x4 != 0)) {
					pInvNode->instance3d.SetDraw(false, CFrontend::_scene_handle);
					pInvNode->field_0x4 = 0;
				}
				columnIndex = columnIndex + 1;
				pInvNode = pInvNode + 1;
			} while (columnIndex < 4);
		}

		pInvNode = this->aInvColumns[1].aNodes;
		if (this->pInterface != (CInterface*)0x0) {
			columnIndex = 0;
			do {
				if ((pInvNode->instance3d.pNode != (edNODE*)0x0) && (pInvNode->field_0x4 != 0)) {
					pInvNode->instance3d.SetDraw(false, CFrontend::_scene_handle);
					pInvNode->field_0x4 = 0;
				}
				columnIndex = columnIndex + 1;
				pInvNode = pInvNode + 1;
			} while (columnIndex < 4);
		}
	}
	else {
		bIsDrawing = GuiDList_BeginCurrent();
		if (bIsDrawing != false) {
			textStyle.Reset();
			textStyle.SetFont(BootDataFont, false);
			textStyle.SetHorizontalAlignment(2);
			textStyle.SetVerticalAlignment(8);
			pPreviousStyle = edTextStyleSetCurrent(&textStyle);
			edCTextFormat textFormat;
			edF32Vector2Add(&textPosition, &this->widgetSlotA.position, &LeftOffset$10739);
			textFormat.FormatString("%[ACTION]b");
			textFormat.Display(textPosition.x * static_cast<float>(gVideoConfig.screenWidth), textPosition.y * static_cast<float>(gVideoConfig.screenHeight));
			edF32Vector2Add(&textPosition, &this->widgetSlotA.position, &RightOffset$10740);
			textFormat.FormatString("%[CATCH]b");
			textFormat.Display(textPosition.x * static_cast<float>(gVideoConfig.screenWidth), textPosition.y * static_cast<float>(gVideoConfig.screenHeight));
			edTextStyleSetCurrent(pPreviousStyle);

			columnIndex = 0;
			do {
				Column_DisplayText(columnIndex);
				columnIndex = columnIndex + 1;
			} while (columnIndex < 2);

			GuiDList_EndCurrent();
		}

		bIsDrawing = Frontend2DDList_BeginCurrent();
		pWidgetScale = &this->widgetSlotA.scale;
		if (bIsDrawing != false) {
			edF32Vector2Add(&this->aInvColumns[0].field_0x930, pWidgetScale, &vCells_Offset);
			this->aInvColumns[0].field_0x930.x = -this->aInvColumns[0].field_0x930.x;
			edF32Vector2Add(&this->aInvColumns[1].field_0x930, pWidgetScale, &vCells_Offset);
			pCurrentSprite = this->aSprites3d;
			pCurrentColumn = this->aInvColumns;
			for (columnIndex = 0; columnIndex < 2; columnIndex = columnIndex + 1) {
				Column_Display(columnIndex);

				edF32Vector2Add(&selectionPosition, &this->widgetSlotA.position, &pCurrentColumn->field_0x930);
				edF32Vector2Add(&selectionPosition, &selectionPosition, &vSelection_Offset);
				pCurrentSprite->bValid = false;
				if (columnIndex == 0) {
					pCurrentSprite->flags = pCurrentSprite->flags | 0x1000;
				}
				else {
					pCurrentSprite->flags = pCurrentSprite->flags & 0xffffefff;
				}
				pCurrentSprite->bValid = false;
				pCurrentSprite->field_0x5c.scale.x = vSelection_Scale.x;
				pCurrentSprite->field_0x5c.scale.y = vSelection_Scale.y;
				pCurrentSprite->bValid = false;
				pCurrentSprite->field_0x5c.position.x = selectionPosition.x;
				pCurrentSprite->field_0x5c.position.y = selectionPosition.y;
				pCurrentSprite->Draw(true);
				pCurrentColumn = pCurrentColumn + 1;
				pCurrentSprite = pCurrentSprite + 1;
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
	int nodeIndex;
	int columnIndex;
	FE_INV_COLUMN* pInvColumn;

	columnIndex = 0;
	pInvColumn = this->aInvColumns;
	do {
		nodeIndex = 0;
		CFE_Avatar* pInvNode = pInvColumn->aNodes;
		do {
			pNode = pInvNode->instance3d.pNode;
			if (pNode != (edNODE*)0x0) {
				ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
				pInvNode->instance3d.pNode = (edNODE*)0x0;
				pInvNode->field_0x0 = (CActor*)0x0;
			}

			nodeIndex = nodeIndex + 1;
			pInvNode = pInvNode + 1;
		} while (nodeIndex < 4);

		columnIndex = columnIndex + 1;
		pInvColumn = pInvColumn + 1;
	} while (columnIndex < 2);

	return;
}

bool CFrontendInventory::ComputeGameScreenCoordinate(edF32VECTOR2* pOutCoordinate, edF32VECTOR4* pPosition, float* pOutDepth, CCamera* pCamera)
{
	CCameraManager* pCameraManager;
	float fov;

	// Position in camera space.
	edF32VECTOR4 cameraPosition;

	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	if (pCamera == (CCamera*)0x0) {
		fov = pCameraManager->fov_0xa34;
		edF32Matrix4MulF32Vector4Hard(&cameraPosition, &pCameraManager->worldToCamera_0x3d0, pPosition);
	}
	else {
		edF32MATRIX4 flippedTransform;
		fov = pCamera->fov;
		edF32Matrix4FlipXZAxes(&flippedTransform, &pCamera->transformationMatrix);
		edF32Matrix4GetInverseOrthoHard(&flippedTransform, &flippedTransform);
		edF32Matrix4MulF32Vector4Hard(&cameraPosition, &flippedTransform, pPosition);
	}

	pOutCoordinate->x = (-cameraPosition.x / (cameraPosition.z * fov)) * 0.5f + 0.5f;
	pOutCoordinate->y = (cameraPosition.y / (cameraPosition.z * fov)) * 0.5f + 0.5f;
	*pOutDepth = -cameraPosition.z * (fov / pCameraManager->pFrontendCamera_0x4e4->fov);

	return true;
}

bool CFrontendInventory::PrepareTransit(CActor* pInventoryOwner, int param_3, edF32VECTOR4* param_4, CActor* param_5)
{
	int columnStartSlot;
	edNODE* pNode;
	bool bSuccess;
	CCameraManager* pCameraManager;
	CInventoryInfo* pInventoryInfo;
	int iVar5;
	int* piVar6;
	float fovScale;
	float fov;
	edF32VECTOR4 cameraSpacePos;
	_msg_5e_param msgParam;
	edF32VECTOR4 worldPos;
	FE_Position activePos;
	edF32VECTOR2 screenPos;
	undefined4 local_10;
	edF32VECTOR4* pWorldPos;
	undefined4 local_8;
	_msg_5e_param* pMsgParam;
	CFrontendDisplay* pFrontendDisplay;

	pFrontendDisplay = CScene::ptable.g_FrontendManager_00451680;
	if (param_5 == (CActor*)0x0) {
		worldPos.x = param_4->x;
		worldPos.y = param_4->y;
		worldPos.z = param_4->z;
		worldPos.w = param_4->w;
	}
	else {
		pMsgParam = &msgParam;
		msgParam.field_0x0 = 2;
		pInventoryOwner->DoMessage(param_5, (ACTOR_MESSAGE)0x5e, pMsgParam);
		worldPos = msgParam.field_0x10;
	}

	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	fov = pCameraManager->fov_0xa34;
	edF32Matrix4MulF32Vector4Hard(&cameraSpacePos, &pCameraManager->worldToCamera_0x3d0, &worldPos);
	fovScale = cameraSpacePos.z * fov;
	screenPos.x = (-cameraSpacePos.x / fovScale) * 0.5f + 0.5f;
	screenPos.y = (cameraSpacePos.y / fovScale) * 0.5f + 0.5f;
	bSuccess = pFrontendDisplay->ComputeSceneCoordinate(-cameraSpacePos.z * (fov / pCameraManager->pFrontendCamera_0x4e4->fov), &this->field_0x14a0, &screenPos);
	if (bSuccess == false) {
		bSuccess = false;
	}
	else {
		pInventoryInfo = pInventoryOwner->GetInventoryInfo();
		bSuccess = static_cast<CInventoryInterface*>(this->pInterface)->FindByType(&this->field_0x14b0, pInventoryInfo->purchaseId);
		if (bSuccess == false) {
			bSuccess = false;
		}
		else {
			int itemSlot = (this->field_0x14b0).itemSlot;
			FE_INV_COLUMN* pInvColumn = this->aInvColumns + (this->field_0x14b0).headerSlot;
			columnStartSlot = pInvColumn->field_0x0;
			if ((itemSlot < columnStartSlot) || (columnStartSlot + 3 < itemSlot)) {
				activePos.headerSlot = (this->field_0x14b0).headerSlot;
				itemSlot = (this->field_0x14b0).itemSlot;
				if (itemSlot < pInvColumn->field_0x0) {
					activePos.itemSlot = pInvColumn->field_0x0 - itemSlot;
				}
				else {
					activePos.itemSlot = itemSlot + -3;
				}

				static_cast<CInventoryInterface*>(this->pInterface)->SetActivePosition(&activePos);
				pInvColumn->field_0x0 = activePos.itemSlot;
			}

			(this->field_0x14b0).itemSlot = (this->field_0x14b0).itemSlot - pInvColumn->field_0x0;

			pNode = this->pNode;
			if (pNode != (edNODE*)0x0) {
				if (pNode != (edNODE*)0x0) {
					ed3DHierarchyRemoveFromScene(CFrontend::_scene_handle, pNode);
					this->pNode = (edNODE*)0x0;
					(this->avatar).field_0x0 = (CActor*)0x0;
				}

				if (this->field_0x1494 == 0) {
					this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x5f, (MSG_PARAM)2);
				}
				else {
					pWorldPos = &worldPos;
					this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x53, pWorldPos);
					this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x5f, (MSG_PARAM)4);
				}
			}

			this->field_0x1490 = 1;
			this->field_0x1494 = param_3;
			this->field_0x1498 = 0.0f;
			this->field_0x149c = pInventoryOwner;
			this->field_0x14b8 = param_5;
			this->avatar.Create(pInventoryOwner);
			bSuccess = true;
		}
	}

	return bSuccess;
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
	bool bIsCoordinateValid;
	CCameraManager* pCameraManager;
	FE_INV_COLUMN* pInvColumn;
	float projectedZ;
	float interpolationT;
	float fov;
	edF32VECTOR4 cameraSpacePos;
	_msg_5e_param msgParam;
	edF32VECTOR4 worldPos;
	edF32VECTOR4 targetPos;
	edF32VECTOR4 currentPos;
	edF32VECTOR2 screenPos;
	undefined4 local_10;
	edF32VECTOR4* pWorldPos;
	undefined4 local_8;
	_msg_5e_param* pMsgParam;

	pFrontendDisplay = CScene::ptable.g_FrontendManager_00451680;
	if (this->field_0x1490 != 0) {
		interpolationT = this->field_0x1498 / 0.3f;
		if (this->field_0x14b8 != (CActor*)0x0) {
			pMsgParam = &msgParam;
			msgParam.field_0x0 = 2;
			this->field_0x149c->DoMessage(this->field_0x14b8, (ACTOR_MESSAGE)0x5e, pMsgParam);
			pCameraManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
			fov = pCameraManager->fov_0xa34;
			edF32Matrix4MulF32Vector4Hard(&cameraSpacePos, &pCameraManager->worldToCamera_0x3d0, &worldPos);
			projectedZ = cameraSpacePos.z * fov;
			screenPos.x = (-cameraSpacePos.x / projectedZ) * 0.5f + 0.5f;
			screenPos.y = (cameraSpacePos.y / projectedZ) * 0.5f + 0.5f;

			bIsCoordinateValid = pFrontendDisplay->ComputeSceneCoordinate(-cameraSpacePos.z * (fov / pCameraManager->pFrontendCamera_0x4e4->fov), &this->field_0x14a0, &screenPos);
			if (bIsCoordinateValid == false) {
				return;
			}
		}
		if (1.0f <= interpolationT) {
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
				pWorldPos = &worldPos;
				this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x53, pWorldPos);
				this->field_0x149c->DoMessage(this->field_0x149c, (ACTOR_MESSAGE)0x5f, (MSG_PARAM)4);
			}

			this->field_0x1490 = 0;
		}
		else {
			if (this->field_0x1494 == 1) {
				interpolationT = 1.0f - interpolationT;
			}

			pInvColumn = this->aInvColumns + (this->field_0x14b0).headerSlot;
			edF32Vector2Add(&screenPos, &this->slotOn.position, &pInvColumn->field_0x930);
			edF32Vector2Add(&screenPos, &screenPos, &pInvColumn->field_0x938);
			screenPos.y = screenPos.y + static_cast<float>((this->field_0x14b0).itemSlot + 1) * pInvColumn->field_0x930.y;
			pFrontendDisplay->ComputeSceneCoordinate(12.0f, &targetPos, &screenPos);
			edF32Vector4LERPHard(interpolationT, &currentPos, &this->field_0x14a0, &targetPos);
			currentPos.w = 1.0f;

			if (this->avatar.instance3d.pNode != (edNODE*)0x0) {
				this->avatar.instance3d.position = currentPos;
				this->avatar.instance3d.position.w = 1.0f;
				(this->avatar).instance3d.ComputeObjectMatrix();
				if (this->avatar.field_0x0->pAnimationController != (CAnimation*)0x0) {
					(this->avatar).instance3d.ManageAnimation(GetTimer()->lastFrameTime);
				}
			}

			this->field_0x1498 = this->field_0x1498 + GetTimer()->cutsceneDeltaTime;
		}
	}

	return;
}

void CFrontendInventory::Column_DisplayText(int index)
{
	int nbUsedSlots;
	FE_INV_COLUMN* pColumn;
	edCTextStyle* pTextStyle;
	int slotIndex;
	edF32VECTOR2 textPosition;

	pColumn = this->aInvColumns + index;
	textPosition.x = this->widgetSlotA.scale.x * 0.8f;
	textPosition.y = this->widgetSlotA.scale.y * 1.3f;
	edF32Vector2Add(&textPosition, &textPosition, &pColumn->field_0x930);
	edF32Vector2Add(&textPosition, &textPosition, &this->widgetSlotA.position);
	pTextStyle = edTextStyleSetCurrent(&pColumn->field_0x870);
	slotIndex = 0;
	if (0 < pColumn->field_0x4) {
		do {
			nbUsedSlots = static_cast<CInventoryInterface*>(this->pInterface)->aHeaderInfo[index].nbUsedSlots;
			if (nbUsedSlots == 0) {
				trap(7);
			}
			if (1 < static_cast<CInventoryInterface*>(this->pInterface)->aSlots[index][(pColumn->field_0x0 + slotIndex) % nbUsedSlots].nbItems) {
				edCTextFormat textFormat;
				textFormat.FormatString("x%d");
				textFormat.Display(textPosition.x * static_cast<float>(gVideoConfig.screenWidth), textPosition.y * static_cast<float>(gVideoConfig.screenHeight));
			}

			slotIndex = slotIndex + 1;
			textPosition.y = textPosition.y + pColumn->field_0x930.y;
		} while (slotIndex < pColumn->field_0x4);
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
			local_20 = index;
			if (pInvNode->instance3d.pNode != (edNODE*)0x0) {
				iVar5 = 0;
				do {
					int nbUsedSlots = static_cast<CInventoryInterface*>(this->pInterface)->aHeaderInfo[index].nbUsedSlots;
					local_1c = (pInvColumn->field_0x0 + iVar9) % nbUsedSlots;
					if (nbUsedSlots == 0) {
						trap(7);
					}
					CActor* pActor = pInvNode->field_0x0;
					int itemId = static_cast<CInventoryInterface*>(this->pInterface)->aSlots[local_20][local_1c].itemId;
					CInventoryInfo* pInvInfo = pActor->GetInventoryInfo();
					if (pInvInfo->purchaseId == itemId) goto LAB_003ca278;
					iVar5 = iVar5 + 1;
				} while (iVar5 < 4);
			}

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
			local_18.itemSlot = (pInvColumn->field_0x0 + iVar12) % nbUsedSlots;
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
	CAnimation* pAnim;
	ed_g3d_manager* pG3D;
	edNODE* pNode;
	int macroAnimId;

	this->field_0x0 = pActor;
	this->field_0x4 = 1;
	pCVar1 = pActor->pCinData;
	if (pCVar1->textureIndex == -1) {
		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetCommonLevelMeshInfo(pCVar1->meshIndex);
	}
	else {
		pG3D = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(pCVar1->meshIndex, pCVar1->textureIndex);
	}

	pNode = ed3DHierarchyAddToScene(CFrontend::_scene_handle, pG3D, (char*)0x0);
	(this->instance3d).pNode = pNode;

	reinterpret_cast<ed_3d_hierarchy*>(this->instance3d.pNode->pData)->pTextureInfo = (pActor->p3DHierNode->base).pTextureInfo;

	this->instance3d.ClearLocalData();
	if (pActor->pAnimationController != (CAnimation*)0x0) {
		this->instance3d.SetAnimationDatas(CScene::ptable.g_AnimManager_00451668->pAnimKeyTable);
		pAnim = pActor->pAnimationController;
		macroAnimId = pActor->GetIdMacroAnim(5);
		macroAnimId = pAnim->GetPhysicalAnimIndex(macroAnimId);
		this->instance3d.SetAnimation(macroAnimId, 1);
	}
	return;
}

void CSprite3D::ClearLocalData()
{
	CSprite::ClearLocalData();

	this->field_0xc0 = 0.0f;
	this->field_0xc4 = 10.0f;

	return;
}

edF32VECTOR2 edF32VECTOR2_00448ce8 = { -1.0f, -1.0f };
edF32VECTOR2 edF32VECTOR2_00448cf0 = { 1.0f, 1.0f };

void CSprite3D::Validate()
{
	CSprite* pParentSprite;
	edF32VECTOR2* pCurrentCoords;
	edF32VECTOR2* pEffectiveScale;
	edF32VECTOR2 worldPosition;
	edF32VECTOR2 combinedScale;
	edF32VECTOR2 bottomRightOffset;
	edF32VECTOR2 topLeftOffset;

	topLeftOffset = edF32VECTOR2_00448ce8;
	bottomRightOffset = edF32VECTOR2_00448cf0;

	pParentSprite = this->pParent;
	if (pParentSprite == (CSprite*)0x0) {
		pEffectiveScale = &this->field_0x5c.scale;
	}
	else {
		edF32Vector2Mul(&combinedScale, &(pParentSprite->field_0x5c).scale, &this->field_0x5c.scale);
		edF32Vector2Mul(&worldPosition, &((this->pParent)->field_0x5c).scale, &this->field_0x5c.position);
		edF32Vector2Add(&worldPosition, &(this->pParent)->field_0x5c.position, &worldPosition);
		pEffectiveScale = &combinedScale;
	}

	edF32Vector2Mul(&this->screenCoordsTL, &topLeftOffset, pEffectiveScale);
	pCurrentCoords = &this->screenCoordsTL;
	edF32Vector2Mul(pCurrentCoords, pCurrentCoords, this->field_0xc4);
	edF32Vector2Mul(&this->screenCoordsBR, &bottomRightOffset, pEffectiveScale);
	pEffectiveScale = &this->screenCoordsBR;
	edF32Vector2Mul(pEffectiveScale, pEffectiveScale, this->field_0xc4);
	this->bValid = true;

	return;
}

void CSprite3D::DrawSprite()
{
	bool bVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 eStack96;
	edF32MATRIX4 local_50;
	float local_10;
	float local_c;
	float local_8;
	float local_4;

	local_50 = gF32Matrix4Unit;

	bVar1 = CScene::ptable.g_FrontendManager_00451680->ComputeSceneCoordinate(this->field_0xc4, &eStack96, &this->field_0x5c.position);
	if (bVar1 != false) {
		local_8 = this->texCoordA.x;
		local_4 = this->texCoordA.y;
		fVar2 = this->texCoordB.x;
		fVar3 = this->texCoordB.y;
		local_c = fVar3;

		if ((this->flags & 0x2000) != 0) {
			local_c = local_4;
			local_4 = fVar3;
		}

		local_10 = fVar2;
		if ((this->flags & 0x1000) != 0) {
			local_10 = local_8;
			local_8 = fVar2;
		}

		edDListLoadMatrix(&local_50);
		if (this->field_0xc0 != 0.0f) {
			edDListRotateZ(this->field_0xc0);
		}

		edDListTranslatev(&eStack96);
		edDListBegin(1.0f, 1.0f, 1.0f, 8, 4);
		edDListTexCoo2f(local_8, local_4);
		edDListVertex4f(this->screenCoordsTL.x, this->screenCoordsTL.y, 0.0f, 0.0f);
		edDListTexCoo2f(local_10, local_4);
		edDListVertex4f(this->screenCoordsBR.x, this->screenCoordsTL.y, 0.0f, 0.0f);
		edDListTexCoo2f(local_8, local_c);
		edDListVertex4f(this->screenCoordsTL.x, this->screenCoordsBR.y, 0.0f, 0.0f);
		edDListTexCoo2f(local_10, local_c);
		edDListVertex4f(this->screenCoordsBR.x, this->screenCoordsBR.y, 0.0f, 0.0f);
		edDListEnd();
	}

	return;
}
