#include "edDevGetRoutePressed.h"
#include "edDev.h"

bool edDevGetRoutePressed(uint routeId)
{
	bool bPressed;

	if (routeId < edDevRoot.nbRoutes) {
		bPressed = edDevRoot.aPorts[edDevRoot.aRoutes[routeId].portId].pPadD[edDevRoot.aRoutes[routeId].controlId].bPressed;
	}
	else {
		bPressed = false;
	}
	return bPressed;
}
