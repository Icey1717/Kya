#include "CameraViewManager.h"
#include "ed3D.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include <math.h>

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "TimeController.h"
#include "Rendering/DisplayList.h"
#include "edDlist.h"
#include "DlistManager.h"
#include "ActorHero.h"

#define EDFCAMERA_LOG(level, format, ...) MY_LOG_CATEGORY("edFCamera", level, format, ##__VA_ARGS__)

CCameraManager* CCameraManager::_gThis = NULL;
CCameraManager* g_CameraViewManager_00448e9c = NULL;
edFCamera _gDisplayCamera = { 0 };
edFCamera gSceneCameras[10] = { 0 };

edFCamera CCameraManager::_gFrontEndCamera = { 0 };

CCameraManager::CCameraManager()
{
	_gThis = this;
	this->field_0xa7c = 0;
	Level_ClearInternalData();
	return;
}

void CCameraManager::Game_Init()
{
	this->aspectRatio = 1.333333f;
	return;
}

void ActiveCamManager::SetActiveCam(CCamera* pCamera)

{
	this->pActiveCamera = pCamera;
	/* WARNING: Could not recover jumptable at 0x003feb1c. Too many branches */
	/* WARNING: Treating indirect jump as call */
	pCamera->AlertCamera(2, 1, (CCamera*)0x0);
	return;
}

void ActiveCamManager::ClearActiveCam()
{
	this->activeIndex = -1;
	return;
}

ActiveCamManager::ActiveCamManager()
{
	this->pActiveCamera = (CCamera*)0x0;
	this->activeIndex = -1;
}

bool ActiveCamManager::SwitchActiveCam(float param_1, CCamera* pCamera, SWITCH_MODE switchMode)
{
	bool bVar1;
	CCamera* pCVar2;
	int iVar3;
	int iVar4;

	iVar4 = this->activeIndex;
	if (iVar4 < -1) {
		pCVar2 = (CCamera*)0x0;
	}
	else {
		if (iVar4 == -1) {
			pCVar2 = this->pActiveCamera;
		}
		else {
			pCVar2 = *(CCamera**)(&this->field_0x40 + iVar4 * 0x50);
		}
	}
	if (pCVar2 == pCamera) {
		bVar1 = false;
	}
	else {
		if (switchMode == SWITCH_MODE_B) {
			pCVar2 = (CCamera*)0x0;
			if (-2 < iVar4) {
				if (iVar4 == -1) {
					pCVar2 = this->pActiveCamera;
				}
				else {
					IMPLEMENTATION_GUARD();
					pCVar2 = *(CCamera**)(&this->field_0x40 + iVar4 * 0x50);
				}
			}
			pCVar2->AlertCamera(1, 1, pCamera);
			iVar4 = this->activeIndex;
			pCVar2 = (CCamera*)0x0;
			if (-2 < iVar4) {
				if (iVar4 == -1) {
					pCVar2 = this->pActiveCamera;
				}
				else {
					IMPLEMENTATION_GUARD();
					pCVar2 = *(CCamera**)(&this->field_0x40 + iVar4 * 0x50);
				}
			}
			pCamera->AlertCamera(2, 1, pCVar2);
			bVar1 = false;
			this->activeIndex = -1;
			this->pActiveCamera = pCamera;
		}
		else {
			if (iVar4 == 7) {
				bVar1 = false;
			}
			else {
				IMPLEMENTATION_GUARD(
				if ((iVar4 != -1) &&
					(iVar3 = &this->field_0x0 + this->activeIndex * 0x50,
						*(float*)(iVar3 + 0x4c) <= *(float*)(iVar3 + 0x48) * (CScene::ptable.g_CameraManager_0045167c)->time_0x4)) {
					this->activeIndex = this->activeIndex + -1;
					iVar4 = this->activeIndex;
					pCVar2 = (CCamera*)0x0;
					if (-2 < iVar4) {
						if (iVar4 == -1) {
							pCVar2 = this->pActiveCamera;
						}
						else {
							pCVar2 = *(CCamera**)(&this->field_0x40 + iVar4 * 0x50);
						}
					}
					if (pCVar2 == pCamera) {
						return false;
					}
				}
				pCVar2 = (CCamera*)0x0;
				if (-2 < iVar4) {
					if (iVar4 == -1) {
						pCVar2 = this->pActiveCamera;
					}
					else {
						pCVar2 = *(CCamera**)(&this->field_0x40 + iVar4 * 0x50);
					}
				}
				(*(code*)((pCVar2->objBase).pVTable)->AlertCamera)(pCVar2, 1, switchMode, pCamera);
				iVar4 = 0;
				bVar1 = false;
				do {
					iVar3 = this->activeIndex - iVar4;
					iVar4 = iVar4 + 1;
					if (iVar3 < -1) {
						pCVar2 = (CCamera*)0x0;
					}
					else {
						if (iVar3 == -1) {
							pCVar2 = this->pActiveCamera;
						}
						else {
							pCVar2 = *(CCamera**)((int)this + iVar3 * 0x50 + 0x40);
						}
					}
					if (pCVar2 == pCamera) {
						bVar1 = true;
					}
				} while (iVar4 < this->activeIndex + 2);
				if (!bVar1) {
					iVar4 = this->activeIndex;
					pCVar2 = (CCamera*)0x0;
					if (-2 < iVar4) {
						if (iVar4 == -1) {
							pCVar2 = this->pActiveCamera;
						}
						else {
							pCVar2 = *(CCamera**)(&this->field_0x40 + iVar4 * 0x50);
						}
					}
					(*(code*)((pCamera->objBase).pVTable)->AlertCamera)(pCamera, 2, switchMode, pCVar2);
				}
				iVar4 = &this->field_0x50 + this->activeIndex * 0x50;
				*(CCamera**)(iVar4 + 0x40) = pCamera;
				*(SWITCH_MODE*)(iVar4 + 0x44) = switchMode;
				*(undefined4*)(iVar4 + 0x4c) = 0;
				if (param_1 == 0.0) {
					*(undefined4*)(iVar4 + 0x48) = 0x3fa00000;
				}
				else {
					*(float*)(iVar4 + 0x48) = 1.0 / param_1;
				}
				bVar1 = true;
				this->activeIndex = this->activeIndex + 1)
			}
		}
	}
	return bVar1;
}

char* s_Mouse_Quake_0042ae48 = "Mouse Quake";
char* s_Mouse_Around_0042ae58 = "Mouse Around";
char* s_Around_Special_0042ae68 = "Around Special";
char* s_Debug_Quake_0042ae78 = "Debug Quake";
char* s_Cinematic_0042ae88 = "Cinematic";
char* s_Frontend_0042ae98 = "Frontend";
char* s_Shadow_Sun_0042aea8 = "Shadow Sun";

