#ifndef ED_DEV_H
#define ED_DEV_H

#include "Types.h"

#include "ps2/_edDevScanPsxPorts.h"
#include "ps2/_edDevScanPorts.h"
#include "ps2/_edDevInitRoot.h"
#include "ps2/_edDevInitLibs.h"
#include "ps2/_edDevFindPsxDevice.h"
#include "ps2/_edDevDualShock2.h"
#include "ps2/_edDevAttachPsxPort.h"
#include "edDevSetRouteRates.h"
#include "edDevSetRepeatRate.h"
#include "edDevSetRepeatDelay.h"
#include "edDevSetRates.h"
#include "edDevReadUpdate.h"
#include "edDevInit.h"
#include "edDevGetRouteType.h"
#include "edDevGetRouteReleased.h"
#include "edDevGetRoutePressed.h"
#include "edDevGetRouteClick.h"
#include "edDevGetRouteAnalog.h"
#include "edDevGetNumPorts.h"
#include "edDevGetNumChans.h"
#include "edDevGetDevType.h"
#include "edDevGetDevConected.h"
#include "edDevGetChanType.h"
#include "edDevGetChanPressed.h"
#include "edDevGetChanClick.h"
#include "edDevAddRouteList.h"
#include "edDevAddRoute.h"
#include "edDevGetChanAnalog.h"

#define MOUSE_SUPPORT_EXTENSION_ENABLED

#define INPUT_TYPE_DUALSHOCK 0x108
#define INPUT_TYPE_MOUSE 0x010

#define MOUSE_INPUT_BUTTON1 0x0
#define MOUSE_INPUT_BUTTON2 0x1
#define MOUSE_INPUT_BUTTON3 0x2
#define MOUSE_INPUT_DX 0x3
#define MOUSE_INPUT_DY 0x4
#define MOUSE_INPUT_WHEEL 0x5

struct EDDEV_ROUTE {
	uint portId;
	uint controlId;
};

struct EDDEV_RATES {
	float field_0x0;
	float field_0x4;
};

struct Pad_d {
	uint type;
	undefined4 field_0x4;
	float analogValue;
	undefined4 field_0xc;
	float clickValue;
	byte field_0x14;
	bool bPressed;
	bool bReleased;
	undefined field_0x17;
	float field_0x18;
	float field_0x1c;
	float rateA;
	float rateB;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
};

struct Pad_180 {
	uint port;
	undefined4 slot;
	undefined4 field_0x8;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	uint field_0x10;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined4 state;
	undefined4 prevState;
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
	byte pad2ButtonData[18];
	undefined field_0x42;
	undefined field_0x43;
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
	int socketNumber;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	undefined instanceIndex;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	ulonglong padBuffer[32];
};

typedef uint(*ControllerEventFuncPtr)(uint, EDDEV_PORT*, void*);
typedef void(*ControllerConnectedFuncPtr)(uint eventID);

struct EDDEV_PORT {
	uint controllerId;
	uint flags;
	uint maxControlId;
	Pad_d* pPadD;
	Pad_180* pPad180;
	ControllerEventFuncPtr pEventFunc;
	ControllerConnectedFuncPtr pDisconnectedHandler;
	ControllerConnectedFuncPtr pConnectedHandler;
	int field_0x2c;
};

struct DevRoot {
	int nbRoutes;
	int nbMaxRoutes;
	int nbPadD;
	int nbMaxPadD;
	uint nbPorts;
	uint nbMaxPorts;
	EDDEV_PORT* aPorts;
	EDDEV_ROUTE* aRoutes;
	Pad_d* aPadD;
	byte field_0x3c;
};

extern DevRoot edDevRoot;

#endif //ED_DEV_H