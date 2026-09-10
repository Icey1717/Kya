#include "FxSound.h"
#include "MemoryStream.h"
#include "ActorMovable.h"
#include "MathOps.h"
#include "TimeController.h"

void CFxSoundScenaricData::Init()
{
	this->soundRef.Init();
	this->sampleRef.Init();

	CSound* pCVar2 = (CSound*)this->soundRef.Get();
	if (pCVar2 == (CSound*)0x0) {
		pCVar2 = LOAD_POINTER_CAST(CSound*, this->sampleRef.pStream);
	}
	this->soundRef.pObj = STORE_POINTER(pCVar2);

	return;
}

void CFxSoundScenaricData::Create(ByteCode* pByteCode)
{
	this->soundRef.index = pByteCode->GetS32();
	this->sampleRef.index = pByteCode->GetS32();

	return;
}

void CFxSoundScenaricData::Term()
{
	return;
}

CFxNewSound::CFxNewSound()
{
	this->field_0x84 = (CFxSoundScenaricData*)0x0;
	this->field_0x88 = 0;

	return;
}

void CFxNewSound::Draw()
{
	return;
}

void CFxNewSound::Kill()
{
	CSound* pCVar1;
	bool bVar2;
	uint uVar3;

	bVar2 = this->soundInstance.IsAlive();
	if ((bVar2 != false) && (NoAudio == 0)) {
		pCVar1 = (this->soundInstance).pSound;
		if (pCVar1 != (CSound*)0x0) {
			uVar3 = pCVar1->Stop((this->soundInstance).soundId);
			(this->soundInstance).soundId = uVar3;
		}
	}

	if (this->field_0x88 != 0) {
		CScene::ptable.g_AudioManager_00451698->ReleaseSound3DData((this->soundInstance).pSound3dData);
	}

	CNewFx::Kill();

	return;
}

void CFxNewSound::Start(float param_1, float param_2)
{
	bool bVar1;
	CActor* pCVar3;
	FX_MATERIAL_SELECTOR FVar4;
	bool bVar5;
	uint uVar6;
	uint uVar7;
	CSound* pCVar8;
	edsound_3d_data* pCVar9;
	float fVar10;
	float pCVar11;
	float fVar12;
	CActorMovable* pCVar2;

	this->flags = this->flags | 3;
	this->flags = this->flags & 0xfffffff3;
	if (0.0f < param_1) {
		this->flags = this->flags | 0x40;
	}

	this->field_0xc = param_1;
	this->field_0x10 = param_2;
	uVar6 = this->flags;
	bVar1 = (uVar6 & 0x40) != 0;
	if (bVar1) {
		this->flags = uVar6 & 0xffffffbf;
	}

	if (this->field_0x84 != (CFxSoundScenaricData*)0x0) {
		uVar6 = this->flags;
		pCVar9 = (edsound_3d_data*)0x0;
		if (((uVar6 & 0x100) != 0) || ((uVar6 & 0x80) != 0)) {
			this->sound3dData.position = this->position.xyz;
			pCVar11 = this->field_0x84->soundRef.Get()->field_0x78;
			this->sound3dData.field_0x1c = pCVar11 * pCVar11;
			this->sound3dData.field_0x18 = pCVar11;

			if (((this->flags & 0x100) == 0) || (pCVar3 = (CActor*)this->pActor, bVar5 = pCVar3->IsKindOfObject(2), bVar5 == false)) {
				this->sound3dData.rotation = gF32Vector3Zero;
			}
			else {
				pCVar2 = (CActorMovable*)this->pActor;
				this->sound3dData.rotation = (pCVar2->dynamic).velocityDirectionEuler.xyz;
			}

			pCVar9 = &this->sound3dData;
		}

		pCVar8 = this->field_0x84->soundRef.Get();
		FVar4 = this->field_0x80;
		if ((NoAudio == 0) && (((this->soundInstance).pSound3dData = pCVar9, pCVar8 != (CSoundBase*)0x0 || ((this->soundInstance).pSound != (CSoundBase*)0x0)))) {
			if (pCVar8 == (CSoundBase*)0x0) {
				pCVar8 = (this->soundInstance).pSound;
			}
			else {
				(this->soundInstance).pSound = pCVar8;
			}

			if (FVar4 != ~FX_MATERIAL_SELECTOR_WATER) {
				(this->soundInstance).field_0x20 = FVar4;
			}

			uVar6 = pCVar8->Play(this->soundInstance.soundId, this->soundInstance.field_0x20, pCVar9, &this->soundInstance, (uint*)0x0, &this->soundInstance.soundId);
			(this->soundInstance).soundId = uVar6;
		}

		if (bVar1) {
			fVar10 = this->field_0xc;
			if ((this->soundInstance).pSound != (CSound*)0x0) {
				if (this->field_0x80 != ~FX_MATERIAL_SELECTOR_WATER) {
					(this->soundInstance).field_0x20 = this->field_0x80;
				}
				uVar7 = (this->soundInstance).pSound->PlayAlt(0.0f, -2.0f, fVar10, (this->soundInstance).soundId, this->soundInstance.field_0x20);
				(this->soundInstance).soundId = uVar7;
			}
		}
	}

	this->flags = this->flags & 0xfffffffd;

	return;
}

