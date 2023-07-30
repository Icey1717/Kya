// Full-Screen Quad Vertex Shader
#version 450 core

layout(location = 0) out vec2 texCoord;

void main() {
    // Full-screen quad vertex positions
    vec2 positions[4] = vec2[](
        vec2(-1.0, -1.0),
        vec2(1.0, -1.0),
        vec2(-1.0, 1.0),
        vec2(1.0, 1.0)
    );

    // Pass the vertex position to the fragment shader
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    // Calculate the texture coordinate based on the vertex position
    texCoord = (positions[gl_VertexIndex] + vec2(1.0)) * 0.5;
}