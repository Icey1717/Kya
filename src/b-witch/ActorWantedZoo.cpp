#include "ActorWantedZoo.h"
#include "MemoryStream.h"
#include "BootData.h"
#include "TimeController.h"

void CActorWantedZoo::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

void CActorWantedZoo::Init()
{
	CActor::Init();

	this->textStyle.SetShadow(0x100);
	(this->textStyle).rgbaColour = 0xffffffff;
	(this->textStyle).alpha = 0xff;
	this->textStyle.SetHorizontalAlignment(2);
	this->textStyle.SetVerticalAlignment(8);
	this->textStyle.SetScale(1.5f, 1.5f);
	this->textStyle.SetFont(BootDataFont, false);
	this->field_0x238 = false;
	this->field_0x23c = 0.0f;

	return;
}

void CActorWantedZoo::Term()
{
	if (this->field_0x234 != (Zoo_14*)0x0) {
		delete[] this->field_0x234;
		this->field_0x234 = (Zoo_14*)0x0;
	}

	CActor::Term();

	return;
}

void CActorWantedZoo::Manage()
{
	float fVar2;

	CActor::Manage();

	if (this->field_0x238 == false) {
		if (0.0f < this->field_0x23c) {
			fVar2 = this->field_0x23c - GetTimer()->cutsceneDeltaTime * 2.0f;
			this->field_0x23c = fVar2;
			if (fVar2 < 0.0f) {
				this->field_0x23c = 0.0f;
			}
		}
	}
	else {
		if (this->field_0x23c < 1.0f) {
			fVar2 = this->field_0x23c + GetTimer()->cutsceneDeltaTime * 2.0f;
			this->field_0x23c = fVar2;
			if (1.0f < fVar2) {
				this->field_0x23c = 1.0f;
			}
		}
	}

	return;
}

void CActorWantedZoo::Draw()
{
	IMPLEMENTATION_GUARD();
}
