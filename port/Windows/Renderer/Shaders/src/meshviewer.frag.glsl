#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragTexCoord;

layout(location = 0) out vec4 outColor;

// Texture sampler
layout(binding = 1) uniform sampler2D textureSampler;

void main() {
	

	// Sample texture color using fragTexCoord
	vec4 textureColor = texture(textureSampler, fragTexCoord.xy);

	// Combine texture color with fragment color
	outColor = fragColor * textureColor;

	outColor.a = 1.0;
}