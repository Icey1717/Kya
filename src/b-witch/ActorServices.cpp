#include "ActorServices.h"
#include "MemoryStream.h"

void CLifeBase::Create(ByteCode* pByteCode)
{
	this->maxValue = pByteCode->GetF32();

	return;
}

void CLifeBase::Reset()
{
	this->value = this->maxValue;

	return;
}

void CLifeBase::LifeDecrease(float amount)
{
	float fVar1;

	fVar1 = GetValue();
	if (fVar1 <= amount) {
		SetValue(0.0f);
	}
	else {
		fVar1 = GetValue();
		SetValue(fVar1 - amount);
	}

	return;
}