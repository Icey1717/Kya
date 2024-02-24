#include "edDevSetRates.h"
#include "edDev.h"

uint edDevSetRouteRates(uint routeId, EDDEV_RATES* pRates)
{
	Pad_d* pPVar1;
	uint ret;
	uint controlId;

	if (routeId < edDevRoot.nbRoutes) {
		ret = 0;
		controlId = edDevRoot.aRoutes[routeId].controlId;
		pPVar1 = edDevRoot.aPorts[edDevRoot.aRoutes[routeId].portId].pPadD;
		pPVar1[controlId].rateA = pRates->field_0x0;
		pPVar1[controlId].rateB = pRates->field_0x4;
	}
	else {
		ret = 0xfffffffa;
	}
	return ret;
}
