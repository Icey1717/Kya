#include "FxComposite.h"
#include "MemoryStream.h"

void CFxCompositeScenaricData::Init()
{
	return;
}

CFxCompositeScenaricData::CFxCompositeScenaricData()
{
	this->nbData = 0;
	this->field_0x4 = (uint*)0x0;

	return;
}

void CFxCompositeScenaricData::Create(ByteCode* pByteCode)
{
	this->nbData = pByteCode->GetU32();

	if (this->nbData != 0) {
		void* pAlloc = edMemAlloc(TO_HEAP(H_MAIN), this->nbData * (sizeof(uint) + sizeof(float)));

		this->field_0x4 = reinterpret_cast<uint*>(pAlloc);
		this->field_0x8 = reinterpret_cast<float*>(this->field_0x4 + this->nbData);

		for (uint i = 0; i < this->nbData; i++) {
			this->field_0x4[i] = pByteCode->GetU32();
			this->field_0x8[i] = pByteCode->GetF32();
		}
	}

	return;
}
