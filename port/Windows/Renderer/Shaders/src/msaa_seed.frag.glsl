#version 450
layout(set = 0, binding = 0) uniform sampler2D sceneColor;
layout(set = 0, binding = 1) uniform sampler2D sceneDepth;
layout(location = 0) out vec4 outColor;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    outColor = texelFetch(sceneColor, pixel, 0);
    gl_FragDepth = texelFetch(sceneDepth, pixel, 0).r;
}
