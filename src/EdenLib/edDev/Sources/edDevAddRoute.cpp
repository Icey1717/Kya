#include "edDevAddRoute.h"
#include "edDev.h"

uint edDevAddRoute(uint deviceId, uint controlId)
{
	uint ret;
	EDDEV_ROUTE* pRoute;

	ret = 0xfffffff9;
	if (edDevRoot.nbRoutes < edDevRoot.nbMaxRoutes) {
		pRoute = edDevRoot.aRoutes + edDevRoot.nbRoutes;
		if (edDevRoot.nbPorts < deviceId) {
			ret = 0xfffffffe;
		}
		else {
			ret = 0xfffffffc;
			if (controlId <= edDevRoot.aPorts[deviceId].maxControlId) {
				pRoute->portId = deviceId;
				pRoute->controlId = controlId;
				ret = edDevRoot.nbRoutes;
				edDevRoot.nbRoutes = edDevRoot.nbRoutes + 1;
			}
		}
	}
	return ret;
}