#include "CameraStack.h"
#include "TimeController.h"
#include "CameraViewManager.h"
#include "../ActorHero.h"
#include "CameraGame.h"
#include "CameraViewManager.h"
#include "../CameraFightData.h"

CCamera* CCameraStack::FindActivableCameraInStack(uint param_2)
{
	ECameraType EVar1;
	CameraStackEntry* pCVar2;
	int iVar3;
	CameraStackEntry* puVar4;
	int iVar4;
	int iVar5;
	int iVar6;

	iVar6 = 1;
	iVar5 = 0;
	if (1 < this->stackSize + 1) {
		puVar4 = this->aCameras;
		iVar4 = iVar5;

		do {
			puVar4 = puVar4 + 1;
			iVar5 = iVar6;

			if ((puVar4->field_0x0 & 1) == 0) {
				EVar1 = this->pActiveCamera->GetMode();
				iVar5 = iVar4;

				if (EVar1 == CT_Cinematic) {
					iVar3 = this->stackSize;

					for (pCVar2 = this->aCameras + iVar3;
						(-1 < iVar3 && (pCVar2->pCamera != this->pActiveCamera)); pCVar2 = pCVar2 + -1) {
						iVar3 = iVar3 + -1;
					}

					if (-1 < iVar3) goto LAB_00198768;
				}

				iVar3 = puVar4->pCamera->field_0x8;
				if ((iVar3 < 0) || (iVar3 != (this->aCameras[iVar4].pCamera)->field_0x8)) {
					if ((param_2 & puVar4->pCamera->flags_0xc) == 0) {
						if (iVar3 == -0x9f) {
							iVar5 = iVar6;
						}
					}
					else {
						if (((this->aCameras[iVar4].pCamera)->field_0x8 == -100) || (iVar3 != -100)) {
							iVar5 = iVar6;
						}
					}
				}
			}

		LAB_00198768:
			iVar6 = iVar6 + 1;
			iVar4 = iVar5;
		} while (iVar6 < this->stackSize + 1);
	}

	return this->aCameras[iVar5].pCamera;
}

bool CCameraStack::FindCameraState()
{
	bool bUpdated;
	CCamera* pActivatableCamera;
	CameraStackEntry* pCurEntry;
	int iVar5;

	pActivatableCamera = FindActivableCameraInStack(GetCurHeroState());

	if (this->pActiveCamera == pActivatableCamera) {
		bUpdated = false;
	}
	else {
		this->pPrevActiveCamera = this->pActiveCamera;
		this->pActiveCamera = pActivatableCamera;
		CCamera* pPrevCamera = this->pPrevActiveCamera;

		if ((pPrevCamera->field_0x8c != 0.0f) && (0.0f < this->field_0x200)) {
			iVar5 = this->stackSize;
			pCurEntry = this->aCameras + iVar5;

			for (; ((pCurEntry->pCamera != pPrevCamera) && (0 < iVar5)); iVar5 = iVar5 + -1) {
				pCurEntry = pCurEntry + -1;
			}

			if (0 < iVar5) {
				pPrevCamera->flags_0xc = pPrevCamera->flags_0xc & 0xffdfffff;

				if (iVar5 < this->stackSize) {
					pCurEntry = this->aCameras + iVar5;
					do {
						iVar5 = iVar5 + 1;
						pCurEntry->field_0x0 = pCurEntry[1].field_0x0;
						pCurEntry->pCamera = pCurEntry[1].pCamera;
						pCurEntry = pCurEntry + 1;
					} while (iVar5 < this->stackSize);
				}

				this->stackSize = this->stackSize + -1;
			}
		}

		bUpdated = true;
	}

	return bUpdated;
}

