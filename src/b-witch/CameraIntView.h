#ifndef _CAMERA_INT_VIEW_H
#define _CAMERA_INT_VIEW_H

#include "Types.h"
#include "CameraBoomy.h"

class CCameraIntView : public CCameraAimBase
{
public:
	virtual ECameraType GetMode();
	virtual void Init();
	virtual bool Manage();
	virtual bool AlertCamera(int alertType, void* pParams, CCamera* param_4);

	void Manage_Pad(float* param_2, float* param_3);

	float field_0x100;
	float field_0x104;
};

#endif // _CAMERA_INT_VIEW_H