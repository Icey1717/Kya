#version 450
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 2) readonly buffer ModelBuffer { mat4 modelMatrix[]; } model;
layout(set = 0, binding = 3) readonly buffer AnimBuffer { mat4 animMatrix[]; } anim;

#include "environment_mapping.glsl"

// Caster draws reuse the material descriptor set created for the main native
// pipeline, so keep the otherwise-unused bindings layout-compatible.
layout(set = 0, binding = 4) readonly buffer LightingData { LightingDataBlock lightData[]; } lightingBuf;
layout(set = 0, binding = 5) readonly buffer AnimStData { vec4 animST[]; } animStData;

#include "per_draw_data.glsl"

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
	vec3 extrusionNormal = inNormal.xyz;
	uint animFlags = inFlags & 0x7ff;
	uint animBaseOffset = GetAnimationBaseOffset(perDrawData.stripFlags);
	if ((perDrawData.stripFlags & 0x10000u) != 0 && animFlags >= animBaseOffset) {
		uint animIndex = (animFlags - animBaseOffset) / 4;
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
	fragTexCoord = vec2(inST) * 0.000244140625;
	if ((perDrawData.stripFlags & 0x8000000u) != 0 && (perDrawData.renderFlags & 0x40) != 0) {
		fragTexCoord = GetEnvironmentST(lightingBuf.lightData[perDrawData.lightingDataIndex], extrusionNormal);
	}
	if ((perDrawData.renderFlags & 0x200) != 0) {
		fragTexCoord += animStData.animST[perDrawData.animStDataIndex].xy;
	}
	fragAlpha = float(inColor.a) / 255.0;
}