void CCameraManager::Level_Init(bool bProcessEvents)
{
	float fVar1;
	float fVar2;
	float fVar3;
	CCamera* frontend;
	ECameraType EVar4;
	int iVar5;
	int iVar6;
	CCameraManager* pCVar7;
	int pEVar8;
	ByteCode newView;

	this->flags = 0x21000000;
	/* Mouse Quake */
	AddCamera(CT_MouseQuake, &newView, s_Mouse_Quake_0042ae48);
	/* Mouse Around */
	AddCamera(CT_MouseAround, &newView, s_Mouse_Around_0042ae58);
	/* Around Special */
	AddCamera(CT_AroundSpecial, &newView, s_Around_Special_0042ae68);
	/* Debug Quake */
	frontend = AddCamera(CT_MouseQuake, &newView, s_Debug_Quake_0042ae78);
	this->pMouseQuakeCamera_0x4e8 = frontend;
	/* Cinematic */
	AddCamera(CT_Cinematic, &newView, s_Cinematic_0042ae88);
	/* Frontend */
	frontend = AddCamera(CT_Frontend, &newView, s_Frontend_0042ae98);
	this->pFrontendCamera_0x4e4 = (FrontendCameraView*)frontend;
	frontend = this->pFrontendCamera_0x4e4;
	(frontend->lookAt).x = 0.0;
	(frontend->lookAt).y = 0.0;
	(frontend->lookAt).z = 10.0;
	(frontend->lookAt).w = 1.0;
	frontend = this->pFrontendCamera_0x4e4;
	(frontend->transformationMatrix).da = 0.0;
	(frontend->transformationMatrix).db = 0.0;
	(frontend->transformationMatrix).dc = 0.0;
	(frontend->transformationMatrix).dd = 1.0;
	this->pFrontendCamera_0x4e4->fov = 0.3333333;
	frontend = this->pFrontendCamera_0x4e4;
	_gFrontEndCamera.position.x = (frontend->transformationMatrix).da;
	_gFrontEndCamera.position.y = (frontend->transformationMatrix).db;
	_gFrontEndCamera.position.z = (frontend->transformationMatrix).dc;
	_gFrontEndCamera.position.w = (frontend->transformationMatrix).dd;
	frontend = this->pFrontendCamera_0x4e4;
	_gFrontEndCamera.lookAt.x = (frontend->lookAt).x;
	_gFrontEndCamera.lookAt.y = (frontend->lookAt).y;
	_gFrontEndCamera.lookAt.z = (frontend->lookAt).z;
	_gFrontEndCamera.lookAt.w = (frontend->lookAt).w;
	CameraSet3DPos(&_gFrontEndCamera);
	edFCameraSetSizeRatioFov(0.03f, this->aspectRatio, this->pFrontendCamera_0x4e4->fov / 1.333333f, &_gFrontEndCamera);
	this->field_0xa00 = CScene::_pinstance->field_0x11c;
	this->field_0xa04 = CScene::_pinstance->count_0x120;
	if (bProcessEvents != false) {
		pEVar8 = 0x0;
		do {
			/* Shadow Sun */
			CCameraShadow* pCameraShadow = (CCameraShadow*)AddCamera(CT_ShadowSun, &newView, s_Shadow_Sun_0042aea8);
			if ((pCameraShadow != (CCameraShadow*)0x0) && (pCameraShadow != (CCameraShadow*)0x0)) {
				EVar4 = pCameraShadow->GetMode();
				if (EVar4 == CT_ShadowSun) {
					this->aCameraShadow[pEVar8] = pCameraShadow;
					pCameraShadow->sceneIndex = pEVar8;
					pCameraShadow->sceneFlags = 1 << (pEVar8 & 0x1fU);
				}
				else {
					this->pActiveCamera = pCameraShadow;
				}
			}
			pEVar8 = pEVar8 + 1;
		} while (pEVar8 < 10);
	}
	CCameraShadow* pCamera = (CCameraShadow*)this->pInitialView_0x4b4;
	do {
		pCamera->Init();
		pCamera->flags_0xc = pCamera->flags_0xc & 0xffdfffff;
		pCamera = (CCameraShadow*)pCamera->pNextCameraView_0xa4;
	} while (pCamera != (CCamera*)0x0);

	for (pCamera = (CCameraShadow*)this->pInitialView_0x4b4;
		(pCamera != (CCameraShadow*)0x0 && ((EVar4 = pCamera->GetMode(), EVar4 != CT_MouseQuake || (pCamera->field_0x8 != -0x9f)))); 
		pCamera = (CCameraShadow*)pCamera->pNextCameraView_0xa4) {
	}

	if (pCamera != (CCamera*)0x0) {
		this->cameraStack.SetMainCamera(pCamera);
		this->activeCamManager.SetActiveCam(pCamera);
		bool bVar4 = this->activeCamManager.SwitchActiveCam(0.0f, pCamera, SWITCH_MODE_B);
		if ((bVar4 == 0) && (pCamera != (CCamera*)0x0)) {
			EVar4 = pCamera->GetMode();
			if (EVar4 == CT_ShadowSun) {
				this->pActiveCamera = pCamera;
				pCamera->sceneIndex = -1;
				pCamera->sceneFlags = 0;
			}
			else {
				this->pActiveCamera = pCamera;
			}
		}
		this->flags = this->flags & 0xfbffffff;
	}
	this->field_0x4a0 = 0;
	(this->transformationMatrix).rowT = gF32Vertex4Zero;
	(this->shadowCameraLookat) = gF32Vertex4Zero;
	this->fov_0xa34 = 0.84f;
	edF32Matrix4SetIdentityHard(&this->matrix_0x10);
	this->activeCamManager.ClearActiveCam();
	this->field_0xa78 = 0;
	//iVar5 = 0;
	//pCVar7 = this;
	//do {
	//	*(undefined4*)&pCVar7->field_0xa38 = 0;
	//	*(undefined4*)&pCVar7->field_0xa3c = 0;
	//	iVar5 = iVar5 + 8;
	//	*(undefined4*)&pCVar7->field_0xa40 = 0;
	//	*(undefined4*)&pCVar7->field_0xa44 = 0;
	//	*(undefined4*)&pCVar7->field_0xa48 = 0;
	//	*(undefined4*)&pCVar7->field_0xa4c = 0;
	//	*(undefined4*)&pCVar7->field_0xa50 = 0;
	//	*(undefined4*)&pCVar7->field_0xa54 = 0;
	//	pCVar7 = (CameraManager*)&(pCVar7->matrix_0x10).ba;
	//} while (iVar5 < 0x10);
	iVar5 = 0;
	if (0 < this->loadedCameraCount_0x9f0) {
		do {
			frontend = this->pInitialView_0x4b4;
			iVar6 = 0;
			if (0 < iVar5) {
				do {
					if (frontend->pNextCameraView_0xa4 != (CCamera*)0x0) {
						frontend = frontend->pNextCameraView_0xa4;
					}
					iVar6 = iVar6 + 1;
				} while (iVar6 < iVar5);
			}
			EVar4 = frontend->GetMode();
			if ((EVar4 == 0x14) && (iVar6 = 0, pCVar7 = this, (frontend->flags_0xc & 0x200000) != 0)) {
				IMPLEMENTATION_GUARD(
				for (; (*(int*)&pCVar7->field_0xa38 != 0 && (iVar6 < 0x10)); iVar6 = iVar6 + 1) {
					pCVar7 = (CCameraManager*)&pCVar7->time_0x4;
				}
				if (iVar6 < 0x10) {
					*(CCamera**)(&this->field_0xa38 + iVar6 * 4) = frontend;
					this->field_0xa78 = this->field_0xa78 + 1;
				})
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < this->loadedCameraCount_0x9f0);
	}
	BuildDisplayMatrix();
	//PTR_CameraManager_00449ee0 = (CameraManager*)0x0;
	//PTR_Singleton_00449ee4 = (AActorBase*)0x0;
	//DAT_00449eec = 0;
	//DAT_00449ff0 = 0;
	return;
}

void CCameraManager::Level_Init()
{
	Level_Init(true);
}

void CCameraManager::KeepSameParam(CCamera* pNewCamera, uint flag)
{
	bool bVar1;
	edF32VECTOR4 eStack32;
	edF32VECTOR3 local_10;

	if (this->pActiveCamera != (CCamera*)0x0) {
		if ((flag & 8) != 0) {
			pNewCamera->fov = this->fov_0xa34;
		}
		if ((flag & 4) != 0) {
			pNewCamera->SetDistance(this->distance_0xa30);
		}
		if ((flag & 1) != 0) {
			(pNewCamera->transformationMatrix).rowT = (this->transformationMatrix).rowT;
		}
		if ((flag & 2) != 0) {
			pNewCamera->lookAt = this->shadowCameraLookat;
		}
		if ((flag & 0x70) != 0) {
			local_10.x = (this->angle_0xa08).x;
			local_10.y = (this->angle_0xa08).y;
			local_10.z = (this->angle_0xa08).z;
			bVar1 = pNewCamera->IsKindOfObject(0x40);
			if (bVar1 == false) {
				SetVectorFromAngles(&eStack32, &local_10);
				edF32Vector4SubHard(&eStack32, &this->shadowCameraLookat, &transformationMatrix.rowT);
				edF32Vector4NormalizeHard(&eStack32, &eStack32);
			}
			else {
				if ((flag & 0x10) != 0) {
					pNewCamera->SetAngleAlpha(local_10.x);
				}
				if ((flag & 0x20) != 0) {
					pNewCamera->SetAngleBeta(local_10.y);
				}
				if ((flag & 0x40) != 0) {
					pNewCamera->SetAngleGamma(local_10.z);
				}
			}
		}
	}
	return;
}

void CCameraManager::Func_001947e0()
{
	undefined4 uVar1;
	ECameraType EVar2;
	//Actor* pActor;
	long lVar3;
	int iVar4;
	undefined4* puVar5;
	undefined4* puVar6;
	CCameraManager** ppCVar7;
	int local_230;
	undefined4 local_22c[67];
	int local_120[68];
	edF32VECTOR4 local_10;

	if ((this->pActiveCamera != (CCamera*)0x0) &&
		(lVar3 = (long)this->pActiveCamera->GetTarget(), lVar3 != 0)) {
		IMPLEMENTATION_GUARD_LOG(); // Check ghidra for function.
	}
}

float CCameraManager::Manage_EarthQuake(edF32VECTOR4* param_2)
{
	long lVar1;
	float fVar2;
	float fVar3;
	float t;
	edF32VECTOR4 local_10;

	t = 1.0;
	local_10.x = (this->field_0x480).x;
	local_10.y = (this->field_0x480).y;
	local_10.z = (this->field_0x480).z;
	fVar3 = 0.0;
	if ((this->field_0x4a0 & 1) != 0) {
		IMPLEMENTATION_GUARD();
	}
	return fVar3 * t;
}

void edF32Matrix4TransposeHard(edF32MATRIX4* m0)
{
	edF32MATRIX4 m1 = *m0;
	m0->aa = m1.aa;
	m0->ab = m1.ba;
	m0->ac = m1.ca;
	m0->ad = m1.da;
	m0->ba = m1.ab;
	m0->bb = m1.bb;
	m0->bc = m1.cb;
	m0->bd = m1.db;
	m0->ca = m1.ac;
	m0->cb = m1.bc;
	m0->cc = m1.cc;
	m0->cd = m1.dc;
	m0->da = m1.ad;
	m0->db = m1.bd;
	m0->dc = m1.cd;
	m0->dd = m1.dd;
	return;
}

void CCameraManager::ComputeFrustrumPlanes(float param_1, edF32MATRIX4* m0)
{
	float* pfVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	edF32Vector4ScaleHard(1.0f / (param_1 / 1.333333f), &eStack16, &m0->rowZ);
	edF32Vector4ScaleHard(1.0f, &eStack48, &m0->rowY);
	edF32Vector4ScaleHard(this->aspectRatio, &eStack32, &m0->rowX);
	edF32Vector4AddHard(&eStack64, &eStack16, &eStack32);
	edF32Vector4AddHard(&eStack64, &eStack64, &eStack48);
	edF32Vector4AddHard(&eStack80, &eStack16, &eStack32);
	edF32Vector4SubHard(&eStack80, &eStack80, &eStack48);
	edF32Vector4SubHard(&eStack96, &eStack16, &eStack32);
	edF32Vector4AddHard(&eStack96, &eStack96, &eStack48);
	edF32Vector4SubHard(&eStack112, &eStack16, &eStack32);
	edF32Vector4SubHard(&eStack112, &eStack112, &eStack48);

	edF32Vector4CrossProductHard(&this->frustumPlane.rowX, &eStack80, &eStack64);
	edF32Vector4NormalizeHard(&this->frustumPlane.rowX, &this->frustumPlane.rowX);

	edF32Vector4CrossProductHard(&this->frustumPlane.rowY, &eStack96, &eStack112);
	edF32Vector4NormalizeHard(&this->frustumPlane.rowY, &this->frustumPlane.rowY);

	edF32Vector4CrossProductHard(&this->frustumPlane.rowZ, &eStack64, &eStack96);
	edF32Vector4NormalizeHard(&this->frustumPlane.rowZ, &this->frustumPlane.rowZ);

	edF32Vector4CrossProductHard(&this->frustumPlane.rowT, &eStack112, &eStack80);
	edF32Vector4NormalizeHard(&this->frustumPlane.rowT, &this->frustumPlane.rowT);

	edF32Matrix4TransposeHard(&this->frustumPlane);
	(this->frustumPlane).dd = 1.0f;
	(this->frustumPlane).dc = 1.0f;
	(this->frustumPlane).db = 1.0f;
	(this->frustumPlane).da = 1.0f;

	this->field_0x450 = m0->rowZ;
	this->frustumLocation = m0->rowT;

	return;
}

void CCameraManager::BuildDisplayMatrix()
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	int iVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;

	if (((this->pActiveCamera->flags_0xc & 0x20000) == 0) ||
		((this->flags & 0x4000000) != 0)) {
		edF32VECTOR4 oldT = (this->transformationMatrix).rowT;
		edF32Matrix4FromEulerSoft(&this->transformationMatrix, &this->angle_0xa08, "ZXY");
		(this->transformationMatrix).rowT = oldT;
	}
	else {
		edF32Matrix4CopyHard(&this->transformationMatrix, &this->pActiveCamera->transformationMatrix);
	}
	fVar1 = Manage_EarthQuake(&(this->transformationMatrix).rowT);
	this->displayTransformMatrix = this->transformationMatrix;
	if ((this->flags & 0x18000000) == 0x18000000) {
		this->displayTransformMatrix = this->aCameraShadow[0]->transformationMatrix;
	}
	else {
		if ((this->flags & 0x10000000) != 0) {
			this->displayTransformMatrix = this->pMouseQuakeCamera_0x4e8->transformationMatrix;
		}
	}
	edF32Matrix4GetInverseOrthoHard(&this->field_0x350, &this->transformationMatrix);
	edF32Vector4ScaleHard(-1.0f, &this->transMatrix_0x390.rowX, &this->transformationMatrix.rowX);
	edF32Vector4ScaleHard(-1.0f, &(this->transMatrix_0x390).rowZ, &(this->transformationMatrix).rowZ);

	(this->transMatrix_0x390).rowY = (this->transformationMatrix).rowY;
	(this->transMatrix_0x390).rowT = (this->transformationMatrix).rowT;

	edF32Matrix4GetInverseOrthoHard(&this->worldToCamera_0x3d0, &this->transMatrix_0x390);
	_gDisplayCamera.position = (this->displayTransformMatrix).rowT;
	_gDisplayCamera.lookAt = this->shadowCameraLookat;
	_gDisplayCamera.rotationZ = 0.0;

	edF32Vector4ScaleHard(-1.0f, &_gDisplayCamera.transformMatrix.rowX, &this->displayTransformMatrix.rowX);
	edF32Vector4ScaleHard(-1.0f, &_gDisplayCamera.transformMatrix.rowZ, &this->displayTransformMatrix.rowZ);

	_gDisplayCamera.transformMatrix.rowY = this->displayTransformMatrix.rowY;
	_gDisplayCamera.transformMatrix.rowT = this->displayTransformMatrix.rowT;

	edF32Matrix4GetInverseOrthoHard(&_gDisplayCamera.worldToCamera, &_gDisplayCamera.transformMatrix);

	_gDisplayCamera.calculatedRotation = _gDisplayCamera.transformMatrix.rowX;

	if ((this->flags & 0x18000000) == 0x18000000) {
		fVar1 = fVar1 + this->aCameraShadow[0]->fov;
	}
	else {
		if ((this->flags & 0x10000000) == 0) {
			fVar1 = fVar1 + this->fov_0xa34;
		}
		else {
			fVar1 = fVar1 + this->pMouseQuakeCamera_0x4e8->fov;
		}
	}
	edFCameraSetSizeRatioFov(0.03f, this->aspectRatio, fVar1 / 1.333333f, &_gDisplayCamera);
	ComputeFrustrumPlanes(fVar1, &this->displayTransformMatrix);
	return;
}

void CCameraManager::Level_Manage()
{
	SWITCH_MODE switchMode;
	CCameraShadow* pCVar1;
	FrontendCameraView* pFVar2;
	bool bVar3;
	Timer* pTVar4;
	ECameraType EVar5;
	int iVar6;
	float* pfVar7;
	CCamera* pCVar8;
	uint* puVar9;
	float fVar10;
	float fVar11;
	undefined4 uVar12;
	float fVar13;

	if (this->pActiveCamera != (CCamera*)0x0) {
		if ((this->flags & 0x1000000) == 0) {
			pTVar4 = GetTimer();
			this->time_0x4 = pTVar4->cutsceneDeltaTime;
		}
		else {
			pTVar4 = GetTimer();
			this->time_0x4 = pTVar4->lastFrameTime;
		}
		pCVar8 = this->pInitialView_0x4b4;
		do {
			pCVar8->flags_0xc = pCVar8->flags_0xc & 0xff7fffff;
			pCVar8 = pCVar8->pNextCameraView_0xa4;
		} while (pCVar8 != (CCamera*)0x0);
		bVar3 = this->cameraStack.Manage();
		if (bVar3 != false) {
			switchMode = (this->cameraStack).switchMode;
			pCVar1 = (CCameraShadow*)(this->cameraStack).pActiveCamera;
			bVar3 = this->activeCamManager.SwitchActiveCam((this->cameraStack).field_0x218, pCVar1, switchMode);
			if ((bVar3 == false) && (pCVar1 != (CCameraShadow*)0x0)) {
				EVar5 = pCVar1->GetMode();
				if (EVar5 == CT_ShadowSun) {
					this->pActiveCamera = pCVar1;
					pCVar1->sceneIndex = -1;
					pCVar1->sceneFlags = 0;
				}
				else {
					this->pActiveCamera = pCVar1;
				}
			}
			puVar9 = &this->flags;
			if (switchMode == SWITCH_MODE_B) {
				*puVar9 = *puVar9 & 0xfbffffff;
			}
			else {
				*puVar9 = *puVar9 | 0x4000000;
			}
		}
		Func_001947e0();
		pTVar4 = GetTimer();
		if ((pTVar4->timeScale != 0.0f) || ((this->pActiveCamera->flags_0xc & 0x40000) != 0)) {
			/* Camera? */
			pActiveCamera->Manage();
			if ((this->flags & 0x4000000) == 0) {
				pCVar1 = (CCameraShadow*)this->pActiveCamera;
				if (pCVar1 != (CCameraShadow*)0x0) {
					(this->transformationMatrix).rowT = pCVar1->transformationMatrix.rowT;
					(this->shadowCameraLookat) = pCVar1->lookAt;

					/* Focus object camera? */
					this->distance_0xa30 = pActiveCamera->GetDistance();
					this->fov_0xa34 = pActiveCamera->fov;
					this->angle_0xa08.alpha = pActiveCamera->GetAngleAlpha();
					this->angle_0xa08.beta = pActiveCamera->GetAngleBeta();
					this->angle_0xa08.gamma = pActiveCamera->GetAngleGamma();
				}
			}
			else {
				IMPLEMENTATION_GUARD(
				iVar6 = FUN_003feb30((edF32VECTOR4*)&this->activeCamManager);
				if ((iVar6 != 0) &&
					(pCVar1 = (CCameraShadow*)(this->activeCamManager).pActiveCamera, pCVar1 != (CCameraShadow*)0x0)) {
					EVar5 = (*((pCVar1->cameraExt).camera.objBase.pVTable)->GetMode)((CCamera*)pCVar1);
					if (EVar5 == CT_ShadowSun) {
						this->pActiveCamera = pCVar1;
						pCVar1->sceneIndex = -1;
						pCVar1->sceneFlags = 0;
					}
					else {
						this->pActiveCamera = pCVar1;
					}
				}
				puVar9 = &this->flags;
				if ((this->activeCamManager).field_0x280 == -1) {
					*puVar9 = *puVar9 & 0xfbffffff;
				}
				else {
					*puVar9 = *puVar9 | 0x4000000;
				}
				if ((this->activeCamManager).field_0x280 != -1) {
					pfVar7 = (float*)FUN_003fe590((int)&this->activeCamManager);
					fVar13 = pfVar7[1];
					fVar10 = pfVar7[2];
					fVar11 = pfVar7[3];
					(this->shadowCameraLookat).x = *pfVar7;
					(this->shadowCameraLookat).y = fVar13;
					(this->shadowCameraLookat).z = fVar10;
					(this->shadowCameraLookat).w = fVar11;
					fVar13 = pfVar7[5];
					fVar10 = pfVar7[6];
					fVar11 = pfVar7[7];
					(this->field_0x80).x = pfVar7[4];
					(this->field_0x80).y = fVar13;
					(this->field_0x80).z = fVar10;
					(this->field_0x80).w = fVar11;
					*(float*)&this->angle_0xa08 = pfVar7[8];
					*(float*)&this->field_0xa0c = pfVar7[9];
					*(float*)&this->field_0xa10 = pfVar7[10];
					*(float*)&this->distance_0xa30 = pfVar7[0xb];
					this->fov_0xa34 = pfVar7[0xc];
				})
			}
			pCVar8 = this->pInitialView_0x4b4;
			do {
				if (((pCVar8->flags_0xc & 0x200000) != 0) && ((pCVar8->flags_0xc & 2) != 0)) {
					IMPLEMENTATION_GUARD(
					CCamera::Func_0019a120(pCVar8);)
				}
				pCVar8 = pCVar8->pNextCameraView_0xa4;
			} while (pCVar8 != (CCamera*)0x0);
			//IMPLEMENTATION_GUARD(
			//Func_00194e00(this);)
		}
		if ((this->flags & 0x10000000) != 0) {
			//IMPLEMENTATION_GUARD(
			//this->pMouseQuakeCamera_0x4e8->Manage();
			//)
		}
		BuildDisplayMatrix();
		if (this->pFrontendCamera_0x4e4 != (FrontendCameraView*)0x0) {
			this->pFrontendCamera_0x4e4->Manage();
			pFVar2 = this->pFrontendCamera_0x4e4;
			_gFrontEndCamera.position.x = pFVar2->transformationMatrix.da;
			_gFrontEndCamera.position.y = pFVar2->transformationMatrix.db;
			_gFrontEndCamera.position.z = pFVar2->transformationMatrix.dc;
			_gFrontEndCamera.position.w = pFVar2->transformationMatrix.dd;
			pFVar2 = this->pFrontendCamera_0x4e4;
			_gFrontEndCamera.lookAt = pFVar2->lookAt;
			CameraSet3DPos(&_gFrontEndCamera);
			edFCameraSetSizeRatioFov (0.03f, this->aspectRatio, this->pFrontendCamera_0x4e4->fov / 1.333333f, &_gFrontEndCamera);
		}
		ed3DSceneComputeCameraToScreenMatrix(CScene::_scene_handleA, &this->matrix_0x10);
	}
	return;
}

