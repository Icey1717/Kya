#include "InputManager.h"

#include <math.h>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

#include "MathOps.h"
#include "LargeObject.h"
#include "CameraViewManager.h"
#include "IniFile.h"
#include "edVideo/VideoD.h"
#include "TimeController.h"

extern EFileLoadMode g_FileLoadMode_00448810;

CPlayerInput gPlayerInput = { 0 };
CPlayerInput gPlayerInput_2 = { 0 };

#define NUM_CONTROLLER_PORTS 8
#define NUM_ROUTES 0x20

static int gbPortStatusIsInitialized = 0;
static int gPortStatus[NUM_CONTROLLER_PORTS];

static EDDEV_ROUTE PlayerInput_DefaultConfig[32] =
{
{
INPUT_TYPE_DUALSHOCK,
0x14
},{
INPUT_TYPE_DUALSHOCK,
0x15
},{
INPUT_TYPE_DUALSHOCK,
0x16
},{
INPUT_TYPE_DUALSHOCK,
0x17
},{
INPUT_TYPE_DUALSHOCK,
0x6
},{
INPUT_TYPE_DUALSHOCK,
0x0
},{
INPUT_TYPE_DUALSHOCK,
0x1
},{
INPUT_TYPE_DUALSHOCK,
0x4
},{
INPUT_TYPE_DUALSHOCK,
0x5
},{
INPUT_TYPE_DUALSHOCK,
0x7
},{
INPUT_TYPE_DUALSHOCK,
0x3
},{
INPUT_TYPE_DUALSHOCK,
0xA
},{
INPUT_TYPE_DUALSHOCK,
0x10
},{
INPUT_TYPE_DUALSHOCK,
0x11
},{
INPUT_TYPE_DUALSHOCK,
0x12
},{
INPUT_TYPE_DUALSHOCK,
0x13
},{
INPUT_TYPE_DUALSHOCK,
0xC
},{
INPUT_TYPE_DUALSHOCK,
0xE
},{
INPUT_TYPE_DUALSHOCK,
0xB
},{
INPUT_TYPE_DUALSHOCK,
0x8
},{
INPUT_TYPE_DUALSHOCK,
0xC
},{
INPUT_TYPE_DUALSHOCK,
0xE
},{
INPUT_TYPE_DUALSHOCK,
0xF
},{
INPUT_TYPE_DUALSHOCK,
0xD
},{
INPUT_TYPE_DUALSHOCK,
0x6
},{
INPUT_TYPE_DUALSHOCK,
0x5
},{
INPUT_TYPE_DUALSHOCK,
0x4
},{
INPUT_TYPE_DUALSHOCK,
0x2
},{
INPUT_TYPE_DUALSHOCK,
0x3
},{
INPUT_TYPE_DUALSHOCK,
0x0
},{
INPUT_TYPE_DUALSHOCK,
0x1
},{
INPUT_TYPE_DUALSHOCK,
0x7
}
};

