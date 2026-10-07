// Matches the C++ PerDrawData alpha/LOD bitfields. The original VU divides
// STQ (Q starts at 1) by GS clip W. Retain the GS projection's W offset.
#include "gs_projection.glsl"
vec4 SampleMaterialTexture(sampler2D image, vec2 uv, uint alphaAndLod, vec2 gsQProjection)
{
	if ((alphaAndLod & 0x80000000u) == 0) return texture(image, uv);
	int k = int((alphaAndLod >> 8) & 0xfffu);
	if ((k & 0x800) != 0) k -= 0x1000;
	float lod = float(k) / 16.0;
	if ((alphaAndLod & (1u << 22)) == 0) {
		float q = max(abs(GsQFromNative(gsQProjection, gl_FragCoord.z, gl_FragCoord.w)), 1e-30);
		lod -= log2(q) * float(1u << ((alphaAndLod >> 20) & 3u));
	}
	// Keep negative LOD for magnification filtering, while MXL limits mips.
	lod = min(lod, float((alphaAndLod >> 23) & 7u));
	return textureLod(image, uv, lod);
}
