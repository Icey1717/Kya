#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlphaBlend;

void main() {
    outColor = fragColor;

	// For dual source blending
	vec4 alpha_blend = vec4(outColor.a / (128.0 / 255.0));
	outAlphaBlend = alpha_blend;
}