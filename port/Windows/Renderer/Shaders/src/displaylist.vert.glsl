#version 450

layout(location = 0) in vec2 inST;
layout(location = 1) in ivec4 inColor;
layout(location = 2) in float inQ;
layout(location = 3) in ivec2 inXY;
layout(location = 4) in uint inZ;
layout(location = 5) in ivec2 inUV;
layout(location = 6) in vec4 inFOG;
layout(location = 7) in vec3 inXYZ;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = vec4(inXYZ, 1.0);
    fragColor = inColor;
    fragTexCoord = inST;
}