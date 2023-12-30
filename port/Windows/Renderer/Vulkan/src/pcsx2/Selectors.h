/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2023  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <stdint.h>

#include <functional>
#include <utility>

#define __fi __forceinline

enum GS_TFX
{
	TFX_MODULATE = 0,
	TFX_DECAL = 1,
	TFX_HIGHLIGHT = 2,
	TFX_HIGHLIGHT2 = 3,
	TFX_NONE = 4,
};

enum GS_CLAMP
{
	CLAMP_REPEAT = 0,
	CLAMP_CLAMP = 1,
	CLAMP_REGION_CLAMP = 2,
	CLAMP_REGION_REPEAT = 3,
};

enum GS_ZTST
{
	ZTST_NEVER = 0,
	ZTST_ALWAYS = 1,
	ZTST_GEQUAL = 2,
	ZTST_GREATER = 3,
};

enum GS_ATST
{
	ATST_NEVER = 0,
	ATST_ALWAYS = 1,
	ATST_LESS = 2,
	ATST_LEQUAL = 3,
	ATST_EQUAL = 4,
	ATST_GEQUAL = 5,
	ATST_GREATER = 6,
	ATST_NOTEQUAL = 7,
};

enum GS_AFAIL
{
	AFAIL_KEEP = 0,
	AFAIL_FB_ONLY = 1,
	AFAIL_ZB_ONLY = 2,
	AFAIL_RGB_ONLY = 3,
};

enum class GS_MIN_FILTER : uint8_t
{
	Nearest = 0,
	Linear = 1,
	Nearest_Mipmap_Nearest = 2,
	Nearest_Mipmap_Linear = 3,
	Linear_Mipmap_Nearest = 4,
	Linear_Mipmap_Linear = 5,
};

template <typename T, typename... Rest>
static inline void HashCombine(std::size_t& seed, const T& v, Rest&&... rest)
{
	seed ^= std::hash<T>{}(v)+0x9e3779b9u + (seed << 6) + (seed >> 2);
	(HashCombine(seed, std::forward<Rest>(rest)), ...);
}

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

enum HWBlendFlags
{
	// Flags to determine blending behavior
	BLEND_CD = 0x1,   // Output is Cd, hw blend can handle it
	BLEND_HW_CLR1 = 0x2,   // Clear color blending (use directly the destination color as blending factor)
	BLEND_HW_CLR2 = 0x4,   // Clear color blending (use directly the destination color as blending factor)
	BLEND_HW_CLR3 = 0x8,   // Multiply Cs by (255/128) to compensate for wrong Ad/255 value, should be Ad/128
	BLEND_MIX1 = 0x10,  // Mix of hw and sw, do Cs*F or Cs*As in shader
	BLEND_MIX2 = 0x20,  // Mix of hw and sw, do Cs*(As + 1) or Cs*(F + 1) in shader
	BLEND_MIX3 = 0x40,  // Mix of hw and sw, do Cs*(1 - As) or Cs*(1 - F) in shader
	BLEND_ACCU = 0x80,  // Allow to use a mix of SW and HW blending to keep the best of the 2 worlds
	BLEND_NO_REC = 0x100, // Doesn't require sampling of the RT as a texture
	BLEND_A_MAX = 0x200, // Impossible blending uses coeff bigger than 1
};

// Determines the HW blend function for DX11/OGL
struct HWBlend
{
	u16 flags;
	u8 op, src, dst;
};

struct alignas(16) GSHWDrawConfig
{
	enum class Topology : u8
	{
		Point,
		Line,
		Triangle,
	};
	enum class VSExpand : u8
	{
		None,
		Point,
		Line,
		Sprite,
	};
#pragma pack(push, 1)
	struct VSSelector
	{
		union
		{
			struct
			{
				u8 fst : 1;
				u8 tme : 1;
				u8 iip : 1;
				//u8 hdw : 1;
				u8 point_size : 1;		///< Set when points need to be expanded without VS expanding.
				VSExpand expand : 2;
				u8 _free : 2;
			};
			u8 key;
		};
		VSSelector() : key(0) {}
		VSSelector(u8 k) : key(k) {}

		/// Returns true if the fixed index buffer should be used.
		__fi bool UseExpandIndexBuffer() const { return (expand == VSExpand::Point || expand == VSExpand::Sprite); }
	};
	static_assert(sizeof(VSSelector) == 1, "VSSelector is a single byte");
#pragma pack(pop)
#pragma pack(push, 4)
	struct PSSelector
	{
		// Performance note: there are too many shader combinations
		// It might hurt the performance due to frequent toggling worse it could consume
		// a lots of memory.
		union
		{
			struct
			{
				// Format
				u32 aem_fmt : 2;
				u32 pal_fmt : 2;
				u32 dfmt : 2; // 0 → 32-bit, 1 → 24-bit, 2 → 16-bit
				u32 depth_fmt : 2; // 0 → None, 1 → 32-bit, 2 → 16-bit, 3 → RGBA
				// Alpha extension/Correction
				u32 aem : 1;
				u32 fba : 1;
				// Fog
				u32 fog : 1;
				// Flat/goround shading
				u32 iip : 1;
				// Pixel test
				u32 date : 3;
				u32 atst : 3;
				// Color sampling
				u32 fst : 1; // Investigate to do it on the VS
				u32 tfx : 3;
				u32 tcc : 1;
				u32 wms : 2;
				u32 wmt : 2;
				u32 adjs : 1;
				u32 adjt : 1;
				u32 ltf : 1;
				// Shuffle and fbmask effect
				u32 shuffle : 1;
				u32 shuffle_same : 1;
				u32 real16src : 1;
				u32 read_ba : 1;
				u32 write_rg : 1;
				u32 fbmask : 1;

