#ifndef GS_DEPTH_GLSL
#define GS_DEPTH_GLSL

// Shared AA/fog plane operation. Positive offsets use ALPHA 0x8000000068
// to add to the green byte; negative offsets use 0x8000000062 to subtract.
uint AdjustGsDepthGreenByte(uint z, int offset) {
    int green = clamp(int((z >> 8) & 255u) + offset, 0, 255);
    return (z & 0xffff00ffu) | (uint(green) << 8);
}

#endif
