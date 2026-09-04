#include "AudioWind.h"
#include "ActorWind.h"

void CSoundWind::Init()
{
	this->field_0x0.Init();
	this->field_0x4.Init();
	this->field_0x8.Init();

	return;
}

void CSoundWind::Init(CFxWind* pFx)
{
	pFx->field_0x3b4 = 0.0f;

	return;
}

void CSoundWind::Add(ByteCode* pByteCode)
{
	this->field_0x0.index = pByteCode->GetS32();
	this->field_0x4.index = pByteCode->GetS32();
	this->field_0x8.index = pByteCode->GetS32();

	return;
}