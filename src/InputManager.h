#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "Types.h"
#include "EdenLib/edDev/Sources/edDev.h"

struct InputTest {
	float analogValue;
	float clickValue;

	float clickedDuration;
	float notClickedDuration;

	float pressedDuration;
	float releasedDuration;

	int bActive;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

struct RouteEntry
{
	int portId;
	char* pName;
};

struct AnalogStickData {
	float x;
	float y;
	float magnitude;
	float prevMagnitude;
	int leftRouteId;
	int rightRouteId;
	int upRouteId;
	int downRouteId;
};

class CIniFile;

#define KEY_START 0x40000

struct CPlayerInputSubObj {
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	int field_0x18;
	int field_0x1c;
	float field_0x20;
};

class CPlayerInput {

public:
	static void ReadConfig(CIniFile* pIniFile);
	static void Update(float delta);
	static void InitDev();

	static void FUN_001b66f0(float param_1, float param_2, float param_3, float param_4, CPlayerInputSubObj* param_5, int param_6);

	static int RegisterControllerDisconectedHandler(uint index, ControllerConnectedFuncPtr pNewHandler);
	static int RegisterControllerConnectedHandler(uint index, ControllerConnectedFuncPtr pNewHandler);

	void Init(int bInitialActive);
	void ScanPorts();
	bool SoftReset();
	float GetAngleWithPlayerStick(edF32VECTOR4* param_2);
	void GetPadRelativeToNormal2D(edF32VECTOR4* param_2, float* param_3, float* param_4, float* param_5);
	void ComputeForce();
	void UpdateOne(float delta);

	void ComputeForce3D(edF32VECTOR4* pOutForce);

	void FUN_001b6e20(float param_1, float param_2);

	int bActive;
	int disconnectedController;
	int bDisconnected;
	int bConnected;
	float floatFieldA;
	int field_0x14;
	uint portIndex;
	CPlayerInputSubObj field_0x1c;
	CPlayerInputSubObj field_0x40;
	EDDEV_ROUTE aRoutes[32];
	uint aRouteIndexes[32];
	InputTest aButtons[32];
	uint pressedBitfield;
	uint releasedBitfield;
	uint field_0x5ec;
	undefined4 field_0x5f0;
	AnalogStickData aAnalogSticks[2];
	undefined field_0x634;
	undefined field_0x635;
	undefined field_0x636;
	undefined field_0x637;
	undefined field_0x638;
	undefined field_0x639;
	undefined field_0x63a;
	undefined field_0x63b;
	undefined field_0x63c;
	undefined field_0x63d;
	undefined field_0x63e;
	undefined field_0x63f;
	edF32VECTOR4 lAnalogStick;
};

extern CPlayerInput gPlayerInput;

CPlayerInput* GetPlayerInput(int playerId);

#endif //_INPUT_MANAGER_H
