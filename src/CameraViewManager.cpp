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

#include "camera.h"
#include "CameraCinematic.h"
#include "CameraMouseQuake.h"
#include "CameraGame.h"
#include "CameraFixePerso.h"
#include "CinematicManager.h"
#include "ActorManager.h"
#include "Actor_Cinematic.h"
#include "ActorFactory.h"
#include "CameraFixe.h"
#include "CameraRail.h"

CCameraManager* CCameraManager::_gThis = NULL;
CCameraManager* CCamera::_gpcam_man = NULL;
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
	this->state.pCamera = pCamera;
	/* WARNING: Could not recover jumptable at 0x003feb1c. Too many branches */
	/* WARNING: Treating indirect jump as call */
	pCamera->AlertCamera(2, 1, (CCamera*)0x0);
	return;
}

void Lerp_ManageLinear(float alpha, ActiveCamManagerEntry* pTarget, ActiveCamManagerEntry* pSource)
{
	float fVar1;
	float fVar2;
	edF32MATRIX4 eStack64;

	CAMERA_LOG(LogLevel::Verbose, "Lerp_ManageLinear Target initial location: {} look at: {} fov: {} distance: {} alpha: {} beta: {} gamma: {}", 
		pTarget->props.location.ToString(), pTarget->props.lookAt.ToString(), pTarget->props.fov, pTarget->props.distance,
		pTarget->props.alpha, pTarget->props.beta, pTarget->props.gamma);

	CAMERA_LOG(LogLevel::Verbose, "Lerp_ManageLinear Source initial location: {} look at: {} fov: {} distance: {} alpha: {} beta: {} gamma: {}",
		pSource->props.location.ToString(), pSource->props.lookAt.ToString(), pSource->props.fov, pSource->props.distance,
		pSource->props.alpha, pSource->props.beta, pSource->props.gamma);

	pTarget->props.fov = pSource->props.fov + alpha * (pTarget->pCamera->fov - pSource->props.fov);
	fVar1 = pTarget->pCamera->GetAngleAlpha();
	fVar1 = edF32GetAnglesDelta(pSource->props.alpha, fVar1);
	fVar1 = edF32Between_2Pi(pSource->props.alpha + alpha * fVar1);
	pTarget->props.alpha = fVar1;
	fVar1 = pTarget->pCamera->GetAngleBeta();
	fVar1 = edF32GetAnglesDelta(pSource->props.beta, fVar1);
	fVar1 = edF32Between_2Pi(pSource->props.beta + alpha * fVar1);
	pTarget->props.beta = fVar1;
	fVar1 = pTarget->pCamera->GetAngleGamma();
	fVar1 = edF32GetAnglesDelta(pSource->props.gamma, fVar1);
	fVar1 = edF32Between_2Pi(pSource->props.gamma + alpha * fVar1);
	pTarget->props.gamma = fVar1;
	edF32Vector4LERPHard(alpha, &pTarget->props.lookAt, &pSource->props.lookAt, &pTarget->pCamera->lookAt);
	(pTarget->props.lookAt).w = 1.0f;
	fVar2 = pSource->props.distance;
	fVar1 = pTarget->pCamera->GetDistance();
	pTarget->props.distance = fVar2 + alpha * (fVar1 - fVar2);

	(pTarget->props.location).x = 0.0f;
	(pTarget->props.location).y = 0.0f;
	(pTarget->props.location).z = pTarget->props.distance;
	(pTarget->props.location).w = 0.0f;

	edF32Matrix4RotateXHard(pTarget->props.alpha, &eStack64, &gF32Matrix4Unit);
	edF32Matrix4MulF32Vector4Hard(&pTarget->props.location, &eStack64, &pTarget->props.location);
	edF32Matrix4RotateYHard(pTarget->props.beta, &eStack64, &gF32Matrix4Unit);
	edF32Matrix4MulF32Vector4Hard(&pTarget->props.location, &eStack64, &pTarget->props.location);
	edF32Vector4SubHard(&pTarget->props.location, &pTarget->props.lookAt, &pTarget->props.location);

	CAMERA_LOG(LogLevel::Verbose, "Lerp_ManageLinear Target post location: {} look at: {} fov: {} distance: {} alpha: {} beta: {} gamma: {}",
		pTarget->props.location.ToString(), pTarget->props.lookAt.ToString(), pTarget->props.fov, pTarget->props.distance,
		pTarget->props.alpha, pTarget->props.beta, pTarget->props.gamma);

	return;
}

