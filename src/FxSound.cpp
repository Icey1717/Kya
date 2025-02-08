#include "FxSound.h"
#include "MemoryStream.h"

void CFxSoundScenaricData::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetS32();
	this->field_0x4 = pByteCode->GetS32();

	return;
}
