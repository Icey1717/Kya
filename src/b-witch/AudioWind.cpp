#include "AudioWind.h"
#include "ActorWind.h"
#include "Camera.h"
#include "CameraViewManager.h"
#include "MathOps.h"

void CSoundWind::Init()
{
	this->field_0x0[0].Init();
	this->field_0x0[1].Init();
	this->field_0x0[2].Init();

	return;
}

void CSoundWind::Init(CFxWind* pFx)
{
	pFx->field_0x3b4 = 0.0f;

	return;
}

void CSoundWind::Add(ByteCode* pByteCode)
{
	this->field_0x0[0].index = pByteCode->GetS32();
	this->field_0x0[1].index = pByteCode->GetS32();
	this->field_0x0[2].index = pByteCode->GetS32();

	return;
}

void CSoundWind::Reset(CFxWind* pFx)
{
	pFx->field_0x3b4 = 0.0f;
	return;
}

void CSoundWind::Play(CActor* pActor, CFxWind* pFx)
{
	float fVar1;
	float fVar2;
	bool bVar3;
	StateConfig* pSVar4;
	int index;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	edsound_3d_data* p3dData;
	float local_4;
	CCamera* pCamera;
	CActorSound* pSound;

	if (pActor == (CActor*)0x0) {
		uVar7 = 0;
	}
	else {
		uVar7 = 0;
		if (pActor->actorState != -1) {
			pSVar4 = pActor->GetStateCfg(pActor->actorState);
			uVar7 = pSVar4->flags_0x4;
		}
	}

	p3dData = &pFx->field_0x378;
	pCamera = (CScene::ptable.g_CameraManager_0045167c)->pActiveCamera;
	(pFx->field_0x378).rotation = gF32Vector3Zero;
	pFx->field_0x3a0.ComputeResultingPosition(p3dData, &local_4, &(pCamera->transformationMatrix).rowT);
	(pFx->field_0x378).field_0x1c = local_4;
	(pFx->field_0x378).field_0x18 = -1.0f;
	pSound = &pFx->field_0x374->node;
	uVar6 = 0;
	if (uVar7 != pFx->field_0x3b4) {
		uVar5 = 0x100;
		do {
			if (((uVar7 & uVar5) != 0) && ((uVar5 & pFx->field_0x3b4) == 0)) {
				index = 1 - uVar6;
				bVar3 = pSound->IsInstanceAlive(index);
				if (bVar3 == false) {
					pSound->SoundStart(pActor, index, this->field_0x0[index].Get(), 1, 2, reinterpret_cast<SOUND_SPATIALIZATION_PARAM*>(&p3dData));
					pSound->SetVolume(0.0f, index);
				}

				pSound->FadeTo(1.0f, -1.0f, 0.5f, index);
			}

			if (((uVar7 & uVar5) == 0) && ((uVar5 & pFx->field_0x3b4) != 0)) {
				pSound->FadeTo(0.0f, -2.0f, 1.0f, 1 - uVar6);
			}

			uVar6 = uVar6 + 1;
			uVar5 = uVar5 << 1;
		} while (uVar6 < 2);

		if (uVar7 == 0) {
			bVar3 = pSound->IsInstanceAlive(uVar6);
			if (bVar3 == false) {
				pSound->SoundStart(pActor, 2, this->field_0x0[2].Get(), 1, 2, reinterpret_cast<SOUND_SPATIALIZATION_PARAM*>(&p3dData));
				pSound->SetVolume(0.0f, 2);
			}

			pSound->FadeTo(1.0f, -1.0f, 0.5f, 2);
		}
		else {
			if ((pFx->field_0x3b4 == 0) && (bVar3 = pSound->IsInstanceAlive(2), bVar3 != false)) {
				pSound->FadeTo(0.0f, -2.0f, 1.0f, 2);
			}
		}

		pFx->field_0x3b4 = uVar7;
	}

	return;
}