void CPlayerInput::Init(int bInitialActive)
{
	CPlayerInput* pCVar1;
	CPlayerInput* pCVar2;
	EDDEV_ROUTE* pEVar3;
	int iVar4;

	if (gbPortStatusIsInitialized == 0) {
		gbPortStatusIsInitialized = 1;

		for (int i = 0; i < NUM_CONTROLLER_PORTS; i++) {
			gPortStatus[i] = 0;
		}
	}

	this->bActive = bInitialActive;
	iVar4 = 0;
	this->disconnectedController = this->bDisconnected;
	this->bConnected = 0;

	for (int i = 0; i < NUM_ROUTES; i++) {
		this->aRouteIndexes[i] = 0xffffffff;

		this->aButtons[i].analogValue = 0.0f;
		this->aButtons[i].clickValue = 0.0f;
		this->aButtons[i].pressedDuration = 0.0f;
		this->aButtons[i].releasedDuration = 0.0f;
		this->aButtons[i].clickedDuration = 0.0f;
		this->aButtons[i].notClickedDuration = 0.0f;
		this->aButtons[i].bActive = 0;
	}

	this->aButtons[0].bActive = 1;
	this->aButtons[1].bActive = 1;
	this->aButtons[2].bActive = 1;
	this->aButtons[3].bActive = 1;
	this->aButtons[0xc].bActive = 1;
	this->aButtons[0xd].bActive = 1;
	this->aButtons[0xe].bActive = 1;
	this->aButtons[0xf].bActive = 1;
	this->aButtons[0x1b].bActive = 1;
	this->aButtons[0x12].bActive = 1;
	this->aButtons[0x13].bActive = 1;
	this->aButtons[0x14].bActive = 1;
	this->aButtons[0x15].bActive = 1;
	this->aButtons[0x16].bActive = 1;
	this->aButtons[0x17].bActive = 1;
	this->aButtons[0x18].bActive = 1;
	this->aButtons[0x19].bActive = 1;
	this->aButtons[0x1a].bActive = 1;
	this->aButtons[0x1c].bActive = 1;
	this->aButtons[0x1d].bActive = 1;
	this->aButtons[0x1e].bActive = 1;
	this->aButtons[0x1f].bActive = 1;
	this->aButtons[5].bActive = 1;
	this->aButtons[6].bActive = 1;

	this->field_0x14 = 1;
	
	(this->field_0x1c).field_0x0 = 0.0f;
	(this->field_0x1c).field_0x4 = 0.0f;
	(this->field_0x1c).field_0x8 = 0.0f;
	(this->field_0x1c).field_0xc = 0.0f;
	(this->field_0x1c).field_0x10 = 0.0f;
	(this->field_0x1c).field_0x14 = 0.0f;
	(this->field_0x1c).field_0x18 = 4;
	(this->field_0x1c).field_0x20 = 0;
	(this->field_0x1c).field_0x1c = 0;

	(this->field_0x40).field_0x0 = 0.0f;
	(this->field_0x40).field_0x4 = 0.0f;
	(this->field_0x40).field_0x8 = 0.0f;
	(this->field_0x40).field_0xc = 0.0f;
	(this->field_0x40).field_0x10 = 0.0f;
	(this->field_0x40).field_0x14 = 0.0f;
	(this->field_0x40).field_0x18 = 4;
	(this->field_0x40).field_0x20 = 0;
	(this->field_0x40).field_0x1c = 0;

	this->pressedBitfield = 0;
	this->releasedBitfield = 0;
	this->field_0x5ec = 0;
	this->field_0x5f0 = 0;
	this->aAnalogSticks[0].x = 0.0f;
	this->aAnalogSticks[0].y = 0.0f;
	this->aAnalogSticks[0].magnitude = 0.0f;
	this->aAnalogSticks[0].prevMagnitude = 0.0f;
	this->aAnalogSticks[0].downRouteId = 0;
	this->aAnalogSticks[0].upRouteId = 0;
	this->aAnalogSticks[0].rightRouteId = 0;
	this->aAnalogSticks[0].leftRouteId = 0;
	this->aAnalogSticks[1].x = 0.0;
	this->aAnalogSticks[1].y = 0.0;
	this->aAnalogSticks[1].magnitude = 0.0f;
	this->aAnalogSticks[1].prevMagnitude = 0.0f;
	this->aAnalogSticks[1].downRouteId = 0;
	this->aAnalogSticks[1].upRouteId = 0;
	this->aAnalogSticks[1].rightRouteId = 0;
	this->aAnalogSticks[1].leftRouteId = 0;
	(this->lAnalogStick).x = 0.0f;
	(this->lAnalogStick).y = 0.0f;
	(this->lAnalogStick).z = 0.0f;
	(this->lAnalogStick).w = 0.0f;

	for (int i = 0; i < NUM_ROUTES; i++) {
		this->aRoutes[i] = PlayerInput_DefaultConfig[i];
	}

	// L
	this->aAnalogSticks[0].leftRouteId = 0;
	this->aAnalogSticks[0].rightRouteId = 1;
	this->aAnalogSticks[0].upRouteId = 2;
	this->aAnalogSticks[0].downRouteId = 3;

	// R
	this->aAnalogSticks[1].leftRouteId = 0xc;
	this->aAnalogSticks[1].rightRouteId = 0xd;
	this->aAnalogSticks[1].upRouteId = 0xe;
	this->aAnalogSticks[1].downRouteId = 0xf;
	return;
}

RouteEntry PlayerInputConfig_DevicesList[10] = {
	{ 0x013, "JOYSTICK" },
	{ 0x011, "KEYBOARD" },
	{ 0x010, "MOUSE" },
	{ 0x102, "NEGCON" },
	{ 0x103, "GUN" },
	{ 0x106, "GUNCON" },
	{ 0x107, "DUALSHOCK" },
	{ INPUT_TYPE_DUALSHOCK, "DUALSHOCK2" },
	{ 0x10E, "JOGCON" },
	{ 0x000, "" },
};

RouteEntry* PlayerInputConfig_GetDevicesList(void)
{
	return PlayerInputConfig_DevicesList;
}

