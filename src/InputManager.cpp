#include "InputManager.h"

extern EFileLoadMode g_FileLoadMode_00448810;

InputManager g_InputManager_00450960 = { 0 };

bool InputManager::SoftReset()
{
	bool bVar1;

	if ((((this->buttonArray[5].field_0x8 <= 0.1) || (this->buttonArray[0x1b].field_0x8 <= 0.1)) ||
		(this->buttonArray[6].field_0x8 <= 0.1)) ||
		((this->buttonArray[10].field_0x8 <= 0.1 || (bVar1 = true, g_FileLoadMode_00448810 == FLM_CD_DVD)))) {
		bVar1 = false;
	}
	return bVar1;
}