void CFxNewSound::Pause()
{
	bool bVar1;
	CSound* pSound;

	this->flags = this->flags | 4;

	if (((NoAudio == 0) && (bVar1 = this->soundInstance.IsAlive(), bVar1 != false)) && (pSound = (this->soundInstance).pSound, pSound != (CSound*)0x0)) {
		pSound->SetPause((this->soundInstance).soundId, 1);
	}

	return;
}

void CFxNewSound::Resume()
{
	bool bVar1;
	CSound* pSound;

	this->flags = this->flags & 0xfffffffb;
	if (((NoAudio == 0) && (bVar1 = this->soundInstance.IsAlive(), bVar1 != false)) && (pSound = (this->soundInstance).pSound, pSound != (CSound*)0x0)) {
		pSound->SetPause((this->soundInstance).soundId, 0);
	}

	return;
}

void CFxNewSound::Stop(float param_1)
{
	CSound* pCVar1;

	this->flags = this->flags | 0x20000;

	if (param_1 != -1.0f) {
		this->field_0x10 = param_1;
	}

	if (this->field_0x10 == 0.0f) {
		Kill();
	}
	else {
		IMPLEMENTATION_GUARD_AUDIO(
		pCVar1 = (this->soundInstance).SoundStructPtr;
		if (pCVar1 != (CSound*)0x0) {
			CSound::FadeTo(0.0, -2.0, param_1, pCVar1, (this->soundInstance).SoundID);
		})
	}

	return;
}

int CFxNewSound::GetType()
{
	return FX_TYPE_SOUND;
}

void CFxNewSound::SetTimeScaler(float param_1)
{
	CSound* pSound;

	pSound = (this->soundInstance).pSound;
	if (pSound != (CSound*)0x0) {
		pSound->SetFrequency(param_1, (this->soundInstance).soundId);
	}

	return;
}

void CFxNewSound::Manage()
{
	uint uVar1;
	CSound* pCVar2;
	bool bVar3;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;

	CNewFx::Manage();

	uVar1 = this->flags;
	if ((uVar1 & 0x20) == 0) {
		bVar3 = (uVar1 & 1) != 0;
		if (bVar3) {
			bVar3 = (uVar1 & 4) == 0;
		}

		if (bVar3) {
			bVar3 = this->soundInstance.IsAlive();
			if (bVar3 == false) {
				Kill();
			}
			else {
				if ((this->flags & 0x180) != 0) {
					fVar5 = this->position.x;
					fVar10 = (this->sound3dData).position.x;
					fVar6 = this->position.y;
					fVar11 = (this->sound3dData).position.y;
					fVar7 = this->position.z;
					fVar12 = (this->sound3dData).position.z;

					fVar8 = GetTimer()->cutsceneDeltaTime;
					if (fVar8 != 0.0f) {
						fVar8 = 1.0f / fVar8;
					}

					(this->sound3dData).position = this->position.xyz;
					(this->sound3dData).rotation.x = (fVar5 - fVar10) * fVar8;
					(this->sound3dData).rotation.y = (fVar6 - fVar11) * fVar8;
					(this->sound3dData).rotation.z = (fVar7 - fVar12) * fVar8;
				}
			}

			if ((this->flags & 0x40) != 0) {
				pCVar2 = (this->soundInstance).pSound;
				if (pCVar2 != (CSound*)0x0) {
					pCVar2->SetVolume(this->field_0x70, (this->soundInstance).soundId);
				}
			}
		}
	}
	else {
		Kill();
	}

	return;
}

void CFxNewSound::Instanciate(CFxSoundScenaricData* pData, FX_MATERIAL_SELECTOR selector)
{
	this->field_0x84 = pData;
	this->field_0x80 = selector;

	return;
}

void* CFxSoundManager::InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector)
{
	CFxNewSound* pNewParticle;

	pNewParticle = _InstanciateFx();

	if ((pNewParticle != 0) && (scenaricDataIndex != 0xffffffff)) {
		pNewParticle->Instanciate(this->aScenaricData + scenaricDataIndex, selector);
	}

	return pNewParticle;
}
