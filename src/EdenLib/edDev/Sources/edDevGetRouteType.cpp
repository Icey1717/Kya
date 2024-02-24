#include "edDevGetRouteType.h"
#include "edDev.h"

uint edDevGetRouteType(uint routeId)
{
	uint uVar1;

	if (routeId < edDevRoot.nbRoutes) {
		uVar1 = edDevRoot.aPorts[edDevRoot.aRoutes[routeId].portId].pPadD[edDevRoot.aRoutes[routeId].controlId].type;
	}
	else {
		uVar1 = 0xfffffffa;
	}
	return uVar1;
}