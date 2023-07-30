#version 450 core

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D sceneImage;

void main() {
    vec4 texColor = texture(sceneImage, texCoord);
    float grayValue = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    outColor = vec4(texColor.r, texColor.g, texColor.b, 1.0);
}
