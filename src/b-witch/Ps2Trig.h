#ifndef KYA_PS2_TRIG_H
#define KYA_PS2_TRIG_H

// Parse the standard declarations before replacing the game library's calls.
#include <cmath>

extern const float edFCosinus[8192];

// SLES_514.73: scale bits 0x44a2f983; pi/2 bits 0x3fc90fdb.
// Each argument is evaluated once. Preserve float rounding before the index cast.
#undef cosf
#define cosf(angle) (edFCosinus[static_cast<int>(std::fabs(static_cast<float>(angle) * 0x1.45f306p+10f) + 0.5f) & 0x1fff])
#undef sinf
#define sinf(angle) (edFCosinus[static_cast<int>(std::fabs((static_cast<float>(angle) - 0x1.921fb6p+0f) * 0x1.45f306p+10f) + 0.5f) & 0x1fff])

#endif
