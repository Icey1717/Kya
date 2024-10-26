#ifndef CAMERA_FIXE_H
#define CAMERA_FIXE_H

#include "Types.h"
#include "camera.h"

struct ByteCode;

class CCameraFixe : public CCamera {
public:
	CCameraFixe(ByteCode* pByteCode);

	virtual ECameraType GetMode();
	virtual void Init();
	virtual bool Manage();
	virtual float GetAngleGamma();
	virtual void SetAngleGamma(float angle);

	edF32MATRIX4 field_0xb0;
	edF32VECTOR4 field_0xf0;
	float angleGamma;
};

#endif // !CAMERA_FIXE_H