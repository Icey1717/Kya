#ifndef _CAMERA_CINEMATIC_H
#define _CAMERA_CINEMATIC_H

#include "Types.h"
#include "camera.h"

class CCameraCinematic : public CCamera
{
public:
	CCameraCinematic();

	void SetTransform(edF32MATRIX4* transformMatrix);

	virtual ECameraType GetMode(void);
	virtual void Init();
	virtual float GetAngleAlpha();
	virtual float GetAngleBeta();
	virtual float GetAngleGamma();

	edF32VECTOR3 field_0xb0;

	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
};

#endif // !_CAMERA_CINEMATIC_H