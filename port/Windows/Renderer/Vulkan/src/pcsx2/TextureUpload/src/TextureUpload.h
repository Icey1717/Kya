#pragma once

#include <cstdint>

namespace TextureUpload
{
	void UploadPalette(uint8_t* pSrc, uint64_t bitBltBuf, uint64_t trxPos, uint64_t trxReg, uint64_t tex0);
	uint8_t* UploadTexture(uint8_t* pSrc, uint64_t bitBltBuf, uint64_t trxPos, uint64_t trxReg, uint64_t tex0);

	// Debug
	uint8_t* GetClut();
}