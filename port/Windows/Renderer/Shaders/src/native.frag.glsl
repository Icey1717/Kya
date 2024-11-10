#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragTexCoord;

layout(location = 0, index = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlphaBlend;

// Texture sampler
layout(binding = 1) uniform sampler2D textureSampler;

layout(binding = 4) uniform Test 
{
	bool enable;
	int atst;
	int aref;
} test;

#define ATST_NEVER 0
#define ATST_ALWAYS 1
#define ATST_LESS 2
#define ATST_LEQUAL 3
#define ATST_EQUAL 4
#define ATST_GEQUAL 5
#define ATST_GREATER 6
#define ATST_NOTEQUAL 7

bool atst(vec4 outColor)
{
	if (test.enable) {
		if (test.atst == ATST_GEQUAL) {
			int a = int(outColor.a * 255);

			if (a < test.aref) {
				return false;
			}
		}
		else if (test.atst == ATST_GREATER) {
			int a = int(outColor.a * 255);

			if (a <= test.aref) {
				return false;
			}
		}
		else if (test.atst == ATST_EQUAL) {
			int a = int(outColor.a * 255);

			if (a != test.aref) {
				return false;
			}
		}
		else if (test.atst == ATST_NOTEQUAL) {
			int a = int(outColor.a * 255);

			if (a == test.aref) {
				return false;
			}
		}
		else if (test.atst == ATST_LESS) {
			int a = int(outColor.a * 255);

			if (a >= test.aref) {
				return false;
			}
		}
		else if (test.atst == ATST_LEQUAL) {
			int a = int(outColor.a * 255);

			if (a > test.aref) {
				return false;
			}
		}
		else if (test.atst == ATST_NEVER) {
			return false;
		}
	}

	return true;
}

void main() 
{
	// Sample texture color using fragTexCoord
	vec4 textureColor = texture(textureSampler, fragTexCoord.xy);

	// Combine texture color with fragment color
	outColor = fragColor * textureColor / (128.0 / 255.0);

	bool atst_pass = atst(outColor);

	if (!atst_pass) {
		discard;
	}

	// For dual source blending
	vec4 alpha_blend = vec4(outColor.a / (128.0 / 255.0));
	outAlphaBlend = alpha_blend;
}