#version 450

float int12_to_float(int x) {
	return float(x) * 0.000244140625;
}

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
} ubo;

layout(set = 0, binding = 2) uniform ModelBuffer {
	mat4 modelMatrix;
} model;

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in uint inFlags;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragTexCoord;

void main() {
	// _$XYZW_16_Conv_EndBones_Rigid

	vec4 fixedPos = vec4(inPosition, 1.0);
	vec4 pos = ubo.proj * ubo.view * model.modelMatrix * fixedPos;

	pos.y = -pos.y;
	gl_Position = pos;

	fragColor.x = inColor.x / 255.0;
	fragColor.y = inColor.y / 255.0;
	fragColor.z = inColor.z / 255.0;
	fragColor.w = inColor.w / 255.0;

	vec2 outST = vec2(int12_to_float(inST.x), int12_to_float(inST.y));

	fragTexCoord = vec4(outST, inQ);
}