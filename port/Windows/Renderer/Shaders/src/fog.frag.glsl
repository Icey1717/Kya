#version 450
#extension GL_GOOGLE_include_directive : require
#include "gs_projection.glsl"
#include "gs_depth.glsl"

layout(set = 0, binding = 0) uniform sampler2D sceneDepth;
layout(push_constant) uniform FogParams {
    vec4 color;
    vec4 params; // GS near, far, signed green-byte offset, flags.
    vec4 depthProjection; // GS depth = (a * nativeDepth + b) / (c * nativeDepth + d).
} fog;
layout(location = 0, index = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 fogWeight;

void main() {
    float depth = texelFetch(sceneDepth, ivec2(gl_FragCoord.xy), 0).r;
    vec4 p = fog.depthProjection;
    float gsDepth = GsDepthFromNative(p, depth);
    uint z = depth == 0.0 ? 0u : uint(clamp(gsDepth, 0.0, 16777215.0));
    int offset = int(fog.params.z);
    uint flags = uint(fog.params.w);
    uint shifted = AdjustGsDepthGreenByte(z, offset);
    float visibility = float((shifted >> 8) & 127u) / 128.0;
    bool applyColor = (flags & 4u) == 0u && shifted <= 0x7fffu;
    // GS ALPHA=(Cd-Cs)*Ad/128+Cs. Dual-source alpha keeps the
    // RGB fog weight independent of the alpha byte written by the sprite.
    outColor = vec4(fog.color.rgb, applyColor ? fog.color.a : visibility * (128.0 / 255.0));
    fogWeight = vec4(applyColor ? 1.0 - visibility : 0.0);
    // The original trailing plane repeats the same signed adjustment.
    uint finalZ = (flags & 32u) != 0u ? AdjustGsDepthGreenByte(shifted, offset) : shifted;
    gl_FragDepth = finalZ == z ? depth : clamp(NativeDepthFromGs(p, float(finalZ)), 0.0, 1.0);
}
