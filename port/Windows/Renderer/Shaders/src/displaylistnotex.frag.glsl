#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlphaBlend;

//push constants block
layout( push_constant ) uniform BlendState
{
	int blendMode;
} blendState;

void main() {
	outColor = fragColor;

	if (blendState.blendMode == 1) {
		outColor.rgb = vec3(1.0);
	}

	// For dual source blending
	vec4 alpha_blend = vec4(outColor.a);
	outAlphaBlend = alpha_blend;
}