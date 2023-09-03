#pragma once
#include <stdint.h>
#include "renderer.h"
#include "GIFReg.h"

namespace PS2 {

	enum GS_PSM
	{
		PSMCT32 = 0, // 0000-0000
		PSMCT24 = 1, // 0000-0001
		PSMCT16 = 2, // 0000-0010
		PSMCT16S = 10, // 0000-1010
		PSGPU24 = 18, // 0001-0010
		PSMT8 = 19, // 0001-0011
		PSMT4 = 20, // 0001-0100
		PSMT8H = 27, // 0001-1011
		PSMT4HL = 36, // 0010-0100
		PSMT4HH = 44, // 0010-1100
		PSMZ32 = 48, // 0011-0000
		PSMZ24 = 49, // 0011-0001
		PSMZ16 = 50, // 0011-0010
		PSMZ16S = 58, // 0011-1010
	};

	struct VertexAlpha
	{
		int min, max;
		bool valid;
	};

	inline VertexAlpha GetAlphaMinMax()
	{
		return VertexAlpha{ 0, 0xff, false };
		//if (!m_vt.m_alpha.valid)
		//	CalcAlphaMinMax(0, 500);
		//return m_vt.m_alpha;
	}

	struct alignas(32) GSState
	{
		inline void SetST(float S, float T) {
			this->S = S;
			this->T = T;
		}

		inline void SetRGBAQ(uint32_t R, uint32_t G, uint32_t B, uint32_t A, float Q) {
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
			this->Q = Q;
		}

		uint8_t R;
		uint8_t G;
		uint8_t B;
		uint8_t A;
		float Q;

		float S;
		float T;

		inline void SetFrame(int fbp, int fbw, int psm, int fbmask) {
			this->FRAME.PSM = psm;
			this->FRAME.FBP = fbp;
			this->FRAME.FBW = fbw;
			this->FRAME.FBMSK = fbmask;
		}

		GIFReg::GSTest TEST;

		inline void SetAlpha(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t fix) {
			this->ALPHA.A = a;
			this->ALPHA.B = b;
			this->ALPHA.C = c;
			this->ALPHA.D = d;
			this->ALPHA.FIX = fix;
		}

		GIFReg::GSPrim PRIM;
		GIFReg::GSClamp CLAMP;
		GIFReg::GSColClamp COLCLAMP;
		GIFReg::GSAlpha ALPHA;
		GIFReg::GSPabe PABE;
		GIFReg::GSFrame FRAME;
		GIFReg::GSTex TEX;

		bool bTexSet = false;

		inline bool GSState::IsOpaque() const
		{
			if (PRIM.AA1)
				return false;

			if (!PRIM.ABE)
				return true;

			int amin = 0;
			int amax = 0xff;

			if (ALPHA.A != ALPHA.B)
			{
				if (ALPHA.C == 0)
				{
					amin = GetAlphaMinMax().min;
					amax = GetAlphaMinMax().max;
				}
				else if (ALPHA.C == 1)
				{
					if (FRAME.PSM == PSMCT24 || FRAME.PSM == PSMZ24)
						amin = amax = 0x80;
				}
				else if (ALPHA.C == 2)
				{
					amin = amax = ALPHA.FIX;
				}
			}

			return ALPHA.IsOpaque(amin, amax);
		}
	};
}

namespace PS2 {
	GSState& GetGSState();
}