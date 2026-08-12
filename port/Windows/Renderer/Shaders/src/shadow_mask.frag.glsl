#version 450

layout(binding = 1) uniform sampler2D textureSampler;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragAlpha;
layout(location = 0) out float outCoverage;

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
	float alpha = texture(textureSampler, fragTexCoord).a * fragAlpha;
	if (!AlphaTestPass(alpha)) discard;
	outCoverage = 1.0;
}
