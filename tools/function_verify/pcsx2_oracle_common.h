// Test boundary for the unmodified upstream FPU.cpp instruction interpreter.
// CPU/FPU register definitions are the actual pinned upstream definitions.
// Unused VM services trap: they are never successful emulation summaries.
#pragma once
#include <algorithm>
#include <cstdlib>
#include "pcsx2/R5900.h"

inline void intDoBranch(u32) { std::abort(); }
inline u32 memRead32(u32) { std::abort(); }
inline void memWrite32(u32, u32) { std::abort(); }
struct OracleConsole {
    template<class... T> void Error(T...) { std::abort(); }
};
inline OracleConsole Console;
