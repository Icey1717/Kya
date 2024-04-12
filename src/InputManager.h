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

class CPlayerInput {

public:
	static void ReadConfig(CIniFile* pIniFile);
	static void Update(float delta);
	static void InitDev();

	static int RegisterControllerDisconectedHandler(uint index, ControllerConnectedFuncPtr pNewHandler);
	static int RegisterControllerConnectedHandler(uint index, ControllerConnectedFuncPtr pNewHandler);

	void Init(int bInitialActive);
	void ScanPorts();
	bool SoftReset();
	float GetAngleWithPlayerStick(edF32VECTOR4* param_2);
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
	float field_0x1c;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	float field_0x30;
	int field_0x34;
	int field_0x38;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	float field_0x40;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	float field_0x54;
	int field_0x58;
	int field_0x5c;
	undefined field_0x60;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
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
