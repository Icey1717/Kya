#pragma once

#include "Types.h"

class CActor;

namespace Debug::Camera {
	void ShowCamera();
	void FocusActor(CActor* actor);
	void SetFreeCameraPose(const edF32VECTOR4& position, float yaw, float pitch);
	void ResetFreeCamera();
}
