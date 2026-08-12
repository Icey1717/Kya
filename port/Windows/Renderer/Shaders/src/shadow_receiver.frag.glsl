#version 450

layout(binding = 1) uniform sampler2D shadowMask;
layout(location = 0) in vec4 shadowCoord;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PerDrawData
{
	mat4 projXView;
	uint renderFlags;
	uint alphaEnable;
	int alphaAtst;
	int alphaAref;
	int alphaAfail;
	uint modelMatrixIndex;
	uint animStDataIndex;
	uint animMatrixStart;
	uint lightingDataIndex;
	uint globalAlpha;
	uint shadowProjectionIndex;
	uint _pad[5];
} perDrawData;

void main()
{
	if (shadowCoord.w <= 0.0) discard;
	vec2 uv = shadowCoord.xy / shadowCoord.w;
	if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) discard;
	float coverage = texture(shadowMask, uv).r;
	float opacity = coverage * min(float(perDrawData.globalAlpha), 128.0) / 128.0;
	if (opacity <= 0.0) discard;
	outColor = vec4(0.0, 0.0, 0.0, opacity);
}