void Lerp_Manage(ActiveCamManagerEntry* pTarget, ActiveCamManagerEntry* pSource)
{
	SWITCH_MODE SVar1;
	CCamera* pCVar2;
	undefined8 uVar3;
	CCameraManager* pCVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	pCVar4 = CScene::ptable.g_CameraManager_0045167c;
	pTarget->pCamera->Manage();

	fVar8 = pTarget->blendAlpha;
	if (fVar8 < 1.0f) {
		SVar1 = pTarget->switchMode;

		if (SVar1 == 6) {
			IMPLEMENTATION_GUARD(
			uVar3 = *(undefined8*)&pSource->lookAt;
			fVar8 = (pSource->lookAt).z;
			fVar6 = (pSource->lookAt).w;
			(pTarget->lookAt).x = (float)uVar3;
			(pTarget->lookAt).y = (float)((ulong)uVar3 >> 0x20);
			(pTarget->lookAt).z = fVar8;
			(pTarget->lookAt).w = fVar6;
			uVar3 = *(undefined8*)&pSource->location;
			fVar8 = (pSource->location).z;
			fVar6 = (pSource->location).w;
			(pTarget->location).x = (float)uVar3;
			(pTarget->location).y = (float)((ulong)uVar3 >> 0x20);
			(pTarget->location).z = fVar8;
			(pTarget->location).w = fVar6;
			fVar8 = pSource->beta;
			fVar6 = pSource->gamma;
			pTarget->alpha = pSource->alpha;
			pTarget->beta = fVar8;
			pTarget->gamma = fVar6;
			pTarget->distance = pSource->distance;
			pTarget->fov = pSource->fov;
			iVar5 = (*(code*)((CActorHero::_gThis->character).characterBase.base.base.pVTable)->GetInputManager)
				(CActorHero::_gThis, 0, 1);
			if (*(int*)(iVar5 + 0x5f0) != 0) {
				pTarget->switchMode = SWITCH_MODE_F;
			})
		}
		else {
			if (SVar1 == SWITCH_MODE_F) {
				const float alpha = cosf(pTarget->blendAlpha * 3.141593f + 3.141593f);
				Lerp_ManageLinear(alpha * 0.5f + 0.5f, pTarget, pSource);
				pTarget->blendAlpha = pTarget->blendAlpha + pTarget->blendSpeed * pCVar4->time_0x4;
			}
			else {
				if (SVar1 == SWITCH_MODE_E) {
					Lerp_ManageLinear(fVar8, pTarget, pSource);
					pTarget->blendAlpha = pTarget->blendAlpha + pTarget->blendSpeed * pCVar4->time_0x4;
				}
				else {
					if (SVar1 == SWITCH_MODE_D) {
						fVar6 = cosf(fVar8 * 3.141593f + 3.141593f) * 0.5f + 0.5f;
						pTarget->props.fov = pSource->props.fov + fVar6 * (pTarget->pCamera->fov - pSource->props.fov);

						fVar8 = pTarget->pCamera->GetAngleAlpha();
						fVar8 = edF32GetAnglesDelta(pSource->props.alpha, fVar8);
						fVar8 = edF32Between_2Pi(pSource->props.alpha + fVar6 * fVar8);
						pTarget->props.alpha = fVar8;

						fVar8 = pTarget->pCamera->GetAngleBeta();
						fVar8 = edF32GetAnglesDelta(pSource->props.beta, fVar8);
						fVar8 = edF32Between_2Pi(pSource->props.beta + fVar6 * fVar8);
						pTarget->props.beta = fVar8;

						fVar8 = pTarget->pCamera->GetAngleGamma();
						fVar8 = edF32GetAnglesDelta(pSource->props.gamma, fVar8);
						fVar8 = edF32Between_2Pi(pSource->props.gamma + fVar6 * fVar8);
						pTarget->props.gamma = fVar8;

						edF32Vector4LERPHard(fVar6, &pTarget->props.lookAt, &pSource->props.lookAt, &pTarget->pCamera->lookAt);
						(pTarget->props.lookAt).w = 1.0f;

						edF32Vector4LERPHard(fVar6, &pTarget->props.location, &pSource->props.location, &(pTarget->pCamera->transformationMatrix).rowT);
						(pTarget->props.location).w = 1.0f;

						fVar8 = pTarget->pCamera->GetDistance();
						pTarget->props.distance = pSource->props.distance + fVar6 * (fVar8 - pSource->props.distance);

						pTarget->blendAlpha = pTarget->blendAlpha + pTarget->blendSpeed * pCVar4->time_0x4;
					}
					else {
						if (SVar1 == SWITCH_MODE_C) {
							IMPLEMENTATION_GUARD(
							pTarget->fov = pSource->fov + fVar8 * (pTarget->pCamera->fov - pSource->fov);
							fVar6 = (*((pTarget->pCamera->objBase).pVTable)->GetAngleAlpha)();
							fVar6 = edF32GetAnglesDelta(pSource->alpha, fVar6);
							fVar6 = edF32Between_2Pi(pSource->alpha + fVar8 * fVar6);
							pTarget->alpha = fVar6;
							fVar6 = (*((pTarget->pCamera->objBase).pVTable)->GetAngleBeta)();
							fVar6 = edF32GetAnglesDelta(pSource->beta, fVar6);
							fVar6 = edF32Between_2Pi(pSource->beta + fVar8 * fVar6);
							pTarget->beta = fVar6;
							fVar6 = (*((pTarget->pCamera->objBase).pVTable)->GetAngleGamma)();
							fVar6 = edF32GetAnglesDelta(pSource->gamma, fVar6);
							fVar6 = edF32Between_2Pi(pSource->gamma + fVar8 * fVar6);
							pTarget->gamma = fVar6;
							edF32Vector4LERPHard(fVar8, (edF32VECTOR4*)pTarget, (edF32VECTOR4*)pSource, &pTarget->pCamera->lookAt);
							(pTarget->lookAt).w = 1.0;
							edF32Vector4LERPHard
							(fVar8, &pTarget->location, &pSource->location,
								(edF32VECTOR4*)&(pTarget->pCamera->transformationMatrix).da);
							(pTarget->location).w = 1.0;
							fVar6 = (*((pTarget->pCamera->objBase).pVTable)->GetDistance)();
							pTarget->distance = pSource->distance + fVar8 * (fVar6 - pSource->distance);
							pTarget->blendAlpha = pTarget->blendAlpha + pTarget->blendSpeed * pCVar4->time_0x4;)
						}
					}
				}
			}
		}

		if (1.0f < pTarget->blendAlpha) {
			pTarget->blendAlpha = 1.0f;
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		pCVar2 = pTarget->pCamera;
		fVar7 = (pCVar2->lookAt).y;
		fVar8 = (pCVar2->lookAt).z;
		fVar6 = (pCVar2->lookAt).w;
		(pTarget->lookAt).x = (pCVar2->lookAt).x;
		(pTarget->lookAt).y = fVar7;
		(pTarget->lookAt).z = fVar8;
		(pTarget->lookAt).w = fVar6;
		pCVar2 = pTarget->pCamera;
		fVar7 = (pCVar2->transformationMatrix).db;
		fVar8 = (pCVar2->transformationMatrix).dc;
		fVar6 = (pCVar2->transformationMatrix).dd;
		(pTarget->location).x = (pCVar2->transformationMatrix).da;
		(pTarget->location).y = fVar7;
		(pTarget->location).z = fVar8;
		(pTarget->location).w = fVar6;
		fVar8 = (*((pTarget->pCamera->objBase).pVTable)->GetDistance)();
		pTarget->distance = fVar8;
		pTarget->fov = pTarget->pCamera->fov;
		fVar8 = (*((pTarget->pCamera->objBase).pVTable)->GetAngleAlpha)();
		pTarget->alpha = fVar8;
		fVar8 = (*((pTarget->pCamera->objBase).pVTable)->GetAngleBeta)();
		pTarget->beta = fVar8;
		fVar8 = (*((pTarget->pCamera->objBase).pVTable)->GetAngleGamma)();
		pTarget->gamma = fVar8;)
	}
	return;
}

