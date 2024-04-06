#include "CameraMouseQuake.h"

#include "MathOps.h"
#include "CameraViewManager.h"
#include "TimeController.h"
#include "Debug.h"
#include "EdenLib/edDev/Sources/edDev.h"

uint CCameraMouse::_buttons = 0;

uint CCameraMouse::_counter_left = 0;
uint CCameraMouse::_state_left = 0;
uint CCameraMouse::_counter_right = 0;
uint CCameraMouse::_state_right = 0;
uint CCameraMouse::_counter_middle = 0;
uint CCameraMouse::_state_middle = 0;

CCameraMouse::CCameraMouse()
{
	this->mousePortId = -1;
	this->field_0xd0 = 1.0f;
	this->flags_0xc = this->flags_0xc | 0x40000;
}

void CCameraMouse::GetValue(float* pDx, float* pDy, float* pWheel)
{
	bool bMouseConnected;
	uint uVar2;
	uint uVar3;
	uint index;
	float fVar4;

	// Scan for mouse.
	if (this->mousePortId == -1) {
		uVar2 = edDevGetNumPorts();
		index = 0;
		if (uVar2 != 0) {
			do {
				uVar3 = edDevGetDevType(index);

				if (uVar3 == 0x210) {
					this->mousePortId = index;
					bMouseConnected = true;
					goto LAB_0018eea8;
				}

				index = index + 1;
			} while (index < uVar2);
		}
		bMouseConnected = false;
	}
	else {
		bMouseConnected = true;
	}

LAB_0018eea8:
	if (bMouseConnected) {
		fVar4 = edDevGetChanClick(this->mousePortId, MOUSE_INPUT_BUTTON1);
		uVar2 = (uint)(fVar4 != 0.0f);

		fVar4 = edDevGetChanClick(this->mousePortId, MOUSE_INPUT_BUTTON2);
		if (fVar4 != 0.0f) {
			uVar2 = uVar2 | 2;
		}

		fVar4 = edDevGetChanClick(this->mousePortId, MOUSE_INPUT_BUTTON3);
		if (fVar4 != 0.0f) {
			uVar2 = uVar2 | 4;
		}
		_buttons = uVar2;

		fVar4 = edDevGetChanAnalog(this->mousePortId, MOUSE_INPUT_DX);
		*pDx = fVar4;

		fVar4 = edDevGetChanAnalog(this->mousePortId, MOUSE_INPUT_DY);
		*pDy = fVar4;

		fVar4 = edDevGetChanAnalog(this->mousePortId, MOUSE_INPUT_WHEEL);
		*pWheel = fVar4;
	}
	else {
		*pDx = 0.0f;
		*pDy = 0.0f;
		*pWheel = 0.0f;
	}

	if (((*pDx == 0.0f) && (*pDy == 0.0f)) && (*pWheel == 0.0f)) {
		if (_state_left == 2) {
			if ((_buttons & 1) == 0) {
				_counter_left = _counter_left + 1;

				if (_counter_left == 0x1e) {
					_state_left = 0;
				}
			}
			else {
				_state_left = 0;
				_buttons = _buttons | 8;
			}
		}
		else {
			if (_state_left == 1) {
				if ((_buttons & 1) == 0) {
					_state_left = 2;
				}
			}
			else {
				if (_state_left == 0) {
					if ((_buttons & 1) == 0) {
						_counter_left = 0;
					}
					else {
						_state_left = 1;
					}
				}
			}
		}

		if (_state_right == 2) {
			if ((_buttons & 2) == 0) {
				_counter_right = _counter_right + 1;
				if (_counter_right == 0x1e) {
					_state_right = 0;
				}
			}
			else {
				_state_right = 0;
				_buttons = _buttons | 0x10;
			}
		}
		else {
			if (_state_right == 1) {
				if ((_buttons & 2) == 0) {
					_state_right = 2;
				}
			}
			else {
				if (_state_right == 0) {
					if ((_buttons & 2) == 0) {
						_counter_right = 0;
					}
					else {
						_state_right = 1;
					}
				}
			}
		}

		if (_state_middle == 2) {
			if ((_buttons & 4) == 0) {
				_counter_middle = _counter_middle + 1;
				if (_counter_middle == 0x1e) {
					_state_middle = 0;
				}
			}
			else {
				_state_middle = 0;
				_buttons = _buttons | 0x20;
			}
		}
		else {
			if (_state_middle == 1) {
				if ((_buttons & 4) == 0) {
					_state_middle = 2;
				}
			}
			else {
				if (_state_middle == 0) {
					if ((_buttons & 4) == 0) {
						_counter_middle = 0;
					}
					else {
						_state_middle = 1;
					}
				}
			}
		}
	}
	else {
		_counter_left = 0;
		_state_left = 0;
		_counter_right = 0;
		_state_right = 0;
		_counter_middle = 0;
		_state_middle = 0;
	}
	return;
}

CCameraMouseQuake::CCameraMouseQuake()
{
	this->fov = 0.6f;
}

