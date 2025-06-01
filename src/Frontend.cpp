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

bool CFrontend::ComputeSceneCoordinate(float z, edF32VECTOR4* pWorldPosition, edF32VECTOR2* pScreenCoordinate)
{
	bool bSuccess;
	edF32VECTOR2 position;

	position.x = (pScreenCoordinate->x - 0.5f) * 2.0f;
	position.y = -((pScreenCoordinate->y - 0.5f) * 2.0f);

	bSuccess = false;
	if ((CScene::ptable.g_CameraManager_0045167c)->pFrontendCamera_0x4e4 != (CCamera*)0x0) {
		bSuccess = ed3DComputeScreenCoordinate(z, pWorldPosition, &position, CFrontend::_scene_handle);
	}

	return bSuccess;
}

long CFrontendSamplePlayer::PlaySample(float, float*, int, int)
{
	IMPLEMENTATION_GUARD_AUDIO();
	return 0;
}
