#version 450
#extension GL_GOOGLE_include_directive : require
#include "gs_projection.glsl"
#include "gs_depth.glsl"

layout(set = 0, binding = 0) uniform sampler2D sceneColor;
layout(set = 0, binding = 1) uniform sampler2D reducedColor;
layout(set = 0, binding = 2) uniform sampler2D sceneDepth;
layout(push_constant) uniform AAParams {
    vec4 params; // Mode, submitted fog offset, AA depth offset, FXAA multiplier / PS2 blur radius.
    vec4 depthProjection;
    vec4 viewport;
} aa;
layout(location = 0) out vec4 outColor;

vec2 BoundedUV(vec2 uv, vec2 texel) {
    return clamp(uv, aa.viewport.xy + texel * 0.5,
        max(aa.viewport.xy + texel * 0.5, aa.viewport.xy + aa.viewport.zw - texel * 0.5));
}
vec3 Color(vec2 uv) {
    return textureLod(sceneColor, BoundedUV(uv, 1.0 / vec2(textureSize(sceneColor, 0))), 0.0).rgb;
}
float Luma(vec3 rgb) { return dot(rgb, vec3(0.299, 0.587, 0.114)); }
float LumaAt(vec2 uv) { return Luma(Color(uv)); }

// Native implementation of the local-contrast, edge-search and subpixel steps
// described by Timothy Lottes, NVIDIA FXAA white paper. No temporal history.
vec3 FXAA(vec2 uv, vec2 texel) {
    vec3 center = Color(uv);
    float m = Luma(center);
    float n = LumaAt(uv - vec2(0, texel.y));
    float s = LumaAt(uv + vec2(0, texel.y));
    float w = LumaAt(uv - vec2(texel.x, 0));
    float e = LumaAt(uv + vec2(texel.x, 0));
    float lo = min(m, min(min(n, s), min(w, e)));
    float hi = max(m, max(max(n, s), max(w, e)));
    float range = hi - lo;
    if (range < max(1.0 / 32.0, hi / 8.0)) return center;
    float nw = LumaAt(uv - texel);
    float ne = LumaAt(uv + vec2(texel.x, -texel.y));
    float sw = LumaAt(uv + vec2(-texel.x, texel.y));
    float se = LumaAt(uv + texel);
    float horizontal = abs(nw + sw - 2.0 * w) + 2.0 * abs(n + s - 2.0 * m) + abs(ne + se - 2.0 * e);
    float vertical = abs(nw + ne - 2.0 * n) + 2.0 * abs(w + e - 2.0 * m) + abs(sw + se - 2.0 * s);
    bool alongX = horizontal >= vertical;
    float first = alongX ? n : w;
    float second = alongX ? s : e;
    float gradient1 = abs(first - m), gradient2 = abs(second - m);
    float stepSign = gradient1 >= gradient2 ? -1.0 : 1.0;
    float gradient = max(gradient1, gradient2);
    float pairLuma = 0.5 * (m + (gradient1 >= gradient2 ? first : second));
    vec2 perpendicular = alongX ? vec2(0, texel.y) : vec2(texel.x, 0);
    vec2 along = alongX ? vec2(texel.x, 0) : vec2(0, texel.y);
    vec2 edge = uv + perpendicular * (0.5 * stepSign);
    int searchSteps = 12 * clamp(int(aa.params.w), 1, 4);
    float distance1 = float(searchSteps), distance2 = float(searchSteps);
    float delta1 = 0.0, delta2 = 0.0;
    bool found1 = false, found2 = false;
    for (int i = 1; i <= searchSteps; ++i) {
        if (!found1) {
            delta1 = LumaAt(edge - along * float(i)) - pairLuma;
            distance1 = float(i);
            found1 = abs(delta1) >= gradient * 0.25;
        }
        if (!found2) {
            delta2 = LumaAt(edge + along * float(i)) - pairLuma;
            distance2 = float(i);
            found2 = abs(delta2) >= gradient * 0.25;
        }
        if (found1 && found2) break;
    }
    float nearestDelta = distance1 < distance2 ? delta1 : delta2;
    float edgeBlend = ((nearestDelta < 0.0) != (m - pairLuma < 0.0))
        ? 0.5 - min(distance1, distance2) / (distance1 + distance2) : 0.0;
    float average = (2.0 * (n + s + w + e) + nw + ne + sw + se) / 12.0;
    float subpixel = clamp(abs(average - m) / range, 0.0, 1.0);
    subpixel = subpixel * subpixel * (3.0 - 2.0 * subpixel);
    subpixel = subpixel * subpixel * 0.75;
    return Color(uv + perpendicular * (max(edgeBlend, subpixel) * stepSign));
}

void main() {
    vec2 texel = 1.0 / vec2(textureSize(sceneColor, 0));
    vec2 uv = gl_FragCoord.xy * texel;
    vec3 result = Color(uv);
    if (int(aa.params.x) == 2) {
        result = FXAA(uv, texel);
    } else if (int(aa.params.x) == 1) {
        float depth = texelFetch(sceneDepth, ivec2(gl_FragCoord.xy), 0).r;
        vec4 p = aa.depthProjection;
        uint z = depth == 0.0 ? 0u : uint(clamp(GsDepthFromNative(p, depth), 0.0, 16777215.0));
        z = AdjustGsDepthGreenByte(AdjustGsDepthGreenByte(z, -int(aa.params.y)), int(aa.params.z));
        if (z <= 0x7fffu) {
            vec2 reducedTexel = 1.0 / vec2(textureSize(reducedColor, 0));
            vec3 blurred = textureLod(reducedColor, BoundedUV(uv, reducedTexel), 0.0).rgb;
            // Original default: eight ring draws, radius one, FIX=floor(128/(i+2)).
            // Snapshots replace the GS's overlapping in-place texture feedback.
            const vec2 offsets[8] = vec2[8](vec2(1,0), vec2(0), vec2(0,1), vec2(0),
                vec2(-1,0), vec2(0), vec2(0,-1), vec2(0));
            for (int i = 0; i < 8; ++i) {
                vec3 sampleColor = textureLod(reducedColor,
                    BoundedUV(uv + offsets[i] * reducedTexel * aa.params.w, reducedTexel), 0.0).rgb;
                blurred = mix(blurred, sampleColor, float(128 / (i + 2)) / 128.0);
            }
            float visibility = float((z >> 8) & 127u) / 128.0;
            result = mix(blurred, result, visibility);
        }
    }
    outColor = vec4(result, 1.0); // Pipeline writes RGB only; GS alpha remains available to later effects.
}
