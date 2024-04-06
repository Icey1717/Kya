#include "edDevGetRouteAnalog.h"
#include "edDev.h"

float edDevGetChanAnalog(uint index, uint padDIndex)
{
	float fVar1;

	if (index < edDevRoot.nbPorts) {
		if (padDIndex < edDevRoot.aPorts[index].maxControlId) {
			fVar1 = edDevRoot.aPorts[index].pPadD[padDIndex].analogValue;
		}
		else {
			fVar1 = 0.0f;
		}
	}
	else {
		fVar1 = 0.0f;
	}

	return fVar1;
}
