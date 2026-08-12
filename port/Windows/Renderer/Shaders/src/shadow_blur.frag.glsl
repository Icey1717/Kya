#version 450

layout(binding = 0) uniform sampler2D shadowMask;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out float outCoverage;

layout(push_constant) uniform BlurData
{
	vec2 texelSize;
	uint sampleCount;
	float radius;
} blur;

void main()
{
	if (blur.sampleCount == 0 || blur.radius <= 0.0) {
		outCoverage = texture(shadowMask, fragTexCoord).r;
		return;
	}

	const float goldenAngle = 2.39996323;
	float coverage = 0.0;
	for (uint i = 0; i < blur.sampleCount; ++i) {
		float distanceFromCenter = sqrt((float(i) + 0.5) / float(blur.sampleCount));
		float angle = float(i) * goldenAngle;
		vec2 offset = vec2(cos(angle), sin(angle)) * distanceFromCenter * blur.radius * blur.texelSize;
		coverage += texture(shadowMask, fragTexCoord + offset).r;
	}
	outCoverage = coverage / float(blur.sampleCount);
}
