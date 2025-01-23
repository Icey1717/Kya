#version 450

layout(binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 textureColor = texture(textureSampler, fragTexCoord.xy);
    outColor = textureColor; //vec4(1.0f, 1.0f, 1.0f, 1.0f);
}