int CCameraStack::GetCurHeroState(void)
{
	uint uVar1;
	Timer* pTVar2;
	CCamFigData* pCVar3;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		uVar1 = CActorHero::_gThis->TestState_IsInTheWind(0xffffffff);
		if (uVar1 != 0) {
			return 0x10;
		}

		uVar1 = CActorHero::_gThis->TestState_IsFlying(0xffffffff);
		if (uVar1 != 0) {
			pTVar2 = GetTimer();

			if ((0.5f < pTVar2->scaledTotalTime - pHero->field_0x1548) && (pHero->field_0xa88 < -1.0f)) {
				return 0x20;
			}

			return 0x10;
		}

		uVar1 = CActorHero::_gThis->TestState_IsOnAToboggan(0xffffffff);
		if (uVar1 != 0) {
			return 8;
		}

		pCVar3 = (CCamFigData*)0x0;
		if (CCameraGame::_b_use_fig_data != 0) {
			pCVar3 = CCameraGame::_pfig_data;
		}
		if (pCVar3 != (CCamFigData*)0x0) {
			pCVar3 = (CCamFigData*)0x0;

			if (CCameraGame::_b_use_fig_data != 0) {
				pCVar3 = CCameraGame::_pfig_data;
			}

			if (pCVar3->field_0x2a0 != 0) {
				return 0x40;
			}
		}

		uVar1 = pHero->TestState_IsGrippedOrClimbing(0xffffffff);
		if (uVar1 != 0) {
			return 0x100;
		}

		uVar1 = pHero->TestState_IsOnCeiling(0xffffffff);
		if (uVar1 != 0) {
			return 0x200;
		}
	}

	return 4;
}

bool CCameraStack::Contains(CCamera* pCamera)
{
	CameraStackEntry* pCVar1;
	int iVar2;

	iVar2 = this->stackSize;
	for (pCVar1 = this->aCameras + iVar2; (-1 < iVar2 && (pCVar1->pCamera != pCamera)); pCVar1 = pCVar1 + -1) {
		iVar2 = iVar2 + -1;
	}

	return -1 < iVar2;
}

bool CCameraStack::Pop(CCamera* pCamera)
{
	int iVar1;
	bool bUpdated;
	CameraStackEntry* pCVar2;
	int iVar3;

	if (this->stackSize == 0) {
		bUpdated = false;
	}
	else {
		iVar1 = this->stackSize;
		pCVar2 = this->aCameras + iVar1;

		for (iVar3 = iVar1; (pCVar2->pCamera != pCamera && (0 < iVar3)); iVar3 = iVar3 + -1) {
			pCVar2 = pCVar2 + -1;
		}

		bUpdated = false;

		if (0 < iVar3) {
			if (iVar3 < iVar1) {
				pCVar2 = this->aCameras + iVar3;
				do {
					iVar3 = iVar3 + 1;
					*pCVar2 = *(pCVar2 + 1);
					pCVar2 = pCVar2 + 1;
				} while (iVar3 < this->stackSize);
			}

			this->stackSize = this->stackSize + -1;
			bUpdated = FindCameraState();

			if (this->pPrevActiveCamera != 0) {
				this->switchMode = this->pPrevActiveCamera->field_0x94;
				this->field_0x218 = this->pPrevActiveCamera->field_0x9c;
			}

			if (this->switchMode == SWITCH_MODE_A) {
				this->switchMode = this->pActiveCamera->switchMode;
				this->field_0x218 = this->pActiveCamera->field_0x98;

				if (this->switchMode == SWITCH_MODE_A) {
					this->switchMode = SWITCH_MODE_B;
					this->field_0x218 = 0.8f;
				}
			}
		}
	}

	return bUpdated;
}

