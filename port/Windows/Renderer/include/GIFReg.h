#pragma once
#include <stdint.h>

namespace GIFReg
{
	union GSBitBltBuf
	{
		struct {
			uint32_t SBP : 14;
			uint32_t _PAD1 : 2;
			uint32_t SBW : 6;
			uint32_t _PAD2 : 2;
			uint32_t SPSM : 6;
			uint32_t _PAD3 : 2;
			uint32_t DBP : 14;
			uint32_t _PAD4 : 2;
			uint32_t DBW : 6;
			uint32_t _PAD5 : 2;
			uint32_t DPSM : 6;
			uint32_t _PAD6 : 2;
		};

		struct
		{
			uint64_t CMD;
		};
	};

	union GSTrxPos
	{
		struct {
			uint32_t SSAX : 11;
			uint32_t _PAD1 : 5;
			uint32_t SSAY : 11;
			uint32_t _PAD2 : 5;
			uint32_t DSAX : 11;
			uint32_t _PAD3 : 5;
			uint32_t DSAY : 11;
			uint32_t DIRY : 1;
			uint32_t DIRX : 1;
			uint32_t _PAD4 : 3;
		};

		struct
		{
			uint64_t CMD;
		};
	};

	union GSTrxReg
	{
		struct {
			uint32_t RRW : 12;
			uint32_t _PAD1 : 20;
			uint32_t RRH : 12;
			uint32_t _PAD2 : 20;
		};

		struct
		{
			uint64_t CMD;
		};
	};

	union GSPrim
	{
		struct {
			uint32_t PRIM : 3;
			uint32_t IIP : 1;
			uint32_t TME : 1;
			uint32_t FGE : 1;
			uint32_t ABE : 1;
			uint32_t AA1 : 1;
			uint32_t FST : 1;
			uint32_t CTXT : 1;
			uint32_t FIX : 1;
			uint32_t _PAD1 : 21;
			uint32_t _PAD2 : 32;
		};

		struct
		{
			uint64_t CMD;
		};
	};

	union GSTex
	{
		struct
		{
			uint32_t TBP0 : 14;
			uint32_t TBW : 6;
			uint32_t PSM : 6;
			uint32_t TW : 4;
			uint32_t _PAD1 : 2;
			uint32_t _PAD2 : 2;
			uint32_t TCC : 1;
			uint32_t TFX : 2;
			uint32_t CBP : 14;
			uint32_t CPSM : 4;
			uint32_t CSM : 1;
			uint32_t CSA : 5;
			uint32_t CLD : 3;
		};

		struct
		{
			uint64_t _PAD3 : 30;
			uint64_t TH : 4;
			uint64_t _PAD4 : 30;
		};

		struct
		{
			uint64_t CMD;
		};

		bool operator==(const GSTex& other) const {
			return TBP0 == other.TBP0 &&
				TBW == other.TBW &&
				PSM == other.PSM &&
				TW == other.TW &&
				TH == other.TH &&
				TCC == other.TCC &&
				TFX == other.TFX &&
				CBP == other.CBP &&
				CPSM == other.CPSM &&
				CSM == other.CSM &&
				CSA == other.CSA &&
				CLD == other.CLD;
		}
	};

	struct GSXYOffset {
		uint32_t X;
		uint32_t Y;
	};

	union GSClamp
	{
		struct
		{
			uint32_t WMS : 2;
			uint32_t WMT : 2;
			uint32_t MINU : 10;
			uint32_t MAXU : 10;
			uint32_t _PAD1 : 8;
			uint32_t _PAD2 : 2;
			uint32_t MAXV : 10;
			uint32_t _PAD3 : 20;
		};

		struct
		{
			uint64_t _PAD4 : 24;
			uint64_t MINV : 10;
			uint64_t _PAD5 : 30;
		};

		struct
		{
			uint64_t CMD;
		};
	};

	union GSColClamp
	{
		struct
		{
			uint32_t CLAMP : 1;
			uint32_t _PAD1 : 31;
			uint32_t _PAD2 : 32;
		};

		struct
		{
			uint64_t CMD;
		};
	};

	union GSAlpha
	{
		struct
		{
			uint32_t A : 2;
			uint32_t B : 2;
			uint32_t C : 2;
			uint32_t D : 2;
			uint32_t _PAD1 : 24;
			uint8_t FIX;
			uint8_t _PAD2[3];
		};

		struct
		{
			uint64_t CMD;
		};

		__forceinline bool IsBlack() const { return ((C == 2 && FIX == 0) || (A == 2 && A == B)) && D == 2; }
		__forceinline bool IsOpaque(int amin, int amax) const { return ((A == B || amax == 0) && D == 0) || (A == 0 && B == D && amin == 0x80 && amax == 0x80); }
	};

	union GSPabe
	{
		struct
		{
			uint32_t PABE : 1;
			uint32_t _PAD1 : 31;
			uint32_t _PAD2 : 32;
		};
	};

	union GSFrame
	{
		struct
		{
			uint32_t FBP : 9;
			uint32_t _PAD1 : 7;
			uint32_t FBW : 6;
			uint32_t _PAD2 : 2;
			uint32_t PSM : 6;
			uint32_t _PAD3 : 2;
			uint32_t FBMSK;
		};
	};

	union GSTest {
		struct
		{
			uint32_t ATE : 1;
			uint32_t ATST : 3;
			uint32_t AREF : 8;
			uint32_t AFAIL : 2;
			uint32_t DATE : 1;
			uint32_t DATM : 1;
			uint32_t ZTE : 1;
			uint32_t ZTST : 2;
			uint32_t _PAD1 : 13;
			uint32_t _PAD2 : 32;
		};

		struct
		{
			uint64_t CMD;
		};

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
}