void CPlayerInput::ScanPorts()
{
	RouteEntry* pCurrentPlayerInputDevice;
	uint nbPorts;
	uint devTypeFlags;
	RouteEntry* pPlayerInputDevice;
	int* pPortStatus;
	uint routeIndex;
	uint portIndex;
	EDDEV_RATES rates;

	nbPorts = edDevGetNumPorts();
	portIndex = 0;
	if (nbPorts != 0) {
		pPortStatus = gPortStatus;
		do {
			uint connectedFlags = edDevGetDevConnected(portIndex);
			if (connectedFlags != 0x40000000) {
				devTypeFlags = edDevGetDevType(portIndex);
				if (*pPortStatus == 0) {
					routeIndex = 0;
					do {
						if ((this->aRouteIndexes[routeIndex] == 0xffffffff) && (this->aRoutes[routeIndex].portId == (devTypeFlags & 0xfffffdff))) {
							*pPortStatus = 1;
							this->aRouteIndexes[routeIndex] = 0;
							this->aRoutes[routeIndex].portId = portIndex;

							if ((devTypeFlags & 0xfffffdff) == 0x11) {
								rates.field_0x4 = 0.5f;
								rates.field_0x0 = 0.5f;
								if ((((routeIndex == 3) || (routeIndex == 2)) || (routeIndex == 1)) || (routeIndex == 0)) {
									edDevSetRouteRates(this->aRouteIndexes[routeIndex], &rates);
								}
							}
						}
						routeIndex = routeIndex + 1;
					} while (routeIndex < 0x20);
				}
			}

			portIndex = portIndex + 1;
			pPortStatus = pPortStatus + 1;
		} while (portIndex < nbPorts);
	}

	this->bConnected = 1;

	routeIndex = 0;
	do {
		if (this->aRouteIndexes[routeIndex] == 0xffffffff) {
			this->bConnected = 0;
			uint portId = this->aRoutes[routeIndex].portId;
			pPlayerInputDevice = PlayerInputConfig_GetDevicesList();
			pCurrentPlayerInputDevice = pPlayerInputDevice;

			for (; (pCurrentPlayerInputDevice->pName != (char*)0x0 && (pPlayerInputDevice->portId != portId)); pPlayerInputDevice = pPlayerInputDevice + 1) {
				pCurrentPlayerInputDevice = pCurrentPlayerInputDevice + 1;
			}

			/* There's still a device not found (from the .INI): %s\n */
			edDebugPrintf("There's still a device not found (from the .INI): %s\n", pCurrentPlayerInputDevice->pName);
		}
		routeIndex = routeIndex + 1;
	} while (routeIndex < 0x20);

	if (this->bConnected != 0) {
		edDevAddRouteList(this->aRoutes, this->aRouteIndexes, 0x20);

		routeIndex = 0;
		do {
			edDevGetRouteType(this->aRouteIndexes[routeIndex]);
			routeIndex = routeIndex + 1;
		} while (routeIndex < 0x20);
	}
	return;
}


bool CPlayerInput::SoftReset()
{
	bool bVar1;

	if ((((this->aButtons[5].clickedDuration <= 0.1f) || (this->aButtons[0x1b].clickedDuration <= 0.1f)) ||
		(this->aButtons[6].clickedDuration <= 0.1f)) ||
		((this->aButtons[10].clickedDuration <= 0.1f || (bVar1 = true, g_FileLoadMode_00448810 == FLM_CD_DVD)))) {
		bVar1 = false;
	}
	return bVar1;
}

float CPlayerInput::GetAngleWithPlayerStick(edF32VECTOR4* param_2)
{
	float puVar1;
	float puVar2;
	float fVar3;
	edF32VECTOR4 local_10;

	if (this->aAnalogSticks[0].magnitude < 0.3f) {
		edF32Vector4ScaleHard(-1.0f, &local_10, param_2);
	}
	else {
		local_10 = this->lAnalogStick;
	}

	puVar1 = edF32Vector4DotProductHard(param_2, &local_10);
	if (1.0f < puVar1) {
		puVar2 = 1.0f;
	}
	else {
		puVar2 = -1.0f;
		if (-1.0f <= puVar1) {
			puVar2 = puVar1;
		}
	}

	fVar3 = acosf(puVar2);
	if (param_2->x * local_10.z - local_10.x * param_2->z < 0.0f) {
		fVar3 = fVar3 * -1.0f;
	}
	return fVar3;
}

CPlayerInput* GetPlayerInput(int playerId)
{
	CPlayerInput* pPlayerInput;

	if (playerId == 0) {
		pPlayerInput = &gPlayerInput;
	}
	else {
		pPlayerInput = &gPlayerInput_2;
	}
	return pPlayerInput;
}

