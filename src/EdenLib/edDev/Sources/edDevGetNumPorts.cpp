#include "edDevGetNumPorts.h"
#include "edDev.h"

uint edDevGetNumPorts(void)
{
	return edDevRoot.nbPorts;
}
