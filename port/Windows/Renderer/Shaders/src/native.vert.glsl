#version 450

float int12_to_float(int x) {
	return float(x) * 0.000244140625;
}

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
} ubo;

layout(set = 0, binding = 2) uniform ModelBuffer {
	mat4 modelMatrix;
} model;

layout(set = 0, binding = 3) uniform AnimBuffer {
	mat4 animMatrix[27];
} anim;

//push constants block
layout( push_constant ) uniform LightingData
{
	mat4 lightDirection;
	mat4 lightColor;
	vec4 lightAmbient;
} lightingData;

layout(location = 0) in ivec2 inST;
layout(location = 1) in vec2 inQ;
layout(location = 2) in ivec4 inColor;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in uint inFlags;
layout(location = 5) in vec4 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragTexCoord;

void main() {
	// _$XYZW_16_Conv_EndBones_Rigid

	uint baseFlags = inFlags & 0xFFFF;

	uint stripIndex = inFlags >> 16;

	uint animFlags = baseFlags & 0x7ff;
	uint flags = baseFlags & 0xc000;

	vec4 fixedPos = vec4(inPosition, 1.0);

	if (animFlags > 0) {
		uint animIndex = animFlags - 0x3dc;
		animIndex = animIndex / 4;

		mat4 currentAnimMatrix = anim.animMatrix[animIndex];
		fixedPos = currentAnimMatrix * fixedPos;

		vec4 normal = currentAnimMatrix * inNormal;

		normal = (lightingData.lightDirection[2] * normal.z) + (lightingData.lightDirection[1] * normal.y) + (lightingData.lightDirection[0] * normal.x);

		normal.x = max(normal.x, 0.0);
		normal.y = max(normal.y, 0.0);
		normal.z = max(normal.z, 0.0);
		normal.w = max(normal.w, 0.0);

		vec4 lightAmbientAdjusted = vec4(lightingData.lightAmbient.x, lightingData.lightAmbient.y, lightingData.lightAmbient.z, 0.0f);

		normal = (lightAmbientAdjusted + vec4(0.0, 0.0, 0.0, 1.0)) + (lightingData.lightColor[3] * normal.w) + (lightingData.lightColor[2] * normal.z) + (lightingData.lightColor[1] * normal.y) + (lightingData.lightColor[0] * normal.x);

		normal.x = min(normal.x, 255.0);
		normal.y = min(normal.y, 255.0);
		normal.z = min(normal.z, 255.0);
		normal.w = min(normal.w, 255.0);

		vec4 color = vec4(inColor.x, inColor.y, inColor.z, inColor.w);
		color = color * lightingData.lightAmbient.w;
		
		color = color * normal;

		fragColor.x = color.x / 255.0;
		fragColor.y = color.y / 255.0;
		fragColor.z = color.z / 255.0; 
		fragColor.w = inColor.w / 255.0;
	}
	else {
		fragColor.x = inColor.x / 255.0;
		fragColor.y = inColor.y / 255.0;
		fragColor.z = inColor.z / 255.0;
		fragColor.w = inColor.w / 255.0;
	}

	vec4 pos = ubo.proj * ubo.view * model.modelMatrix * fixedPos;

	// Do this in the projection matrix
	//pos.y = -pos.y;
	gl_Position = pos;

	vec2 outST = vec2(int12_to_float(inST.x), int12_to_float(inST.y));

	fragTexCoord = vec4(outST, inQ);
}