void CPlayerInput::ComputeForce()
{
	CCameraManager* pCameraManager;
	float fVar2;
	float puVar3;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	pCameraManager = CCameraManager::_gThis;
	if (CCameraManager::_gThis != (CCameraManager*)0x0) {
		local_70 = this->lAnalogStick;

		local_10.x = this->aAnalogSticks[0].x;
		local_10.y = this->aAnalogSticks[0].y;
		local_10.z = 0.0f;

		if ((local_10.x == 0.0f) && (local_10.y == 0.0f)) {
			(this->lAnalogStick).x = 0.0f;
			(this->lAnalogStick).y = 0.0f;
			(this->lAnalogStick).z = 0.0f;
			(this->lAnalogStick).w = 0.0f;
		}
		else {
			edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
			if (1.0f < local_10.x) {
				puVar3 = 1.0f;
			}
			else {
				puVar3 = -1.0f;
				if (-1.0f <= local_10.x) {
					puVar3 = local_10.x;
				}
			}
			fVar2 = acosf(puVar3);

			if (local_10.y < 0.0f) {
				fVar2 = 6.283185f - fVar2;
			}

			if ((pCameraManager->transformationMatrix).bb < 0.0f) {
				fVar2 = -fVar2;
			}

			edF32Matrix4RotateYHard(fVar2, &eStack96, &gF32Matrix4Unit);
			local_20.y = 0.0f;
			local_20.x = -pCameraManager->transformationMatrix.rowX.x;
			local_20.z = -pCameraManager->transformationMatrix.rowX.z;
			local_20.w = 0.0f;
			edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
			edF32Matrix4MulF32Vector4Hard(&this->lAnalogStick, &eStack96, &local_20);

			if (1.0f < (this->lAnalogStick).x) {
				(this->lAnalogStick).x = 1.0f;
			}
			else {
				if ((this->lAnalogStick).x < -1.0f) {
					(this->lAnalogStick).x = -1.0f;
				}
			}

			fVar2 = (this->lAnalogStick).y;
			if (1.0f < fVar2) {
				(this->lAnalogStick).y = 1.0f;
			}
			else {
				if (fVar2 < -1.0f) {
					(this->lAnalogStick).y = -1.0f;
				}
			}

			fVar2 = (this->lAnalogStick).z;
			if (1.0f < fVar2) {
				(this->lAnalogStick).z = 1.0f;
			}
			else {
				if (fVar2 < -1.0f) {
					(this->lAnalogStick).z = -1.0f;
				}
			}
		}

		edF32Vector4SubHard(&local_70, &this->lAnalogStick, &local_70);
		edF32Vector4SafeNormalize0Hard(&local_70, &local_70);
	}

	return;
}