				// Blend and Colclip
				u32 blend_a : 2;
				u32 blend_b : 2;
				u32 blend_c : 2;
				u32 blend_d : 2;
				u32 fixed_one_a : 1;
				u32 blend_hw : 2;
				u32 a_masked : 1;
				u32 hdr : 1;
				u32 colclip : 1;
				u32 blend_mix : 2;
				u32 round_inv : 1; // Blending will invert the value, so rounding needs to go the other way
				u32 pabe : 1;
				u32 no_color : 1; // disables color output entirely (depth only)
				u32 no_color1 : 1; // disables second color output (when unnecessary)
				u32 no_ablend : 1; // output alpha blend in col0 (for no-DSB)
				u32 only_alpha : 1; // don't bother computing RGB

				// Others ways to fetch the texture
				u32 channel : 3;

				// Dithering
				u32 dither : 2;

				// Depth clamp
				u32 zclamp : 1;

				// Hack
				u32 tcoffsethack : 1;
				u32 urban_chaos_hle : 1;
				u32 clr1 : 1;
				//u32 tales_of_abyss_hle : 1;
				u32 tex_is_fb : 1; // Jak Shadows
				u32 automatic_lod : 1;
				u32 manual_lod : 1;
				u32 point_sampler : 1;
				u32 region_rect : 1;

				// Scan mask
				u32 scanmsk : 2;
			};

			struct
			{
				u64 key_lo;
				u32 key_hi;
			};
		};
		__fi PSSelector() : key_lo(0), key_hi(0) {}

		__fi bool operator==(const PSSelector& rhs) const { return (key_lo == rhs.key_lo && key_hi == rhs.key_hi); }
		__fi bool operator!=(const PSSelector& rhs) const { return (key_lo != rhs.key_lo || key_hi != rhs.key_hi); }
		__fi bool operator<(const PSSelector& rhs) const { return (key_lo < rhs.key_lo || key_hi < rhs.key_hi); }

		__fi bool IsFeedbackLoop() const
		{
			const u32 sw_blend_bits = blend_a | blend_b | blend_d;
			const bool sw_blend_needs_rt = sw_blend_bits != 0 && ((sw_blend_bits | blend_c) & 1u);
			return tex_is_fb || fbmask || (date > 0 && date != 3) || sw_blend_needs_rt;
		}

		/// Disables color output from the pixel shader, this is done when all channels are masked.
		__fi void DisableColorOutput()
		{
			// remove software blending, since this will cause the color to be declared inout with fbfetch.
			blend_a = blend_b = blend_c = blend_d = 0;

			// TEX_IS_FB relies on us having a color output to begin with.
			tex_is_fb = 0;

			// no point having fbmask, since we're not writing. DATE has to stay.
			fbmask = 0;

			// disable both outputs.
			no_color = no_color1 = 1;
		}
	};
	static_assert(sizeof(PSSelector) == 12, "PSSelector is 12 bytes");
#pragma pack(pop)
	struct PSSelectorHash
	{
		size_t operator()(const PSSelector& p) const
		{
			size_t h = 0;
			HashCombine(h, p.key_lo, p.key_hi);
			return h;
		}
	};
#pragma pack(push, 1)
	struct SamplerSelector
	{
		union
		{
			struct
			{
				u8 tau : 1;
				u8 tav : 1;
				u8 biln : 1;
				u8 triln : 3;
				u8 aniso : 1;
				u8 lodclamp : 1;
			};
			u8 key;
		};
		SamplerSelector() : key(0) {}
		SamplerSelector(u8 k) : key(k) {}
		static SamplerSelector Point() { return SamplerSelector(); }
		static SamplerSelector Linear()
		{
			SamplerSelector out;
			out.biln = 1;
			return out;
		}

		/// Returns true if the effective minification filter is linear.
		__fi bool IsMinFilterLinear() const
		{
			if (triln < static_cast<u8>(GS_MIN_FILTER::Nearest_Mipmap_Nearest))
			{
				// use the same filter as mag when mipmapping is off
				return biln;
			}
			else
			{
				// Linear_Mipmap_Nearest or Linear_Mipmap_Linear
				return (triln >= static_cast<u8>(GS_MIN_FILTER::Linear_Mipmap_Nearest));
			}
		}

