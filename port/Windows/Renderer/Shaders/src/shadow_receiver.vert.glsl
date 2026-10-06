#version 450

layout(set = 0, binding = 2) readonly buffer ModelBuffer { mat4 modelMatrix[]; } model;
layout(set = 0, binding = 3) readonly buffer AnimBuffer { mat4 animMatrix[]; } anim;
layout(set = 0, binding = 5) readonly buffer AnimStData { vec4 animST[]; } animStData;
layout(set = 0, binding = 6) readonly buffer ShadowProjectionBuffer { mat4 projection[]; } shadow;

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
	uint frameBufferMode;
	float frameBufferScaleX;
	float frameBufferScaleY;
	uint animBaseOffset;
	uint stripFlags; // Authored geometry flags, separate from VU renderFlags.
} perDrawData;

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in uint inFlags;
layout(location = 5) in vec4 inNormal;
layout(location = 0) out vec4 shadowCoord;

void main()
{
	vec4 position = vec4(inPosition, 1.0);
	vec3 extrusionNormal = inNormal.xyz;
	uint animFlags = inFlags & 0x7ff;
	if ((perDrawData.stripFlags & 0x10000u) != 0 && animFlags >= perDrawData.animBaseOffset) {
		uint animIndex = (animFlags - perDrawData.animBaseOffset) / 4;
		mat4 currentAnimMatrix = anim.animMatrix[perDrawData.animMatrixStart + animIndex];
		position = currentAnimMatrix * position;
		if ((perDrawData.stripFlags & 0x8000000u) != 0) {
			extrusionNormal = mat3(currentAnimMatrix) * extrusionNormal;
		}
	}
	if ((perDrawData.renderFlags & 0x100) != 0) {
		position.xyz += extrusionNormal * animStData.animST[perDrawData.animStDataIndex].z;
	}
	gl_Position = perDrawData.projXView * model.modelMatrix[perDrawData.modelMatrixIndex] * position;
	shadowCoord = shadow.projection[perDrawData.shadowProjectionIndex] * position;
}
