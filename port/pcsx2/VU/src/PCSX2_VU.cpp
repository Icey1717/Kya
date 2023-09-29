#include "PCSX2_VU.h"
#include "VUmicro.h"

std::function<void(int)> gKickCallback;

void pcsx2_VU::SetMicro(unsigned char* addr)
{
	VU1.Micro = addr;
}

void pcsx2_VU::SetMem(unsigned char* addr)
{
	VU1.Mem = addr;
}

void pcsx2_VU::Execute(unsigned int addr)
{
	vif1Regs.itop = vif1Regs.itops;
	vif1Regs.top = vif1Regs.tops & 0x3ff;

	vu1ExecMicro(addr);
}

void pcsx2_VU::SetKickCallback(std::function<void(int)> callback)
{
	gKickCallback = callback;
}

void pcsx2_VU::SetItop(unsigned int newItop)
{
	vif1Regs.itops = newItop & 0x3ff;
}
