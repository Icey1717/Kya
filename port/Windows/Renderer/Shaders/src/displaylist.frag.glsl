#version 450

layout(binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlphaBlend;

layout(push_constant) uniform BlendState
{
	int  blendMode;
	uint alphaEnable;
	int  alphaAtst;
	int  alphaAref;
	int  alphaAfail;
} blendState;

#define ATST_NEVER 0
#define ATST_ALWAYS 1
#define ATST_LESS 2
#define ATST_LEQUAL 3
#define ATST_EQUAL 4
#define ATST_GEQUAL 5
#define ATST_GREATER 6
#define ATST_NOTEQUAL 7

bool atst(vec4 color)
{
	if (bool(blendState.alphaEnable)) {
		int a = int(color.a * 255.0);

		if (blendState.alphaAtst == ATST_GEQUAL) {
			if (a < blendState.alphaAref) {
				return false;
			}
		}
		else if (blendState.alphaAtst == ATST_GREATER) {
			if (a <= blendState.alphaAref) {
				return false;
			}
		}
		else if (blendState.alphaAtst == ATST_EQUAL) {
			if (a != blendState.alphaAref) {
				return false;
			}
		}
		else if (blendState.alphaAtst == ATST_NOTEQUAL) {
			if (a == blendState.alphaAref) {
				return false;
			}
		}
		else if (blendState.alphaAtst == ATST_LESS) {
			if (a >= blendState.alphaAref) {
				return false;
			}
		}
		else if (blendState.alphaAtst == ATST_LEQUAL) {
			if (a > blendState.alphaAref) {
				return false;
			}
		}
		else if (blendState.alphaAtst == ATST_NEVER) {
			return false;
		}
	}

	return true;
}

void afail()
{
	switch (blendState.alphaAfail) {
		case 0:
			discard;

		case 2:
			outColor.a = 0.0;
		break;

		case 1:
		case 3:
		break;
	}
}

void main() {
	vec4 textureColor = texture(textureSampler, fragTexCoord.xy);
	outColor = textureColor * fragColor;

	if (!atst(outColor)) {
		afail();
	}

	// For dual source blending
	vec4 alpha_blend = vec4(outColor.a / (128.0 / 255.0));
	outAlphaBlend = alpha_blend;
}