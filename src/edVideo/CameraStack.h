#ifndef CAMERA_STACK_H
#define CAMERA_STACK_H

#include "Types.h"

struct CCamera;

struct CameraStackEntry {
	uint field_0x0;
	CCamera* pCamera;
};

struct CCameraStack {
	CCamera* FindActivableCameraInStack(uint param_2);
	bool FindCameraState();
	static int GetCurHeroState(void);
	bool Contains(CCamera* pCamera);
	bool Pop(CCamera* pCamera);
	bool Push(CCamera* pCamera, int param_3);
	void SetMainCamera(CCamera* pCamera);
	bool Manage();
	void Reset();
	void Level_SectorChange(int oldSectorId);
	CCameraStack();

	CameraStackEntry aCameras[64];
	float field_0x200;
	CCamera* pActiveCamera;
	int stackSize;
	CCamera* pPrevActiveCamera;
	int field_0x210;
	enum SWITCH_MODE switchMode;
	float field_0x218;
};

#endif // CAMERA_STACK_H
