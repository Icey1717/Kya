#include "TextureUpload.h"
#include "GSLocalMemory.h"

#include <windows.h>
#include "common/Assertions.h"

#pragma comment(lib, "mincore") 

namespace TextureUpload
{
	GSLocalMemory m_mem{};

	void Write(uint8_t* pSrc, GIFRegBITBLTBUF& bitbltbuf, GIFRegTRXPOS& trxpos, GIFRegTRXREG& trxreg)
	{
		int tx = 0;
		int ty = 0;

		const int bpp = GSLocalMemory::m_psm[bitbltbuf.DPSM].trbpp;
		const int tex_size = (((trxreg.RRW * trxreg.RRH * bpp) + 7) >> 3);

		GSLocalMemory::m_psm[bitbltbuf.DPSM].wi(m_mem, tx, ty, pSrc, tex_size, bitbltbuf, trxpos, trxreg);
	}
}

void TextureUpload::UploadPalette(uint8_t* pSrc, uint64_t bitBltBuf, uint64_t trxPos, uint64_t trxReg, uint64_t tex0)
{
	GIFRegBITBLTBUF bitbltbuf = *reinterpret_cast<GIFRegBITBLTBUF*>(&bitBltBuf);
	GIFRegTRXPOS trxpos = *reinterpret_cast<GIFRegTRXPOS*>(&trxPos);
	GIFRegTRXREG trxreg = *reinterpret_cast<GIFRegTRXREG*>(&trxReg);
	GIFRegTEX0 TEX0 = *reinterpret_cast<GIFRegTEX0*>(&tex0);

	Write(pSrc, bitbltbuf, trxpos, trxreg);

	GIFRegTEXCLUT TEXCLUT = {};
	m_mem.m_clut.Write(TEX0, TEXCLUT);

	GIFRegTEXA texa = {};
	m_mem.m_clut.Read32(TEX0, texa);
}

uint8_t* TextureUpload::UploadTexture(uint8_t* pSrc, uint64_t bitBltBuf, uint64_t trxPos, uint64_t trxReg, uint64_t tex0)
{
	GIFRegBITBLTBUF bitbltbuf = *reinterpret_cast<GIFRegBITBLTBUF*>(&bitBltBuf);
	GIFRegTRXPOS trxpos = *reinterpret_cast<GIFRegTRXPOS*>(&trxPos);
	GIFRegTRXREG trxreg = *reinterpret_cast<GIFRegTRXREG*>(&trxReg);
	GIFRegTEX0 TEX0 = *reinterpret_cast<GIFRegTEX0*>(&tex0);

	if (trxreg.RRW == (1 << TEX0.TW) && trxreg.RRH == (1 << TEX0.TH)) {
		pSrc[0] = pSrc[0];
	}

	Write(pSrc, bitbltbuf, trxpos, trxreg);

	// compute width/height
	const GSLocalMemory::psm_t& psm_ = GSLocalMemory::m_psm[TEX0.PSM];
	const GSVector2i& bs = psm_.bs;
	const int tw = (1 << TEX0.TW);
	const int th = (1 << TEX0.TH);
	const GSVector4i rect(0, 0, tw, th);
	const GSVector4i block_rect(rect.ralign<Align_Outside>(bs));
	const int read_width = block_rect.width();
	const int read_height = block_rect.height();
	const u32 pitch = static_cast<u32>(read_width) * sizeof(u32);

	// use per-texture buffer so we can compress the texture asynchronously and not block the GS thread
	// must be 32 byte aligned for ReadTexture().
	u8* buffer = static_cast<u8*>(_aligned_malloc(pitch * static_cast<u32>(read_height), 32));

	GIFRegTEXA texa = {};
	psm_.rtx(m_mem, m_mem.GetOffset(TEX0.TBP0, TEX0.TBW, TEX0.PSM), block_rect, buffer, pitch, texa);

	return buffer;
}

uint8_t* TextureUpload::GetClut()
{
	const u32* pal = m_mem.m_clut;
	return reinterpret_cast<uint8_t*>(const_cast<u32*>(pal));
}

static HANDLE s_fh = NULL;

void* GSAllocateWrappedMemory(size_t size, size_t repeat)
{
	pxAssertRel(!s_fh, "Has no file mapping");

	s_fh = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, nullptr);
	if (s_fh == NULL)
	{
		//Console.Error("Failed to create file mapping of size %zu. WIN API ERROR:%u", size, GetLastError());
		return nullptr;
	}

	// Reserve the whole area with repeats.
	u8* base = static_cast<u8*>(VirtualAlloc2(
		GetCurrentProcess(), nullptr, repeat * size,
		MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS,
		nullptr, 0));
	if (base)
	{
		bool okay = true;
		for (size_t i = 0; i < repeat; i++)
		{
			// Everything except the last needs the placeholders split to map over them. Then map the same file over the region.
			u8* addr = base + i * size;
			if ((i != (repeat - 1) && !VirtualFreeEx(GetCurrentProcess(), addr, size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER)) ||
				!MapViewOfFile3(s_fh, GetCurrentProcess(), addr, 0, size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0))
			{
				//Console.Error("Failed to map repeat %zu of size %zu.", i, size);
				okay = false;

				for (size_t j = 0; j < i; j++)
					UnmapViewOfFile2(GetCurrentProcess(), addr, MEM_PRESERVE_PLACEHOLDER);
			}
		}

		if (okay)
		{
			//DbgCon.WriteLn("fifo_alloc(): Mapped %zu repeats of %zu bytes at %p.", repeat, size, base);
			return base;
		}

		VirtualFreeEx(GetCurrentProcess(), base, 0, MEM_RELEASE);
	}

	//Console.Error("Failed to reserve VA space of size %zu. WIN API ERROR:%u", size, GetLastError());
	CloseHandle(s_fh);
	s_fh = NULL;
	return nullptr;
}

void GSFreeWrappedMemory(void* ptr, size_t size, size_t repeat)
{
	pxAssertRel(s_fh, "Has a file mapping");

	for (size_t i = 0; i < repeat; i++)
	{
		u8* addr = (u8*)ptr + i * size;
		UnmapViewOfFile2(GetCurrentProcess(), addr, MEM_PRESERVE_PLACEHOLDER);
	}

	VirtualFreeEx(GetCurrentProcess(), ptr, 0, MEM_RELEASE);
	s_fh = NULL;
}