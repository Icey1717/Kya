#include "edDevGetDevType.h"
#include "edDev.h"

uint edDevGetDevType(uint index)
{
	uint devType;

	if (index < edDevRoot.nbPorts) {
		devType = edDevRoot.aPorts[index].flags & 0xbfffffff;
	}
	else {
		devType = 0xfffffffe;
	}
	return devType;
}
