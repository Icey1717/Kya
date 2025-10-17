#ifndef _CAMERA_MOUSE_QUAKE_H
#define _CAMERA_MOUSE_QUAKE_H

#include "Types.h"
#include "camera.h"

class CCameraMouse : public CCameraExt {
public:
	static uint _buttons;
	static uint _counter_left;
	static uint _state_left;
	static uint _counter_right;
	static uint _state_right;
	static uint _counter_middle;
	static uint _state_middle;

	CCameraMouse();
	void GetValue(float* pDx, float* pDy, float* pWheel);

	float field_0xd0;
	int mousePortId;
};

class CCameraMouseQuake : public CCameraMouse {
public:
	CCameraMouseQuake();

	// CCamera
	virtual ECameraType GetMode();
	virtual void Init();
	virtual void Reset() {}
	virtual bool Manage();
	virtual void Draw() {}
	virtual bool AlertCamera(int alertType, int param_3, CCamera* param_4);
};

class CCameraMouseAroundPerso : public CCameraMouse {
public:
	CCameraMouseAroundPerso();

	virtual ECameraType GetMode();
	virtual void Init();

	edF32VECTOR4 field_0xe0;
	int isSpecial;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined field_0xf8;
	undefined field_0xf9;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
};

struct CCameraMouseAroundPersoSpecial : public CCameraMouseAroundPerso {
	CCameraMouseAroundPersoSpecial();
};

#endif // !_CAMERA_MOUSE_QUAKE_H
