#version 450

layout(set = 0, binding = 2) readonly buffer ModelBuffer { mat4 modelMatrix[]; } model;
layout(set = 0, binding = 3) readonly buffer AnimBuffer { mat4 animMatrix[]; } anim;

struct LightingDataBlock {
	mat4 lightDirection;
	mat4 lightColor;
	vec4 lightAmbient;
	vec4 flare;
};

// Caster draws reuse the material descriptor set created for the main native
// pipeline, so keep the otherwise-unused bindings layout-compatible.
layout(set = 0, binding = 4) readonly buffer LightingData { LightingDataBlock lightData[]; } lightingBuf;
layout(set = 0, binding = 5) readonly buffer AnimStData { vec4 animST[]; } animStData;

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

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in uint inFlags;
layout(location = 5) in vec4 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragAlpha;

void main()
{
	vec4 position = vec4(inPosition, 1.0);
	uint animFlags = inFlags & 0x7ff;
	if (animFlags > 0) {
		uint animIndex = (animFlags - 0x3dc) / 4;
		position = anim.animMatrix[perDrawData.animMatrixStart + animIndex] * position;
	}
	gl_Position = perDrawData.projXView * model.modelMatrix[perDrawData.modelMatrixIndex] * position;
	fragTexCoord = vec2(inST) * 0.000244140625;
	fragAlpha = float(inColor.a) / 255.0;
}