bool ActiveCamManager::FUN_003feb30()
{
	CCamera* pCVar1;
	undefined8 uVar2;
	undefined8 uVar3;
	undefined8 uVar4;
	undefined8 uVar5;
	ActiveCamManagerEntry* pAVar6;
	bool bVar7;
	undefined4 uVar8;
	undefined4 uVar9;
	float fVar10;
	ActiveCamManagerEntry* pAVar11;
	ActiveCamManagerEntry* pAVar12;
	float fVar13;
	float fVar14;
	ActiveCamManagerEntry* pAVar15;
	int iVar16;
	float fVar17;
	float fVar18;
	float fVar19;

	if (this->activeIndex == -1) {
		bVar7 = false;
	}
	else {
		pCVar1 = (this->state).pCamera;
		(this->state.props).lookAt = pCVar1->lookAt;

		pCVar1 = (this->state).pCamera;
		(this->state.props).location = (pCVar1->transformationMatrix).rowT;

		fVar17 = (this->state).pCamera->GetDistance();
		(this->state.props).distance = fVar17;
		(this->state.props).fov = ((this->state).pCamera)->fov;
		fVar17 = (this->state).pCamera->GetAngleAlpha();
		(this->state.props).alpha = fVar17;
		fVar17 = (this->state).pCamera->GetAngleBeta();
		(this->state.props).beta = fVar17;
		fVar17 = (this->state).pCamera->GetAngleGamma();
		(this->state.props).gamma = fVar17;

		pAVar12 = &this->state;
		iVar16 = 0;
		pAVar15 = this->aEntries;
		pAVar11 = pAVar12;
		if (0 < this->activeIndex + 1) {
			do {
		
				pAVar12 = pAVar15;
				Lerp_Manage(pAVar12, pAVar11);
				iVar16 = iVar16 + 1;
				pAVar15 = pAVar12 + 1;
				pAVar11 = pAVar12;
			} while (iVar16 < this->activeIndex + 1);
		}

		this->state.props = pAVar12->props;

		if (1.0f <= this->aEntries[0].blendAlpha) {
			pCVar1 = (this->state).pCamera;
			pCVar1->AlertCamera(3, this->aEntries[0].switchMode, this->aEntries[0].pCamera);
			pCVar1 = this->aEntries[0].pCamera;
			pCVar1->AlertCamera(4, this->aEntries[0].switchMode, (this->state).pCamera);
			(this->state).pCamera = this->aEntries[0].pCamera;
			iVar16 = 1;
			if (1 < this->activeIndex + 1) {
				pAVar15 = this->aEntries;
				pAVar6 = this->aEntries + 1;
				do {
					*pAVar15 = *pAVar6;
					iVar16 = iVar16 + 1;
					pAVar15 = pAVar15 + 1;
					pAVar6 = pAVar6 + 1;
				} while (iVar16 < this->activeIndex + 1);
			}
			bVar7 = true;
			this->activeIndex = this->activeIndex + -1;
		}
		else {
			bVar7 = false;
		}
	}
	return bVar7;
}

CCamera* ActiveCamManager::FUN_003feab0(int index)
{
	CCamera* pCVar1;
	int iVar2;

	iVar2 = this->activeIndex - index;

	pCVar1 = (CCamera*)0x0;
	if (-2 < iVar2) {
		if (iVar2 == -1) {
			pCVar1 = (this->state).pCamera;
		}
		else {
			pCVar1 = aEntries[iVar2].pCamera;
		}
	}

	return pCVar1;
}

ActiveCamManagerEntry* ActiveCamManager::GetState()
{
	ActiveCamManagerEntry* pAVar1;

	pAVar1 = &this->state;
	if (this->activeIndex == -1) {
		pAVar1 = (ActiveCamManagerEntry*)0x0;
	}
	return pAVar1;
}

void ActiveCamManager::ClearActiveCam()
{
	this->activeIndex = -1;
	return;
}

