#version 450
#extension GL_GOOGLE_include_directive : require
#include "per_draw_data.glsl"

float int12_to_float(int x) {
	return float(x) * 0.000244140625;
}

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 animMatrices[0x60];
	uint animStripToIndex[0x20][0x20];
} ubo;

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in uint inFlags;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragTexCoord;

void main() {
	// _$XYZW_16_Conv_EndBones_Rigid
	uint baseFlags = inFlags & 0xFFFF;

	uint stripIndex = inFlags >> 16;

	uint anim = baseFlags & 0x7ff;
	uint flags = baseFlags & 0xc000;

	vec4 fixedPos = vec4(inPosition, 1.0);

	uint animBase = GetAnimationBaseOffset(perDrawData.stripFlags);
	if ((perDrawData.stripFlags & 0x10000u) != 0 && anim >= animBase && stripIndex < 0x20u) {
		uint animIndex = anim - animBase;
		animIndex = animIndex / 4;
		uint intoAnimMatrices = animIndex < 0x20u ? ubo.animStripToIndex[stripIndex][animIndex] : 0u;
		intoAnimMatrices = min(intoAnimMatrices, 0x5fu);

		mat4 animMatrix = ubo.animMatrices[intoAnimMatrices];

		vec4 pos = ubo.proj * ubo.view * ubo.model * animMatrix * fixedPos;
		gl_Position = pos;
	}
	else {
		vec4 pos = ubo.proj * ubo.view * ubo.model * fixedPos;
		gl_Position = pos;
	}

	fragColor.x = inColor.x / 255.0;
	fragColor.y = inColor.y / 255.0;
	fragColor.z = inColor.z / 255.0;
	fragColor.w = inColor.w / 255.0;
	if ((perDrawData.renderFlags & 0x20u) != 0) {
		fragColor.a = floor(float(inColor.a) * float(perDrawData.globalAlpha & 0xffu) / 128.0) / 255.0;
	}

	vec2 outST = vec2(int12_to_float(inST.x), int12_to_float(inST.y));

	fragTexCoord = vec4(outST, inQ);
}
