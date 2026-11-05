// Executes actual pinned PCSX2 COP1 interpreter handlers, not reimplementations.
#include "Common.h"
#include <cstdio>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <sstream>
#include <string>

alignas(16) cpuRegistersPack _cpuRegistersPack{};
namespace R5900::Interpreter::OpcodeImpl::COP1 {
void ADD_S(); void SUB_S(); void MUL_S(); void DIV_S(); void SQRT_S();
void RSQRT_S(); void ABS_S(); void NEG_S(); void MOV_S();
void ADDA_S(); void SUBA_S(); void MULA_S(); void MADD_S(); void MSUB_S();
void MADDA_S(); void MSUBA_S(); void MIN_S(); void MAX_S();
void C_EQ(); void C_LT(); void C_LE(); void CVT_S(); void CVT_W();
}

int main()
{
    using namespace R5900::Interpreter::OpcodeImpl::COP1;
    struct Handler { const char* name; void (*function)(); };
    const Handler handlers[] = {
        {"ADD.S", ADD_S}, {"SUB.S", SUB_S}, {"MUL.S", MUL_S},
        {"DIV.S", DIV_S}, {"SQRT.S", SQRT_S}, {"RSQRT.S", RSQRT_S},
        {"ABS.S", ABS_S}, {"NEG.S", NEG_S}, {"MOV.S", MOV_S},
        {"ADDA.S", ADDA_S}, {"SUBA.S", SUBA_S}, {"MULA.S", MULA_S},
        {"MADD.S", MADD_S}, {"MSUB.S", MSUB_S}, {"MADDA.S", MADDA_S}, {"MSUBA.S", MSUBA_S},
        {"MIN.S", MIN_S}, {"MAX.S", MAX_S}, {"C.EQ.S", C_EQ}, {"C.LT.S", C_LT}, {"C.LE.S", C_LE},
        {"CVT.S.W", CVT_S}, {"CVT.W.S", CVT_W}
    };
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        std::string opcode, extra;
        u32 a, b, acc, control, mxcsr;
        if (!(input >> opcode >> std::hex >> a >> b >> acc >> control >> mxcsr) || input >> extra
            || (mxcsr & 0xffff0000) || (mxcsr & 0x1f80) != 0x1f80) return 2;
        auto handler = std::find_if(std::begin(handlers), std::end(handlers),
            [&](const Handler& h) { return opcode == h.name; });
        if (handler == std::end(handlers)) return 3;
        std::memset(&_cpuRegistersPack, 0, sizeof(_cpuRegistersPack));
        cpuRegs.code = (1u << 11) | (2u << 16) | (3u << 6);
        fpuRegs.fpr[1].UL = a;
        fpuRegs.fpr[2].UL = b;
        fpuRegs.fpr[3].UL = 0xdeadbeef;
        fpuRegs.ACC.UL = acc;
        fpuRegs.fprc[31] = control;
        _mm_setcsr(mxcsr);
        handler->function();
        const u32 observedMxcsr = _mm_getcsr();
        _mm_setcsr(0x1f80);
        std::printf("%08x %08x %08x %08x\n", fpuRegs.fpr[3].UL, fpuRegs.ACC.UL,
            fpuRegs.fprc[31], observedMxcsr);
    }
    return 0;
}
