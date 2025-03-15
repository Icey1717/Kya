#include "FxRandom.h"
#include "MemoryStream.h"

CFxRandomScenaricData::CFxRandomScenaricData()
{
	this->nbData = 0;

	return;
}

void CFxRandomScenaricData::Init()
{
	return;
}

void CFxRandomScenaricData::Create(ByteCode* pByteCode)
{
	this->nbData = pByteCode->GetU32();
	this->field_0x44 = 0.0f;

	if (this->nbData != 0) {
		for (uint i = 0; i < this->nbData; i++) {
			this->aSubObjs[i].field_0x0 = pByteCode->GetU32();
			this->aSubObjs[i].field_0x4 = pByteCode->GetF32();
		}
	}

	return;
}
