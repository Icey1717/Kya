#include "Frontend.h"
#include "TimeController.h"
#include "CameraViewManager.h"

ed_3D_Scene* CFrontend::_scene_handle = (ed_3D_Scene*)0x0;

void CFrontend::Level_ClearAll()
{
	return;
}

float CFrontend::GetTime()
{
	return Timer::GetTimer()->totalTime;
}

bool CFrontend::ComputeSceneCoordinate(float param_1, edF32VECTOR4* v0, edF32VECTOR2* pRect)
{
	bool bSuccess;
	edF32VECTOR2 position;

	position.x = (pRect->x - 0.5) * 2.0;
	position.y = -((pRect->y - 0.5) * 2.0);
	bSuccess = false;
	if ((CScene::ptable.g_CameraManager_0045167c)->pFrontendCamera_0x4e4 != (CCamera*)0x0) {
		bSuccess = ed3DComputeScreenCoordinate(param_1, v0, &position, CFrontend::_scene_handle);
	}

	return bSuccess;
}

