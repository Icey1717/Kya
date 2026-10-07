#ifndef PER_DRAW_DATA_GLSL
#define PER_DRAW_DATA_GLSL

// Matches Renderer::Native::PerDrawData in NativeRendererInternal.h (128 bytes).
layout(push_constant) uniform PerDrawData
{
	mat4 projXView;
	uint renderFlags;
	uint alphaEnable;
	int alphaAtst;
	int alphaAref;
	int alphaAfail;
	uint modelMatrixIndex;
	uint animStDataIndex;
	uint animMatrixStart;
	uint lightingDataIndex;
	uint globalAlpha; // Packed C++ alpha/texture-LOD bitfields; alpha occupies the low byte.
	uint shadowProjectionIndex;
	uint frameBufferMode;
	vec2 samplingParams; // GS Q denominator, or framebuffer UV scale when frameBufferMode != 0.
	uint stripFlags; // Authored geometry flags, separate from VU renderFlags.
	uint samplingPadding;
} perDrawData;

// Matches the VU upload destination in ed3DFlushStripInit.
uint GetAnimationBaseOffset(uint stripFlags)
{
	return (stripFlags & 0x8000000u) != 0 ? 0x3dcu : 0x394u;
}

#endif
