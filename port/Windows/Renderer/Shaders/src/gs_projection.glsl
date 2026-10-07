#ifndef GS_PROJECTION_GLSL
#define GS_PROJECTION_GLSL

float GsDepthFromNative(vec4 p, float depth) {
    return (p.x * depth + p.y) / (p.z * depth + p.w);
}

float NativeDepthFromGs(vec4 p, float depth) {
    return (p.y - depth * p.w) / (depth * p.z - p.x);
}

float GsQFromNative(vec2 p, float depth, float reciprocalW) {
    return reciprocalW / (p.x * depth + p.y);
}

#endif
