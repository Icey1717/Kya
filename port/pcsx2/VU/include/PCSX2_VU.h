#pragma once

#include <functional>

namespace pcsx2_VU {
	void SetMicro(unsigned char* addr);
	void SetMem(unsigned char* addr);
	void Execute(unsigned int addr);

	void SetKickCallback(std::function<void(int)> callback);
	void SetItop(unsigned int newItop);
}