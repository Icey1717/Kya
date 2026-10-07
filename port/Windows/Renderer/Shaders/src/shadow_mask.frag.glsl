#version 450
#extension GL_GOOGLE_include_directive : require
#include "texture_sampling.glsl"

layout(set = 1, binding = 0) uniform sampler2D textureSampler;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragAlpha;
layout(location = 0) out float outCoverage;

#include "per_draw_data.glsl"

bool AlphaTestPass(float alpha)
{
	if (perDrawData.alphaEnable == 0) return true;
	int value = int(alpha * 255.0);
	switch (perDrawData.alphaAtst) {
	case 0: return false;
	case 1: return true;
	case 2: return value < perDrawData.alphaAref;
	case 3: return value <= perDrawData.alphaAref;
	case 4: return value == perDrawData.alphaAref;
	case 5: return value >= perDrawData.alphaAref;
	case 6: return value > perDrawData.alphaAref;
	case 7: return value != perDrawData.alphaAref;
	}
	return true;
}

void main()
{
	float alpha = SampleMaterialTexture(textureSampler, fragTexCoord, perDrawData.globalAlpha, perDrawData.samplingParams).a * fragAlpha;
	if (!AlphaTestPass(alpha)) discard;
	outCoverage = 1.0;
}
