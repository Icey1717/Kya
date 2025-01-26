#version 450

layout(lines) in; // Input primitive type: line
layout(triangle_strip, max_vertices = 4) out; // Output primitive type: triangle strip, max 4 vertices

// Input from vertex shader
layout(location = 0) in vec4 inFragColor[];     // Interpolated color
layout(location = 1) in vec2 inFragTexCoord[]; // Texture coordinates

// Output to fragment shader
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Fetch input vertices
    vec4 ltColor = inFragColor[0];
    vec2 ltTexCoord = inFragTexCoord[0];
    vec4 ltPosition = gl_in[0].gl_Position;

    vec4 rbColor = inFragColor[1];
    vec2 rbTexCoord = inFragTexCoord[1];
    vec4 rbPosition = gl_in[1].gl_Position;

    // Flat depth: set the z-component of the left-top position to match the right-bottom position
    ltPosition.z = rbPosition.z;

    // Flat color: use the color of the right-bottom vertex
    ltColor = rbColor;

    // Vertex adjustments
    vec4 lbPosition = rbPosition; // Bottom-left
    lbPosition.x = ltPosition.x;

    vec4 rtPosition = rbPosition; // Top-right
    rtPosition.y = ltPosition.y;

    // Output vertices
    // Left-top
    gl_Position = ltPosition;
    fragColor = ltColor;
    fragTexCoord = ltTexCoord;
    EmitVertex();

    // Left-bottom
    gl_Position = lbPosition;
    fragColor = rbColor;
    fragTexCoord = vec2(ltTexCoord.x, rbTexCoord.y);
    EmitVertex();

    // Right-top
    gl_Position = rtPosition;
    fragColor = rbColor;
    fragTexCoord = vec2(rbTexCoord.x, ltTexCoord.y);
    EmitVertex();

    // Right-bottom
    gl_Position = rbPosition;
    fragColor = rbColor;
    fragTexCoord = rbTexCoord;
    EmitVertex();

    EndPrimitive();
}