void CCameraManager::LevelLoading_Begin()
{
	Level_Init(false);
	return;
}

bool CCameraManager::LevelLoading_Manage()
{
	Level_Manage();
	return false;
}

void edFCameraSetSizeRatioFov(float halfFOV, float aspectRatio, float sizeRatio, edFCamera* pCamera)
{
	pCamera->horizontalHalfFOV = halfFOV * aspectRatio;
	pCamera->halfFOV = halfFOV;
	pCamera->verticalHalfFOV = halfFOV / sizeRatio;
	return;
}

void CCameraManager::Level_ClearInternalData()
{
	edFCamera* m0;
	int iVar2;

	_gDisplayCamera.position.x = 0.0f;
	_gDisplayCamera.position.y = 0.0f;
	_gDisplayCamera.position.z = 0.0f;
	_gDisplayCamera.position.w = 1.0f;
	_gDisplayCamera.lookAt.x = 0.0f;
	_gDisplayCamera.lookAt.y = 0.0f;
	_gDisplayCamera.lookAt.z = 1.0f;
	_gDisplayCamera.lookAt.w = 1.0f;
	_gDisplayCamera.rotationZ = 0.0f;
	edFCameraSetSizeRatioFov(0.0001875f, 1.333333f, 0.25f, &_gDisplayCamera);
	CameraSet3DPos(&_gDisplayCamera);
	_gFrontEndCamera.position.x = 0.0f;
	_gFrontEndCamera.position.y = 0.0f;
	_gFrontEndCamera.position.z = 0.0f;
	_gFrontEndCamera.position.w = 1.0f;
	_gFrontEndCamera.lookAt.x = 0.0f;
	_gFrontEndCamera.lookAt.y = 0.0f;
	_gFrontEndCamera.lookAt.z = 1.0f;
	_gFrontEndCamera.lookAt.w = 1.0f;
	_gFrontEndCamera.rotationZ = 0.0f;
	edFCameraSetSizeRatioFov(0.0001875f, 1.333333f, 0.25f, &_gFrontEndCamera);
	CameraSet3DPos(&_gFrontEndCamera);
	iVar2 = 0;
	m0 = gSceneCameras;
	do {
		(m0->position).x = 0.0f;
		(m0->position).y = 0.0f;
		(m0->position).z = 0.0f;
		(m0->position).w = 1.0f;
		(m0->lookAt).x = 0.0f;
		(m0->lookAt).y = 0.0f;
		(m0->lookAt).z = 1.0f;
		(m0->lookAt).w = 1.0f;
		edFCameraSetSizeRatioFov(0.0001875f, 1.333333f, 0.25f, m0);
		CameraSet3DPos(m0);
		this->aCameraShadow[iVar2] = (CCameraShadow*)0x0;
		iVar2 = iVar2 + 1;
		m0 = m0 + 1;
	} while (iVar2 < 10);
	this->cameraStack.Reset();
	this->activeCamManager.ClearActiveCam();
	this->loadedCameraCount_0x9f0 = 0;
	this->count_0x9f4 = 0;
	this->field_0x9f8 = 0;
	this->count_0x9fc = 0;
	this->pInitialView_0x4b4 = (CCamera*)0x0;
	this->pActiveCamera = (CCamera*)0x0;
	this->pFrontendCamera_0x4e4 = (FrontendCameraView*)0x0;
	this->pMouseQuakeCamera_0x4e8 = (CCamera*)0x0;
	this->field_0xa78 = 0;
	this->time_0x4 = 0.0;
	return;
}

CCamera* CCameraManager::AddCamera(ECameraType type, ByteCode* pMemoryStream, char* objName)
{
	CCamera* pCVar1;
	CCamera* pCVar2;
	CCameraGame* pMVar3;
	//MouseCameraView* pMVar4;
	//VSilverBoomy* pVVar5;
	CCameraCinematic* pCinematicCameraView;
	//VExtendedCameraView_110* pVVar6;
	void* pvVar7;
	//FrontendCameraView* pFVar8;
	//DeathCameraViewLocationObject* pDeathCameraView;
	CCamera* pCVar9;
	CCamera* newObjectPtr;

	newObjectPtr = (CCamera*)0x0;
	switch (type) {
	case CT_MouseQuake:
		newObjectPtr = new CCameraMouseQuake();
		break;
	case CT_MouseAround:
		newObjectPtr = new CCameraMouseAroundPerso();
		break;
	case CT_AroundSpecial:
		newObjectPtr = new CCameraMouseAroundPersoSpecial();
		break;
	case CT_Main:
	case 4:
	case CT_KyaWindWall:
	case 6:
	case CT_KyaJamgut:
	case CT_MainCamera:
		newObjectPtr = new CCameraGame(type, pMemoryStream);
		break;
	case CT_SilverBoomy:
		IMPLEMENTATION_GUARD(
		pVVar5 = (VSilverBoomy*)operator.new(0x180);
		newObjectPtr = (long)(int)pVVar5;
		if (newObjectPtr != 0) {
			newObjectPtr = VSilverBoomy::Constructor_0018adb0(pVVar5);
		})
		break;
	case CT_IntView:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = IntViewConstructor ? (newObjectPtr);
		})
		break;
	case CT_ShadowSun:
		newObjectPtr = new CCameraShadow(pMemoryStream);
		break;
	case 0xd:
		// What camera is this?
		IMPLEMENTATION_GUARD();
		newObjectPtr = new CCameraShadow(pMemoryStream);
		break;
	case 0xe:
		IMPLEMENTATION_GUARD(
		pVVar6 = (VExtendedCameraView_110*)operator.new(0x110);
		newObjectPtr = (long)(int)pVVar6;
		if (newObjectPtr != 0) {
			newObjectPtr = VExtendedCameraView_110::Deserialize_0018c630(pVVar6, pMemoryStream);
		})
		break;
	case 0x11:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x100);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = FUN_0018dbe0(newObjectPtr, pMemoryStream, objName);
		})
		break;
	case 0x12:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = FUN_0018d490(newObjectPtr, (int)pMemoryStream);
		})
		break;
	case 0x13:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x120);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = FUN_0018cb70(newObjectPtr, (int)pMemoryStream);
		})
		break;
	case 0x14:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = View::DeserializeView_0018c420(newObjectPtr, pMemoryStream);
		})
		break;
	case CT_Cinematic:
		newObjectPtr = new CCameraCinematic();
		break;
	case 0x17:
		IMPLEMENTATION_GUARD(
		pFVar8 = (FrontendCameraView*)operator.new(0xc0);
		newObjectPtr = (long)(int)pFVar8;
		if (newObjectPtr != 0) {
			FrontendCameraViewConstructor(pFVar8);
			(pFVar8->viewBase).pVTable = (CameraFunctionData*)&DAT_00440940;
		})
		break;
	case CT_Frontend:
		newObjectPtr = new FrontendCameraView();
		break;
	case CT_Death:
		IMPLEMENTATION_GUARD(
		pDeathCameraView = (DeathCameraViewLocationObject*)operator.new(0x120);
		newObjectPtr = (long)(int)pDeathCameraView;
		if (newObjectPtr != 0) {
			newObjectPtr = DeathViewConstructor(pDeathCameraView);
		})
	}
	pCVar9 = newObjectPtr;
	if (newObjectPtr != (CCamera*)0x0) {
		pCVar1 = this->pInitialView_0x4b4;
		if (pCVar1 == (CCamera*)0x0) {
			this->pInitialView_0x4b4 = pCVar9;
			g_CameraViewManager_00448e9c = this;
		}
		else {
			pCVar2 = pCVar1->pNextCameraView_0xa4;
			while (pCVar2 != (CCamera*)0x0) {
				pCVar1 = pCVar1->pNextCameraView_0xa4;
				pCVar2 = pCVar1->pNextCameraView_0xa4;
			}
			pCVar1->pNextCameraView_0xa4 = pCVar9;
		}
		this->loadedCameraCount_0x9f0 = this->loadedCameraCount_0x9f0 + 1;
	}
	return pCVar9;
}

CCamera* CCameraManager::GetDefGameCamera(ECameraType type)
{
	CCamera* pCVar1;
	ECameraType EVar2;

	EDFCAMERA_LOG(LogLevel::Info, "CCameraManager::GetDefGameCamera Looking for: 0x{:x}", (int)type);

	for (pCVar1 = this->pInitialView_0x4b4; (pCVar1 != (CCamera*)0x0 && ((EVar2 = pCVar1->GetMode(), type != EVar2 || (pCVar1->field_0x8 != -100))));
		pCVar1 = pCVar1->pNextCameraView_0xa4) {
	}
	return pCVar1;
}

bool CCameraManager::IsSphereVisible(float param_1, edF32VECTOR4* pSphere, float* param_4)
{
	float fVar1;
	bool bVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar3 = pSphere->x - (this->frustumLocation).x;
	fVar4 = pSphere->y - (this->frustumLocation).y;
	fVar5 = pSphere->z - (this->frustumLocation).z;
	fVar6 = pSphere->w;
	fVar1 = fVar3 * (this->field_0x450).x + fVar4 * (this->field_0x450).y + fVar5 * (this->field_0x450).z;
	*param_4 = fVar1;
	if ((fVar1 + pSphere->w < 0.0f) || (param_1 < fVar1 - pSphere->w)) {
		bVar2 = false;
	}
	else {
		bVar2 = 0.0f <= (this->frustumPlane).ad * fVar3 + (this->frustumPlane).bd * fVar4 + (this->frustumPlane).cd * fVar5 +
			(this->frustumPlane).dd * fVar6 &&
			(0.0f <= (this->frustumPlane).ac * fVar3 + (this->frustumPlane).bc * fVar4 + (this->frustumPlane).cc * fVar5
				+ (this->frustumPlane).dc * fVar6 &&
				(0.0f <= (this->frustumPlane).aa * fVar3 + (this->frustumPlane).ba * fVar4 + (this->frustumPlane).ca * fVar5
					+ (this->frustumPlane).da * fVar6 &&
					0.0f <= (this->frustumPlane).ab * fVar3 + (this->frustumPlane).bb * fVar4 + (this->frustumPlane).cb * fVar5 +
					(this->frustumPlane).db * fVar6));
	}
	return bVar2;
}

bool CCameraManager::IsSphereVisible(float other, edF32VECTOR4* pSphere)
{
	float fVar1;
	bool bVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar3 = pSphere->x - (this->frustumLocation).x;
	fVar4 = pSphere->y - (this->frustumLocation).y;
	fVar5 = pSphere->z - (this->frustumLocation).z;
	fVar6 = pSphere->w;
	fVar1 = fVar3 * (this->field_0x450).x + fVar4 * (this->field_0x450).y + fVar5 * (this->field_0x450).z;
	if ((fVar1 + pSphere->w < 0.0f) || (other < fVar1 - pSphere->w)) {
		bVar2 = false;
	}
	else {
		bVar2 = 0.0f <= (this->frustumPlane).ad * fVar3 + (this->frustumPlane).bd * fVar4 + (this->frustumPlane).cd * fVar5 +
			(this->frustumPlane).dd * fVar6 &&
			(0.0f <= (this->frustumPlane).ac * fVar3 + (this->frustumPlane).bc * fVar4 + (this->frustumPlane).cc * fVar5
				+ (this->frustumPlane).dc * fVar6 &&
				(0.0f <= (this->frustumPlane).aa * fVar3 + (this->frustumPlane).ba * fVar4 + (this->frustumPlane).ca * fVar5
					+ (this->frustumPlane).da * fVar6 &&
					0.0f <= (this->frustumPlane).ab * fVar3 + (this->frustumPlane).bb * fVar4 + (this->frustumPlane).cb * fVar5 +
					(this->frustumPlane).db * fVar6));
	}
	return bVar2;
}

