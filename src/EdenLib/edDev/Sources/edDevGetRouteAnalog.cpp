#include "edDevGetRouteAnalog.h"
#include "edDev.h"

float edDevGetRouteAnalog(uint routeId)
{
	float analogValue;

	if (routeId < edDevRoot.nbRoutes) {
		analogValue = edDevRoot.aPorts[edDevRoot.aRoutes[routeId].portId].pPadD[edDevRoot.aRoutes[routeId].controlId].analogValue;
	}
	else {
		analogValue = 0.0f;
	}
	return analogValue;
}