#include "LightBase.h"
#include "MemoryStream.h"
#include "MathOps.h"

void FullColorModel::Read(ByteCode* pByteCode)
{
	byte bVar1;
	float fVar2;

	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).x = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).y = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).z = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->ambientColor).w = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).x = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).y = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).z = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->color).w = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).x = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).y = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).z = fVar2 * 2.0f;
	bVar1 = pByteCode->GetU8();
	if ((char)bVar1 < '\0') {
		fVar2 = (float)((uint)(int)(char)bVar1 >> 1 | (int)(char)bVar1 & 1U);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)(int)(char)bVar1;
	}
	(this->field_0x20).w = fVar2 * 2.0f;
	fVar2 = pByteCode->GetF32();
	edF32Vector4ScaleHard(fVar2, &this->ambientColor, &this->ambientColor);
	edF32Vector4ScaleHard(fVar2, &this->color, &this->color);
	edF32Vector4ScaleHard(fVar2, &this->field_0x20, &this->field_0x20);

	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read color {}", this->ambientColor.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read 0x10 {}", this->color.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "FullColorModel::Read 0x20 {}", this->field_0x20.ToString());

	return;
}

void SpotShape::Read(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->position.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.z = fVar1;
	this->position.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fallout0 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->fallout1 = fVar1;

	fVar1 = pByteCode->GetF32();
	this->direction.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.z = fVar1;
	this->direction.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fov0 = cosf(fVar1);

	fVar1 = pByteCode->GetF32();
	this->fov1 = cosf(fVar1);

	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read position: {}", this->position.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read fallout0 {}", this->fallout0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read fallout1 {}", this->fallout1);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read direction {}", this->direction.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read field_0x30 {}", this->fov0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SpotShape::Read field_0x34 {}", this->fov1);
}

void LightShape::Read(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->position.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.z = fVar1;
	this->position.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fallout0 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->fallout1 = fVar1;

	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read position: {}", this->position.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read fallout0 {}", this->fallout0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "LightShape::Read fallout1 {}", this->fallout1);
}

void SunShape::Read(ByteCode* pByteCode)
{
	float fVar1;

	fVar1 = pByteCode->GetF32();
	this->position.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->position.z = fVar1;
	this->position.w = 1.0f;

	fVar1 = pByteCode->GetF32();
	this->fallout0 = fVar1;
	fVar1 = pByteCode->GetF32();
	this->fallout1 = fVar1;

	fVar1 = pByteCode->GetF32();
	this->direction.x = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.y = fVar1;
	fVar1 = pByteCode->GetF32();
	this->direction.z = fVar1;
	this->direction.w = 1.0f;

	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read");
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read position: {}", this->position.ToString());
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read fallout0 {}", this->fallout0);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read fallout1 {}", this->fallout1);
	LIGHT_MANAGER_LOG(LogLevel::Info, "SunShape::Read direction {}", this->direction.ToString());
	return;
}