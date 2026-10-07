#version 450
#extension GL_GOOGLE_include_directive : require
#include "texture_sampling.glsl"

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragTexCoord;

layout(location = 0, index = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlphaBlend;

// Texture sampler
layout(set = 1, binding = 0) uniform sampler2D textureSampler;

#include "per_draw_data.glsl"

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
	if (bool(perDrawData.alphaEnable)) {
		if (perDrawData.alphaAtst == ATST_GEQUAL) {
			int a = int(outColor.a * 255);

			if (a < perDrawData.alphaAref) {
				return false;
			}
		}
		else if (perDrawData.alphaAtst == ATST_GREATER) {
			int a = int(outColor.a * 255);

			if (a <= perDrawData.alphaAref) {
				return false;
			}
		}
		else if (perDrawData.alphaAtst == ATST_EQUAL) {
			int a = int(outColor.a * 255);

			if (a != perDrawData.alphaAref) {
				return false;
			}
		}
		else if (perDrawData.alphaAtst == ATST_NOTEQUAL) {
			int a = int(outColor.a * 255);

			if (a == perDrawData.alphaAref) {
				return false;
			}
		}
		else if (perDrawData.alphaAtst == ATST_LESS) {
			int a = int(outColor.a * 255);

			if (a >= perDrawData.alphaAref) {
				return false;
			}
		}
		else if (perDrawData.alphaAtst == ATST_LEQUAL) {
			int a = int(outColor.a * 255);

			if (a > perDrawData.alphaAref) {
				return false;
			}
		}
		else if (perDrawData.alphaAtst == ATST_NEVER) {
			return false;
		}
	}

	return true;
}

void main() 
{
	// Sample texture color using fragTexCoord
	vec2 uv = fragTexCoord.xy;
	if (perDrawData.frameBufferMode != 0) {
		uv *= perDrawData.samplingParams;
	}
	vec4 textureColor = perDrawData.frameBufferMode != 0 ? texture(textureSampler, uv)
		: SampleMaterialTexture(textureSampler, uv, perDrawData.globalAlpha, perDrawData.samplingParams);

	// Combine texture color with fragment color
	outColor = fragColor * textureColor / (128.0 / 255.0);
	if (perDrawData.frameBufferMode == 2) {
		// GS DECAL with TCC enabled takes both RGB and alpha from the texture.
		outColor = textureColor;
	}

	bool atst_pass = atst(outColor);

	// Bit 4 selects the failure replay; write masks are set independently.
	if (atst_pass == ((perDrawData.alphaAfail & 16) != 0)) discard;

	// For dual source blending
	vec4 alpha_blend = vec4(outColor.a / (128.0 / 255.0));
	outAlphaBlend = alpha_blend;
	if (perDrawData.blendMode == 1u) {
		// DST_COLOR * 1 + destination * SRC1_COLOR = Cd * (1 + As).
		outColor.rgb = vec3(1.0);
	}
}
