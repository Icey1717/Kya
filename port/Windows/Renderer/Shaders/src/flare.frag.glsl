#version 450

layout(set = 0, binding = 0) uniform sampler2D flareTexture;
layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in float visibility;
layout(location = 0) out vec4 outColor;

void main() {
	// Native approximation: visibility modulates the draw instead of modifying GS palette/texture alpha.
	// GS flare tint (0x58, 0x50, 0x40), using GS neutral color 0x80.
	vec3 tint = vec3(88.0, 80.0, 64.0) / 128.0;
	outColor = vec4(texture(flareTexture, texCoord).rgb * tint * visibility, 0.0);
}
