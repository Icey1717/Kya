#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(binding = 0) uniform sampler2D inputImage;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(inputImage, fragTexCoord);
    outColor = vec4(color.rgb, 1.0); // Set alpha to 1.0 for uniformity
}