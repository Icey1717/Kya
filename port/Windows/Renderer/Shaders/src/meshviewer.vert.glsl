#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 animMatrices[0x18];
} ubo;

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec4 inPosition;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragTexCoord;

void main() {
	vec4 fixedPos = vec4(inPosition.xyz, 1.0);
    vec4 pos = ubo.proj * ubo.view * ubo.model * fixedPos;
    gl_Position = pos;
    fragColor.x = inColor.x / 255.0;
    fragColor.y = inColor.y / 255.0;
    fragColor.z = inColor.z / 255.0;
    fragColor.w = inColor.w / 255.0;
    fragTexCoord = vec4(vec2(inST), inQ);
}