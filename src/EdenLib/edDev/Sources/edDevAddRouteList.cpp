#include "edDevAddRouteList.h"
#include "edDevAddRoute.h"
#include "edDev.h"

uint edDevAddRouteList(EDDEV_ROUTE* aRoutes, uint* pNbAdded, uint count)
{
	uint ret;
	uint routeIndex;

	routeIndex = 0;

	if (count != 0) {
		do {
			ret = edDevAddRoute(aRoutes->portId, aRoutes->controlId);
			if (ret == 0xfffffff9) {
				return 0xfffffff9;
			}
			if (ret == 0xfffffffe) {
				return 0xfffffffe;
			}
			if (ret == 0xfffffffc) {
				return 0xfffffffc;
			}
			*pNbAdded = ret;
			routeIndex = routeIndex + 1;
			aRoutes = aRoutes + 1;
			pNbAdded = pNbAdded + 1;
		} while (routeIndex < count);
	}

	return 0;
}
