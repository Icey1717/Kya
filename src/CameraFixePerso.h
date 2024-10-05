#ifndef _CAMERA_FIXE_PERSO_H
#define _CAMERA_FIXE_PERSO_H

#include "Types.h"
#include "camera.h"

class CCameraFixePerso : public CCameraExt {
public:
	CCameraFixePerso();

	virtual ECameraType GetMode();
	virtual void Init();
	virtual bool Manage();
	virtual bool AlertCamera(int alertType, int param_3, CCamera* param_4);

	int field_0xd0;
	uint field_0xd4;
	float field_0xd8;
	undefined4 field_0xdc;
	edF32MATRIX4 field_0xe0;
};

#endif // !_CAMERA_FIXE_PERSO_H