CCamera::CCamera(ByteCode* pMemoryStream)
	: CObject()
{
	CheckpointManagerSubObjB* pCVar1;
	int* piVar2;
	int iVar3;
	uint uVar4;
	EventChunk_24* pEVar5;
	float fVar6;
	edF32VECTOR4 local_10;

	this->specCondition.pData = (int*)0x0;
	this->pNextCameraView_0xa4 = (CCamera*)0x0;
	*(undefined*)&this->field_0xa0 = 0;
	Func_0x34(0);
	fVar6 = pMemoryStream->GetF32();
	this->fov = fVar6;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 1.0;
	(this->transformationMatrix).da = local_10.x;
	(this->transformationMatrix).db = local_10.y;
	(this->transformationMatrix).dc = local_10.z;
	(this->transformationMatrix).dd = 1.0f;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 1.0;
	(this->lookAt).x = local_10.x;
	(this->lookAt).y = local_10.y;
	(this->lookAt).z = local_10.z;
	(this->lookAt).w = 1.0f;
	edF32Vector4SubHard(&local_10, &this->lookAt, (edF32VECTOR4*)&(this->transformationMatrix).da);
	edF32Vector4NormalizeHard(&local_10, &local_10);
	(this->transformationMatrix).ca = local_10.x;
	(this->transformationMatrix).cb = local_10.y;
	(this->transformationMatrix).cc = local_10.z;
	(this->transformationMatrix).cd = local_10.w;
	local_10.x = pMemoryStream->GetF32();
	local_10.y = pMemoryStream->GetF32();
	local_10.z = pMemoryStream->GetF32();
	local_10.w = 0.0;
	(this->transformationMatrix).ba = local_10.x;
	(this->transformationMatrix).bb = local_10.y;
	(this->transformationMatrix).bc = local_10.z;
	(this->transformationMatrix).bd = 0.0;
	edF32Vector4CrossProductHard(&local_10, (edF32VECTOR4*)&(this->transformationMatrix).ba, (edF32VECTOR4*)&(this->transformationMatrix).ca);
	local_10.w = 0.0;
	(this->transformationMatrix).aa = local_10.x;
	(this->transformationMatrix).ab = local_10.y;
	(this->transformationMatrix).ac = local_10.z;
	(this->transformationMatrix).ad = 0.0f;
	iVar3 = pMemoryStream->GetS32();
	this->objectId = iVar3;
	iVar3 = pMemoryStream->GetS32();
	this->field_0x8 = iVar3;
	uVar4 = pMemoryStream->GetU32();
	this->flags_0xc = uVar4 | 0x200000;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50.x = fVar6;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50.y = fVar6;
	fVar6 = pMemoryStream->GetF32();
	this->field_0x50.z = fVar6;
	this->field_0x50.w = 0.0f;
	if ((this->flags_0xc & 2) == 0) {
		this->field_0x8c = 0.0f;
		this->switchMode = SWITCH_MODE_F;
		this->field_0x98 = 0.8f;
		this->field_0x94 = 0;
		this->field_0x9c = 0.8f;
	}
	else {
		pEVar5 = (EventChunk_24*)pMemoryStream->GetS32();
		//this->field_0x80 = pEVar5;
		//pCVar1 = (CheckpointManagerSubObjB*)pMemoryStream->currentSeekPos;
		//pMemoryStream->currentSeekPos = (char*)&pCVar1->field_0x4;
		//if (pCVar1->pActor_0x0 != (AActorBase*)0x0) {
		//	pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + (int)pCVar1->pActor_0x0 * 4;
		//}
		//this->field_0x84 = pCVar1;
		//piVar2 = (int*)pMemoryStream->currentSeekPos;
		//pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
		//if (*piVar2 != 0) {
		//	pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 4;
		//}
		//this->field_0x88 = (float)piVar2;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x8c = fVar6;
		iVar3 = pMemoryStream->GetS32();
		this->switchMode = (SWITCH_MODE)iVar3;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x98 = fVar6;
		iVar3 = pMemoryStream->GetS32();
		this->field_0x94 = iVar3;
		fVar6 = pMemoryStream->GetF32();
		this->field_0x9c = fVar6;
		if (this->field_0x80 == (EventChunk_24*)0xffffffff) {
			this->flags_0xc = this->flags_0xc & 0xfffffffd;
		}
	}
	piVar2 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar2 * 0x10;
	}
	(this->specCondition).pData = (int*)piVar2;
	return;
}

CCamera::CCamera()
	:CObject()
{
	(this->specCondition).pData = (int*)0x0;
	this->pNextCameraView_0xa4 = (CCamera*)0x0;
	edF32Matrix4SetIdentityHard(&this->transformationMatrix);
	this->flags_0xc = 0x200000;
	this->field_0x50.x = 0.0f;
	this->field_0x50.y = 0.0f;
	this->field_0x50.z = 0.0f;
	this->field_0x50.w = 0.0f;
	*(undefined*)&this->field_0xa0 = 0;
	this->objectId = -1;
	this->field_0x8 = -1;
	this->field_0x8c = 0.0f;
	Func_0x34(0x0);
	this->switchMode = SWITCH_MODE_A;
	this->field_0x98 = 0.8f;
	this->field_0x94 = 0;
	this->field_0x9c = 0.8f;
	return;
}

void CCamera::Init()
{
	CEventManager* pEVar1;
	//EventChunk_24* pEVar2;
	//CheckpointManagerSubObjB* pCVar3;
	//Actor* pAVar4;
	edF32VECTOR3 local_10;

	pEVar1 = CScene::ptable.g_EventManager_006f5080;
	edF32Matrix4ToEulerSoft(&this->transformationMatrix, &local_10, "ZXY");
	SetAngleAlpha(local_10.x);
	SetAngleBeta(local_10.y);
	SetAngleGamma(local_10.z);
	if ((this->flags_0xc & 2) != 0) {
		IMPLEMENTATION_GUARD(
		this->field_0x7c = 0;
		pEVar2 = (EventChunk_24*)0x0;
		if (this->field_0x80 != (EventChunk_24*)0xffffffff) {
			pEVar2 = edEventGetChunkZone(pEVar1->activeEventChunkID_0x8, (int)this->field_0x80);
		}
		this->field_0x80 = pEVar2;
		pCVar3 = this->field_0x84;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		}
		pCVar3 = (CheckpointManagerSubObjB*)this->field_0x88;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		})
	}
	return;
}

bool CCamera::Manage()
{
	CActor* pActorA;
	bool returnVal;
	CActor* pAVar1;
	void* actorAddrA;

	if ((this->flags_0xc & 0x800000) == 0) {
		this->flags_0xc = this->flags_0xc | 0x800000;
		actorAddrA = GetTarget();
		if (actorAddrA != 0) {
			pActorA = GetTarget();
			pAVar1 = pActorA->pTiedActor;
			if (pAVar1 == (CActor*)0x0) {
				pAVar1 = GetTarget();
				pAVar1 = pAVar1->GetCollidingActor();
			}
			SetTarget(pAVar1);
		}
		returnVal = false;
	}
	else {
		returnVal = true;
	}
	return returnVal;
}

bool CCamera::AlertCamera(int param_2, int param_3, CCamera* param_4)
{
	int* piVar1;
	int iVar2;
	uint* puVar3;
	int iVar4;
	int iVar5;
	float fVar6;

	if (param_2 == 1) {
		fVar6 = this->field_0x8c;
		if ((this->flags_0xc & 2) != 0) {
			iVar5 = 0;
			iVar4 = 0;
			IMPLEMENTATION_GUARD(
			while (true) {
				piVar1 = (int*)this->field_0x88;
				iVar2 = 0;
				if (piVar1 != (int*)0x0) {
					iVar2 = *piVar1;
				}
				if (iVar2 <= iVar5) break;
				CActor::DoMessage
				((Actor*)this->field_0x84->field_0x4, *(Actor**)((int)piVar1 + iVar4 + 4), 0x2a,
					(ActorCompareStruct*)(uint)(0.0 < fVar6));
				iVar4 = iVar4 + 4;
				iVar5 = iVar5 + 1;
			})
		}
	}
	else {
		if (param_2 == 2) {
			fVar6 = this->field_0x8c;
			if ((this->flags_0xc & 2) != 0) {
				IMPLEMENTATION_GUARD(
				iVar5 = 0;
				iVar4 = 0;
				while (true) {
					piVar1 = (int*)this->field_0x88;
					iVar2 = 0;
					if (piVar1 != (int*)0x0) {
						iVar2 = *piVar1;
					}
					if (iVar2 <= iVar5) break;
					CActor::DoMessage
					((Actor*)this->field_0x84->field_0x4, *(Actor**)((int)piVar1 + iVar4 + 4), 0x29,
						(ActorCompareStruct*)(uint)(0.0 < fVar6));
					iVar4 = iVar4 + 4;
					iVar5 = iVar5 + 1;
				})
			}
		}
		else {
			if (param_2 == 0) {
				puVar3 = &this->flags_0xc;
				if (param_3 == 0) {
					*puVar3 = *puVar3 & 0xffefffff;
				}
				else {
					*puVar3 = *puVar3 | 0x100000;
				}
			}
		}
	}

	return false;
}

float CCamera::GetDistance()
{
	float fVar1;
	edF32VECTOR4 eStack16;

	edF32Vector4SubHard(&eStack16, &this->lookAt, (edF32VECTOR4*)&(this->transformationMatrix).da);
	fVar1 = edF32Vector4GetDistHard(&eStack16);
	return fVar1;
}

float GetAngleXFromVector(edF32VECTOR4* v0)
{
	float pitch;
	float fVar1;

	pitch = atan2f(-v0->y, sqrt(v0->z * v0->z + v0->x * v0->x));
	/* Second Param: 6.283185 (2 Pi) */
	fVar1 = fmodf(pitch, 6.283185);
	return fVar1;
}

float GetAngleYFromVector(edF32VECTOR4* v0)
{
	float fVar1;

	fVar1 = atan2f(v0->x, v0->z);
	fVar1 = fmodf(fVar1, 6.283185);
	return fVar1;
}

float CCamera::GetAngleAlpha()
{
	return GetAngleXFromVector((edF32VECTOR4*)&(this->transformationMatrix).ca);
}

float CCamera::GetAngleBeta()
{
	return GetAngleYFromVector((edF32VECTOR4*)&(this->transformationMatrix).ca);
}

float CCamera::GetAngleGamma()
{
	return 0.0f;
}

void CCameraExt::Init()
{
	CEventManager* pEVar1;
	//EventChunk_24* pEVar2;
	//CheckpointManagerSubObjB* pCVar3;
	//Actor* pAVar4;
	edF32VECTOR3 local_10;

	pEVar1 = CScene::ptable.g_EventManager_006f5080;

	edF32Matrix4ToEulerSoft(&this->transformationMatrix, &local_10, "ZXY");
	SetAngleAlpha(local_10.x);
	SetAngleBeta(local_10.y);
	SetAngleGamma(local_10.z);

	if ((this->flags_0xc & 2) != 0) {
		IMPLEMENTATION_GUARD(
			this->field_0x7c = 0;
		pEVar2 = (EventChunk_24*)0x0;
		if (this->field_0x80 != (EventChunk_24*)0xffffffff) {
			pEVar2 = edEventGetChunkZone(pEVar1->activeEventChunkID_0x8, (int)this->field_0x80);
		}
		this->field_0x80 = pEVar2;
		pCVar3 = this->field_0x84;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		}
		pCVar3 = (CheckpointManagerSubObjB*)this->field_0x88;
		for (pAVar4 = pCVar3->pActor_0x0; pAVar4 != (Actor*)0x0; pAVar4 = (Actor*)((int)&pAVar4[-1].data.field_0x158 + 3))
		{
			pCVar3 = (CheckpointManagerSubObjB*)&pCVar3->field_0x4;
			LinkCheckpointActor_00119a00(pCVar3);
		})
	}
	return;
}

CActor* CCameraExt::GetTarget()
{
	return this->pActorView;
}

void CCameraExt::SetTarget(CActor* pActor)
{
	this->pActorView = pActor;
}

float CCameraExt::GetDistance()
{
	return this->distance;
}

void CCameraExt::SetDistance(float distance)
{
	this->distance = distance;
	return;
}

float CCameraExt::GetAngleAlpha()
{
	return this->angles.alpha;
}

float CCameraExt::GetAngleBeta()
{
	return this->angles.beta;
}

float CCameraExt::GetAngleGamma()
{
	return this->angles.gamma;
}

void CCameraExt::SetAngleAlpha(float angle)
{
	(this->angles).alpha = angle;
	return;
}

void CCameraExt::SetAngleBeta(float angle)
{
	(this->angles).beta = angle;
	return;
}

void CCameraExt::SetAngleGamma(float angle)
{
	(this->angles).gamma = angle;
	return;
}

ECameraType CCamera::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_Frontend: 0x{:x}", (int)CT_Frontend);
	return CT_Frontend;
}

void CCamera::Func_0x34(undefined4 param_2)
{
	this->field_0x70 = param_2;
}

CCameraExt::CCameraExt(ByteCode* pMemoryStream)
	: CCamera(pMemoryStream)
{
	this->pActorView = (CActor*)0x0;
	this->distance = 0.0f;
	this->angles = gF32Vector3Zero;
	return;
}

CCameraExt::CCameraExt()
	: CCamera()
{
	this->pActorView = (CActor*)0x0;
	this->distance = 0.0f;
	this->angles = gF32Vector3Zero;
	return;
}

CCameraGame::CCameraGame(ECameraType type, ByteCode* pMemoryStream)
	: CCameraExt(pMemoryStream)
{
	ECameraType EVar1;
	long lVar2;
	CAMERA_CONFIG* puVar3;

	this->field_0x340 = 0;
	cameraConfig.Create(pMemoryStream);
	lVar2 = subObj_12.Init(pMemoryStream);

	puVar3 = &this->cameraConfig;
	if (lVar2 == 0) {
		puVar3->flags = puVar3->flags & 0xfffdffff;
	}
	else {
		puVar3->flags = puVar3->flags | 0x20000;
	}

	SetMode(type);

	EVar1 = GetMode();
	if (((EVar1 == CT_Main) && ((this->cameraConfig).field_0x28.y < 1.22173f)) && (1.22173f < (this->cameraConfig).field_0x28.z)) {
		(this->cameraConfig).field_0x28.z = 1.22173f;
	}

	flags_0xc = flags_0xc | 0x20000;
	return;
}

CCamFigData* _pfig_data;

void CCamFigData::Create(ByteCode* pByteCode)
{
	float fVar1;
	float fVar2;
	float* pfVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float local_40[14];

	iVar4 = 0;
	pfVar3 = local_40;
	do {
		fVar5 = pByteCode->GetF32();
		iVar4 = iVar4 + 1;
		*pfVar3 = fVar5;
		pfVar3 = pfVar3 + 1;
	} while (iVar4 < 0xe);
	//this->field_0x4 = local_40[0] * 0.01745329;
	//fVar5 = local_40[2] * 0.01745329;
	//this->field_0x64 = fVar5;
	//this->field_0x68 = fVar5 * 3.0;
	//this->field_0x40 = fVar5 * 0.5;
	//this->field_0x44 = fVar5 * 3.0 * 0.5;
	//this->field_0x0 = local_40[3] * 0.01745329;
	//this->field_0x8 = local_40[1];
	//this->field_0xc = local_40[4];
	//this->field_0x10 = local_40[5];
	//this->field_0x14 = local_40[6];
	//this->field_0x94 = local_40[7];
	//this->field_0x98 = local_40[7] * 3.0;
	//this->field_0x204 = local_40[7];
	//this->field_0x208 = local_40[7] * 3.0;
	//this->field_0x8c = local_40[8];
	//this->field_0x18 = local_40[9];
	//this->field_0x1c = local_40[10];
	//this->field_0x20 = 0.2;
	//this->field_0x24 = 0.25;
	//fVar6 = local_40[11] * 3.0;
	//SetMatrixTransformFunc_00213e40(0, &this->field_0xa0, &gF32Vertex4Zero);
	//SetMatrixTransformFunc_00213e40(0, &this->field_0x1b0, &gF32Vertex4Zero);
	//SetMatrixTransformFunc_00213e40(0, &this->field_0x160, &gF32Vertex4Zero);
	//FloatFunc_002144c0(0, 0, (int)&this->field_0x1f0);
	//fVar2 = gF32Vertex4Zero.w;
	//fVar1 = gF32Vertex4Zero.z;
	//fVar5 = gF32Vertex4Zero.y;
	//this->field_0x2e0 = gF32Vertex4Zero.x;
	//this->field_0x2e4 = fVar5;
	//this->field_0x2e8 = fVar1;
	//this->field_0x2ec = fVar2;
	//fVar2 = gF32Vertex4Zero.w;
	//fVar1 = gF32Vertex4Zero.z;
	//fVar5 = gF32Vertex4Zero.y;
	//this->field_0x250 = gF32Vertex4Zero.x;
	//this->field_0x254 = fVar5;
	//this->field_0x258 = fVar1;
	//this->field_0x25c = fVar2;
	//SetMatrixTransformFunc_00213e40(0, &this->field_0x120, &gF32Vertex4Zero);
	//this->field_0x28 = 0.5;
	//(this->field_0xa0).bb = local_40[11];
	//(this->field_0xa0).bc = fVar6;
	//(this->field_0x1b0).bb = local_40[11];
	//(this->field_0x1b0).bc = fVar6;
	//(this->field_0x160).bb = local_40[11];
	//(this->field_0x160).bc = fVar6;
	//(this->field_0x120).bb = local_40[11];
	//(this->field_0x120).bc = fVar6;
	//(this->field_0xa0).ad = local_40[12];
	//(this->field_0x160).ad = local_40[13];
	//fVar2 = gF32Vector4Zero.w;
	//fVar1 = gF32Vector4Zero.z;
	//fVar5 = gF32Vector4Zero.y;
	//(this->field_0x290).x = gF32Vector4Zero.x;
	//(this->field_0x290).y = fVar5;
	//(this->field_0x290).z = fVar1;
	//(this->field_0x290).w = fVar2;
	return;
}

