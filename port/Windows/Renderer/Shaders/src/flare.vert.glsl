#version 450

layout(set = 0, binding = 1) uniform sampler2D sceneDepth;
layout(push_constant) uniform FlareData {
	vec4 rect;
	vec4 marker;
	vec4 params; // Vulkan depth, occlusion enabled.
} flare;

layout(location = 0) out vec2 texCoord;
layout(location = 1) flat out float visibility;

void main() {
	const vec2 corners[6] = vec2[6](vec2(0, 0), vec2(0, 1), vec2(1, 0),
		vec2(1, 0), vec2(0, 1), vec2(1, 1));
	texCoord = corners[gl_VertexIndex];
	vec2 position = flare.rect.xy + (texCoord * 2.0 - 1.0) * flare.rect.zw;
	gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);

	visibility = 1.0;
	if (flare.params.y != 0.0) {
		// Approximate the GS marker/mipmap reduction with 16 x 16 depth comparisons.
		float visible = 0.0;
		for (int y = 0; y < 16; ++y) {
			for (int x = 0; x < 16; ++x) {
				vec2 offset = (vec2(x, y) + 0.5) / 8.0 - 1.0;
				vec2 uv = flare.marker.xy + offset * flare.marker.zw;
				if (all(greaterThanEqual(uv, vec2(0.0))) && all(lessThan(uv, vec2(1.0)))) {
					float depth = textureLod(sceneDepth, uv, 0.0).r;
					// Native scene depth is reversed: larger values are closer.
					visible += flare.params.x >= depth ? 1.0 : 0.0;
				}
			}
		}
		visibility = visible / 256.0;
	}
}
