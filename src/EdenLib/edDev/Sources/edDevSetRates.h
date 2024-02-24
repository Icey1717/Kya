#ifndef ED_DEV_SET_RATES_H
#define ED_DEV_SET_RATES_H

#include "Types.h"

struct EDDEV_RATES;

uint edDevSetRouteRates(uint routeId, EDDEV_RATES* pRates);

#endif //ED_DEV_SET_RATES_H