int _b_use_fig_data;

void CCamConfig::ResetWithConfig()
{
	this->field_0x74 = this->field_0xc;
	this->field_0x78 = this->field_0x10;
	this->field_0x7c = this->field_0x14;
	this->field_0x84 = this->field_0x28.x;
	this->field_0x90 = 0;
	this->field_0x8c = 0;
	this->field_0x80 = 0;
	this->field_0x94 = this->field_0x38.x;
	this->field_0x98 = this->field_0x38.y;
	this->field_0xa0 = this->field_0x38.z;
	this->field_0xac = 0;
	this->field_0xa8 = 0;
	this->field_0x9c = 0;
	this->field_0xb0 = this->field_0x48.z;
	this->field_0xb4 = this->field_0x48.w;
	this->field_0xbc = this->field_0x58.x;
	this->field_0xc0 = this->field_0x58.x;
	this->field_0xc8 = 0;
	this->field_0xc4 = 0;
	this->field_0xb8 = 0;
	this->field_0xcc = this->field_0x68;
	this->field_0xd0 = this->field_0x6c;
	return;
}

void CCameraGame::InitFromConfig(CAMERA_CONFIG* pConfig)
{
	bool bVar1;
	ECameraType EVar2;
	CCamConfig* pCVar3;
	uint* puVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	*static_cast<CAMERA_CONFIG*>(&this->cameraConfig) = *pConfig;

	this->cameraConfig.ResetWithConfig();

	this->fov = this->cameraConfig.field_0x18;
	SetTarget(this->cameraConfig.pActorRefA.pObj);
	(this->cameraConfig).field_0x70 = 0;
	this->field_0x1ac = 0;
	this->field_0x1b0 = 0;
	this->field_0x1b4 = 0;
	puVar4 = &(this->cameraConfig).field_0x70;

	if ((this->cameraConfig.flags & 0x100) == 0) {
		bVar1 = this->cameraConfig.field_0x38.w != 0.0f;
		if ((!bVar1) && (bVar1 = true, this->cameraConfig.field_0x48.x == 0.0f)) {
			bVar1 = false;
		}
		pCVar3 = &this->cameraConfig;
		if (bVar1) {
			pCVar3->flags = pCVar3->flags | 0x100;
		}
		else {
			pCVar3->flags = pCVar3->flags & 0xfffffeff;
		}
	}

	pCVar3 = &this->cameraConfig;
	if (this->cameraConfig.field_0x1c == 0.0) {
		pCVar3->flags = pCVar3->flags & 0xfffffdff;
	}
	else {
		pCVar3->flags = pCVar3->flags | 0x200;
	}

	EVar2 = GetMode();
	if ((EVar2 == CT_Main) && (this->field_0x8 == -100)) {
		this->cameraConfig.flags = this->cameraConfig.flags | 2;
		this->cameraConfig.flags = this->cameraConfig.flags | 0x80;
		this->cameraConfig.flags = this->cameraConfig.flags | 0x800;
	}

	*puVar4 = *puVar4 | 0x100;
	if ((this->cameraConfig.flags & 2) == 0) {
		*puVar4 = *puVar4 & 0xffffffdf;
	}
	else {
		*puVar4 = *puVar4 | 0x20;
	}
	if ((this->cameraConfig.flags & 0x80) == 0) {
		*puVar4 = *puVar4 & 0xffffffbf;
	}
	else {
		*puVar4 = *puVar4 | 0x40;
	}
	if ((this->cameraConfig.flags & 0x800) == 0) {
		*puVar4 = *puVar4 & 0xffffff7f;
	}
	else {
		*puVar4 = *puVar4 | 0x80;
	}
	if ((this->cameraConfig.flags & 4) == 0) {
		*puVar4 = *puVar4 & 0xfffffffb;
	}
	else {
		*puVar4 = *puVar4 | 4;
	}
	if ((this->cameraConfig.flags & 0x10) == 0) {
		*puVar4 = *puVar4 & 0xfffffff7;
	}
	else {
		*puVar4 = *puVar4 | 8;
	}
	return;
}

void CCameraExt::ComputeTargetOffset(edF32VECTOR4* v0)
{
	undefined8 uVar1;
	char cVar2;
	CActor* pReceiver;
	CActor* this_00;
	CActor* pOutActor;
	edF32VECTOR4 local_50;
	ActorMessage_7 auStack64;

	cVar2 = false;
	if ((this->flags_0xc & 1) != 0) {
		auStack64.field_0x0 = 5;
		auStack64.field_0x20 = gF32Vector4Zero;
		pReceiver = GetTarget();
		this_00 = GetTarget();
		cVar2 = this_00->DoMessage(pReceiver, (ACTOR_MESSAGE)7, (MSG_PARAM)&auStack64);
		if (cVar2 != false) {
			*v0 = auStack64.field_0x20;
		}
	}

	if (cVar2 == false) {
		local_50 = this->field_0x50;

		if ((this->flags_0xc & 0x80) == 0) {
			*v0 = local_50;
		}
		else {
			pOutActor = GetTarget();
			edF32Matrix4MulF32Vector4Hard(v0, &pOutActor->pMeshTransform->base.transformA, &local_50);
		}
	}
	return;
}

void CCameraExt::ComputeTargetPosition(edF32VECTOR4* param_2)
{
	bool bVar1;
	CActor* this_00;
	CActor* pReceiver;
	CActor* pOutActor;
	edF32VECTOR4 local_20;

	this_00 = GetTarget();
	pReceiver = GetTarget();
	bVar1 = this_00->DoMessage(pReceiver, (ACTOR_MESSAGE)0x49, (MSG_PARAM)&local_20);
	if (bVar1 == false) {
		pOutActor = GetTarget();
		*param_2 = pOutActor->currentLocation;
	}
	else {
		*param_2 = local_20;
	}
	return;
}

void CCameraGame::UpdateTarget(edF32VECTOR4* v0, bool doSomething)
{
	ECameraType EVar1;
	CActor* pActorA;
	CActor* pCVar2;
	CActor* pActorB;
	CActorHero* pHero;
	uint uVar3;
	long lVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	fVar5 = 1.0;
	ComputeTargetOffset(v0);
	EVar1 = this->GetMode();
	if (((EVar1 == CT_Main) &&
		(pActorA = this->GetTarget(), pActorA->typeID == 6)) &&
		((this->field_0x2ec & 1) == 0)) {
		pHero = (CActorHero*)this->GetTarget();
		lVar4 = pHero->IsFightRelated(pHero->curBehaviourId);
		if (lVar4 != 0) {
			v0->x = 0.0f;
			v0->y = 1.4f;
			v0->z = 0.0f;
		}
	}
	EVar1 = this->GetMode();
	if (((EVar1 == 9) && ((this->flags_0xc & 1) != 0)) &&
		(pActorB = this->GetTarget(),
			pActorB->typeID == 6)) {
		v0->x = 0.0f;
		v0->y = 0.72f;
		v0->z = 0.0f;
	}

	if (((this->flags_0xc & 1) == 0) &&
		(pCVar2 = this->GetTarget(), pCVar2->typeID == 6)) {
		pHero = (CActorHero*)this->GetTarget();
		IMPLEMENTATION_GUARD_LOG(
		uVar3 = pHero->TestState_IsCrouched(0xffffffff);
		if (uVar3 != 0) {
			v0->y = (float)&DAT_3f333333;
		})
	}

	fVar7 = (this->cameraConfig).field_0x58.x;
	if (this->field_0x208 < fVar7 - 0.001f) {
		fVar5 = edFIntervalUnitDstLERP(this->field_0x208, (this->cameraConfig).field_0x58.y, fVar7);
		v0->x = v0->x * fVar5;
		v0->z = v0->z * fVar5;
	}
	EVar1 = this->GetMode();
	if (EVar1 == CT_KyaJamgut) {
		fVar5 = 1.0f;
	}
	else {
		if (this->field_0x208 < (this->cameraConfig).field_0x58.x - 0.001f) {
			fVar5 = edFIntervalUnitSrcLERP(fVar5, 0.5f, 0.05f);
		}
		else {
			fVar5 = 0.05f;
		}

		EVar1 = this->GetMode();
		if ((((EVar1 == CT_Main) && (v0->y = v0->y - 0.15f, ((this->cameraConfig).flags & 0x20000) != 0)) &&
			((this->field_0x2ec & 1) != 0)) && ((this->field_0x2ec & 2) == 0)) {
			v0->x = 0.0f;
			v0->z = 0.0f;
		}
	}

	if (doSomething != false) {
		edF32Vector4LERPHard(fVar5, v0, &this->field_0x2a0, v0);
		this->field_0x2a0 = *v0;
	}
	return;
}

edF32VECTOR4 g_CameraVector;
edF32VECTOR4 g_CameraVectorB;

int g_CameraNum = 1;
int DAT_0048d344 = 0;
int DAT_0048d348 = 0;
int DAT_0048d34c = 0;

void CCameraGame::Reset()
{
	edF32VECTOR4* peVar1;
	CActor* pCVar2;
	edF32VECTOR4* peVar3;
	ECameraType EVar4;
	edF32VECTOR4* peVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 eStack16;

	g_CameraViewManager_00448e9c->KeepSameParam(this, 0x37);

	pCVar2 = GetTarget();
	if (pCVar2 != (CActor*)0x0) {
		UpdateTarget(&this->field_0x2a0, true);
		ComputeTargetPosition(&this->lookAt);
		UpdateTarget(&eStack16, true);
		peVar5 = &this->lookAt;
		edF32Vector4AddHard(peVar5, peVar5, &eStack16);
		fVar8 = this->lookAt.y;
		fVar6 = this->lookAt.z;
		fVar7 = this->lookAt.w;
		this->field_0x1c0 = this->lookAt;
		
		*(undefined4*)&this->field_0x1d0 = 0;
		*(undefined4*)&this->field_0x1d4 = 0;
		*(undefined4*)&this->field_0x1e0 = 0;
		*(undefined4*)&this->field_0x1d8 = 0;
		*(undefined4*)&this->field_0x1dc = 0;
		this->field_0x1e4 = -1.0f;
		pCVar2 = GetTarget();
		fVar6 = GetAngleYFromVector(&pCVar2->rotationQuat);
		this->field_0x1e8 = fVar6;
	}

	fVar6 = 0.25f;
	this->gameLookAt = this->lookAt;
	fVar7 = this->cameraConfig.field_0x28.x;
	this->field_0x290 = fVar7;
	this->field_0x200 = fVar7;
	this->field_0x204 = this->cameraConfig.field_0x38.z;
	fVar7 = (this->cameraConfig).field_0xc0;
	if (0.25 <= fVar7) {
		fVar6 = fVar7;
	}
	this->field_0x208 = fVar6;
	//peVar3 = (edF32VECTOR4*)&DAT_00000010;
	//peVar5 = &local_70;
	//peVar1 = peVar5;
	//while (peVar1 != (edF32VECTOR4*)0x0) {
	//	*(undefined*)&peVar5->x = 0;
	//	peVar5 = (edF32VECTOR4*)((int)&peVar5->x + 1);
	//	peVar3 = (edF32VECTOR4*)((int)&peVar3[-1].w + 3);
	//	peVar1 = peVar3;
	//}
	local_70.z = -this->field_0x208;
	edF32Matrix4RotateXHard(this->field_0x200, &eStack80, &gF32Matrix4Unit);
	edF32Matrix4RotateYHard(this->field_0x204, &eStack80, &eStack80);
	edF32Matrix4MulF32Vector4Hard(&local_70, &eStack80, &local_70);
	edF32Vector4AddHard(&eStack96, &this->lookAt, &local_70);
	edF32Vector4SubHard(&local_70, &this->lookAt, &eStack96);
	fVar6 = edF32Vector4GetDistHard(&local_70);
	this->distance = fVar6;
	fVar6 = GetAngleXFromVector(&local_70);
	this->angles.x = fVar6;
	fVar6 = GetAngleYFromVector(&local_70);
	this->angles.y = fVar6;
	SetAngleGamma(0.0f);
	this->field_0x2b4 = this->cameraConfig.field_0xc;
	*(undefined4*)&this->field_0x234 = 0;
	*(undefined4*)&this->field_0x230 = 0;
	*(undefined4*)&this->field_0x22c = 0;
	*(undefined4*)&this->field_0x228 = 0;
	*(undefined4*)&this->field_0x224 = 0;
	*(undefined4*)&this->field_0x220 = 0;

	this->field_0x260 = gF32Vertex4Zero;

	this->field_0x270 = gF32Vector4Zero;

	*(undefined4*)&this->field_0x280 = 0;
	*(undefined4*)&this->field_0x1bc = 0;
	*(undefined4*)&this->field_0x20c = 0;
	*(undefined4*)&this->field_0x214 = 0x40000000;
	//SPEED_DYN::Init(0, this->cameraConfig.field_0x28.w, &this->field_0x290);
	this->field_0x2e4 = 3;
	this->field_0x2e8 = 0;
	this->field_0x2ec = 0;
	this->field_0x2e0 = (this->cameraConfig).field_0x70;
	this->field_0x300 = gF32Vertex4Zero;
	this->field_0x330 = gF32Vertex4Zero;
	this->field_0x444 = 0;
	this->field_0x2f0 = 0;
	this->field_0x340 = 0;
	EVar4 = GetMode();
	switch (EVar4) {
	case CT_Main:
		(this->field_0x450).x = 999.0f;
		(this->field_0x450).y = 0.0f;
		g_CameraNum = 1;
		DAT_0048d344 = 0;
		DAT_0048d348 = 0;
		DAT_0048d34c = 0;
		g_CameraVector = gF32Vertex4Zero;
		g_CameraVectorB = gF32Vector4Zero;
		break;
	case 4:
		_b_use_fig_data = 0;
		break;
	case CT_KyaJamgut:
		this->field_0x460 = 0;
		this->field_0x450 = gF32Vertex4Zero;

		this->field_0x464 = 0.05;
		this->field_0x468 = 0.5;
		this->field_0x46c = 0.5;
		(this->field_0x470).x = 65.0;
		(this->field_0x470).y = 0.25;
		(this->field_0x470).z = 0.125;
		(this->field_0x470).w = 0.0625;
		this->field_0x480 = 0.0375;
	}
	return;
}

