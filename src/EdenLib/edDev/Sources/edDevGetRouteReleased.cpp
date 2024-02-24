#include "edDevGetRouteReleased.h"
#include "edDev.h"

bool edDevGetRouteReleased(uint routeId)
{
	bool bReleased;

	if (edDevRoot.nbRoutes < routeId) {
		bReleased = false;
	}
	else {
		bReleased = edDevRoot.aPorts[edDevRoot.aRoutes[routeId].portId].pPadD[edDevRoot.aRoutes[routeId].controlId].bReleased;
	}
	return bReleased;
}
