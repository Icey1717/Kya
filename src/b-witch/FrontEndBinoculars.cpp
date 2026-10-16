#include "FrontEndBinoculars.h"
#include "FrontEndBank.h"
#include "Frontend.h"
#include "CameraViewManager.h"
#include "camera.h"
#include "DlistManager.h"
#include "MathOps.h"
#include "kya.h"

// PS2 binocular widget: object 0x004544c0, vtable 0x00446370.
// Slot/mesh constants: 0x004375e0-0x004375fc, 0x00437638-0x0043764c.
static edF32VECTOR2 binocularPosOn = { 0.32f, 0.26f };
static edF32VECTOR2 binocularScaleOn = { 0.375f, 0.5f };
static edF32VECTOR2 binocularPosOff = { -0.14f, -0.02f };
static edF32VECTOR2 binocularScaleOff = { 0.75f, 1.0f };
static edF32VECTOR2 binocularMeshPosition = { 0.5f, -0.5f };
static edF32VECTOR4 binocularMeshScale = { 0.155f, 0.155f, -0.25f, 0.0f };

extern float FLOAT_00448a04;

// PS2: 0x003c95d0.
CFrontendBinoculars::CFrontendBinoculars()
{
	this->popupTime = 0.15f;
	this->slotOn.position = binocularPosOn;
	this->slotOn.scale = binocularScaleOn;
	this->slotOff.position = binocularPosOff;
	this->slotOff.scale = binocularScaleOff;
}

// PS2: 0x003c9450.
void CFrontendBinoculars::Init()
{
	CWidget::Init();

	this->aHoleSprites[0].Install(CScene::ptable.g_FrontEndBank_00451674->GetG2D(0), "binocular_hole");

	for (uint spriteIndex = 1; spriteIndex < 4; spriteIndex = spriteIndex + 1) {
		CSprite* pSprite = &this->aHoleSprites[spriteIndex];
		pSprite->Install(&this->aHoleSprites[0].materialInfo);
		if ((spriteIndex & 1) != 0) {
			pSprite->bValid = false;
			pSprite->flags = pSprite->flags | 0x1000;
		}

		if (1 < spriteIndex) {
			pSprite->bValid = false;
			pSprite->flags = pSprite->flags | 0x2000;
		}
	}

	for (int spriteIndex = 0; spriteIndex < 4; spriteIndex = spriteIndex + 1) {
		CSprite* pSprite = &this->aBorderSprites[spriteIndex];
		pSprite->Install(&this->aHoleSprites[0].materialInfo);
		pSprite->texCoordA = { 0.0f, 0.0f };
		pSprite->texCoordB = { 0.1f, 0.1f };
	}

	this->aBorderSprites[0].flags = 0x22;
	this->aBorderSprites[1].flags = 0xa;
	this->aBorderSprites[2].flags = 0x14;
	this->aBorderSprites[3].flags = 0x11;

	return;
}

// PS2: 0x003c93c0.
void CFrontendBinoculars::Term()
{
	SetInterface((CInterface*)0x0);

	for (int spriteIndex = 0; spriteIndex < 4; spriteIndex = spriteIndex + 1) {
		this->aHoleSprites[spriteIndex].Remove();
	}

	for (int spriteIndex = 0; spriteIndex < 4; spriteIndex = spriteIndex + 1) {
		this->aBorderSprites[spriteIndex].Remove();
	}

	return;
}

// PS2: 0x003c9370. The mesh manager is at bank +0x130 (entry 0).
void CFrontendBinoculars::Level_Init()
{
	this->instance3d.pNode = ed3DHierarchyAddToScene(CScene::_scene_handleB,
		&CScene::ptable.g_FrontEndBank_00451674->mesh.aEntries[0].manager, (char*)0x0);

	Reset();

	return;
}

// PS2: 0x003c9330.
void CFrontendBinoculars::Level_Term()
{
	if (this->instance3d.pNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(CScene::_scene_handleB, this->instance3d.pNode);
	}

	this->instance3d.pNode = (edNODE*)0x0;

	return;
}

// PS2: 0x003c92a0. CheckpointReset uses the inherited CWidget implementation.
void CFrontendBinoculars::Reset()
{
	CWidget::Reset();
	this->field_0x4c = 3;
	this->widgetSlotC.position = this->slotOn.position;
	this->widgetSlotC.scale = this->slotOn.scale;
	this->widgetSlotB.position = this->slotOn.position;
	this->widgetSlotB.scale = this->slotOn.scale;
	this->instance3d.SetDraw(false, CScene::_scene_handleB);
	this->bDisplayDirty = 1;

	return;
}

