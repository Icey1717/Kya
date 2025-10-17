#include "FrontEndInventory.h"
#include "camera.h"
#include "CameraViewManager.h"
#include "MathOps.h"

edF32VECTOR2 INV_POS_DRAW_ON{ 0.8f, 0.05f };
edF32VECTOR2 INV_POS_DRAW_OFF{ 1.25f, -0.2f };
edF32VECTOR2 INV_SCALE{ 0.05f, 0.05f };

#define IMPLEMENTATION_GUARD_INVENTORY_UI(x)

CFrontendInventory::CFrontendInventory()
{
	this->animatedInstance3d.pNode = (edNODE*)0x0;
	this->popupTime = 0.15f;
	this->field_0x50 = 0.15f;

	(this->slotOn).position = INV_POS_DRAW_ON;
	(this->slotOn).scale = INV_SCALE;

	(this->slotOff).position = INV_POS_DRAW_OFF;
	(this->slotOff).scale = INV_SCALE;
}

void CFrontendInventory::Reset()
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::Init()
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::Term()
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::UpdatePos_StateWait(float time)
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

bool CFrontendInventory::UpdateDisp(float time)
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::Update(float time)
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::Draw()
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::Level_Init()
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
}

void CFrontendInventory::Level_Term()
{
	IMPLEMENTATION_GUARD_INVENTORY_UI();
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