void CPlayerInput::UpdateOne(float delta)
{
	bool bVar1;
	bool bVar2;
	uint* puVar3;
	uint uVar4;
	int iVar5;
	uint i;
	AnalogStickData* pAnalog;
	uint uVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	uVar4 = GameFlags;
	this->field_0x5f0 = 0;
	uVar4 = uVar4 & 0x20;

	for (i = 0; i < 0x20; i = i + 1) {
		edDevGetDevConnected(this->aRoutes[i].portId);

		if ((this->aButtons[i].bActive != 0) || (uVar4 == 0)) {
			if ((this->aButtons[i].bActive == 0) && ((uVar4 == 0 && (this->field_0x5ec != 0)))) {
				if (this->aButtons[i].clickValue != 0.0f) {
					fVar7 = edDevGetRouteClick(this->aRouteIndexes[i]);
					if (fVar7 < 2.147484e+09f) {
						uVar6 = (uint)fVar7;
					}
					else {
						uVar6 = (int)(fVar7 - 2.147484e+09f) | 0x80000000;
					}

					if (uVar6 == 0) {
						this->releasedBitfield = this->releasedBitfield | 1 << (i & 0x1f);
						this->aButtons[i].releasedDuration = 0;
					}
				}

				fVar7 = edDevGetRouteAnalog(this->aRouteIndexes[i]);
				this->aButtons[i].analogValue = fVar7;

				fVar7 = edDevGetRouteClick(this->aRouteIndexes[i]);
				if (fVar7 < 2.147484e+09f) {
					fVar7 = (float)(int)fVar7;
				}
				else {
					fVar7 = (float)((int)(fVar7 - 2.147484e+09f) | 0x80000000);
				}

				this->aButtons[i].clickValue = fVar7;
			}
			else {
				// Normal path:
				this->aButtons[i].analogValue = edDevGetRouteAnalog(this->aRouteIndexes[i]);

				fVar7 = edDevGetRouteClick(this->aRouteIndexes[i]);
				if (fVar7 < 2.147484e+09f) {
					fVar7 = (float)(int)fVar7;
				}
				else {
					fVar7 = (float)((int)(fVar7 - 2.147484e+09f) | 0x80000000);
				}
				this->aButtons[i].clickValue = fVar7;

				bVar1 = edDevGetRoutePressed(this->aRouteIndexes[i]);
				if (bVar1 == false) {
					this->pressedBitfield = this->pressedBitfield & ~(1 << (i & 0x1f));
					this->aButtons[i].pressedDuration = this->aButtons[i].pressedDuration + delta;
				}
				else {
					this->pressedBitfield = this->pressedBitfield | 1 << (i & 0x1f);
					this->aButtons[i].pressedDuration = 0.0f;
				}

				bVar1 = edDevGetRouteReleased(this->aRouteIndexes[i]);
				if (bVar1 == false) {
					this->releasedBitfield = this->releasedBitfield & ~(1 << (i & 0x1f));
					this->aButtons[i].releasedDuration = this->aButtons[i].releasedDuration + delta;
				}
				else {
					this->releasedBitfield = this->releasedBitfield | 1 << (i & 0x1f);
					this->aButtons[i].releasedDuration = 0.0f;


					if ((this->releasedBitfield & KEY_START) != 0) {
						this->aButtons[i].releasedDuration = 0.0f;
					}
				}

				if (this->aButtons[i].clickValue == 0.0f) {
					this->aButtons[i].clickedDuration = 0.0f;
					this->aButtons[i].notClickedDuration = this->aButtons[i].notClickedDuration + delta;
				}
				else {
					this->aButtons[i].clickedDuration = this->aButtons[i].clickedDuration + delta;
					this->aButtons[i].notClickedDuration = 0.0f;
					this->field_0x5f0 = 1;
				}
			}
		}
	}

	this->field_0x5ec = uVar4;

	pAnalog = this->aAnalogSticks;
	for (iVar5 = 0; iVar5 < 2; iVar5 = iVar5 + 1) {
		pAnalog->prevMagnitude = pAnalog->magnitude;

		fVar7 = this->aButtons[pAnalog->rightRouteId].analogValue;
		if (0.0f < fVar7) {
			// Right
			pAnalog->x = fVar7;
			fVar7 = this->aButtons[pAnalog->upRouteId].analogValue;
			if (0.0f < fVar7) {
				// Right and Up
				pAnalog->y = fVar7;
				fVar7 = edFIntervalUnitDstLERP(sqrtf(pAnalog->x * pAnalog->x + fVar7 * fVar7), 0.4f, 0.9f);
				fVar9 = pAnalog->y / pAnalog->x;
				fVar8 = sqrtf((fVar7 * fVar7) / (fVar9 * fVar9 + 1.0f));
				pAnalog->x = fVar8;
				pAnalog->y = fVar8 * fVar9;
				pAnalog->x = pAnalog->x * 1.0f;
				pAnalog->y = pAnalog->y * 1.0f;
				pAnalog->magnitude = fVar7;
			}
			else {
				fVar7 = this->aButtons[pAnalog->downRouteId].analogValue;
				if (0.0f < fVar7) {
					// Right and Down
					pAnalog->y = fVar7;
					fVar7 = edFIntervalUnitDstLERP(sqrtf(pAnalog->x * pAnalog->x + fVar7 * fVar7), 0.4f, 0.9f);
					fVar8 = pAnalog->y / pAnalog->x;
					fVar9 = sqrtf((fVar7 * fVar7) / (fVar8 * fVar8 + 1.0f));
					pAnalog->x = fVar9;
					pAnalog->y = fVar9 * fVar8;
					pAnalog->x = pAnalog->x * 1.0f;
					pAnalog->y = pAnalog->y * -1.0f;
					pAnalog->magnitude = fVar7;
				}
				else {
					// Right only
					pAnalog->y = 0.0f;
					fVar7 = pAnalog->x;
					if (0.0f < fVar7) {
						fVar7 = edFIntervalUnitDstLERP(fVar7, 0.4f, 0.9f);
						pAnalog->x = fVar7;
					}
					else {
						fVar7 = edFIntervalLERP(fVar7, -0.4f, -0.9f, 0.0f, -1.0f);
						pAnalog->x = fVar7;
					}
					pAnalog->magnitude = fabs(pAnalog->x);
				}
			}
		}
		else {
			// No right input
			fVar7 = this->aButtons[pAnalog->leftRouteId].analogValue;
			if (0.0f < fVar7) {
				// Left
				pAnalog->x = fVar7;
				if (0.0f < this->aButtons[pAnalog->upRouteId].analogValue) {
					// Left and up
					pAnalog->y = this->aButtons[pAnalog->upRouteId].analogValue;
					fVar7 = edFIntervalUnitDstLERP(sqrtf(pAnalog->x * pAnalog->x + pAnalog->y * pAnalog->y), 0.4f, 0.9f);
					fVar8 = pAnalog->y / pAnalog->x;
					pAnalog->x = sqrtf((fVar7 * fVar7) / (fVar8 * fVar8 + 1.0f));
					pAnalog->y = pAnalog->x * fVar8;
					pAnalog->x = pAnalog->x * -1.0f;
					pAnalog->y = pAnalog->y * 1.0f;
					pAnalog->magnitude = fVar7;
				}
				else {
					if (0.0f < this->aButtons[pAnalog->downRouteId].analogValue) {
						// Left and down
						pAnalog->y = this->aButtons[pAnalog->downRouteId].analogValue;
						fVar7 = edFIntervalUnitDstLERP(sqrtf(pAnalog->x * pAnalog->x + pAnalog->y * pAnalog->y), 0.4f, 0.9f);
						fVar8 = pAnalog->y / pAnalog->x;
						pAnalog->x = sqrtf((fVar7 * fVar7) / (fVar8 * fVar8 + 1.0f));
						pAnalog->y = pAnalog->x * fVar8;
						pAnalog->x = pAnalog->x * -1.0f;
						pAnalog->y = pAnalog->y * -1.0f;
						pAnalog->magnitude = fVar7;
					}
					else {
						// Left only

						// BUG!?

#ifdef PLATFORM_PS2
						pAnalog->y = 0.0f;
						fVar7 = pAnalog->x;
						if (0.0f < fVar7) {
							fVar7 = edFIntervalUnitDstLERP(fVar7, 0.4f, 0.9f);
							pAnalog->x = fVar7;
						}
						else {
							fVar7 = edFIntervalLERP(fVar7, -0.4f, -0.9f, 0.0f, -1.0f);
							pAnalog->x = fVar7;
						}
#else
						pAnalog->x = edFIntervalLERP(fVar7, 0.4f, 0.9f, 0.0f, -1.0f);
#endif
						pAnalog->magnitude = fabs(pAnalog->x);
					}
				}
			}
			else {
				// No horizontal input.
				pAnalog->x = 0.0f;
				fVar7 = this->aButtons[pAnalog->upRouteId].analogValue;
				if (0.0f < fVar7) {
					// Up
					fVar7 = edFIntervalUnitDstLERP(fVar7, 0.4f, 0.9f);
					pAnalog->y = fVar7;
				}
				else {
					// Down
					fVar7 = this->aButtons[pAnalog->downRouteId].analogValue;
					if (0.0f < fVar7) {
						fVar7 = edFIntervalLERP(fVar7, 0.4f, 0.9f, 0.0f, -1.0f);
						pAnalog->y = fVar7;
					}
					else {
						pAnalog->y = 0.0f;
					}
				}

				pAnalog->magnitude = fabs(pAnalog->y);
			}
		}
		pAnalog = pAnalog + 1;
	}

	ComputeForce();

#if 0
	bVar1 = FUN_001b6540(&this->field_0x1c);
	bVar2 = FUN_001b6540(&this->field_0x40);
	if ((bVar1 < '\0') || (bVar2 < '\0')) {
		InputManagerFloatFunc_001b6e20(0.0f, 0.0f, this);
	}
	else {
		if ((bVar1 != false) || (bVar2 != false)) {
			InputManagerFloatFunc_001b6e20(this->field_0x30, this->field_0x54, this);
		}
	}
#endif
	return;
}