// PS2: 0x003c9050.
void CFrontendBinoculars::UpdatePos_StateWait(float time)
{
	this->slotAlpha = 1.0f;
	if (this->field_0x4c == 2) {
		this->field_0x4c = 3;
		this->bVisible = 0;
	}
	else {
		if (this->field_0x4c != 5) {
			if (this->field_0x4c == 4) {
				if ((GetInterface() != (CInterface*)0x0) && (GetInterface()->IsActive() == false)) {
					this->field_0x4c = 2;
					MoveToNext(&this->slotOff);
				}
			}
			else {
				if (this->field_0x4c == 1) {
					this->field_0x4c = 5;
				}
				else {
					if (this->field_0x4c == 0) {
						this->field_0x4c = 4;
					}
				}
			}
		}
	}

	return;
}

// PS2: 0x003c9130.
bool CFrontendBinoculars::UpdateDisp(float time)
{
	if ((this->bDisplayDirty == 0) || (GetInterface() == (CInterface*)0x0)) {
		return false;
	}

	this->bDisplayDirty = 0;

	return true;
}

// PS2: 0x003c9190.
void CFrontendBinoculars::Update(float time)
{
	if (this->pInterface == (CInterface*)0x0) {
		return;
	}

	if (GetInterface()->IsActive() != false) {
		if (this->field_0x4c != 2) {
			if (this->field_0x4c == 3) {
				MoveToNext(&this->slotOn);
				this->field_0x4c = 0;
				this->bVisible = 1;
			}
			else {
				if (this->field_0x4c == 4) {
					this->prevTime = CFrontend::GetTime();
				}
			}
		}

		this->bDisplayDirty = 1;
	}

	float width = 0.5f / CScene::ptable.g_CameraManager_0045167c->aspectRatio;
	this->slotOn.position.x = 0.5f - width * 0.5f;
	this->slotOn.position.y = 0.25f;
	this->slotOn.scale.x = width;
	this->slotOn.scale.y = 0.5f;

	CWidget::Update(time);

	return;
}

// PS2: 0x003c8d10.
void CFrontendBinoculars::Draw()
{
	CCameraManager* pCameraManager = CScene::ptable.g_CameraManager_0045167c;
	CCameraManager* pViewManager = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	edF32VECTOR4 cameraPosition;
	edF32VECTOR4 worldPosition;
	edF32VECTOR2 screenPosition;

	screenPosition.x = binocularMeshPosition.x * 2.0f - 1.0f;
	screenPosition.y = binocularMeshPosition.y * -2.0f - 1.0f;
	if (ed3DComputeScreenCoordinate(0.5f, &cameraPosition, &screenPosition, CFrontend::_scene_handle) == false) {
		this->instance3d.SetDraw(false, CScene::_scene_handleB);
	}
	else {
		cameraPosition.z = cameraPosition.z * (pViewManager->pFrontendCamera_0x4e4->fov / pViewManager->fov_0xa34);
		edF32Matrix4MulF32Vector4Hard(&worldPosition, &pViewManager->transformationMatrix, &cameraPosition);
		FLOAT_00448a04 = 0.5f;
		ed3DHierarchyNodeSetFlag(this->instance3d.pNode, 0x400);
		this->instance3d.position = worldPosition;
		this->instance3d.rotation.x = pCameraManager->angle_0xa08.x;
		this->instance3d.rotation.y = pCameraManager->angle_0xa08.y;
		this->instance3d.rotation.z = 0.0f;
		this->instance3d.scale = binocularMeshScale;
		this->instance3d.ComputeObjectMatrix();
		this->instance3d.SetDraw(this->field_0x4c != 3, CScene::_scene_handleB);
	}

	if (((this->bVisible != 0) && (this->field_0x4c != 3)) && ((GameFlags & 0x3c) == 0)) {
		if (GuiDList_BeginCurrent() != false) {
			for (uint spriteIndex = 0; spriteIndex < 4; spriteIndex = spriteIndex + 1) {
				float x = this->widgetSlotA.position.x;
				float y = this->widgetSlotA.position.y;
				float width = this->widgetSlotA.scale.x;
				float height = this->widgetSlotA.scale.y;
				if ((spriteIndex & 1) != 0) {
					x = 1.0f - x;
				}
				if (1 < spriteIndex) {
					y = 1.0f - y;
				}
				this->aHoleSprites[spriteIndex].UpdateSlotPositionAndScale(x, y, width, height);
				this->aHoleSprites[spriteIndex].Draw(true);

				if (spriteIndex < 2) {
					y = (this->widgetSlotA.position.y - height * 0.5f) * 0.5f;
					height = y * 2.0f;
					width = 1.0f;
					x = 0.5f;
					if (spriteIndex == 1) {
						y = 1.0f - y;
					}
				}
				else {
					x = (this->widgetSlotA.position.x - width * 0.5f) * 0.5f;
					width = x * 2.0f;
					height = 1.0f;
					y = 0.5f;
					if (spriteIndex == 3) {
						x = 1.0f - x;
					}
				}
				this->aBorderSprites[spriteIndex].UpdateSlotPositionAndScale(x, y, width, height);
				this->aBorderSprites[spriteIndex].Draw(true);
			}

			GuiDList_EndCurrent();
		}
	}

	return;
}