ECameraType CCameraMouseQuake::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_MouseQuake: 0x{:x}", (int)CT_MouseQuake);
	return CT_MouseQuake;
}

void CCameraMouseQuake::Init()
{
	float fVar1;
	float fVar2;
	float fVar3;

	CCameraExt::Init();
	this->SetDistance(6.0f);
	this->SetAngleAlpha(0.0f);
	this->SetAngleBeta(0.0f);
	this->SetAngleGamma(0.0f);

	this->transformationMatrix.rowT = gF32Vertex4Zero;

	this->lookAt.x = 0.0f;
	this->lookAt.y = 0.0f;
	this->lookAt.z = 1.0f;
	this->lookAt.w = 1.0f;

	this->transformationMatrix.rowZ = gF32Vector4UnitZ;

	this->flags_0xc = this->flags_0xc | 0x20000;
	this->field_0x8 = -0x9f;
	return;
}

bool CCameraMouseQuake::Manage()
{
	undefined8 uVar1;
	bool bVar2;
	Timer* pTVar3;
	edF32VECTOR3* v0;
	ulong uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	float local_c;
	float local_8;
	float mouseDx;

	if (((CCamera::_gpcam_man->flags & CAMERA_HACK_ACTIVE) == 0) || (bVar2 = false, this == (CCameraMouseQuake*)CCamera::_gpcam_man->pMouseQuakeCamera_0x4e8)) {
		bVar2 = CCamera::Manage();
		if (bVar2 == false) {
			local_20 = transformationMatrix.rowT;

			GetValue(&mouseDx, &local_8, &local_c);

			if (local_c != 0.0f) {
				if (0.0f < local_c) {
					this->field_0xd0 = this->field_0xd0 * 1.1f;
				}
				else {
					this->field_0xd0 = this->field_0xd0 / 1.1f;
				}

				if (10.0f < this->field_0xd0) {
					this->field_0xd0 = 10.0f;
				}
				else {
					if (this->field_0xd0 < 0.01f) {
						this->field_0xd0 = 0.01f;
					}
				}
			}

			pTVar3 = GetTimer();
			fVar8 = pTVar3->lastFrameTime * this->field_0xd0;

			if (CCameraMouse::_buttons == 6) {
				fVar6 = 0.5f;
				if (fVar8 * 8.0f <= 0.5f) {
					fVar6 = fVar8 * 8.0f;
				}

				fVar5 = GetAngleGamma();
				fVar6 = edF32Between_2Pi(mouseDx * fVar6 * 0.01745329f + fVar5);
				SetAngleGamma(fVar6);
			}
			else {
				if (CCameraMouse::_buttons == 3) {
					if (local_8 < -1.0f) {
						this->fov = this->fov * 0.975f;
					}
					if (1.0f < local_8) {
						this->fov = this->fov * 1.025f;
					}
				}
				else {
					if ((CCameraMouse::_buttons == 2) || (CCameraMouse::_buttons == 0x10)) {
						edF32Vector4ScaleHard(fVar8 * local_8, &local_30, &this->transformationMatrix.rowY);
						edF32Vector4SubHard(&local_20, &local_20, &local_30);
						edF32Vector4ScaleHard(fVar8 * mouseDx, &local_30, &this->transformationMatrix.rowX);
						edF32Vector4SubHard(&local_20, &local_20, &local_30);
					}
					else {
						fVar6 = 0.5f;
						if (fVar8 * 8.0f <= 0.5f) {
							fVar6 = fVar8 * 8.0f;
						}

						fVar7 = mouseDx * fVar6 * 0.01745329f;
						fVar5 = GetAngleBeta();
						fVar5 = edF32Between_2Pi(fVar5 - fVar7);
						SetAngleBeta(fVar5);

						fVar5 = local_8 * fVar6 * 0.01745329f;
						fVar6 = GetAngleAlpha();
						SetAngleAlpha(fVar6 - fVar5);

						fVar6 = this->angles.x;
						if (1.566796f < fVar6) {
							this->angles.x = 1.566796f;
						}
						else {
							if (fVar6 < -1.566796f) {
								this->angles.x = -1.566796f;
							}
						}

						if ((CCameraMouse::_buttons & 1) == 0) {
							if ((CCameraMouse::_buttons & 4) != 0) {
								edF32Vector4ScaleHard(-fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowZ);
								edF32Vector4AddHard(&local_20, &local_20, &local_30);
							}
						}
						else {
							edF32Vector4ScaleHard(fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowZ);
							edF32Vector4AddHard(&local_20, &local_20, &local_30);
						}
					}
				}
			}

			uVar4 = edDebugMenuIsDisplayed();
			if (uVar4 == 0) {
				bVar2 = false;
				if (edDebugMenu.KeyboardID_00468f24 != 0x80000000) {
					fVar6 = edDevGetChanAnalog(edDebugMenu.KeyboardID_00468f24, 0);
					bVar2 = (int)fVar6 == 0x51;
				}
			}
			else {
				bVar2 = false;
			}

			if (bVar2) {
				edF32Vector4ScaleHard(-fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowZ);
				edF32Vector4AddHard(&local_20, &local_20, &local_30);
			}

			uVar4 = edDebugMenuIsDisplayed();
			if (uVar4 == 0) {
				bVar2 = false;
				if (edDebugMenu.KeyboardID_00468f24 != 0x80000000) {
					fVar6 = edDevGetChanAnalog(edDebugMenu.KeyboardID_00468f24, 0);
					bVar2 = (int)fVar6 == 0x52;
				}
			}
			else {
				bVar2 = false;
			}

			if (bVar2) {
				edF32Vector4ScaleHard(fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowZ);
				edF32Vector4AddHard(&local_20, &local_20, &local_30);
			}

			uVar4 = edDebugMenuIsDisplayed();
			if (uVar4 == 0) {
				bVar2 = false;
				if (edDebugMenu.KeyboardID_00468f24 != 0x80000000) {
					fVar6 = edDevGetChanAnalog(edDebugMenu.KeyboardID_00468f24, 0);
					bVar2 = (int)fVar6 == 0x50;
				}
			}
			else {
				bVar2 = false;
			}

			if (bVar2) {
				edF32Vector4ScaleHard(fVar8 * 6.0, &local_30, &this->transformationMatrix.rowX);
				edF32Vector4AddHard(&local_20, &local_20, &local_30);
			}

			uVar4 = edDebugMenuIsDisplayed();
			if (uVar4 == 0) {
				bVar2 = false;
				if (edDebugMenu.KeyboardID_00468f24 != 0x80000000) {
					fVar6 = edDevGetChanAnalog(edDebugMenu.KeyboardID_00468f24, 0);
					bVar2 = (int)fVar6 == 0x4f;
				}
			}
			else {
				bVar2 = false;
			}

			if (bVar2) {
				edF32Vector4ScaleHard(-fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowX);
				edF32Vector4AddHard(&local_20, &local_20, &local_30);
			}

			uVar4 = edDebugMenuIsDisplayed();
			if (uVar4 == 0) {
				bVar2 = false;
				if (edDebugMenu.KeyboardID_00468f24 != 0x80000000) {
					fVar6 = edDevGetChanAnalog(edDebugMenu.KeyboardID_00468f24, 0);
					bVar2 = (int)fVar6 == 0x4e;
				}
			}
			else {
				bVar2 = false;
			}

			if (bVar2) {
				edF32Vector4ScaleHard(-fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowY);
				edF32Vector4AddHard(&local_20, &local_20, &local_30);
			}

			uVar4 = edDebugMenuIsDisplayed();
			if (uVar4 == 0) {
				bVar2 = false;
				if (edDebugMenu.KeyboardID_00468f24 != 0x80000000) {
					fVar6 = edDevGetChanAnalog(edDebugMenu.KeyboardID_00468f24, 0);
					bVar2 = (int)fVar6 == 0x4b;
				}
			}
			else {
				bVar2 = false;
			}

			if (bVar2) {
				edF32Vector4ScaleHard(fVar8 * 6.0f, &local_30, &this->transformationMatrix.rowY);
				edF32Vector4AddHard(&local_20, &local_20, &local_30);
			}

			local_30.x = 0.0f;
			local_30.y = 0.0f;
			local_30.z = 1.0f;
			local_30.w = 0.0f;

			v0 = GetAngles();
			edF32Matrix4FromEulerSoft(&this->transformationMatrix, v0, "ZXY");
			this->transformationMatrix.rowT = local_20;

			fVar8 = GetDistance();
			edF32Vector4ScaleHard(fVar8, &local_30, &this->transformationMatrix.rowZ);
			edF32Vector4AddHard(&local_20, &this->transformationMatrix.rowT, &local_30);

			bVar2 = true;

			this->lookAt = local_20;
		}
		else {
			bVar2 = false;
		}
	}

	return bVar2;
}

