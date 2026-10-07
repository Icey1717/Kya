#ifndef ENVIRONMENT_MAPPING_GLSL
#define ENVIRONMENT_MAPPING_GLSL

// Matches LightingDynamicBufferData, shared by the main and caster passes.
struct LightingDataBlock {
	mat4 lightDirection;
	mat4 lightColor;
	vec4 lightAmbient;
	vec4 flare;
	mat4 environmentNormalTransform;
	vec4 environmentCameraX;
	vec4 environmentCameraY;
};

vec2 GetEnvironmentST(LightingDataBlock data, vec3 normal)
{
	// VU 0xa8..0xab uses vf00.w for the translation column, ignoring normal W.
	vec3 mappedNormal = (data.environmentNormalTransform * vec4(normal, 1.0)).xyz;
	return vec2(1.0 + dot(data.environmentCameraX.xyz, mappedNormal),
		1.0 - dot(data.environmentCameraY.xyz, mappedNormal)) * 0.5;
}

#endif
