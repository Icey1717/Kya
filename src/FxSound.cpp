#include "FxSound.h"
#include "MemoryStream.h"

void CFxSoundScenaricData::Init()
{
	IMPLEMENTATION_GUARD_AUDIO(
	CSoundSample* pCVar1;
	CSoundStream* pCVar2;

	pCVar1 = this->field_0x0;
	if (pCVar1 == (CSoundSample*)0xffffffff) {
		pCVar1 = (CSoundSample*)0x0;
	}
	else {
		if (pCVar1 < (CSoundSample*)(CScene::ptable.g_AudioManager_00451698)->field_0x4) {
			pCVar1 = (CScene::ptable.g_AudioManager_00451698)->aSoundSamplesA + (int)pCVar1;
		}
		else {
			pCVar1 = (CScene::ptable.g_AudioManager_00451698)->aSoundSamplesB +
				(int)((int)pCVar1 - (int)(CSoundSample*)(CScene::ptable.g_AudioManager_00451698)->field_0x4);
		}
	}
	this->field_0x0 = pCVar1;
	if (this->field_0x4 == (CSoundStream*)0xffffffff) {
		pCVar2 = (CSoundStream*)0x0;
	}
	else {
		pCVar2 = (CScene::ptable.g_AudioManager_00451698)->aSoundStreams + (int)this->field_0x4;
	}
	this->field_0x4 = pCVar2;
	pCVar2 = (CSoundStream*)this->field_0x0;
	if (pCVar2 == (CSoundStream*)0x0) {
		pCVar2 = this->field_0x4;
	}
	this->field_0x0 = (CSoundSample*)pCVar2;)
	return;
}

void CFxSoundScenaricData::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetS32();
	this->field_0x4 = pByteCode->GetS32();

	return;
}

void CFxNewSound::Draw()
{
	return;
}

void CFxNewSound::Kill()
{
	IMPLEMENTATION_GUARD_AUDIO(
	CSound* pCVar1;
	bool bVar2;
	uint uVar3;

	bVar2 = edSoundInstanceIsAlive(&this->soundInstance);
	if ((bVar2 != false) && (NoAudio == 0)) {
		pCVar1 = (this->soundInstance).SoundStructPtr;
		if (pCVar1 != (CSound*)0x0) {
			uVar3 = (*(code*)pCVar1->vt->Stop)(pCVar1, (this->soundInstance).SoundID);
			(this->soundInstance).SoundID = uVar3;
		}
	}
	if (this->field_0x88 != 0) {
		CAudioManager::ReleaseSound3DData
		((int)CScene::ptable.g_AudioManager_00451698, (long)(this->soundInstance).field_0x14);
	})

	CNewFx::Kill();

	return;
}

void CFxNewSound::Start(float param_1, float param_2)
{
	IMPLEMENTATION_GUARD_AUDIO();
}

int CFxNewSound::GetType()
{
	return FX_TYPE_SOUND;
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