bool CCameraStack::Push(CCamera* pCamera, int param_3)
{
	int iVar1;
	bool bUpdated;
	CameraStackEntry* pCVar2;
	int iVar3;

	if (pCamera == this->aCameras[this->stackSize].pCamera) {
		bUpdated = false;
	}
	else {
		pCVar2 = this->aCameras;
		for (iVar3 = 0; (pCVar2->pCamera != pCamera && (iVar3 < this->stackSize)); iVar3 = iVar3 + 1) {
			pCVar2 = pCVar2 + 1;
		}

		iVar1 = this->stackSize;
		bUpdated = false;

		if ((iVar3 == iVar1) && (bUpdated = false, iVar1 < 0x3f)) {
			this->stackSize = iVar1 + 1;
			this->aCameras[iVar1 + 1].field_0x0 = (uint)(param_3 != 0);
			this->aCameras[iVar1 + 1].pCamera = pCamera;
			bUpdated = FindCameraState();

			this->switchMode = this->pActiveCamera->switchMode;
			this->field_0x218 = this->pActiveCamera->field_0x98;

			if (this->switchMode == SWITCH_MODE_A) {
				if (this->pPrevActiveCamera != 0) {
					this->switchMode = this->pPrevActiveCamera->field_0x94;
					this->field_0x218 = this->pPrevActiveCamera->field_0x9c;
				}

				if (this->switchMode == SWITCH_MODE_A) {
					this->switchMode = SWITCH_MODE_B;
					this->field_0x218 = 0.8f;
				}
			}
		}
	}

	return bUpdated;
}

void CCameraStack::SetMainCamera(CCamera* pCamera)
{
	this->aCameras[0].field_0x0 = 0;
	this->aCameras[0].pCamera = pCamera;
	this->pActiveCamera = pCamera;
	return;
}

bool CCameraStack::Manage()
{
	int iVar1;
	bool bUpdatedState;
	int currentHeroState;
	CameraStackEntry* pCVar4;
	int iVar5;
	float fVar6;

	currentHeroState = GetCurHeroState();
	bUpdatedState = false;

	if (0.0f < this->field_0x200) {
		fVar6 = this->field_0x200 - GetTimer()->cutsceneDeltaTime;
		this->field_0x200 = fVar6;

		if (fVar6 <= 0.0f) {
			this->pActiveCamera->flags_0xc = this->pActiveCamera->flags_0xc & 0xffdfffff;
			if (this->stackSize == 0) {
				bUpdatedState = false;
			}
			else {
				iVar1 = this->stackSize;
				pCVar4 = this->aCameras + iVar1;
				for (iVar5 = iVar1; (pCVar4->pCamera != this->pActiveCamera && (0 < iVar5));
					iVar5 = iVar5 + -1) {
					pCVar4 = pCVar4 + -1;
				}

				bUpdatedState = false;
				if (0 < iVar5) {
					if (iVar5 < iVar1) {
						pCVar4 = this->aCameras + iVar5;
						do {
							iVar5 = iVar5 + 1;
							pCVar4->field_0x0 = pCVar4[1].field_0x0;
							pCVar4->pCamera = pCVar4[1].pCamera;
							pCVar4 = pCVar4 + 1;
						} while (iVar5 < this->stackSize);
					}

					this->stackSize = this->stackSize + -1;

					bUpdatedState = FindCameraState();
					if (this->pPrevActiveCamera != (CCamera*)0x0) {
						this->switchMode = this->pPrevActiveCamera->field_0x94;
						this->field_0x218 = this->pPrevActiveCamera->field_0x9c;
					}

					if (this->switchMode == SWITCH_MODE_A) {
						this->switchMode = this->pActiveCamera->switchMode;
						this->field_0x218 = this->pActiveCamera->field_0x98;

						if (this->switchMode == SWITCH_MODE_A) {
							this->switchMode = SWITCH_MODE_B;
							this->field_0x218 = 0.8f;
						}
					}
				}
			}
		}
	}

	if (this->field_0x210 != currentHeroState) {
		bUpdatedState = FindCameraState();

		this->switchMode = this->pActiveCamera->switchMode;
		this->field_0x218 = this->pActiveCamera->field_0x98;

		if (this->switchMode == SWITCH_MODE_A) {
			if (this->pPrevActiveCamera != 0) {
				this->switchMode = this->pPrevActiveCamera->field_0x94;
				this->field_0x218 = this->pPrevActiveCamera->field_0x9c;
			}

			if (this->switchMode == SWITCH_MODE_A) {
				this->switchMode = SWITCH_MODE_B;
				this->field_0x218 = 0.8f;
			}
		}

		this->field_0x210 = currentHeroState;
	}

	return bUpdatedState;
}

