#pragma once

#include "Types.h"
#include <functional>
#include "EdenLib/edDev/Sources/edDev.h"
#include "input_functions.h"

namespace Input {

	void SetInputFunctions(InputFunctions inputFunctions);
	
	uint _edDevKeyboard(uint eventID, EDDEV_PORT* pController, void* param_3);
}