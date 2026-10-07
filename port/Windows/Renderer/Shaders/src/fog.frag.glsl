#version 450

layout(set = 0, binding = 0) uniform sampler2D sceneDepth;
layout(push_constant) uniform FogParams {
    vec4 color;
    vec4 params; // GS near, far, signed green-byte offset, flags.
    vec4 depthProjection; // GS depth = (a * nativeDepth + b) / (c * nativeDepth + d).
} fog;
layout(location = 0, index = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 fogWeight;

uint ShiftGreen(uint z, int offset) {
    // ALPHA 0x8000000068 adds positive offsets; 0x8000000062
    // subtracts their magnitude for negative offsets (FIX=128).
    int green = clamp(int((z >> 8) & 255u) + offset, 0, 255);
    return (z & 0xffff00ffu) | (uint(green) << 8);
}

void main() {
    float depth = texelFetch(sceneDepth, ivec2(gl_FragCoord.xy), 0).r;
    vec4 p = fog.depthProjection;
    float gsDepth = (p.x * depth + p.y) / (p.z * depth + p.w);
    uint z = depth == 0.0 ? 0u : uint(clamp(gsDepth, 0.0, 16777215.0));
    int offset = int(fog.params.z);
    uint flags = uint(fog.params.w);
    uint shifted = ShiftGreen(z, offset);
    float visibility = float((shifted >> 8) & 127u) / 128.0;
    bool applyColor = (flags & 4u) == 0u && shifted <= 0x7fffu;
    // GS ALPHA=(Cd-Cs)*Ad/128+Cs. Dual-source alpha keeps the
    // RGB fog weight independent of the alpha byte written by the sprite.
    outColor = vec4(fog.color.rgb, applyColor ? fog.color.a : visibility * (128.0 / 255.0));
    fogWeight = vec4(applyColor ? 1.0 - visibility : 0.0);
    // The original trailing plane repeats the same signed adjustment.
    uint finalZ = (flags & 32u) != 0u ? ShiftGreen(shifted, offset) : shifted;
    gl_FragDepth = finalZ == z ? depth : clamp((p.y - float(finalZ) * p.w) / (float(finalZ) * p.z - p.x), 0.0, 1.0);
}
