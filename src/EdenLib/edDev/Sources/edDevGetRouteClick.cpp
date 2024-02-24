#include "edDevGetRouteClick.h"
#include "edDev.h"

float edDevGetRouteClick(uint routeId)
{
	float clickValue;

	if (routeId < edDevRoot.nbRoutes) {
		clickValue = edDevRoot.aPorts[edDevRoot.aRoutes[routeId].portId].pPadD[edDevRoot.aRoutes[routeId].controlId].clickValue;
	}
	else {
		clickValue = 0.0f;
	}
	return clickValue;
}