void CPlayerInput::ComputeForce3D(edF32VECTOR4* pOutForce)
{
	CCameraManager* pCameraManager;
	edF32VECTOR4 yTranslation;
	edF32VECTOR4 xTranslation;

	pCameraManager = CCameraManager::_gThis;

	if (CCameraManager::_gThis != (CCameraManager*)0x0) {
		edF32Vector4ScaleHard(-this->aAnalogSticks[0].x, &xTranslation, &CCameraManager::_gThis->transformationMatrix.rowX);
		edF32Vector4ScaleHard(this->aAnalogSticks[0].y, &yTranslation, &(pCameraManager->transformationMatrix).rowZ);
		edF32Vector4AddHard(pOutForce, &xTranslation, &yTranslation);
		edF32Vector4SafeNormalize0Hard(pOutForce, pOutForce);
	}

	return;
}

void CPlayerInput::FUN_001b6e20(float param_1, float param_2)
{
	float fVar1;
	//InputEventData local_4;

	if ((((this->field_0x14 != 0) && (this->bDisconnected == 0)) && (this->portIndex != 0xffffffff)) &&
		((this->field_0x1c.field_0x1c != 0 || (this->field_0x40.field_0x1c != 0)))) {
		IMPLEMENTATION_GUARD(
		if (this->field_0x38 == 0) {
			local_4.field_0x0 = 0;
		}
		else {
			fVar1 = param_1 * 255.0;
			if (fVar1 < 2.147484e+09f) {
				local_4.field_0x0 = (byte)(int)fVar1;
			}
			else {
				local_4.field_0x0 = (byte)(int)(fVar1 - 2.147484e+09f);
			}
		}
		if (this->field_0x5c == 0) {
			local_4.field_0x1 = 0;
		}
		else {
			fVar1 = param_2 * 255.0;
			if (fVar1 < 2.147484e+09f) {
				local_4.field_0x1 = (byte)(int)fVar1;
			}
			else {
				local_4.field_0x1 = (byte)(int)(fVar1 - 2.147484e+09f);
			}
		}
		InputFunc_00399ab0(this->portIndex, &local_4);)
	}
	return;
}

