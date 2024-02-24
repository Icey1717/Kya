#ifndef ED_DEV_ADD_ROUTE_LIST_H
#define ED_DEV_ADD_ROUTE_LIST_H

#include "Types.h"

struct EDDEV_ROUTE;

uint edDevAddRouteList(EDDEV_ROUTE* aRoutes, uint* pNbAdded, uint count);

#endif //ED_DEV_ADD_ROUTE_LIST_H