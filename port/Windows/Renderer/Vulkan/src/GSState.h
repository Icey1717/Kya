#pragma once
#include <stdint.h>
#include "renderer.h"

namespace PS2 {
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
			this->PSM = psm;
			this->FBP = fbp;
			this->FBW = fbw;
			this->FBMASK = fbmask;
		}
		int PSM;
		int FBP;
		int FBW;
		int FBMASK;

		struct GSTest {
			uint32_t ATE;
			uint32_t ATST;
			uint32_t AREF;
			uint32_t AFAIL;
			uint32_t DATE;
			uint32_t DATM;
			uint32_t ZTE;
			uint32_t ZTST;

			bool operator==(const GSTest& other) const {
				return ATE == other.ATE
					&& ATST == other.ATST
					&& AREF == other.AREF
					&& AFAIL == other.AFAIL
					&& DATE == other.DATE
					&& DATM == other.DATM
					&& ZTE == other.ZTE
					&& ZTST == other.ZTST;
			}

			bool operator!=(const GSTest& other) const {
				return !(*this == other);
			}
		};

		GSTest TEST;

		struct GSPrim {
			uint32_t PRIM;
			uint32_t IIP;
			uint32_t TME;
			uint32_t FGE;
			uint32_t ABE;
			uint32_t AA1;
			uint32_t FST;
			uint32_t CTXT;
			uint32_t FIX;
		};

		GSPrim PRIM;
		Renderer::GSTex TEX;
	};
}

namespace PS2 {
	GSState& GetGSState();
}