void CPlayerInput::ReadConfig(CIniFile* pIniFile)
{
	int iVar1;
	uint MaxControllers;
	uint port;

	edDevReadUpdate();

#ifdef PLATFORM_PS2
	edVideoFlip();
#endif

	edDevReadUpdate();

#ifdef PLATFORM_PS2
	edVideoFlip();
#endif

	edDevReadUpdate();

	port = 0;
	iVar1 = edDevGetNumPorts();
	if (iVar1 != 0) {
		do {
			MaxControllers = edDevGetDevType(port);
			if (MaxControllers == INPUT_TYPE_DUALSHOCK) {
				MaxControllers = edDevGetDevConnected(port);
				if (MaxControllers == 0x40000000) {
					/* Port %d disconnected\n */
					edDebugPrintf("Port %d disconnected\n", port);
				}
				else {
					/* Port %d connected\n */
					edDebugPrintf("Port %d connected\n", port);
				}
			}
			port = port + 1;
			MaxControllers = edDevGetNumPorts();
		} while (port < MaxControllers);
	}

	IMPLEMENTATION_GUARD_LOG(
	/* Devices - SetAction - ReadDevice function */
	CIniFile::GetAllKeys(pIniFile, s_Devices_0042b2f8, s_SetAction_0042b300, _PlayerInput_SetActionCallBack, 0);
	);

	gPlayerInput.Init(1);
	gPlayerInput_2.Init(0);
	return;
}

void CPlayerInput::Update(float delta)
{
	edDevReadUpdate();
	//FUN_001b6fe0(&gPlayerInput);
	if ((gPlayerInput.bActive != 0) && (gPlayerInput.disconnectedController == 0)) {
		if (gPlayerInput.bConnected == 0) {
			gPlayerInput.ScanPorts();
			//ReadInput?(&gPlayerInput);
		}
		else {
			gPlayerInput.UpdateOne(delta);
		}
	}
#if 0
	FUN_001b6fe0(&gPlayerInput_2);
	if ((gPlayerInput_2.bActive != 0) && (gPlayerInput_2.disconnectedController == 0)) {
		if (gPlayerInput_2.bConnected == 0) {
			ScanPorts(&gPlayerInput_2);
			ReadInput?(&gPlayerInput_2);
		}
		else {
			UpdateOne(delta, &gPlayerInput_2);
		}
	}
#endif
	return;
}

int CPlayerInput::RegisterControllerDisconectedHandler(uint index, ControllerConnectedFuncPtr pNewHandler)
{
	int ret;

	if (edDevRoot.nbPorts < index) {
		ret = -2;
	}
	else {
		if (pNewHandler == (ControllerConnectedFuncPtr)0x0) {
			ret = -0xc;
		}
		else {
			edDevRoot.aPorts[index].pDisconnectedHandler = pNewHandler;
			ret = 0;
		}
	}
	return ret;
}

int CPlayerInput::RegisterControllerConnectedHandler(uint index, ControllerConnectedFuncPtr pNewHandler)
{
	int ret;

	if (edDevRoot.nbPorts < index) {
		ret = -2;
	}
	else {
		if (pNewHandler == (ControllerConnectedFuncPtr)0x0) {
			ret = -0xc;
		}
		else {
			edDevRoot.aPorts[index].pConnectedHandler = pNewHandler;
			ret = 0;
		}
	}
	return ret;
}

