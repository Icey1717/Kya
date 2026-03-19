#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "Types.h"
#include "EdenLib/edDev/Sources/edDev.h"

#define INPUT_BUTTON_INDEX_L_ANALOG_L	0x0
#define INPUT_BUTTON_INDEX_L_ANALOG_R	0x1
#define INPUT_BUTTON_INDEX_L_ANALOG_U	0x2
#define INPUT_BUTTON_INDEX_L_ANALOG_D	0x3

#define INPUT_BUTTON_INDEX_CROSS		0x4
#define INPUT_BUTTON_INDEX_L2			0x5
#define INPUT_BUTTON_INDEX_R2			0x6
#define INPUT_BUTTON_INDEX_TRIANGLE		0x7
#define INPUT_BUTTON_INDEX_CIRCLE		0x8
#define INPUT_BUTTON_INDEX_SQUARE		0x9
#define INPUT_BUTTON_INDEX_R1			0xa
#define INPUT_BUTTON_INDEX_R3			0xb

#define INPUT_BUTTON_INDEX_START		0x12
#define INPUT_BUTTON_INDEX_SELECT		0x13

#define INPUT_BUTTON_DPAD_U				0x14
#define INPUT_BUTTON_DPAD_D				0x15
#define INPUT_BUTTON_DPAD_L				0x16
#define INPUT_BUTTON_DPAD_R				0x17

#define INPUT_BUTTON_INDEX_L1			0x1b


#define PAD_BITMASK_LEFT_ANALOG_LEFT	(1 << INPUT_BUTTON_INDEX_L_ANALOG_L)	// 0x00000001
#define PAD_BITMASK_LEFT_ANALOG_RIGHT	(1 << INPUT_BUTTON_INDEX_L_ANALOG_R)	// 0x00000002
#define PAD_BITMASK_LEFT_ANALOG_UP		(1 << INPUT_BUTTON_INDEX_L_ANALOG_U)	// 0x00000004
#define PAD_BITMASK_LEFT_ANALOG_DOWN	(1 << INPUT_BUTTON_INDEX_L_ANALOG_D)	// 0x00000008
#define PAD_BITMASK_CROSS				(1 << INPUT_BUTTON_INDEX_CROSS)			// 0x00000010
#define PAD_BITMASK_L2					(1 << INPUT_BUTTON_INDEX_L2)			// 0x00000020
#define PAD_BITMASK_R2					(1 << INPUT_BUTTON_INDEX_R2)			// 0x00000040
#define PAD_BITMASK_TRIANGLE			(1 << INPUT_BUTTON_INDEX_TRIANGLE)		// 0x00000080
#define PAD_BITMASK_CIRCLE				(1 << INPUT_BUTTON_INDEX_CIRCLE)		// 0x00000100
#define PAD_BITMASK_SQUARE				(1 << INPUT_BUTTON_INDEX_SQUARE)		// 0x00000200
#define PAD_BITMASK_R1					(1 << INPUT_BUTTON_INDEX_R1)			// 0x00000400
#define PAD_BITMASK_R3					(1 << INPUT_BUTTON_INDEX_R3)			// 0x00000800
#define PAD_BITMASK_1					(1 << 0xc)								// 0x00001000
#define PAD_BITMASK_2					(1 << 0xd)								// 0x00002000
#define PAD_BITMASK_3					(1 << 0xe)								// 0x00004000
#define PAD_BITMASK_4					(1 << 0xf)								// 0x00008000
#define PAD_BITMASK_5					(1 << 0x10)								// 0x00010000
#define PAD_BITMASK_6					(1 << 0x11)								// 0x00020000
#define PAD_BITMASK_START				(1 << INPUT_BUTTON_INDEX_START)			// 0x00040000
#define PAD_BITMASK_SELECT				(1 << INPUT_BUTTON_INDEX_SELECT)		// 0x00080000
#define PAD_BITMASK_DPAD_UP				(1 << INPUT_BUTTON_DPAD_U)				// 0x00100000
#define PAD_BITMASK_DPAD_DOWN			(1 << INPUT_BUTTON_DPAD_D)				// 0x00200000
#define PAD_BITMASK_DPAD_LEFT 			(1 << INPUT_BUTTON_DPAD_L)				// 0x00400000
#define PAD_BITMASK_DPAD_RIGHT 			(1 << INPUT_BUTTON_DPAD_R)				// 0x00800000
#define PAD_BITMASK_11 					(1 << 0x18)								// 0x01000000
#define PAD_BITMASK_12 					(1 << 0x19)								// 0x02000000
#define PAD_BITMASK_13 					(1 << 0x1a)								// 0x04000000
#define PAD_BITMASK_L1					(1 << INPUT_BUTTON_INDEX_L1)			// 0x08000000
#define PAD_BITMASK_14 					(1 << 0x1c)								// 0x10000000
#define PAD_BITMASK_15 					(1 << 0x1d)								// 0x20000000
#define PAD_BITMASK_16 					(1 << 0x1e)								// 0x40000000
#define PAD_BITMASK_17 					(1 << 0x1f)								// 0x80000000

#define PAD_BITMASK_START 0x40000

#define PAD_STICK_LEFT 0x0
#define PAD_STICK_RIGHT 0x1

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

class CPlayerInput
{
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

	static void Reset();

	int bActive;
	int disconnectedController;
	int bDisconnected;
	int bConnected;
	float floatFieldA;
	int bEnableVibration;
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