void CCameraGame::Init()
{
	float fVar1;
	bool bVar2;
	CActor* pCVar3;
	ECameraType EVar4;
	float fVar5;
	float fVar6;

	CCameraExt::Init();

	_pfig_data = (CCamFigData*)0x0;
	_b_use_fig_data = 0;
	this->flags_0xc = this->flags_0xc | 0x20000;
	this->cameraConfig.pActorRefA.Init();
	this->cameraConfig.pActorRefB.Init();
	InitFromConfig(&this->cameraConfig);

	if ((this->cameraConfig.flags & 0x18000) == 0) {
		this->field_0x2dc = CScene::_pinstance->field_0x118;
	}
	else {
		this->field_0x2dc = 0.0f;
		if ((this->cameraConfig.flags & 0x8000) != 0) {
			this->field_0x2dc = this->field_0x2dc + 50.0;
		}
		if ((this->cameraConfig.flags & 0x10000) != 0) {
			this->field_0x2dc = this->field_0x2dc + 100.0;
		}
	}

	Reset();

	fVar5 = this->field_0x50.y;
	fVar6 = 1.0f;
	if (fVar5 != 0.0f) {
		fVar6 = fVar5;
	}

	fVar5 = fVar6 * (0.965992f / 0.25857207f);
	this->field_0x23c = sqrtf(fVar6 * fVar6 + fVar5 * fVar5);
	this->field_0x244 = 1.0472f;
	this->field_0x248 = 0.2617994f;
	this->field_0x24c = 0;
	this->field_0x240 = 0;
	pCVar3 = GetTarget();
	if (pCVar3 == (CActor*)0x0) {
		this->field_0xd0 = 0;
	}
	else {
		pCVar3 = GetTarget();
		bVar2 = IsKindOfObject(2);
		if (bVar2 == false) {
			pCVar3 = (CActor*)0x0;
		}
		else {
			pCVar3 = GetTarget();
		}
		this->field_0xd0 = pCVar3;
	}

	this->field_0x4a0 = 0;
	this->field_0x4a4 = 0xffffffff;
	EVar4 = GetMode();
	switch (EVar4) {
	case 4:
		this->flags_0xc = this->flags_0xc | 1;
		this->cameraConfig.flags = this->cameraConfig.flags | 0x6000;
		this->cameraConfig.flags = this->cameraConfig.flags & 0xfffffbbe;
		this->switchMode = SWITCH_MODE_F;
		this->field_0x98 = 0.8f;
		break;
	case CT_KyaWindWall:
		this->cameraConfig.flags = this->cameraConfig.flags | 0x4000;
		this->cameraConfig.field_0x58.y = 0.35f;
		break;
	case 6:
	case 8:
	case 9:
		this->cameraConfig.flags = this->cameraConfig.flags | 0x4000;
		break;
	case CT_KyaJamgut:
		this->switchMode = SWITCH_MODE_F;
		this->field_0x98 = 0.8f;
		this->field_0x460 = 0;
		this->field_0x450 = gF32Vertex4Zero;
	}
	this->field_0x1b8 = 0;
	return;
}

void CCameraGame::SetMode(ECameraType type)
{
	uint uVar1;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	bool bVar6;
	bool bVar7;
	bool bVar8;

	if (type == CT_MainCamera) {
		uVar1 = flags_0xc;
		bVar2 = (uVar1 & 0x40) != 0;
		bVar3 = (uVar1 & 4) != 0;
		bVar4 = (uVar1 & 8) != 0;
		bVar5 = (uVar1 & 0x10) != 0;
		bVar6 = (uVar1 & 0x20) != 0;
		bVar7 = (uVar1 & 0x100) != 0;
		bVar8 = (uVar1 & 0x200) != 0;
		if ((byte)(bVar3 + bVar2 + bVar4 + bVar5 + bVar6 + bVar7 + bVar8) < 2) {
			if (bVar3) {
				type = CT_Main;
			}
			else {
				if (bVar4) {
					type = CT_KyaJamgut;
				}
				else {
					if (bVar5) {
						type = (ECameraType)8;
					}
					else {
						if (bVar6) {
							type = (ECameraType)9;
						}
						else {
							if (bVar2) {
								type = (ECameraType)4;
							}
							else {
								if (bVar7) {
									type = CT_KyaWindWall;
								}
								else {
									type = (ECameraType)6;
									if (!bVar8) {
										type = CT_Main;
									}
								}
							}
						}
					}
				}
			}
		}
		else {
			type = CT_Main;
		}
	}

	EDFCAMERA_LOG(LogLevel::Info, "CCamera::SetMode New Mode: 0x{:x}", (int)type);
	this->cameraType_0xd4 = type;
	return;
}

ECameraType CCameraGame::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found: 0x{:x}", (int)this->cameraType_0xd4);
	return this->cameraType_0xd4;
}

void CCameraGame::_ResetDataForCut(int param_2)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_80;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 eStack16;

	(this->cameraConfig).field_0x74 = this->cameraConfig.field_0xc;
	this->field_0x2b4 = this->cameraConfig.field_0xc;
	(this->cameraConfig).field_0x78 = this->cameraConfig.field_0x10;
	(this->cameraConfig).field_0x7c = this->cameraConfig.field_0x14;
	this->field_0x1b0 = this->field_0x1b0 & 0xfffff9ff;
	(this->cameraConfig).field_0xc4 = 0;
	(this->cameraConfig).field_0xc8 = 0;
	(this->cameraConfig).field_0xcc = this->cameraConfig.field_0x6c;
	this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
	(this->cameraConfig).field_0x8c = 0;
	(this->cameraConfig).field_0x90 = 0;
	(this->cameraConfig).field_0x94 = this->cameraConfig.field_0x38.y;
	this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
	(this->cameraConfig).field_0xa8 = 0;
	(this->cameraConfig).field_0xac = 0;
	(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.w;

	if (param_2 != 0) {
		this->field_0x1d0 = 0;
		this->field_0x1d4 = 1;
		this->field_0x1e0 = 0;
		this->field_0x1e4 = -1.0f;
		ComputeTargetPosition(&this->lookAt);
		UpdateTarget(&eStack16, true);
		peVar3 = &this->lookAt;
		edF32Vector4AddHard(peVar3, peVar3, &eStack16);
	}

	//peVar2 = (edF32VECTOR4*)&DAT_00000010;
	//peVar3 = &local_80;
	//peVar1 = peVar3;
	//while (peVar1 != (edF32VECTOR4*)0x0) {
	//	*(undefined*)&peVar3->x = 0;
	//	peVar3 = (edF32VECTOR4*)((int)&peVar3->x + 1);
	//	peVar2 = (edF32VECTOR4*)((int)&peVar2[-1].w + 3);
	//	peVar1 = peVar2;
	//}

	local_80.z = -this->distance;
	ComputeTargetPosition(&local_60);
	UpdateTarget(&eStack112, true);
	local_60.y = local_60.y + eStack112.y;
	this->gameLookAt = local_60;

	edF32Matrix4RotateXHard(this->angles.x, &eStack80, &gF32Matrix4Unit);
	edF32Matrix4RotateYHard(this->angles.y, &eStack80, &eStack80);
	edF32Matrix4MulF32Vector4Hard(&local_80, &eStack80, &local_80);
	edF32Vector4AddHard(&local_60, &this->lookAt, &local_80);
	edF32Vector4SubHard(&local_80, &this->gameLookAt, &local_60);

	fVar4 = edF32Vector4GetDistHard(&local_80);
	fVar5 = 0.25f;
	if (0.25f <= fVar4) {
		fVar5 = fVar4;
	}
	this->field_0x208 = fVar5;
	fVar5 = GetAngleXFromVector(&local_80);
	this->field_0x290 = fVar5;
	this->field_0x200 = fVar5;
	fVar5 = GetAngleYFromVector(&local_80);
	this->field_0x204 = fVar5;

	IMPLEMENTATION_GUARD_LOG(
	_Manage_TargetPos(this, &this->lookAt);
	_UpdateCameraData(this, &this->lookAt);)
	return;
}

