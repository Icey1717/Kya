#version 450

layout(binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlphaBlend;

void main() {
	vec4 textureColor = texture(textureSampler, fragTexCoord.xy);
    outColor = textureColor; //vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// For dual source blending
	vec4 alpha_blend = vec4(outColor.a / (128.0 / 255.0));
	outAlphaBlend = alpha_blend;
}