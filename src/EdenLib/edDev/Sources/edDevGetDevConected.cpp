#include "edDevGetDevConected.h"
#include "edDev.h"

uint edDevGetDevConnected(uint deviceId)
{
	uint connectedFlags;

	if (edDevRoot.nbPorts < deviceId) {
		connectedFlags = 0xfffffffe;
	}
	else {
		connectedFlags = edDevRoot.aPorts[deviceId].flags & 0x40000000;
	}
	return connectedFlags;
}