bool CCameraMouseQuake::AlertCamera(int alertType, int param_3, CCamera* param_4)
{
	if (alertType == 2) {
		CCamera::_gpcam_man->KeepSameParam(this, 0x37);
	}

	CCamera::AlertCamera(alertType, param_3, param_4);

	return false;
}

CCameraMouseAroundPerso::CCameraMouseAroundPerso()
{
	this->isSpecial = 0;
}

ECameraType CCameraMouseAroundPerso::GetMode()
{
	ECameraType EVar1;

	EVar1 = CT_AroundSpecial;
	if (this->isSpecial == 0) {
		EVar1 = CT_MouseAround;
	}

	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found: 0x{:x}", (int)EVar1);

	return EVar1;
}

void CCameraMouseAroundPerso::Init()
{
	CCameraExt::Init();
	this->fov = 0.84f;

	(this->field_0xe0).x = 0.0f;
	(this->field_0xe0).y = 1.4f;
	(this->field_0xe0).z = 0.0f;
	(this->field_0xe0).w = 0.0f;

	this->flags_0xc = this->flags_0xc | 0x20001;
	this->field_0x8 = -0x9f;

	return;
}

CCameraMouseAroundPersoSpecial::CCameraMouseAroundPersoSpecial()
{
	this->isSpecial = 1;
}