void ControllerDisconnectedHandler(uint index)
{
	uint devType;

	/* Controller %d disconnected\n */
	edDebugPrintf("Controller %d disconnected\n", index);
	devType = edDevGetDevType(index);
	if (devType == INPUT_TYPE_DUALSHOCK) {
		if (index == gPlayerInput.portIndex) {
			gPlayerInput.bDisconnected = 1;
		}
		else {
			if (index == gPlayerInput_2.portIndex) {
				gPlayerInput_2.bDisconnected = 1;
			}
		}
	}
	return;
}

void ControllerConnectedHandler(uint port)
{
	uint devType;

	/* Controller %d reconnected\n */
	edDebugPrintf("Controller %d reconnected\n", port);
	devType = edDevGetDevType(port);
	if (devType == INPUT_TYPE_DUALSHOCK) {
		if (port == gPlayerInput.portIndex) {
			gPlayerInput.bDisconnected = 0;
		}
		else {
			if (port == gPlayerInput_2.portIndex) {
				gPlayerInput_2.bDisconnected = 0;
			}
		}
	}
	return;
}

void CPlayerInput::InitDev()
{
	uint uVar1;
	uint uVar2;
	uint index;
	float fVar3;
	float fVar4;

	gPlayerInput.portIndex = 0xffffffff;
	gPlayerInput_2.portIndex = 0xffffffff;

	edDevInit(0xcaca0001);
	edDevInitPort(0, 0, INPUT_TYPE_DUALSHOCK);
	edDevInitPort(1, 0, INPUT_TYPE_DUALSHOCK);

#ifdef MOUSE_SUPPORT_EXTENSION_ENABLED
	edDevInitPort(0, 0, INPUT_TYPE_MOUSE);
#endif

	gPlayerInput.bDisconnected = 1;
	index = 0;
	gPlayerInput_2.bDisconnected = 1;

	uVar1 = edDevGetNumPorts();
	if (uVar1 != 0) {
		do {
			uVar2 = edDevGetDevType(index);
			uVar1 = gPlayerInput.portIndex;
			if (uVar2 == INPUT_TYPE_DUALSHOCK) {
				RegisterControllerDisconectedHandler(index, ControllerDisconnectedHandler);
				RegisterControllerConnectedHandler(index, ControllerConnectedHandler);

				uVar1 = index;
				if ((gPlayerInput.portIndex != 0xffffffff) &&
					(uVar1 = gPlayerInput.portIndex, gPlayerInput_2.portIndex == 0xffffffff)) {
					gPlayerInput_2.portIndex = index;
				}
			}
			gPlayerInput.portIndex = uVar1;
			index = index + 1;
			uVar1 = edDevGetNumPorts();
		} while (index < uVar1);
	}

	// Wait?
	IMPLEMENTATION_GUARD_LOG(
	fVar3 = edTimerTimeGet();
	do {
		fVar4 = edTimerTimeGet();
	} while (fVar4 - fVar3 < 2.0f);)
	return;
}

void CPlayerInput::FUN_001b66f0(float param_1, float param_2, float param_3, float param_4, CPlayerInputSubObj* param_5, int param_6)
{
	Timer* pTVar1;
	float fVar2;

	fVar2 = param_4 + param_2 + param_3;

	if (fVar2 != 0.0f) {
		if (10.0f < fVar2) {
			fVar2 = 10.0f / fVar2;
			param_2 = param_2 * fVar2;
			param_3 = param_3 * fVar2;
			param_4 = param_4 * fVar2;
		}

		param_5->field_0x0 = param_1;
		param_5->field_0x4 = param_2;
		param_5->field_0x8 = param_3;
		param_5->field_0xc = param_4;

		if ((GameFlags & 0x20) == 0) {
			pTVar1 = GetTimer();
			param_5->field_0x10 = pTVar1->scaledTotalTime;
			param_5->field_0x20 = 0;
		}
		else {
			pTVar1 = GetTimer();
			param_5->field_0x10 = pTVar1->totalTime;
			param_5->field_0x20 = 1;
		}

		param_5->field_0x18 = 0;

		if (param_6 == 0) {
			param_5->field_0x14 = 0.0f;
		}
		else {
			fVar2 = edFIntervalLERP(param_5->field_0x14, 0.0f, param_1, 0.0f, param_2);
			param_5->field_0x10 = param_5->field_0x10 - fVar2;

			if (param_5->field_0x14 == param_5->field_0x0) {
				if ((GameFlags & 0x20) == 0) {
					pTVar1 = GetTimer();
					param_5->field_0x10 = pTVar1->scaledTotalTime;
				}
				else {
					pTVar1 = GetTimer();
					param_5->field_0x10 = pTVar1->totalTime;
				}

				param_5->field_0x18 = 1;
			}
		}
	}

	return;
}
