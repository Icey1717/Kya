#include "MiscFunctions.h"
#include "MathOps.h"

float GetRandomTime(float param_1, float param_2)
{
	float randTime;
	float fVar2;

	randTime = param_1;
	if ((param_2 != 0.0f) && (randTime = 0.0f, param_1 != 0.0f)) {
		fVar2 = param_2 * param_1;
		do {
			randTime = edFRndGauss(param_1, fVar2);
		} while (randTime <= 0.0f);
	}

	return randTime;
}