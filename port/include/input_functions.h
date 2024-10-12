#pragma once

#define ROUTE_L2 0x0
#define ROUTE_R2 0x1
#define ROUTE_L1 0x2
#define ROUTE_R1 0x3
#define ROUTE_TRIANGLE 0x4
#define ROUTE_CIRCLE 0x5
#define ROUTE_CROSS 0x6
#define ROUTE_SQUARE 0x7
#define ROUTE_SELECT 0x8
#define ROUTE_START 0xb

#define ROUTE_UP 0xc
#define ROUTE_DOWN 0xe

#define ROUTE_L_ANALOG_UP 0x16
#define ROUTE_L_ANALOG_DOWN 0x17

#define ROUTE_L_ANALOG_LEFT 0x14
#define ROUTE_L_ANALOG_RIGHT 0x15

namespace Input {

	struct InputFunctions {
		std::function<bool(uint)> keyPressed;
		std::function<bool(uint)> keyReleased;
		std::function<float(uint)> keyAnalog;

		std::function<bool(uint)> mousePressed;
		std::function<bool(uint)> mouseReleased;
		std::function<float(uint)> mouseAnalog;
	};
}