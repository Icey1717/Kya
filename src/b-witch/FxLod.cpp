#include "FxLod.h"

// PS2 0036a150: validate/kill the child handle, clear it only when valid,
// then run the base effect cleanup.
void CLodFx::Kill()
{
	if (this->field_0x80.IsValid()) {
		this->field_0x80.Kill();
		this->field_0x80.Reset();
	}

	CNewFx::Kill();
}

// PS2 0036a1f0: the incoming f12 is forwarded unchanged to the child.
void CLodFx::Stop(float param_1)
{
	this->flags = this->flags | 0x20010;
	if (param_1 != -1.0f) {
		this->field_0x10 = param_1;
		if (0.0f < param_1) {
			this->flags = this->flags | 0x40;
		}
	}

	if (this->field_0x80.IsValid()) {
		this->field_0x80.pFx->Stop(param_1);
	}
}

// PS2 0036a5d0: jr ra; delay-slot lbu v0,0x88(a0).
bool CLodFx::IsLooped()
{
	return this->field_0x88;
}

void CFxLodScenaricData::Init()
{
	return;
}

void CFxLodScenaricData::Create(ByteCode* pByteCode)
{
	IMPLEMENTATION_GUARD();
}

void CFxLodScenaricData::Term()
{
	IMPLEMENTATION_GUARD();
}

void CLodFx::Draw()
{
	return;
}

int CLodFx::GetType()
{
	return FX_TYPE_LOD;
}
