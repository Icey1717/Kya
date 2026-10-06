#version 450
#extension GL_GOOGLE_include_directive : require

layout(binding = 1) uniform sampler2D shadowMask;
layout(location = 0) in vec4 shadowCoord;
layout(location = 0) out vec4 outColor;

#include "per_draw_data.glsl"

void main()
{
	// ed3DShadowManageProjectionSTMtx supplies PS2 STQ: Q is -camera Z in .z.
	if (shadowCoord.z <= 0.0) discard;
	vec2 uv = shadowCoord.xy / shadowCoord.z;
	if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) discard;
	float coverage = texture(shadowMask, uv).r;
	float opacity = coverage * min(float(perDrawData.globalAlpha & 0xffu), 128.0) / 128.0;
	if (opacity <= 0.0) discard;
	outColor = vec4(0.0, 0.0, 0.0, opacity);
}
