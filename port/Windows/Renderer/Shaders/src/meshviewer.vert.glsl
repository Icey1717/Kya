#version 450

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

	uint animIndex = anim - 0x3dc;
    animIndex = animIndex / 4;

	uint intoAnimMatrices = ubo.animStripToIndex[stripIndex][animIndex];

	mat4 animMatrix = ubo.animMatrices[intoAnimMatrices];

	vec4 fixedPos = vec4(inPosition, 1.0);
    vec4 pos = ubo.proj * ubo.view * ubo.model * animMatrix * fixedPos;
    gl_Position = pos;
    fragColor.x = inColor.x / 255.0;
    fragColor.y = inColor.y / 255.0;
    fragColor.z = inColor.z / 255.0;
    fragColor.w = inColor.w / 255.0;

	vec2 outST = vec2(int12_to_float(inST.x), int12_to_float(inST.y));

    fragTexCoord = vec4(outST, inQ);
}