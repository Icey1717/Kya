#version 450

float int12_to_float(int x) {
	return float(x) * 0.000244140625;
}

layout(set = 0, binding = 2) readonly buffer ModelBuffer {
	mat4 modelMatrix[];
} model;

layout(set = 0, binding = 3) readonly buffer AnimBuffer {
	mat4 animMatrix[];
} anim;

//push constants block
layout( push_constant ) uniform PerDrawData
{
	mat4 projXView;
	uint renderFlags;
	uint alphaEnable;
	int  alphaAtst;
	int  alphaAref;
	int  alphaAfail;
	uint modelMatrixIndex;
	uint animStDataIndex;
	uint animMatrixStart;
	uint lightingDataIndex;
	uint globalAlpha;
	uint shadowProjectionIndex;
	uint frameBufferMode;
	float frameBufferScaleX;
	float frameBufferScaleY;

	// Equivalent of VU destination address for animation matrix data.
	// Usually 0x394 or 0x3dc.
	uint animBaseOffset;

	uint stripFlags; // Authored geometry flags, separate from VU renderFlags.
} perDrawData;

struct LightingDataBlock {
	mat4 lightDirection;
	mat4 lightColor;
	vec4 lightAmbient;
	vec4 flare;
};

layout(set = 0, binding = 4) readonly buffer LightingData {
	LightingDataBlock lightData[];
} lightingBuf;

layout(set = 0, binding = 5) readonly buffer AnimStData {
	vec4 animST[];
} animStData;

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in uint inFlags;
layout(location = 5) in vec4 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragTexCoord;

void main() {
	uint animFlags = inFlags & 0x7ff;

	vec4 fixedPos = vec4(inPosition, 1.0);
	vec3 extrusionNormal = inNormal.xyz;

	fragColor = vec4(inColor) / 255.0;
	bool hasNormals = (perDrawData.stripFlags & 0x8000000u) != 0;
	bool rigidAnimation = (perDrawData.stripFlags & 0x10000u) != 0;
	if (rigidAnimation && animFlags >= perDrawData.animBaseOffset) {
		uint animIndex = (animFlags - perDrawData.animBaseOffset) / 4;

		mat4 currentAnimMatrix = anim.animMatrix[perDrawData.animMatrixStart + animIndex];
		fixedPos = currentAnimMatrix * fixedPos;
		if (hasNormals) extrusionNormal = mat3(currentAnimMatrix) * extrusionNormal;
	}

	// Lighting is independent of animation and of the VU bone-table address.
	if (hasNormals && (perDrawData.renderFlags & 0x10) != 0) {
		LightingDataBlock lighting = lightingBuf.lightData[perDrawData.lightingDataIndex];
		vec4 weights = lighting.lightDirection[2] * extrusionNormal.z
			+ lighting.lightDirection[1] * extrusionNormal.y
			+ lighting.lightDirection[0] * extrusionNormal.x;
		weights = max(weights, vec4(0.0));
		vec4 lightColor = vec4(lighting.lightAmbient.xyz, 1.0)
			+ lighting.lightColor[3] * weights.w
			+ lighting.lightColor[2] * weights.z
			+ lighting.lightColor[1] * weights.y
			+ lighting.lightColor[0] * weights.x;
		lightColor = min(lightColor, vec4(255.0));
		vec4 color = vec4(inColor) * lighting.lightAmbient.w;
		fragColor.rgb = (color * lightColor).rgb / 255.0;
	}

	// _$Normal_Extruder: displace in object space after rigid skinning.
	// The VU adds only XYZ, using the decoded normal without normalization.
	if ((perDrawData.renderFlags & 0x100) != 0) {
		fixedPos.xyz += extrusionNormal * animStData.animST[perDrawData.animStDataIndex].z;
	}

	vec4 pos = perDrawData.projXView * model.modelMatrix[perDrawData.modelMatrixIndex] * fixedPos;

	// Do this in the projection matrix
	//pos.y = -pos.y;
	gl_Position = pos;

	vec2 outST = vec2(int12_to_float(inST.x), int12_to_float(inST.y));

	if ((perDrawData.renderFlags & 0x200) != 0) {
		outST.x += animStData.animST[perDrawData.animStDataIndex].x;
		outST.y += animStData.animST[perDrawData.animStDataIndex].y;
	}

	fragTexCoord = vec4(outST, inQ);
}
