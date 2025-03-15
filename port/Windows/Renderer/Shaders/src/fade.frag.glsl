#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(binding = 0) uniform sampler2D inputImage;
layout(binding = 1) uniform UniformBufferObject {
	vec4 color;
} fade;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(inputImage, fragTexCoord);
    vec3 blendedColor = mix(color.rgb, fade.color.rgb, fade.color.a);
    outColor = vec4(blendedColor, 1.0f);
}