#include "CameraBoomy.h"
#include "MathOps.h"

CCameraAimBase::CCameraAimBase()
	: CCameraExt()
{
	this->field_0xd0 = gF32Vector4Zero;
	this->field_0xf0 = 0;

	return;
}

void CCameraAimBase::Init()
{
	CCameraExt::Init();

	this->flags_0xc = this->flags_0xc | 0x20004;
	this->field_0x8 = -100;

	return;
}

CCameraAim::CCameraAim()
	: CCameraAimBase()
{
	this->field_0x114 = 0;
	this->field_0x100 = 0.1186824f;
	this->field_0x104 = 0.06283185f;

	this->switchMode = SWITCH_MODE_F;
	this->field_0x98 = 0.2666667f;
	this->field_0x94 = SWITCH_MODE_F;
	this->field_0x9c = 0.15f;

	return;
}

ECameraType CCameraAim::GetMode()
{
	return CT_SilverBoomy;
}

void CCameraAim::Init()
{
	CCameraAimBase::Init();

	this->fov = 0.364f;

	SetDistance(1.8f);

	this->field_0xe0.Init(0.0f, 6.981317f);
	this->field_0xe8.Init(0.0f, 7.853981f);

	this->field_0x178 = 0;
	this->field_0x17c = 0.12f;

	return;
}

bool CCameraAim::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CCameraAim::Draw()
{
	return;
}

bool CCameraAim::AlertCamera(int param_2, int param_3, CCamera* param_4)
{
	IMPLEMENTATION_GUARD();
}