bool CCameraGame::AlertCamera(int alertType, int pParams, CCamera* param_4)
{
	bool bVar1;
	undefined uVar2;
	ECameraType EVar3;
	CActor* pCVar4;
	CActor* pAVar5;
	CBehaviour* pCVar6;
	CActor* pCVar7;
	long lVar8;
	uint* puVar9;
	edF32VECTOR4* peVar10;
	edF32VECTOR4* peVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack16;

	if (alertType == 5) {
		IMPLEMENTATION_GUARD(
		if (*(int*)pParams == 0) {
			this->field_0x2b4 = this->cameraConfig.field_0xc;
			(this->cameraConfig).field_0x78 = this->cameraConfig.field_0x10;
			(this->cameraConfig).field_0x7c = this->cameraConfig.field_0x14;
			(this->field_0x260).x = 0.0;
			(this->field_0x260).y = 0.0;
			(this->field_0x260).z = 0.0;
			(this->field_0x260).w = 1.0;
			(this->field_0x270).x = 0.0;
			(this->field_0x270).y = 0.0;
			(this->field_0x270).z = 0.0;
			(this->field_0x270).w = 0.0;
			puVar9 = &(this->cameraConfig).field_0x70;
			if ((this->cameraConfig.flags & 2) == 0) {
				*puVar9 = *puVar9 & 0xffffffdf;
			}
			else {
				*puVar9 = *puVar9 | 0x20;
			}
			if ((this->cameraConfig.flags & 0x80) == 0) {
				*puVar9 = *puVar9 & 0xffffffbf;
			}
			else {
				*puVar9 = *puVar9 | 0x40;
			}
			*(undefined4*)&this->field_0x280 = 0;
		}
		else {
			this->field_0x2b4 = 0.0;
			(this->cameraConfig).field_0x78 = 0x3f800000;
			(this->cameraConfig).field_0x7c = 0;
			if (*(int*)&this->field_0x280 == 0) {
				fVar14 = *(float*)(pParams + 0x14);
				fVar12 = *(float*)(pParams + 0x18);
				fVar13 = *(float*)(pParams + 0x1c);
				(this->field_0x260).x = *(float*)(pParams + 0x10);
				(this->field_0x260).y = fVar14;
				(this->field_0x260).z = fVar12;
				(this->field_0x260).w = fVar13;
			}
			edF32Vector4CrossProductHard(&this->field_0x270, (edF32VECTOR4*)(pParams + 0x20), &g_xVector);
			edF32Vector4NormalizeHard(&this->field_0x270, &this->field_0x270);
			fVar12 = edF32Vector4DotProductHard
			(&this->field_0x270, (edF32VECTOR4*)&this->transformationMatrix.ca);
			if (fVar12 < 0.0) {
				edF32Vector4ScaleHard((float)&DAT_bf800000, &this->field_0x270, &this->field_0x270);
			}
			this->field_0x1b0 = this->field_0x1b0 & 0xfffff9ff;
			(this->cameraConfig).field_0xc4 = 0;
			(this->cameraConfig).field_0xc8 = 0;
			(this->cameraConfig).field_0xcc = this->cameraConfig.field_0x6c;
			this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
			(this->cameraConfig).field_0x8c = 0;
			(this->cameraConfig).field_0x90 = 0;
			(this->cameraConfig).field_0x94 = this->cameraConfig.field_0x38.y;
			this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
			(this->cameraConfig).field_0xa8 = 0;
			(this->cameraConfig).field_0xac = 0;
			(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.w;
			*(undefined4*)&this->field_0x280 = 1;
		})
		goto LAB_002c1448;
	}

	if (alertType != 2) {
		if (alertType == 3) {
			IMPLEMENTATION_GUARD(
			pCVar4 = GetTarget();
			if ((pCVar4 != (CActor*)0x0) &&
				(pCVar4 = GetTarget(),
					pCVar4->pMeshNode != (edNODE*)0x0)) {
				pAVar5 = (Actor*)GetTarget();
				ActorFunc_00115680(pAVar5);
				pAVar5 = (Actor*)GetTarget();
				ActorFunc_00115650(pAVar5, 0);
			})
		}
		else {
			if (alertType == 1) {
				IMPLEMENTATION_GUARD(
				EVar3 = GetMode();
				bVar1 = true;
				if (EVar3 == 4) {
					if ((((param_4 != 0) && (lVar8 = (**(code**)(*(int*)param_4 + 0xc))(param_4, 0x80), lVar8 != 0)) &&
						((*(uint*)(param_4 + 0xc) & 1) == 0)) &&
						(((*(uint*)(param_4 + 0xd8) & 0x2000) == 0 && (*(float*)(param_4 + 0x100) == 0.25)))) {
						bVar1 = false;
					}
					if ((bVar1) && ((param_4 == 0 || (lVar8 = (**(code**)(*(int*)param_4 + 0x14))(param_4), lVar8 != 4)))) {
						if (_pfig_data != (CCamFigData*)0x0) {
							_pfig_data->IsValid(1);
						}
						CameraGame::_b_use_fig_data = 0;
					}
				})
			}
		}
		goto LAB_002c1448;
	}

	pCVar4 = GetTarget();
	if ((pCVar4 != (CActor*)0x0) &&
		(pCVar4 = GetTarget(), pCVar4->typeID == 6)) {
		pCVar4 = GetTarget();
		if (pCVar4->curBehaviourId == 8) {
			pCVar6 = pCVar4->GetBehaviour(pCVar4->curBehaviourId);
			IMPLEMENTATION_GUARD(
			*(CBehaviourVtable**)&this->field_0x1ec = pCVar6[0x23].pVTable;)
		}
		else {
			this->field_0x1ec = 0;
		}

		puVar9 = &(this->cameraConfig).field_0x70;
		if (pCVar4->curBehaviourId == 8) {
			*puVar9 = *puVar9 | 0x2000000;
		}
		else {
			*puVar9 = *puVar9 & 0xfdffffff;
		}
	}

	peVar10 = &this->cameraConfig.field_0x28;
	puVar9 = &(this->cameraConfig).field_0x70;
	peVar11 = &this->cameraConfig.field_0x58;
	(this->cameraConfig).field_0x84 = this->cameraConfig.field_0x28.x;
	(this->cameraConfig).field_0xbc = this->cameraConfig.field_0x58.x;
	//FUN_002c8280(this, &this->field_0x2a0, false);

	this->field_0x2e4 = 3;
	this->field_0x2e8 = 0;
	this->field_0x2ec = 0;

	this->field_0x2e0 = *puVar9;

	this->field_0x300 = gF32Vertex4Zero;
	this->field_0x330 = gF32Vertex4Zero;
	this->field_0x444 = 0;
	this->field_0x2f0 = 0;
	this->field_0x340 = 0;

	if ((this->cameraConfig.flags & 0x20000) != 0) {
		this->cameraConfig.field_0x58.z = (this->subObj_12).field_0x20 + peVar11->x + 1.0;
	}

	pCVar4 = GetTarget();
	fVar12 = GetAngleYFromVector(&pCVar4->rotationQuat);
	this->field_0x1e8 = fVar12;
	EVar3 = GetMode();
	if (EVar3 == 4) {
		IMPLEMENTATION_GUARD(
		if (_pfig_data != (CCamFigData*)0x0) {
			CCamFigData::IsValid(_pfig_data, 2);
		}
		CCameraManager::KeepSameParam(g_CameraViewManager_00448e9c, (CCamera*)this, 0x77);
		_ResetDataForCut(this, 0);)
	}
	else {
		ECameraType EVar4;

		if ((param_4 == (CCamera*)0x0) ||
			((((EVar4 = param_4->GetMode(), EVar4 != CT_IntView &&
				(EVar4 = param_4->GetMode(), EVar4 != CT_SilverBoomy)) &&
				(EVar4 = param_4->GetMode(), EVar4 != 0x17)) &&
				((EVar4 = param_4->GetMode(), EVar4 != CT_KyaJamgut ||
					((*puVar9 & 0x2000000) != 0)))))) {

			UpdateTarget(&this->field_0x2a0, false);
			if (pParams == 1) {
				pCVar4 = GetTarget();
				if (pCVar4 != (CActor*)0x0) {
					g_CameraViewManager_00448e9c->KeepSameParam(this, 0x73);
					SetAngleAlpha(peVar10->x);
					EVar3 = GetMode();
					if (EVar3 != 9) {
						SetAngleBeta(this->field_0x1e8);
					}
					_ResetDataForCut(1);
				}
			}
			else {
				g_CameraViewManager_00448e9c->KeepSameParam(this, 0x37);
				if (param_4 != 0) {
					IMPLEMENTATION_GUARD(
					lVar8 = (**(code**)(*(int*)param_4 + 0x2c))(param_4);
					if (lVar8 != 0) {
						pCVar4 = (CActor*)(**(code**)(*(int*)param_4 + 0x2c))(param_4);
						pCVar7 = GetTarget();
						if (pCVar4 == pCVar7) goto LAB_002c10a0;
					}
					CCameraExt::ComputeTargetPosition((CCameraExt*)this, &this->lookAt);
					FUN_002c8280(this, &eStack16, true);
					peVar11 = &this->lookAt;
					edF32Vector4AddHard(peVar11, peVar11, &eStack16);
					edF32Vector4SubHard(&eStack16, &this->lookAt, &this->transformationMatrix.rowT);
					fVar12 = edF32Vector4GetDistHard(&eStack16);
					SetDistance(fVar12);
					fVar12 = GetAngleXFromVector(&eStack16);
					SetAngleAlpha(fVar12);
					fVar12 = GetAngleYFromVector(&eStack16);
					SetAngleBeta(fVar12);)
				}
			LAB_002c10a0:
				EVar3 = GetMode();
				if (EVar3 == CT_KyaJamgut) {
					fVar12 = param_4->GetAngleAlpha();
					if (fVar12 < 0.1963495) {
						(this->cameraConfig).field_0x88 = 0.7853982f;
					}
					else {
						(this->cameraConfig).field_0x88 = peVar10->x;
					}
				}

				pCVar4 = GetTarget();
				if (pCVar4->typeID == 5) {
					pCVar4 = GetTarget();
					SetAngleBeta((pCVar4->rotationEuler).y);
				}
				_ResetDataForCut(0);
				if (((param_4 != 0) && (fVar12 = param_4->GetAngleAlpha(), fVar12 < 0.1963495f)) &&
					((*puVar9 & 0x2000000) != 0)) {
					this->field_0x290 = 0.1963495f;
					this->field_0x200 = 0.1963495f;
				}
			}
		}
		else {
			g_CameraViewManager_00448e9c->KeepSameParam(this, 0x27);
			SetAngleAlpha(peVar10->x);
			SetDistance(peVar11->x);
			_ResetDataForCut(1);
		}
	}

	SetAngleGamma(0.0f);
	pCVar4 = GetTarget();
	uVar2 = IsKindOfObject(4);
	if (uVar2 == 0) {
		this->cameraConfig.flags = this->cameraConfig.flags & 0xffffdfff;
	}
	IMPLEMENTATION_GUARD_LOG(
	_TestRay_Sphere(this);)
LAB_002c1448:
	(this->cameraConfig).field_0x70 = (this->cameraConfig).field_0x70 & 0xfffffffe;
	CCamera::AlertCamera(alertType, pParams, param_4);
	return true;
}

void CAMERA_CONFIG::Create(struct ByteCode* pMemoryStream)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	uVar1 = pMemoryStream->GetU32();
	this->flags = uVar1;
	uVar1 = pMemoryStream->GetS32();
	this->pActorRefA.index = uVar1;
	uVar1 = pMemoryStream->GetS32();
	this->pActorRefB.index = uVar1;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x18 = fVar3;
	fVar3 = this->field_0x18 * 0.01745329f * 0.5f;
	this->field_0x18 = sinf(fVar3) / cosf(fVar3);

	iVar2 = pMemoryStream->GetS32();
	if (iVar2 == 0) {
		this->field_0x20 = 2.0f;
		this->field_0xc = 0.25f;
		this->field_0x10 = 0.25f;
		this->field_0x14 = 0.0f;
		this->field_0x1c = 0.0f;
	}
	else {
		if (iVar2 == 1) {
			fVar3 = pMemoryStream->GetF32();
			this->field_0x20 = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0xc = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0x10 = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0x14 = fVar3;
			fVar3 = pMemoryStream->GetF32();
			this->field_0x1c = fVar3 * 0.01745329f;
		}
	}

	fVar3 = pMemoryStream->GetF32();
	(this->field_0x28).x = fVar3 * 0.01745329f;
	fVar3 = pMemoryStream->GetF32();
	(this->field_0x28).y = fVar3 * 0.01745329f;
	fVar3 = pMemoryStream->GetF32();
	(this->field_0x28).z = fVar3 * 0.01745329f;
	(this->field_0x28).w = 0.8726646f;
	(this->field_0x38).x = 2.443461f;
	(this->field_0x38).y = 2.443461f;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x38.z = fVar3 * 0.01745329f;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x38.w = fVar3 * 0.01745329f;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x48.x = fVar3 * 0.01745329f;
	this->field_0x48.y = 1.047198f;
	this->field_0x48.z = 3.490659f;
	this->field_0x48.w = 3.490659f;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x58.x = fVar3;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x58.y = fVar3;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x58.z = fVar3;
	this->field_0x58.w = 4.0f;
	this->field_0x68 = 8.0f;
	this->field_0x6c = 10.0f;
	fVar3 = this->field_0x58.y;
	if (fVar3 < 2.5f) {
		this->field_0x24 = 2.5f;
	}
	else {
		this->field_0x24 = fVar3 + (this->field_0x58.x - fVar3) * 0.5f;
	}
	return;
}

bool astruct_12::Init(struct ByteCode* pMemoryStream)
{
	uint uVar1;
	float fVar2;

	uVar1 = pMemoryStream->GetU32();
	if (uVar1 == 1) {
		fVar2 = pMemoryStream->GetF32();
		this->field_0x0 = fVar2;
		pMemoryStream->GetF32();
		this->field_0x4 = 0.0;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x8 = fVar2;
		pMemoryStream->GetF32();
		pMemoryStream->GetF32();
		this->field_0xc = 0.1;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x10 = fVar2 * 0.01745329;
		pMemoryStream->GetF32();
		this->field_0x14 = 0.0;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x18 = fVar2 * 0.01745329;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x1c = fVar2;
		fVar2 = pMemoryStream->GetF32();
		this->field_0x20 = fVar2;
		pMemoryStream->GetF32();
		pMemoryStream->GetF32();
	}
	else {
		this->field_0x0 = 0.5;
		this->field_0x4 = 0.0;
		this->field_0x8 = 4.0;
		this->field_0xc = 0.1;
		this->field_0x10 = 0.0;
		this->field_0x14 = 0.0;
		this->field_0x18 = 3.141593;
		this->field_0x1c = 0.0;
		this->field_0x20 = 8.0;
	}
	return uVar1 == 1;
}

CCameraCinematic::CCameraCinematic()
{
	(this->fov) = 0.6f;
	this->transformationMatrix.rowT = gF32Vertex4Zero;
	this->lookAt = gF32Vertex4Zero;
	ResetEvent();
	(this->field_0x8) = -100;
	(this->flags_0xc) = (this->flags_0xc) | 0x37c;
	(this->switchMode) = SWITCH_MODE_B;
	(this->field_0x98) = 0.8f;
	(this->field_0x94) = 0;
	(this->field_0x9c) = 0.8f;
	(this->field_0xb0).x = 0.0f;
	(this->field_0xb0).y = 0.0f;
	(this->field_0xb0).z = 0.0f;
	return;
}


void CCameraCinematic::SetTransform(edF32MATRIX4* transformMatrix)
{
	float fVar1;
	int iVar2;
	edF32VECTOR4 lookAt;

	this->transformationMatrix = *transformMatrix;
	edF32Matrix4ToEulerSoft(transformMatrix, &this->field_0xb0, "ZXY");
	edF32Vector4AddHard(&lookAt, &this->transformationMatrix.rowT, &transformMatrix->rowZ);
	this->lookAt = lookAt;
	return;
}

ECameraType CCameraCinematic::GetMode(void)
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_Cinematic: 0x{:x}", (int)CT_Cinematic);
	return CT_Cinematic;
}

void CCameraCinematic::Init()
{
	CCamera::Init();
	this->flags_0xc = this->flags_0xc | 0x20000;
}

float CCameraCinematic::GetAngleAlpha()
{
	return (this->field_0xb0).alpha;
}

float CCameraCinematic::GetAngleBeta()
{
	return (this->field_0xb0).beta;
}

float CCameraCinematic::GetAngleGamma()
{
	return (this->field_0xb0).gamma;
}

void CameraSet3DPos(edFCamera* pCamera)
{
	float fVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 rowDiffVector;
	edF32VECTOR4 rotationVector;
	edF32MATRIX4 transformedMatrix;

	EDFCAMERA_LOG(LogLevel::Verbose, "CameraSet3DPos Updating position for camera: {:x}", (uintptr_t)pCamera);
	EDFCAMERA_LOG(LogLevel::Verbose, "Position: x={}, y={}, z={}", pCamera->position.x, pCamera->position.y, pCamera->position.z);
	EDFCAMERA_LOG(LogLevel::Verbose, "Look At : x={}, y={}, z={}", pCamera->lookAt.x, pCamera->lookAt.y, pCamera->lookAt.z);

	transformedMatrix = gF32Matrix4Unit;

	rowDiffVector.x = (pCamera->position).x - (pCamera->lookAt).x;
	rowDiffVector.y = (pCamera->position).y - (pCamera->lookAt).y;
	rowDiffVector.z = (pCamera->position).z - (pCamera->lookAt).z;
	rowDiffVector.w = (pCamera->position).w - (pCamera->lookAt).w;
	edF32Vector4NormalizeHard(&rowDiffVector, &rowDiffVector);
	GetAnglesFromVector(&rotationVector, &rowDiffVector);
	if (pCamera->rotationZ != 0.0) {
		edF32Matrix4RotateZHard(pCamera->rotationZ, &transformedMatrix, &transformedMatrix);
	}
	edF32Matrix4RotateXHard(rotationVector.x, &transformedMatrix, &transformedMatrix);
	edF32Matrix4RotateYHard(rotationVector.y, &transformedMatrix, &transformedMatrix);
	edF32Matrix4CopyHard(&pCamera->transformMatrix, &transformedMatrix);
	fVar3 = (pCamera->position).y;
	fVar1 = (pCamera->position).z;
	fVar2 = (pCamera->position).w;
	(pCamera->transformMatrix).da = (pCamera->position).x;
	(pCamera->transformMatrix).db = fVar3;
	(pCamera->transformMatrix).dc = fVar1;
	(pCamera->transformMatrix).dd = fVar2;
	edF32Matrix4GetInverseOrthoHard(&pCamera->worldToCamera, &pCamera->transformMatrix);
	fVar3 = (pCamera->transformMatrix).ab;
	fVar1 = (pCamera->transformMatrix).ac;
	fVar2 = (pCamera->transformMatrix).ad;
	(pCamera->calculatedRotation).x = (pCamera->transformMatrix).aa;
	(pCamera->calculatedRotation).y = fVar3;
	(pCamera->calculatedRotation).z = fVar1;
	(pCamera->calculatedRotation).w = fVar2;
	return;
}

bool CCameraExt::IsKindOfObject(ulong kind)
{
	return (kind & 0x60) != 0;
}

CCameraShadow::CCameraShadow(ByteCode* pByteCode)
{
	int iVar1;

	this->field_0x16cc = 0;
	this->field_0x16c4 = 0;
	iVar1 = GameDListPatch_Register(this, 0x2ee, 0);
	this->patchRegister = iVar1;
	return;
}

bool CCameraShadow::InitDlistPatchable()
{
	CGlobalDListManager* pDlistmanager;
	undefined8 uVar1;
	int iVar2;

	edDListLoadIdentity();
	edDListUseMaterial((edDList_material*)0x0);
	edDListSetProperty(4, this->sceneFlags);
	edDListBegin(0.0f, 0.0f, 0.0f, 3, 0x2ee);
	uVar1 = 0;
	edDListColor4u8(0, 0, 0, 0x7f);
	iVar2 = 0;
	do {
		edDListVertex4f(0.0f, 0.0f, 0.0f, (int)uVar1);
		edDListVertex4f(0.0f, 0.0f, 0.0f, (int)uVar1);
		edDListVertex4f(0.0f, 0.0f, 0.0f, (int)uVar1);
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0xfa);
	edDListEnd();
	edDListSetProperty(4, 0);
	pDlistmanager = (CGlobalDListManager*)CScene::GetManager(MO_GlobalDListManager);
	pDlistmanager->SetActive(this->patchRegister, 1);
	return true;
}

