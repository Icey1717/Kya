/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Pcsx2Defs.h"

static const u32 BIAS = 2;				// Bus is half of the actual ps2 speed
static const u32 PS2CLK = 294912000;	//hz	/* 294.912 mhz */
extern s64 PSXCLK;	/* 36.864 Mhz */


//#include "System.h"
//#include "Memory.h"
//#include "R5900.h"
//#include "Hw.h"
//#include "Dmac.h"
//
//#include "SaveState.h"
//#include "DebugTools/Debug.h"

#include <string>
#include "log.h"

extern std::string ShiftJIS_ConvertString(const char* src);
extern std::string ShiftJIS_ConvertString(const char* src, int maxlen);

typedef void FnType_Void();
typedef FnType_Void* Fnptr_Void;

//#define VUM_LOG(fmt, ...)
#define VUM_LOG(fmt, ...) Log::GetInstance().AddLog(LogLevel::Info, "VU", fmt, __VA_ARGS__)
#define CPU_LOG(...)

//extern char* disVU0MicroUF(u32 code, u32 pc);
//extern char* disVU0MicroLF(u32 code, u32 pc);
extern char* disVU1MicroUF(u32 code, u32 pc);
extern char* disVU1MicroLF(u32 code, u32 pc);

struct DevConObj {
	bool WriteLn(const std::string& str) const { return true; }
	bool Error(const std::string& str) const;
	bool Warning(const std::string& str) const;
};

extern DevConObj DevCon;
extern DevConObj DbgCon;

struct VIFregisters {
	//tVIF_STAT stat;
	u32 _pad0[3];
	u32 fbrst;
	u32 _pad1[3];
	//tVIF_ERR err;
	u32 _pad2[3];
	u32 mark;
	u32 _pad3[3];
	//vifCycle cycle; //data write cycle
	u32 _pad4[3];
	u32 mode;
	u32 _pad5[3];
	u32 num;
	u32 _pad6[3];
	u32 mask;
	u32 _pad7[3];
	u32 code;
	u32 _pad8[3];
	u32 itops;
	u32 _pad9[3];
	u32 base;      // Not used in VIF0
	u32 _pad10[3];
	u32 ofst;      // Not used in VIF0
	u32 _pad11[3];
	u32 tops;      // Not used in VIF0
	u32 _pad12[3];
	u32 itop;
	u32 _pad13[3];
	u32 top;       // Not used in VIF0
	u32 _pad14[3];
	u32 mskpath3;
	u32 _pad15[3];
	u32 r0;        // row0 register
	u32 _pad16[3];
	u32 r1;        // row1 register
	u32 _pad17[3];
	u32 r2;        // row2 register
	u32 _pad18[3];
	u32 r3;        // row3 register
	u32 _pad19[3];
	u32 c0;        // col0 register
	u32 _pad20[3];
	u32 c1;        // col1 register
	u32 _pad21[3];
	u32 c2;        // col2 register
	u32 _pad22[3];
	u32 c3;        // col3 register
	u32 _pad23[3];
	u32 offset;    // internal UNPACK offset
	u32 addr;
};

extern VIFregisters vif0Regs;
extern VIFregisters vif1Regs;

#define pxFailDev(...)

#define CHECK_VU_OVERFLOW(...) false
#define CHECK_VUADDSUBHACK false
#define THREAD_VU1 false
#define INSTANT_VU1 true

inline u32 EmulateCycle() {
	//assert(false);
	return 0;
}

inline void hwIntcIrq(int) {
	assert(false);
};

enum INTCIrqs
{
	INTC_GS = 0,
	INTC_SBUS,
	INTC_VBLANK_S,
	INTC_VBLANK_E,
	INTC_VIF0,
	INTC_VIF1,
	INTC_VU0,
	INTC_VU1,
	INTC_IPU,
	INTC_TIM0,
	INTC_TIM1,
	INTC_TIM2,
	INTC_TIM3,
	INTC_SFIFO,
	INTVU0_WD
};

namespace R5900
{
	void disR5900Fasm(std::string& output, u32 code, u32 pc, bool simplify = false);

	extern const char* const GPR_REG[32];
	extern const char* const COP0_REG[32];
	extern const char* const COP1_REG_FP[32];
	extern const char* const COP1_REG_FCR[32];
	extern const char* const COP2_REG_FP[32];
	extern const char* const COP2_REG_CTL[32];
	extern const char* const COP2_VFnames[4];
	extern const char* const GS_REG_PRIV[19];
	extern const u32 GS_REG_PRIV_ADDR[19];
}