void CCameraStack::Reset()
{
	CCameraStack* pCVar1;
	int iVar2;

	this->pPrevActiveCamera = 0;
	iVar2 = 0;
	this->field_0x210 = 0;
	this->stackSize = 0;
	this->switchMode = SWITCH_MODE_A;
	this->field_0x218 = 0.0;
	pCVar1 = this;
	do {
		pCVar1->aCameras[0].field_0x0 = 0;
		pCVar1->aCameras[0].pCamera = (CCamera*)0x0;
		iVar2 = iVar2 + 8;
		pCVar1->aCameras[1].field_0x0 = 0;
		pCVar1->aCameras[1].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[2].field_0x0 = 0;
		pCVar1->aCameras[2].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[3].field_0x0 = 0;
		pCVar1->aCameras[3].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[4].field_0x0 = 0;
		pCVar1->aCameras[4].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[5].field_0x0 = 0;
		pCVar1->aCameras[5].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[6].field_0x0 = 0;
		pCVar1->aCameras[6].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[7].field_0x0 = 0;
		pCVar1->aCameras[7].pCamera = (CCamera*)0x0;
		pCVar1 = (CCameraStack*)(pCVar1->aCameras + 8);
	} while (iVar2 < 0x40);
	this->pActiveCamera = (CCamera*)0x0;
	return;
}

void CCameraStack::Level_SectorChange(int oldSectorId)
{
	CameraStackEntry* pCVar1;
	CameraStackEntry* pCVar2;
	int iVar3;
	int iVar4;

	iVar4 = this->stackSize;
	if (0 < iVar4) {
		pCVar2 = this->aCameras + iVar4;
		do {
			if ((oldSectorId == pCVar2->pCamera->sectorId) && (oldSectorId != -1)) {
				if (iVar4 < this->stackSize) {
					pCVar1 = this->aCameras + iVar4;
					iVar3 = iVar4;

					do {
						iVar3 = iVar3 + 1;
						pCVar1->field_0x0 = (&pCVar1->field_0x0)[2];
						(&pCVar1->field_0x0)[1] = (&pCVar1->field_0x0)[3];
						pCVar1 = (CameraStackEntry*)(&pCVar1->field_0x0 + 2);
					} while (iVar3 < this->stackSize);
				}

				this->stackSize = this->stackSize + -1;
			}

			iVar4 = iVar4 + -1;
			pCVar2 = pCVar2 + -1;
		} while (0 < iVar4);
	}

	return;
}

CCameraStack::CCameraStack()
{
	CCameraStack* pCVar1;
	int iVar2;

	this->pPrevActiveCamera = 0;
	iVar2 = 0;
	this->field_0x210 = 0;
	this->stackSize = 0;
	this->switchMode = SWITCH_MODE_A;
	this->field_0x218 = 0.0;
	pCVar1 = this;
	do {
		pCVar1->aCameras[0].field_0x0 = 0;
		pCVar1->aCameras[0].pCamera = (CCamera*)0x0;
		iVar2 = iVar2 + 8;
		pCVar1->aCameras[1].field_0x0 = 0;
		pCVar1->aCameras[1].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[2].field_0x0 = 0;
		pCVar1->aCameras[2].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[3].field_0x0 = 0;
		pCVar1->aCameras[3].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[4].field_0x0 = 0;
		pCVar1->aCameras[4].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[5].field_0x0 = 0;
		pCVar1->aCameras[5].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[6].field_0x0 = 0;
		pCVar1->aCameras[6].pCamera = (CCamera*)0x0;
		pCVar1->aCameras[7].field_0x0 = 0;
		pCVar1->aCameras[7].pCamera = (CCamera*)0x0;
		pCVar1 = (CCameraStack*)(pCVar1->aCameras + 8);
	} while (iVar2 < 0x40);
	this->pActiveCamera = (CCamera*)0x0;
	return;
}
