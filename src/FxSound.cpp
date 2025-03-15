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
