#ifndef _ACTOR_SERVICES_H
#define _ACTOR_SERVICES_H

#include "Types.h"

struct ByteCode;

class CLifeBase
{
public:
	void Create(ByteCode* pByteCode);
	void Reset();
	void LifeDecrease(float amount);

	void SetValue(float newValue);
	float GetValue();
	float GetValueMax();

	float value;
	float maxValue;
};

#endif // _ACTOR_SERVICES_H