#include "edDevGetChanClick.h"
#include "edDev.h"

float edDevGetChanClick(uint portIndex, uint padDIndex)
{
	float fVar1;

	if (portIndex < edDevRoot.nbPorts) {
		if (edDevRoot.aPorts[portIndex].maxControlId < padDIndex) {
			fVar1 = 0.0f;
		}
		else {
			fVar1 = edDevRoot.aPorts[portIndex].pPadD[padDIndex].clickValue;
		}
	}
	else {
		fVar1 = 0.0f;
	}

	return fVar1;
}

