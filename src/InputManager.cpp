#include "InputManager.h"

extern EFileLoadMode g_FileLoadMode_00448810;

CPlayerInput gPlayerInput = { 0 };
CPlayerInput gPlayerInput_2 = { 0 };

bool CPlayerInput::SoftReset()
{
	bool bVar1;

	if ((((this->buttonArray[5].field_0x8 <= 0.1) || (this->buttonArray[0x1b].field_0x8 <= 0.1)) ||
		(this->buttonArray[6].field_0x8 <= 0.1)) ||
		((this->buttonArray[10].field_0x8 <= 0.1 || (bVar1 = true, g_FileLoadMode_00448810 == FLM_CD_DVD)))) {
		bVar1 = false;
	}
	return bVar1;
}

CPlayerInput* GetPlayerInput(int playerId)
{
	CPlayerInput* pPlayerInput;

	if (playerId == 0) {
		pPlayerInput = &gPlayerInput;
	}
	else {
		pPlayerInput = &gPlayerInput_2;
	}
	return pPlayerInput;
}