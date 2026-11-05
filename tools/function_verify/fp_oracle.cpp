// Independent native SSE oracle. Build with the verification clang command's
// compiler in a Visual Studio x64 environment; never link into the game.
#include <immintrin.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv)
{
    if (argc != 5) return 2;
    const auto a = static_cast<uint32_t>(std::strtoul(argv[2], nullptr, 16));
    const auto b = static_cast<uint32_t>(std::strtoul(argv[3], nullptr, 16));
    const auto control = static_cast<uint32_t>(std::strtoul(argv[4], nullptr, 16));
    // Oracle callers must mask exceptions. A test must not fault the host.
    if ((control & 0x1f80) != 0x1f80 || (control & 0xffff0000)) return 2;
    __m128 av = _mm_castsi128_ps(_mm_cvtsi32_si128(a));
    __m128 bv = _mm_castsi128_ps(_mm_cvtsi32_si128(b));
    const auto original = _mm_getcsr();
    uint32_t value = 0;
    uint64_t flags = 0;
    _mm_setcsr(control);
    if (std::strcmp(argv[1], "ADDSS") == 0) av = _mm_add_ss(av, bv);
    else if (std::strcmp(argv[1], "CVTSI2SS") == 0) av = _mm_cvtsi32_ss(av, static_cast<int32_t>(a));
    else if (std::strcmp(argv[1], "SUBSS") == 0) av = _mm_sub_ss(av, bv);
    else if (std::strcmp(argv[1], "MULSS") == 0) av = _mm_mul_ss(av, bv);
    else if (std::strcmp(argv[1], "DIVSS") == 0) av = _mm_div_ss(av, bv);
    else if (std::strcmp(argv[1], "UCOMISS") == 0) {
        __asm__ volatile("ucomiss %2, %1; pushfq; popq %0" : "=r"(flags) : "x"(av), "x"(bv) : "cc");
    }
    else if (std::strcmp(argv[1], "COMISS") == 0) {
        __asm__ volatile("comiss %2, %1; pushfq; popq %0" : "=r"(flags) : "x"(av), "x"(bv) : "cc");
    }
    else { _mm_setcsr(original); return 2; }
    value = static_cast<uint32_t>(_mm_cvtsi128_si32(_mm_castps_si128(av)));
    const auto status = _mm_getcsr();
    _mm_setcsr(original);
    std::printf("{\"bits\":%u,\"mxcsr\":%u,\"flags\":%llu}\n", value, status,
        static_cast<unsigned long long>(flags & 0x8d5));
    return 0;
}
