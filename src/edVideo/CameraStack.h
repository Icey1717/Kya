#ifndef CAMERA_STACK_H
#define CAMERA_STACK_H

#include "Types.h"

typedef enum SWITCH_MODE {
	SWITCH_MODE_A = 0,
	SWITCH_MODE_B = 1,
	SWITCH_MODE_C = 2,
	SWITCH_MODE_D = 3,
	SWITCH_MODE_E = 4,
	SWITCH_MODE_F = 5
} SWITCH_MODE;

struct CameraStackEntry {
	uint field_0x0;
	struct CCamera* pCamera;
};

struct CCameraStack {
	CCamera* FindActivableCameraInStack(uint param_2);
	bool FindCameraState();
	int GetCurHeroState(void);
	bool Contains(CCamera* pCamera);
	bool Pop(CCamera* pCamera);
	bool Push(CCamera* pCamera, int param_3);
	void SetMainCamera(CCamera* pCamera);
	bool Manage();
	void Reset();
	CCameraStack();

	CameraStackEntry aCameras[64];
	float field_0x200;
	struct CCamera* pActiveCamera;
	int stackSize;
	CCamera* field_0x20c;
	undefined4 field_0x210;
	enum SWITCH_MODE switchMode;
	float field_0x218;
};

#endif // CAMERA_STACK_H