void CCameraShadow::Init()
{
	CCameraExt::Init();
	//(*(code*)((this->cameraExt).camera.objBase.pVTable)->field_0x30)(this, 0);
	*(undefined*)&this->field_0xd0 = 0;
	SetDistance(40.0f);
	SetAngleGamma(1.5708f);
	this->field_0x16e4 = 0.5735765f;
	this->field_0x16e0 = 0.8191521f;
	*(undefined*)&this->field_0x1700 = 0;
	return;
}

ECameraType CCameraShadow::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_ShadowSun: 0x{:x}", (int)CT_ShadowSun);
	return CT_ShadowSun;
}

bool CCameraShadow::Manage()
{
	int iVar1;
	CActorManager* pCVar2;
	CLightManager* pLVar3;
	bool bVar4;
	int iVar5;
	CActor* pActor;
	int iVar6;
	long lVar7;
	edF32MATRIX4* m0;
	long lVar8;
	int* piVar9;
	float fVar10;
	edF32MATRIX4 eStack400;
	edF32VECTOR4 local_150;
	edF32VECTOR4 local_140;
	edF32VECTOR4 eStack304;
	edF32VECTOR4 local_120;
	int local_110;
	int local_10c[67];

	lVar8 = (long)(int)this;
	bVar4 = CCamera::Manage();
	pLVar3 = CScene::ptable.g_LightManager_004516b0;
	if (bVar4 == false) {
		IMPLEMENTATION_GUARD();
	}
	else {
		bVar4 = false;
	}
	return bVar4;
}

CCameraMouse::CCameraMouse()
{
	this->field_0xd4 = -1;
	this->field_0xd0 = 1.0;
	this->flags_0xc = this->flags_0xc | 0x40000;
}

CCameraMouseQuake::CCameraMouseQuake()
{
	this->fov = 0.6f;
}

ECameraType CCameraMouseQuake::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_MouseQuake: 0x{:x}", (int)CT_MouseQuake);
	return CT_MouseQuake;
}

void CCameraMouseQuake::Init()
{
	float fVar1;
	float fVar2;
	float fVar3;

	CCameraExt::Init();
	this->SetDistance(6.0f);
	this->SetAngleAlpha(0.0f);
	this->SetAngleBeta(0.0f);
	this->SetAngleGamma(0.0f);
	this->transformationMatrix.da = gF32Vertex4Zero.x;
	this->transformationMatrix.db = gF32Vertex4Zero.y;
	this->transformationMatrix.dc = gF32Vertex4Zero.z;
	this->transformationMatrix.dd = gF32Vertex4Zero.w;
	this->lookAt.x = 0.0;
	this->lookAt.y = 0.0;
	this->lookAt.z = 1.0;
	this->lookAt.w = 1.0;
	fVar3 = gF32Vector4UnitZ.w;
	fVar2 = gF32Vector4UnitZ.z;
	fVar1 = gF32Vector4UnitZ.y;
	this->transformationMatrix.ca = gF32Vector4UnitZ.x;
	this->transformationMatrix.cb = fVar1;
	this->transformationMatrix.cc = fVar2;
	this->transformationMatrix.cd = fVar3;
	this->flags_0xc = this->flags_0xc | 0x20000;
	this->field_0x8 = -0x9f;
	return;
}

CCameraMouseAroundPerso::CCameraMouseAroundPerso()
{
	this->isSpecial = 0;
}

ECameraType CCameraMouseAroundPerso::GetMode()
{
	ECameraType EVar1;

	EVar1 = CT_AroundSpecial;
	if (this->isSpecial == 0) {
		EVar1 = CT_MouseAround;
	}

	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found: 0x{:x}", (int)EVar1);
	return EVar1;
}

void CCameraMouseAroundPerso::Init()
{
	CCameraExt::Init();
	this->fov = 0.84;
	(this->field_0xe0).x = 0.0;
	(this->field_0xe0).y = 1.4;
	(this->field_0xe0).z = 0.0;
	(this->field_0xe0).w = 0.0;
	this->flags_0xc = this->flags_0xc | 0x20001;
	this->field_0x8 = -0x9f;
	return;
}

CCameraMouseAroundPersoSpecial::CCameraMouseAroundPersoSpecial()
{
	this->isSpecial = 1;
}

bool CCameraManager::PushCamera(CCamera* pCamera, int param_3)
{
	SWITCH_MODE switchMode;
	CCamera* pCamera_00;
	ECameraType EVar1;
	bool bSuccess;
	ulong uVar2;
	uint* puVar3;

	if (((pCamera == (CCamera*)0x0) || (uVar2 = pCamera->specCondition.IsVerified(), uVar2 == 0)) ||
		((this->flags & 0x20000000) == 0)) {
		bSuccess = false;
	}
	else {
		bSuccess = this->cameraStack.Push(pCamera, param_3);
		if (bSuccess != false) {
			switchMode = (this->cameraStack).switchMode;
			pCamera_00 = (this->cameraStack).pActiveCamera;
			bSuccess = this->activeCamManager.SwitchActiveCam((this->cameraStack).field_0x218, pCamera_00, switchMode);
			if ((bSuccess == false) && (pCamera_00 != (CCamera*)0x0)) {
				EVar1 = pCamera_00->GetMode();
				if (EVar1 == CT_ShadowSun) {
					this->pActiveCamera = pCamera_00;
					pCamera_00[0x22].field_0x80 = (EventChunk_24*)0xffffffff;
					pCamera_00[0x22].field_0x84 = (CheckpointManagerSubObjB*)0x0;
				}
				else {
					this->pActiveCamera = pCamera_00;
				}
			}
			puVar3 = &this->flags;
			if (switchMode == SWITCH_MODE_B) {
				*puVar3 = *puVar3 & 0xfbffffff;
			}
			else {
				*puVar3 = *puVar3 | 0x4000000;
			}
			(this->cameraStack).field_0x200 = pCamera->field_0x8c;
		}
		bSuccess = true;
	}
	return bSuccess;
}

bool CCameraManager::PushCamera(int cameraIndex, int param_3)
{
	SWITCH_MODE switchMode;
	CCamera* pCamera;
	bool bVar1;
	ECameraType EVar2;
	int iVar3;
	int iVar4;
	uint* puVar5;
	CCamera* pActiveCamera;

	iVar3 = 0;
	pActiveCamera = this->pInitialView_0x4b4;
	iVar4 = this->count_0x9fc + cameraIndex;
	if (0 < iVar4) {
		do {
			if (pActiveCamera->pNextCameraView_0xa4 != (CCamera*)0x0) {
				pActiveCamera = pActiveCamera->pNextCameraView_0xa4;
			}
			iVar3 = iVar3 + 1;
		} while (iVar3 < iVar4);
	}
	if (((pActiveCamera == (CCamera*)0x0) ||
		(bVar1 = pActiveCamera->specCondition.IsVerified(), bVar1 == false)) ||
		((this->flags & 0x20000000) == 0)) {
		bVar1 = false;
	}
	else {
		bVar1 = this->cameraStack.Push(pActiveCamera, param_3);
		if (bVar1 != false) {
			switchMode = (this->cameraStack).switchMode;
			pCamera = (this->cameraStack).pActiveCamera;
			bVar1 = this->activeCamManager.SwitchActiveCam((this->cameraStack).field_0x218, pCamera, switchMode);
			if ((bVar1 == false) && (pCamera != (CCamera*)0x0)) {
				EVar2 = pCamera->GetMode();
				if (EVar2 == CT_ShadowSun) {
					this->pActiveCamera = pCamera;
					pCamera[0x22].field_0x80 = (EventChunk_24*)0xffffffff;
					pCamera[0x22].field_0x84 = (CheckpointManagerSubObjB*)0x0;
				}
				else {
					this->pActiveCamera = pCamera;
				}
			}
			puVar5 = &this->flags;
			if (switchMode == SWITCH_MODE_B) {
				*puVar5 = *puVar5 & 0xfbffffff;
			}
			else {
				*puVar5 = *puVar5 | 0x4000000;
			}
			(this->cameraStack).field_0x200 = pActiveCamera->field_0x8c;
		}
		bVar1 = true;
	}
	return bVar1;
}

void CCameraManager::ApplyActiveCamera()
{
	CCamera* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	pCVar1 = this->pActiveCamera;
	if (pCVar1 != (CCamera*)0x0) {

		(this->transformationMatrix).rowT = (pCVar1->transformationMatrix).rowT;
		this->shadowCameraLookat = pCVar1->lookAt;
		
		this->distance_0xa30 = this->pActiveCamera->GetDistance();
		this->fov_0xa34 = this->pActiveCamera->fov;

		(this->angle_0xa08).x = this->pActiveCamera->GetAngleAlpha();
		(this->angle_0xa08).y = this->pActiveCamera->GetAngleBeta();
		(this->angle_0xa08).z = this->pActiveCamera->GetAngleGamma();
	}
	return;
}

bool CCameraManager::PopCamera(CCamera* pCameraView)
{
	SWITCH_MODE switchMode;
	CCamera* pCamera;
	bool bSuccess;
	ECameraType EVar1;
	uint* puVar2;

	if ((pCameraView == (CCamera*)0x0) ||
		(bSuccess = this->cameraStack.Pop(pCameraView), bSuccess == false)) {
		bSuccess = false;
	}
	else {
		switchMode = (this->cameraStack).switchMode;
		pCamera = (this->cameraStack).pActiveCamera;
		bSuccess = this->activeCamManager.SwitchActiveCam((this->cameraStack).field_0x218, pCamera, switchMode);
		if ((bSuccess == false) && (pCamera != (CCamera*)0x0)) {
			EVar1 = pCamera->GetMode();
			if (EVar1 == CT_ShadowSun) {
				IMPLEMENTATION_GUARD(
				this->pActiveCamera = pCamera;
				pCamera[0x22].field_0x80 = (ed_zone_3d*)0xffffffff;
				pCamera[0x22].field_0x84 = (ActorAndWaypoint*)0x0;)
			}
			else {
				this->pActiveCamera = pCamera;
			}
		}
		puVar2 = &this->flags;
		if (switchMode == SWITCH_MODE_B) {
			*puVar2 = *puVar2 & 0xfbffffff;
		}
		else {
			*puVar2 = *puVar2 | 0x4000000;
		}
		bSuccess = true;
	}
	return bSuccess;
}

void CCameraManager::SetMainCamera(CCamera* pCamera)
{
	bool bVar1;
	ECameraType EVar2;

	if (pCamera != (CCamera*)0x0) {
		this->cameraStack.SetMainCamera(pCamera);
		this->activeCamManager.SetActiveCam(pCamera);

		bVar1 = this->activeCamManager.SwitchActiveCam(0.0f, pCamera, SWITCH_MODE_B);
		if ((bVar1 == false) && (pCamera != (CCamera*)0x0)) {
			EVar2 = pCamera->GetMode();
			if (EVar2 == CT_ShadowSun) {
				IMPLEMENTATION_GUARD(
				this->pActiveCamera = pCamera;
				pCamera[0x22].field_0x80 = (ed_zone_3d*)0xffffffff;
				pCamera[0x22].field_0x84 = (ActorAndWaypoint*)0x0;)
			}
			else {
				this->pActiveCamera = pCamera;
			}
		}

		this->flags = this->flags & 0xfbffffff;
	}
	return;
}

PACK(
struct S_STREAM_SIMPLE_ACT_COND {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	float field_0xc;

	bool IsVerified(bool bDefault);
});

bool S_STREAM_SIMPLE_ACT_COND::IsVerified(bool bDefault)
{
	int iVar1;
	//ActorState AVar2;
	bool bVar3;
	bool bVar4;
	//AnimResult* pAVar4;
	uint uVar5;
	CActor* pAVar6;

	iVar1 = this->field_0x4;
	bVar4 = bDefault;
	if (iVar1 != -1) {
		pAVar6 = (CActor*)0x0;
		if (iVar1 != -1) {
			IMPLEMENTATION_GUARD(
			pAVar6 = (*(CScene::ptable.g_ActorManager_004516a4)->aActors)[iVar1];)
		}
		switch (this->field_0x8) {
		case 0:
			IMPLEMENTATION_GUARD(
			AVar2 = (pAVar6->data).actorState;
			if (AVar2 == AS_None) {
				bVar4 = false;
			}
			else {
				pAVar4 = (*pAVar6->pVTable->getAnimForState)(pAVar6, AVar2);
				bVar4 = (bool)((byte)pAVar4->flags_0x4 & 1);
			})
			break;
		case 1:
			IMPLEMENTATION_GUARD(
			AVar2 = (pAVar6->data).actorState;
			if (AVar2 == AS_None) {
				uVar5 = 0;
			}
			else {
				pAVar4 = (*pAVar6->pVTable->getAnimForState)(pAVar6, AVar2);
				uVar5 = pAVar4->flags_0x4 & 1;
			}
			bVar4 = uVar5 == 0;)
			break;
		case 2:
			IMPLEMENTATION_GUARD(
			bVar3 = (*pAVar6->pVTable->checkFuncA)(pAVar6, 2);
			if (bVar3 == false) {
				bVar4 = true;
			}
			else {
				bVar4 = this->field_0xc < pAVar6[1].data.anotherVector.y;
			})
			break;
		case 3:
			IMPLEMENTATION_GUARD(
			bVar3 = (*pAVar6->pVTable->checkFuncA)(pAVar6, 2);
			if (bVar3 == false) {
				bVar4 = true;
			}
			else {
				bVar4 = pAVar6[1].data.anotherVector.y < this->field_0xc;
			})
			break;
		case 4:
			IMPLEMENTATION_GUARD(
			bVar4 = ((pAVar6->data).flags & 0x2000001) == 0;)
			break;
		case 5:
			IMPLEMENTATION_GUARD(
			bVar4 = (bool)((byte)(pAVar6->data).flags & 1);)
			break;
		default:
			bVar4 = true;
		}
		iVar1 = this->field_0x0;
		if (iVar1 == 2) {
			bVar4 = (bool)(bVar4 ^ bDefault);
		}
		else {
			if (iVar1 == 1) {
				bVar4 = (bool)(bVar4 | bDefault);
			}
			else {
				if (iVar1 == 0) {
					bVar4 = (bool)(bVar4 & bDefault);
				}
			}
		}
	}
	return bVar4;
}

bool SpecificCondition::IsVerified()
{
	int* piVar1;
	bool bDefault;
	S_STREAM_SIMPLE_ACT_COND* pSimpleActCond;
	int iVar2;

	piVar1 = this->pData;
	if (piVar1 == (int*)0x0) {
		bDefault = true;
	}
	else {
		bDefault = true;
		pSimpleActCond = (S_STREAM_SIMPLE_ACT_COND*)(piVar1 + 1);
		for (iVar2 = *piVar1; 0 < iVar2; iVar2 = iVar2 + -1) {
			bDefault = pSimpleActCond->IsVerified(bDefault);
			pSimpleActCond = pSimpleActCond + 1;
		}
	}
	return bDefault;
}
