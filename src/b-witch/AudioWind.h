#ifndef AUDIO_WIND_H
#define AUDIO_WIND_H

#include "Types.h"
#include "Audio.h"
#include "MemoryStream.h"

class CFxWind;

class CSoundWind
{
public:
	void Init();
	void Init(CFxWind* pFx);
	void Add(ByteCode* pByteCode);

	S_STREAM_REF<CSound> field_0x0;
	S_STREAM_REF<CSound> field_0x4;
	S_STREAM_REF<CSound> field_0x8;
};

#endif // AUDIO_WIND_H