ActiveCamManager::ActiveCamManager()
{
	this->state.pCamera = (CCamera*)0x0;
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
			pCVar2 = this->state.pCamera;
		}
		else {
			pCVar2 = this->aEntries[iVar4].pCamera;
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
					pCVar2 = this->state.pCamera;
				}
				else {
					pCVar2 = this->aEntries[iVar4].pCamera;
				}
			}
			pCVar2->AlertCamera(1, 1, pCamera);
			iVar4 = this->activeIndex;
			pCVar2 = (CCamera*)0x0;
			if (-2 < iVar4) {
				if (iVar4 == -1) {
					pCVar2 = this->state.pCamera;
				}
				else {
					pCVar2 = this->aEntries[iVar4].pCamera;
				}
			}
			pCamera->AlertCamera(2, 1, pCVar2);
			bVar1 = false;
			this->activeIndex = -1;
			this->state.pCamera = pCamera;
		}
		else {
			if (iVar4 == 7) {
				bVar1 = false;
			}
			else {
				ActiveCamManagerEntry* pEntry;
				if ((iVar4 != -1) &&
					(pEntry = &this->aEntries[this->activeIndex], pEntry->blendAlpha <= pEntry->blendSpeed * (CScene::ptable.g_CameraManager_0045167c)->time_0x4)) {
					this->activeIndex = this->activeIndex + -1;
					iVar4 = this->activeIndex;
					pCVar2 = (CCamera*)0x0;

					if (-2 < iVar4) {
						if (iVar4 == -1) {
							pCVar2 = this->state.pCamera;
						}
						else {
							pCVar2 = this->aEntries[iVar4].pCamera;
						}
					}
					if (pCVar2 == pCamera) {
						return false;
					}
				}

				pCVar2 = (CCamera*)0x0;

				if (-2 < iVar4) {
					if (iVar4 == -1) {
						pCVar2 = this->state.pCamera;
					}
					else {
						pCVar2 = this->aEntries[iVar4].pCamera;
					}
				}

				pCVar2->AlertCamera(1, switchMode, pCamera);
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
							pCVar2 = this->state.pCamera;
						}
						else {
							pCVar2 = this->aEntries[iVar4].pCamera;
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
							pCVar2 = this->state.pCamera;
						}
						else {
							pCVar2 = this->aEntries[iVar4].pCamera;
						}
					}
					pCamera->AlertCamera(2, switchMode, pCVar2);
				}

				ActiveCamManagerEntry* pAVar3 = this->aEntries + this->activeIndex + 1;
				pAVar3->pCamera = pCamera;
				pAVar3->switchMode = switchMode;
				pAVar3->blendAlpha = 0.0f;
				if (param_1 == 0.0f) {
					pAVar3->blendSpeed = 1.25f;
				}
				else {
					pAVar3->blendSpeed = 1.0f / param_1;
				}
				bVar1 = true;
				this->activeIndex = this->activeIndex + 1;
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
	this->pMouseQuakeCamera_0x4e8 = reinterpret_cast<CCameraMouseQuake*>(frontend);
	/* Cinematic */
	AddCamera(CT_Cinematic, &newView, s_Cinematic_0042ae88);
	/* Frontend */
	frontend = AddCamera(CT_Frontend, &newView, s_Frontend_0042ae98);
	this->pFrontendCamera_0x4e4 = frontend;
	frontend = this->pFrontendCamera_0x4e4;
	(frontend->lookAt).x = 0.0f;
	(frontend->lookAt).y = 0.0f;
	(frontend->lookAt).z = 10.0f;
	(frontend->lookAt).w = 1.0f;
	frontend = this->pFrontendCamera_0x4e4;
	(frontend->transformationMatrix).da = 0.0f;
	(frontend->transformationMatrix).db = 0.0f;
	(frontend->transformationMatrix).dc = 0.0f;
	(frontend->transformationMatrix).dd = 1.0f;
	this->pFrontendCamera_0x4e4->fov = 0.3333333f;
	frontend = this->pFrontendCamera_0x4e4;
	_gFrontEndCamera.position = (frontend->transformationMatrix).rowT;
	frontend = this->pFrontendCamera_0x4e4;
	_gFrontEndCamera.lookAt = frontend->lookAt;
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
	this->field_0x480.field_0x20 = 0;
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

void CCameraManager::Level_AddAll(ByteCode* pByteCode)
{
	int iVar1;
	char* name;
	ECameraType mode;

	this->count_0x9fc = this->loadedCameraCount_0x9f0;
	this->count_0x9f4 = pByteCode->GetS32();

	iVar1 = 0;
	if (0 < this->count_0x9f4) {
		do {
			name = pByteCode->GetString();
			mode = (ECameraType)pByteCode->GetU32();
			AddCamera(mode, pByteCode, name);
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->count_0x9f4);
	}

	return;
}

void CCameraManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	int iVar1;
	CCamera* pCamera;

	pCamera = this->pInitialView_0x4b4;
	this->cameraStack.Level_SectorChange(oldSectorId);

	for (; pCamera != (CCamera*)0x0; pCamera = pCamera->pNextCameraView_0xa4) {
		if ((oldSectorId == pCamera->objectId) && (oldSectorId != -1)) {
			pCamera->flags_0xc = pCamera->flags_0xc & 0xffdfffff;
		}

		iVar1 = pCamera->objectId;
		if ((newSectorId == iVar1) || (iVar1 == -1)) {
			pCamera->flags_0xc = pCamera->flags_0xc | 0x200000;
		}
	}

	return;
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
	ulong uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_10;

	fVar2 = 1.0f;
	local_10.xyz = (this->field_0x480).field_0x0.xyz;
	fVar4 = 0.0f;

	if (((this->field_0x480).field_0x20 & 1U) != 0) {
		fVar3 = this->field_0x4b0;

		fVar4 = (this->field_0x480).field_0x28;

		if (fVar3 < fVar4) {
			fVar2 = fVar3 / fVar4;
		}
		else {
			fVar4 = fVar4 + (this->field_0x480).field_0x24;

			if (fVar4 < fVar3) {
				fVar2 = (this->field_0x480).field_0x2c;
				fVar2 = ((fVar2 + fVar4) - fVar3) / fVar2;
			}
		}

		if (((this->field_0x480).field_0x20 & 4U) != 0) {
			IMPLEMENTATION_GUARD(
			local_10.x = (this->field_0x480).field_0x0.x *
				edFCosinus
				[(int)(ABS((this->field_0x4b0 * (this->field_0x480).field_0x10.x - 1.570796) * 1303.797) + 0.5) &
				0x1fff];)
		}

		if (((this->field_0x480).field_0x20 & 2U) != 0) {
			fVar4 = (this->field_0x480).field_0x0.x;
			uVar1 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
			CScene::_pinstance->field_0x38 = uVar1;
			local_10.x = (fVar4 * (float)((uint)(uVar1 >> 0x10) & 0x7fff)) / 32767.0f - (this->field_0x480).field_0x0.x * 0.5f;
		}

		if (((this->field_0x480).field_0x20 & 0x10U) != 0) {
			IMPLEMENTATION_GUARD(
			local_10.y = (this->field_0x480).field_0x0.y *
				edFCosinus
				[(int)(ABS((this->field_0x4b0 * (this->field_0x480).field_0x10.y - 1.570796) * 1303.797) + 0.5) &
				0x1fff];)
		}

		if (((this->field_0x480).field_0x20 & 8U) != 0) {
			fVar4 = (this->field_0x480).field_0x0.y;
			uVar1 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
			CScene::_pinstance->field_0x38 = uVar1;
			local_10.y = (fVar4 * (float)((uint)(uVar1 >> 0x10) & 0x7fff)) / 32767.0f - (this->field_0x480).field_0x0.y * 0.5f;
		}

		if (((this->field_0x480).field_0x20 & 0x40U) != 0) {
			IMPLEMENTATION_GUARD(
			local_10.z = (this->field_0x480).field_0x0.z *
				edFCosinus
				[(int)(ABS((this->field_0x4b0 * (this->field_0x480).field_0x10.z - 1.570796) * 1303.797) + 0.5) &
				0x1fff];)
		}

		if (((this->field_0x480).field_0x20 & 0x20U) != 0) {
			IMPLEMENTATION_GUARD(
			fVar4 = (this->field_0x480).field_0x0.z;
			uVar1 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
			CScene::_pinstance->field_0x38 = uVar1;
			local_10.z = (fVar4 * (float)((uint)(uVar1 >> 0x10) & 0x7fff)) / 32767.0 - (this->field_0x480).field_0x0.z * 0.5;)
		}

		local_10.w = 0.0f;
		fVar4 = (this->field_0x480).field_0x0.w;

		if (((this->field_0x480).field_0x20 & 0x80U) != 0) {
			IMPLEMENTATION_GUARD(
			fVar4 = (this->field_0x480).field_0x0.w;
			uVar1 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
			CScene::_pinstance->field_0x38 = uVar1;
			fVar4 = (fVar4 * (float)((uint)(uVar1 >> 0x10) & 0x7fff)) / 32767.0 - (this->field_0x480).field_0x0.w * 0.5;)
		}

		edF32Vector4ScaleHard(fVar2, &local_10, &local_10);
		edF32Matrix4MulF32Vector4Hard(&local_10, &this->transformationMatrix, &local_10);
		edF32Vector4AddHard(param_2, param_2, &local_10);
		this->field_0x4b0 = this->field_0x4b0 + this->time_0x4;

		if ((this->field_0x480).field_0x2c + (this->field_0x480).field_0x24 + (this->field_0x480).field_0x28 <= this->field_0x4b0) {
			(this->field_0x480).field_0x20 = (this->field_0x480).field_0x20 & 0xfffffffe;
		}
	}

	return fVar4 * fVar2;
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
		IMPLEMENTATION_GUARD_LOG();
		// Check this is correct.
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
		if ((this->flags & CAMERA_HACK_ACTIVE) != 0) {
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
		if ((this->flags & CAMERA_HACK_ACTIVE) == 0) {
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
	CCamera* pFVar2;
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
				
				iVar6 = this->activeCamManager.FUN_003feb30();
				if ((iVar6 != 0) &&
					(pCVar1 = (CCameraShadow*)(this->activeCamManager).state.pCamera, pCVar1 != (CCameraShadow*)0x0)) {
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
				if ((this->activeCamManager).activeIndex == -1) {
					*puVar9 = *puVar9 & 0xfbffffff;
				}
				else {
					*puVar9 = *puVar9 | 0x4000000;
				}

				if ((this->activeCamManager).activeIndex != -1) {
					ActiveCamManagerEntry* pEntry = this->activeCamManager.GetState();
					this->shadowCameraLookat = pEntry->props.lookAt;

					(this->transformationMatrix).rowT = pEntry->props.location;

					(this->angle_0xa08).x = pEntry->props.alpha;
					(this->angle_0xa08).y = pEntry->props.beta;
					(this->angle_0xa08).z = pEntry->props.gamma;

					this->distance_0xa30 = pEntry->props.distance;

					this->fov_0xa34 = pEntry->props.fov;
				}
			}

			pCVar8 = this->pInitialView_0x4b4;
			do {
				if (((pCVar8->flags_0xc & 0x200000) != 0) && ((pCVar8->flags_0xc & 2) != 0)) {
					pCVar8->ManageEvents();
				}
				pCVar8 = pCVar8->pNextCameraView_0xa4;
			} while (pCVar8 != (CCamera*)0x0);

			Func_00194e00();
		}

		if ((this->flags & CAMERA_HACK_ACTIVE) != 0) {
			this->pMouseQuakeCamera_0x4e8->Manage();
		}

		BuildDisplayMatrix();
		if (this->pFrontendCamera_0x4e4 != (CCamera*)0x0) {
			this->pFrontendCamera_0x4e4->Manage();
			pFVar2 = this->pFrontendCamera_0x4e4;
			_gFrontEndCamera.position = pFVar2->transformationMatrix.rowT;
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
	this->pFrontendCamera_0x4e4 = (CCamera*)0x0;
	this->pMouseQuakeCamera_0x4e8 = (CCameraMouseQuake*)0x0;
	this->field_0xa78 = 0;
	this->time_0x4 = 0.0;
	return;
}

CCamera* CCameraManager::AddCamera(ECameraType type, ByteCode* pByteCode, char* objName)
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
		newObjectPtr = new CCameraGame(type, pByteCode);
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
		newObjectPtr = new CCameraShadow(pByteCode);
		break;
	case 0xd:
		// What camera is this?
		IMPLEMENTATION_GUARD();
		newObjectPtr = new CCameraShadow(pByteCode);
		break;
	case 0xe:
		newObjectPtr = new CCameraFixe(pByteCode);
		break;
	case CT_RailSimple:
		newObjectPtr = new CCameraRailSimple(pByteCode);
		break;
	case CT_Rail:
		newObjectPtr = new CCameraRail(pByteCode);
		break;
	case 0x13:
		newObjectPtr = new CCameraFixePerso(pByteCode);
		break;
	case 0x14:
		IMPLEMENTATION_GUARD(
		pvVar7 = operator.new(0x110);
		newObjectPtr = (long)(int)pvVar7;
		if (newObjectPtr != 0) {
			newObjectPtr = View::DeserializeView_0018c420(newObjectPtr, pByteCode);
		})
		break;
	case CT_Cinematic:
		newObjectPtr = new CCameraCinematic();
		break;
	case CT_DCA:
		newObjectPtr = new CCameraDCA();
		break;
	case CT_Frontend:
		newObjectPtr = new CCamera();
		break;
	case CT_Death:
		newObjectPtr = new CCameraFixePerso();
	}
	pCVar9 = newObjectPtr;
	if (newObjectPtr != (CCamera*)0x0) {
		pCVar1 = this->pInitialView_0x4b4;
		if (pCVar1 == (CCamera*)0x0) {
			this->pInitialView_0x4b4 = pCVar9;
			CCamera::_gpcam_man = this;
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

void CCamFigData::IsValid(undefined4 param_2)
{
	this->field_0x2a0 = param_2;
	return;
}

void CCamConfig::ResetWithConfig()
{
	this->field_0x74 = this->field_0xc;
	this->field_0x78 = this->field_0x10;
	this->field_0x7c = this->field_0x14;
	this->targetPitch = this->baseTargetPitch;
	this->field_0x90 = 0.0f;
	this->field_0x8c = 0.0f;
	this->field_0x80 = 0;
	this->field_0x94 = this->field_0x38.x;
	this->field_0x98 = this->field_0x38.y;
	this->field_0xa0 = this->field_0x38.z;
	this->field_0xac = 0.0f;
	this->field_0xa8 = 0.0f;
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
	this->baseTargetPitch = fVar3 * 0.01745329f;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x2c = fVar3 * 0.01745329f;
	fVar3 = pMemoryStream->GetF32();
	this->field_0x30 = fVar3 * 0.01745329f;
	this->field_0x34 = 0.8726646f;
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
		IMPLEMENTATION_GUARD_LOG();
	}
	else {
		bVar4 = false;
	}
	return bVar4;
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
				CCameraShadow* pShadowCam = (CCameraShadow*)pCamera_00;
				if (EVar1 == CT_ShadowSun) {
					this->pActiveCamera = pCamera_00;
					pShadowCam->sceneIndex = -1;
					pShadowCam->sceneFlags = 0;
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
					CCameraShadow* pShadowCam = (CCameraShadow*)pCamera;
					pShadowCam->sceneIndex = -1;
					pShadowCam->sceneFlags = 0;
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

bool CCameraManager::PopCamera(int cameraIndex)
{
	IMPLEMENTATION_GUARD();
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

float CCameraManager::FUN_00197970(edF32VECTOR4* param_2)
{
	edF32VECTOR4 eStack16;

	edF32Matrix4MulF32Vector4Hard(&eStack16, &this->worldToCamera_0x3d0, param_2);
	return (eStack16.y * this->aspectRatio) / (eStack16.z * this->fov_0xa34);
}

void CCameraManager::Func_00194a10(CActorsTable* pTable)
{
	CCamera* pCVar1;
	CCameraShadow* pCVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	CCinematicManager* pCVar6;
	CActorManager* pCVar7;
	ed_3D_Scene* peVar8;
	CActorCinematic* pNewEntry;
	CCinematic* pCVar9;
	uint uVar10;
	int iVar11;
	int iVar12;
	uint uVar13;
	int iVar14;
	float puVar15;
	CActor* pActor;

	pCVar6 = g_CinematicManager_0048efc;
	puVar15 = -1.0f;
	iVar14 = CScene::_pinstance->field_0x11c;
	uVar13 = CScene::_pinstance->count_0x120;

	if (g_CinematicManager_0048efc != (CCinematicManager*)0x0) {
		for (iVar12 = 0; iVar12 < pCVar6->activeCinematicCount; iVar12 = iVar12 + 1) {
			if (iVar12 < pCVar6->activeCinematicCount) {
				pCVar9 = pCVar6->ppCinematicObjB_B[iVar12];
			}
			else {
				pCVar9 = (CCinematic*)0x0;
			}
			if (((pCVar9 != (CCinematic*)0x0) && (pCVar9->state != CS_Stopped)) &&
				(iVar11 = 0, (pCVar9->flags_0x4 & 0x400000) != 0)) {
				for (; iVar11 < pCVar9->loadedActorCinematicCount; iVar11 = iVar11 + 1) {
					if (iVar11 < pCVar9->loadedActorCinematicCount) {
						pNewEntry = pCVar9->aActorCinematic + iVar11;
					}
					else {
						pNewEntry = (CActorCinematic*)0x0;
					}
					if ((((pNewEntry != (CActorCinematic*)0x0) &&
						(pCVar1 = this->pActiveCamera,
							fVar3 = (pCVar1->transformationMatrix).da - pNewEntry->currentLocation.x,
							fVar4 = (pCVar1->transformationMatrix).db - pNewEntry->currentLocation.y,
							fVar5 = (pCVar1->transformationMatrix).dc - pNewEntry->currentLocation.z,
							sqrtf(fVar3 * fVar3 + fVar4 * fVar4 + fVar5 * fVar5) <= 25.0f)) &&
						((uVar10 = pNewEntry->flags, (uVar10 & 0x4000) != 0 && (((uVar10 & 0x100) != 0 && (pNewEntry->pHier != (ed_g3d_hierarchy*)0x0)))))) &&
						(pTable != (CActorsTable*)0x0)) {
						pTable->Add(pNewEntry);
					}
				}

				if (puVar15 < pCVar9->field_0x80) {
					iVar14 = pCVar9->field_0x78;
					uVar13 = pCVar9->field_0x7c;
					puVar15 = pCVar9->field_0x80;
				}
			}
		}
	}

	if (iVar14 != this->field_0xa00) {
		if (iVar14 == 1) {
			for (uVar10 = 0; pCVar7 = CScene::ptable.g_ActorManager_004516a4, uVar10 < uVar13; uVar10 = uVar10 + 1) {
				Func_002b6db0(g_CameraPanStaticMasterArray_00451630[uVar10], 0x80, 0x80);
			}
			for (iVar12 = 0; iVar12 < pCVar7->nbActiveActors; iVar12 = iVar12 + 1) {
				pActor = pCVar7->aActiveActors[iVar12];
				if (((pActor->pHier != (ed_g3d_hierarchy*)0x0) && ((pActor->actorFieldS & 0x80) == 0)) &&
					((CActorFactory::gClassProperties[pActor->typeID].flags & 0x2000) != 0)) {
					pActor->FUN_00115ea0(0xffff);
				}
			}
		}
		else {
			if (iVar14 == 0) {
				Func_002b6db0(g_CameraPanStaticMasterArray_00451630[0], 0x200, 0x100);
				uVar13 = 1;
			}
		}
	}
	uVar10 = 0;
	if (uVar13 != this->field_0xa04) {
		for (; (int)uVar10 < 10; uVar10 = uVar10 + 1) {
			if (uVar10 < uVar13) {
				peVar8 = ed3DGetScene(uVar10);
				peVar8->ed3DSceneRemoveFlag(4);
			}
			else {
				peVar8 = ed3DGetScene(uVar10);
				peVar8->ed3DSceneSetFlag(4);
				pCVar2 = this->aCameraShadow[uVar10];
				if (pCVar2 != (CCameraShadow*)0x0) {
					pCVar2->SetTarget((CActor*)0x0);
					pCVar2->field_0xd0 = 0;
				}
			}
		}
	}

	this->field_0xa00 = iVar14;
	this->field_0xa04 = uVar13;
	return;
}

bool FUN_001946b0(CActor* pActor, void*)
{
	bool bVar1;

	if (((((CActorFactory::gClassProperties[pActor->typeID].flags & 0x1000) == 0) || (pActor->typeID == 6)) ||
		((pActor->flags & 0x4000) == 0)) || ((pActor->flags & 0x100) == 0)) {
		bVar1 = false;
	}
	else {
		bVar1 = true;
	}
	return bVar1;
}

void CCameraManager::Func_00194e00()
{
	CCameraShadow* pCVar1;
	CCamera* pCVar2;
	CCameraShadow* pCVar3;
	//ManagerFunctionData* pMVar4;
	float fVar5;
	float fVar6;
	bool bVar7;
	CActorHero* pCVar8;
	int iVar9;
	CActorHero* pCVar10;
	CActor* pCVar11;
	CActor* pCVar12;
	int iVar13;
	//ManagerFunctionData** ppMVar14;
	int iVar15;
	uint uVar16;
	uint uVar17;
	undefined* puVar18;
	float fVar19;
	undefined* puVar20;
	float fVar21;
	undefined* puVar22;
	edF32VECTOR4 local_230;
	CActorsTable local_220;
	CActorsTable local_110;

	uVar16 = 0;
	local_110.entryCount = 0;
	local_220.entryCount = 0;
	Func_00194a10(&local_110);

	pCVar8 = CActorHero::_gThis;
	if (((this->aCameraShadow[0] != (CCameraShadow*)0x0) && (pCVar1 = this->aCameraShadow[0], CActorHero::_gThis != (CActorHero*)0x0)) &&
		((CActorHero::_gThis->flags & 0x4000) != 0)) {
		pCVar10 = (CActorHero*)pCVar1->GetTarget();
		if (pCVar8 != pCVar10) {
			pCVar1->SetTarget(CActorHero::_gThis);
			pCVar1->field_0xd0 = 1;
		}
		uVar16 = 1;
	}

	if (uVar16 < this->field_0xa04) {
		local_230.w = 25.0f;
		pCVar2 = this->pActiveCamera;
		local_230.xyz = (pCVar2->transformationMatrix).rowT.xyz;

		(CScene::ptable.g_ActorManager_004516a4)->cluster.GetActorsIntersectingSphereWithCriterion(&local_220, &local_230, FUN_001946b0, this);

		local_220.MergeWithTable(&local_110);

		for (; uVar16 < this->field_0xa04; uVar16 = uVar16 + 1) {
			pCVar1 = this->aCameraShadow[uVar16];
			if (pCVar1 != (CCameraShadow*)0x0) {
				iVar13 = -1;
				fVar21 = 3.402823e+38f;
				for (iVar15 = 0; iVar15 < local_220.entryCount; iVar15 = iVar15 + 1) {
					pCVar2 = this->pActiveCamera;
					pCVar12 = local_220.aEntries[iVar15];
					fVar19 = (pCVar2->transformationMatrix).da - (pCVar12->currentLocation).x;
					fVar5 = (pCVar2->transformationMatrix).db - (pCVar12->currentLocation).y;
					fVar6 = (pCVar2->transformationMatrix).dc - (pCVar12->currentLocation).z;
					fVar19 = sqrtf(fVar19 * fVar19 + fVar5 * fVar5 + fVar6 * fVar6);
					iVar9 = iVar15;
					if (fVar21 <= fVar19) {
						fVar19 = fVar21;
						iVar9 = iVar13;
					}
					iVar13 = iVar9;
					fVar21 = fVar19;
				}

				if (iVar13 == -1) {
					pCVar1->SetTarget((CActor*)0x0);
					pCVar1->field_0xd0 = 0;
				}
				else {
					pCVar12 = local_220.Pop(iVar13);
					bVar7 = false;
					for (uVar17 = 0; uVar17 < this->field_0xa04; uVar17 = uVar17 + 1) {
						pCVar3 = this->aCameraShadow[uVar17];
						if ((pCVar3 != (CCameraShadow*)0x0) &&
							(pCVar11 = pCVar3->GetTarget(), pCVar12 == pCVar11)) {
							bVar7 = true;
						}
					}

					if (!bVar7) {
						pCVar1->field_0xd0 = 1;
						pCVar1->SetTarget(pCVar12);
					}
				}
			}
		}
	}
	for (uVar16 = 0; uVar16 < this->field_0xa04; uVar16 = uVar16 + 1) {
		pCVar1 = this->aCameraShadow[uVar16];
		if ((pCVar1 != (CCameraShadow*)0x0) && (pCVar1 != (CCameraShadow*)0x0)) {
			pCVar1->Manage();
			pCVar12 = this->aCameraShadow[uVar16]->GetTarget();
			if (pCVar12 != (CActor*)0x0) {
				CCameraShadow* pCurCameraShadow = this->aCameraShadow[uVar16];				

				gSceneCameras[uVar16].position = pCurCameraShadow->transformationMatrix.rowT;
				gSceneCameras[uVar16].lookAt = pCurCameraShadow->lookAt;

				fVar21 = pCurCameraShadow->GetAngleGamma();
				gSceneCameras[uVar16].rotationZ = -fVar21;

				CameraSet3DPos(gSceneCameras + uVar16);
				edFCameraSetSizeRatioFov(0.0001875f, this->aspectRatio, pCurCameraShadow->fov / 1.333333f, gSceneCameras + uVar16);
			}
		}
	}
	return;
}

CCamera* CCameraManager::GetScenaricCamera(int index)

{
	CCamera* pCVar1;
	int iVar2;
	int iVar3;

	pCVar1 = (CCamera*)0x0;

	if (index != -1) {
		pCVar1 = this->pInitialView_0x4b4;
		iVar2 = this->count_0x9fc + index;
		iVar3 = 0;
		if (0 < iVar2) {
			do {
				if (pCVar1->pNextCameraView_0xa4 != (CCamera*)0x0) {
					pCVar1 = pCVar1->pNextCameraView_0xa4;
				}
				iVar3 = iVar3 + 1;
			} while (iVar3 < iVar2);
		}
	}

	return pCVar1;
}

void CCameraManager::SetEarthQuake(CAM_QUAKE* pCamQuake)
{
	undefined8 uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	CAM_QUAKE* puVar5;

	if (pCamQuake == (CAM_QUAKE*)0x0) {
		(this->field_0x480).field_0x20 = 0;
	}
	else {
		this->field_0x480.field_0x0 = pCamQuake->field_0x0;
		this->field_0x480.field_0x10 = pCamQuake->field_0x10;

		(this->field_0x480).field_0x20 = pCamQuake->field_0x20;

		(this->field_0x480).field_0x24 = pCamQuake->field_0x24;
		(this->field_0x480).field_0x28 = pCamQuake->field_0x28;
		(this->field_0x480).field_0x2c = pCamQuake->field_0x2c;

		(this->field_0x480).field_0x20 = (this->field_0x480).field_0x20 | 1;

		this->field_0x4b0 = 0;
	}
	return;
}

bool CCameraManager::AlertCamera(int param_2, int param_3)
{
	CCamera* pCamera;
	bool bResult;

	pCamera = this->pActiveCamera;
	bResult = false;

	if (pCamera != (CCamera*)0x0) {
		bResult = pCamera->AlertCamera(param_2, param_3, (CCamera*)0x0);
	}

	return bResult;
}

template<>
void S_STREAM_REF<CCamera>::Init()
{
	CCamera* pCVar1;

	pCVar1 = CScene::ptable.g_CameraManager_0045167c->GetScenaricCamera(this->index);

	this->pObj = STORE_SECTION(pCVar1);
	return;
}
