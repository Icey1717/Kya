// Independent concrete implementation of the pinned corrected-add/sub model.
// This tests our symbolic encoding; it is not a PS2 hardware oracle or a build
// of PCSX2. See ee_reference.py for the reference revision and limitations.
#include <immintrin.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static uint32_t exponent(uint32_t bits) { return (bits >> 23) & 255; }
static double decode(uint32_t bits)
{
    uint64_t wide = static_cast<uint64_t>(bits & 0x80000000u) << 32;
    if (exponent(bits)) wide |= (static_cast<uint64_t>(exponent(bits) + 896) << 52)
        | (static_cast<uint64_t>(bits & 0x7fffff) << 29);
    double value;
    std::memcpy(&value, &wide, sizeof(value));
    return value;
}

int main(int argc, char** argv)
{
    if (argc != 5) return 2;
    uint32_t a = static_cast<uint32_t>(std::strtoul(argv[2], nullptr, 16));
    uint32_t b = static_cast<uint32_t>(std::strtoul(argv[3], nullptr, 16));
    uint32_t control = static_cast<uint32_t>(std::strtoul(argv[4], nullptr, 16));
    const bool subtract = std::strcmp(argv[1], "SUB.S") == 0;
    if (!subtract && std::strcmp(argv[1], "ADD.S") != 0) return 2;
    if (!exponent(a)) a &= 0x80000000;
    if (!exponent(b)) b &= 0x80000000;
    const int distance = static_cast<int>(exponent(a)) - static_cast<int>(exponent(b));
    if (distance >= 25) b &= 0x80000000;
    else if (distance > 0) b &= 0xffffffffu << (distance - 1);
    else if (distance <= -25) a &= 0x80000000;
    else if (distance < 0) a &= 0xffffffffu << (-distance - 1);
    const __m128d av = _mm_set_sd(decode(a)), bv = _mm_set_sd(decode(b));
    const auto original = _mm_getcsr();
    _mm_setcsr(0x7f80); // RTZ; no host denormal flushing; all exceptions masked.
    const double result = _mm_cvtsd_f64(subtract ? _mm_sub_sd(av, bv) : _mm_add_sd(av, bv));
    _mm_setcsr(original);
    uint64_t raw;
    std::memcpy(&raw, &result, sizeof(raw));
    const uint32_t sign = static_cast<uint32_t>(raw >> 32) & 0x80000000;
    const uint32_t exp = static_cast<uint32_t>((raw >> 52) & 2047);
    const uint32_t fraction = static_cast<uint32_t>(raw >> 29) & 0x7fffff;
    uint32_t value = sign;
    control &= ~0xc000;
    if (exp >= 1152) { value |= 0x7fffffff; control |= 0x8010; }
    else if ((raw & 0x7fffffffffffffffULL) && exp < 897) { value |= fraction; control |= 0x4008; }
    else if (exp) value |= ((exp - 896) << 23) | fraction;
    std::printf("{\"bits\":%u,\"fcr31\":%u}\n", value, control);
    return 0;
}
