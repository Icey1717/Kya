#include "Common.h"
#include "VUmicro.h"

VIFregisters vif0Regs;
VIFregisters vif1Regs;

DevConObj DevCon;
DevConObj DbgCon;

BaseVUmicroCPU* CpuVU0 = &CpuIntVU0;
BaseVUmicroCPU* CpuVU1 = &CpuIntVU1;

namespace R5900
{
	/*************************CPUS REGISTERS**************************/
	const char* const GPR_REG[32] = {
		"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
		"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
		"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
		"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
	};

	const char* const COP0_REG[32] = {
		"Index","Random","EntryLo0","EntryLo1","Context","PageMask",
		"Wired","C0r7","BadVaddr","Count","EntryHi","Compare","Status",
		"Cause","EPC","PRId","Config","C0r17","C0r18","C0r19","C0r20",
		"C0r21","C0r22","C0r23","Debug","Perf","C0r26","C0r27","TagLo",
		"TagHi","ErrorPC","C0r31"
	};

	//floating point cop1 Floating point reg
	const char* const COP1_REG_FP[32] = {
		"f00","f01","f02","f03","f04","f05","f06","f07",
		"f08","f09","f10","f11","f12","f13","f14","f15",
		"f16","f17","f18","f19","f20","f21","f22","f23",
		"f24","f25","f26","f27","f28","f29","f30","f31"
	};

	//floating point cop1 control registers
	const char* const COP1_REG_FCR[32] = {
		"fcr00","fcr01","fcr02","fcr03","fcr04","fcr05","fcr06","fcr07",
		"fcr08","fcr09","fcr10","fcr11","fcr12","fcr13","fcr14","fcr15",
		"fcr16","fcr17","fcr18","fcr19","fcr20","fcr21","fcr22","fcr23",
		"fcr24","fcr25","fcr26","fcr27","fcr28","fcr29","fcr30","fcr31"
	};

	//floating point cop2 reg
	const char* const COP2_REG_FP[32] = {
		"vf00","vf01","vf02","vf03","vf04","vf05","vf06","vf07",
		"vf08","vf09","vf10","vf11","vf12","vf13","vf14","vf15",
		"vf16","vf17","vf18","vf19","vf20","vf21","vf22","vf23",
		"vf24","vf25","vf26","vf27","vf28","vf29","vf30","vf31"
	};

	//cop2 control registers
	const char* const COP2_REG_CTL[32] = {
		"vi00","vi01","vi02","vi03","vi04","vi05","vi06","vi07",
		"vi08","vi09","vi10","vi11","vi12","vi13","vi14","vi15",
		"Status","MACflag","ClipFlag","c2c19","R","I","Q","c2c23",
		"c2c24","c2c25","TPC","CMSAR0","FBRST","VPU-STAT","c2c30","CMSAR1"
	};

	const char* const COP2_VFnames[4] = { "x", "y", "z", "w" };

	//gs privileged registers
	const char* const GS_REG_PRIV[19] = {
		"PMODE","SMODE1","SMODE2","SRFSH","SYNCH1","SYNCH2","SYNCV",
		"DISPFB1","DISPLAY1","DISPFB2","DISPLAY2","EXTBUF","EXTDATA",
		"EXTWRITE","BGCOLOR","CSR","IMR","BUSDIR","SIGLBLID",
	};

	//gs privileged register addresses relative to 12000000h
	const u32 GS_REG_PRIV_ADDR[19] = {
		0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,
		0xa0,0xb0,0xc0,0xd0,0xE0,0x1000,0x1010,0x1040,0x1080
	};
}

bool DevConObj::Error(const std::string& str) const
{
	assert(false);
	return true;
}

bool DevConObj::Warning(const std::string& str) const
{
	assert(false);
	return true;
}
