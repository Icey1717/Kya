#include "edDevReadUpdate.h"
#include "edDev.h"

#include <string>

void edDevReadUpdate(void)
{
	uint nbPorts;
	uint uVar2;
	EDDEV_PORT* pController;
	uint portId;
	byte acStack8[8];

	pController = edDevRoot.aPorts;
	nbPorts = edDevRoot.nbPorts;
	memset(acStack8, 0, 8);

	portId = 0;
	if (nbPorts != 0) {
		do {
			uVar2 = pController->flags & 0xbfffffff;

			if ((uVar2 < 8) && (acStack8[uVar2] == 0)) {
				(*pController->pEventFunc)(0x90000008, pController, (void*)0x0);
				acStack8[uVar2] = 1;
			}

			(*pController->pEventFunc)(0x90000002, pController, (void*)0x0);
			(*pController->pEventFunc)(0x90000003, pController, (void*)0x0);
			(*pController->pEventFunc)(0x90000006, pController, (void*)0x0);

			if (pController->field_0x2c != 0) {
				(*pController->pEventFunc)(0x90000005, pController, (void*)0x0);
			}

			portId = portId + 1;
			edDevRoot.field_0x3c = 0;
			pController = pController + 1;
		} while (portId < nbPorts);
	}
	return;
}