		/// Returns true if the effective magnification filter is linear.
		__fi bool IsMagFilterLinear() const
		{
			// magnification uses biln regardless of mip mode (they're only used for minification)
			return biln;
		}

		/// Returns true if the effective mipmap filter is linear.
		__fi bool IsMipFilterLinear() const
		{
			return (triln == static_cast<u8>(GS_MIN_FILTER::Nearest_Mipmap_Linear) ||
				triln == static_cast<u8>(GS_MIN_FILTER::Linear_Mipmap_Linear));
		}

		/// Returns true if mipmaps should be used when filtering (i.e. LOD not clamped to zero).
		__fi bool UseMipmapFiltering() const
		{
			return (triln >= static_cast<u8>(GS_MIN_FILTER::Nearest_Mipmap_Nearest));
		}
	};
	struct DepthStencilSelector
	{
		union
		{
			struct
			{
				u8 ztst : 2;
				u8 zwe : 1;
				u8 date : 1;
				u8 date_one : 1;

				u8 _free : 3;
			};
			u8 key;
		};
		DepthStencilSelector() : key(0) {}
		DepthStencilSelector(u8 k) : key(k) {}
		static DepthStencilSelector NoDepth()
		{
			DepthStencilSelector out;
			out.ztst = ZTST_ALWAYS;
			return out;
		}
	};
	struct ColorMaskSelector
	{
		union
		{
			struct
			{
				u8 wr : 1;
				u8 wg : 1;
				u8 wb : 1;
				u8 wa : 1;

				u8 _free : 4;
			};
			struct
			{
				u8 wrgba : 4;
			};
			u8 key;
		};
		ColorMaskSelector() : key(0xF) {}
		ColorMaskSelector(u8 c) : key(0) { wrgba = c; }
	};
#pragma pack(pop)
	struct BlendState
	{
		union
		{
			struct
			{
				u8 enable : 1;
				u8 constant_enable : 1;
				u8 op : 6;
				u8 src_factor;
				u8 dst_factor;
				u8 constant;
			};
			u32 key;
		};
		BlendState() : key(0) {}
		BlendState(bool enable_, u8 src_factor_, u8 dst_factor_, u8 op_, bool constant_enable_, u8 constant_)
			: key(0)
		{
			enable = enable_;
			constant_enable = constant_enable_;
			src_factor = src_factor_;
			dst_factor = dst_factor_;
			op = op_;
			constant = constant_;
		}
	};

	Topology topology;  ///< Draw topology

	alignas(8) PSSelector ps;
	VSSelector vs;

	BlendState blend;
	SamplerSelector sampler;
	ColorMaskSelector colormask;
	DepthStencilSelector depth;

	bool require_one_barrier;  ///< Require texture barrier before draw (also used to requst an rt copy if texture barrier isn't supported)
	bool require_full_barrier; ///< Require texture barrier between all prims
};

enum FeedbackLoopFlag : u8
{
	FeedbackLoopFlag_None = 0,
	FeedbackLoopFlag_ReadAndWriteRT = 1,
	FeedbackLoopFlag_ReadDS = 2,
};

struct alignas(8) PipelineSelector
{
	GSHWDrawConfig::PSSelector ps;

	union
	{
		struct
		{
			u32 topology : 2;
			u32 rt : 1;
			u32 ds : 1;
			u32 line_width : 1;
			u32 feedback_loop_flags : 2;
		};

		u32 key;
	};

	GSHWDrawConfig::BlendState bs;
	GSHWDrawConfig::VSSelector vs;
	GSHWDrawConfig::DepthStencilSelector dss;
	GSHWDrawConfig::ColorMaskSelector cms;
	u8 pad;

	__fi bool operator==(const PipelineSelector& p) const { return memcmp(this, &p, sizeof(PipelineSelector)); }
	__fi bool operator!=(const PipelineSelector& p) const { return !memcmp(this, &p, sizeof(PipelineSelector)); }

	__fi PipelineSelector() { std::memset(this, 0, sizeof(*this)); }

	__fi bool IsRTFeedbackLoop() const { return ((feedback_loop_flags & FeedbackLoopFlag_ReadAndWriteRT) != 0); }
	__fi bool IsTestingAndSamplingDepth() const { return ((feedback_loop_flags & FeedbackLoopFlag_ReadDS) != 0); }
};
static_assert(sizeof(PipelineSelector) == 24, "Pipeline selector is 24 bytes");

struct PipelineSelectorHash
{
	std::size_t operator()(const PipelineSelector& e) const noexcept
	{
		std::size_t hash = 0;
		HashCombine(hash, e.vs.key, e.ps.key_hi, e.ps.key_lo, e.dss.key, e.cms.key, e.bs.key, e.key);
		return hash;
	}
};

inline static bool IsDATEModePrimIDInit(u32 flag)
{
	return flag == 1 || flag == 2;
}