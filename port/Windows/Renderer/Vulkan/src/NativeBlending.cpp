#include "NativeBlending.h"

#include <array>

#include "VulkanRenderer.h"
#include "renderer.h"

namespace Renderer
{
	namespace Native
	{

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

		enum BlendFactor : uint8_t
		{
			// HW blend factors
			SRC_COLOR, INV_SRC_COLOR, DST_COLOR, INV_DST_COLOR,
			SRC1_COLOR, INV_SRC1_COLOR, SRC_ALPHA, INV_SRC_ALPHA,
			DST_ALPHA, INV_DST_ALPHA, SRC1_ALPHA, INV_SRC1_ALPHA,
			CONST_COLOR, INV_CONST_COLOR, CONST_ONE, CONST_ZERO,
		};

		enum BlendOp : uint8_t
		{
			// HW blend operations
			OP_ADD, OP_SUBTRACT, OP_REV_SUBTRACT
		};

		// Determines the HW blend function for DX11/OGL
		struct HWBlend { uint16_t flags, op, src, dst; };

		const std::array<HWBlend, 3 * 3 * 3 * 3> m_blendMap =
		{ {
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0000: (Cs - Cs)*As + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 0001: (Cs - Cs)*As + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 0002: (Cs - Cs)*As +  0 ==> 0
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0010: (Cs - Cs)*Ad + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 0011: (Cs - Cs)*Ad + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 0012: (Cs - Cs)*Ad +  0 ==> 0
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0020: (Cs - Cs)*F  + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 0021: (Cs - Cs)*F  + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 0022: (Cs - Cs)*F  +  0 ==> 0
			{ BLEND_A_MAX | BLEND_MIX2 , OP_SUBTRACT     , CONST_ONE       , SRC1_ALPHA}      , // 0100: (Cs - Cd)*As + Cs ==> Cs*(As + 1) - Cd*As
			{ BLEND_MIX1               , OP_ADD          , SRC1_ALPHA      , INV_SRC1_ALPHA}  , // 0101: (Cs - Cd)*As + Cd ==> Cs*As + Cd*(1 - As)
			{ BLEND_MIX1               , OP_SUBTRACT     , SRC1_ALPHA      , SRC1_ALPHA}      , // 0102: (Cs - Cd)*As +  0 ==> Cs*As - Cd*As
			{ BLEND_A_MAX              , OP_SUBTRACT     , CONST_ONE       , DST_ALPHA}       , // 0110: (Cs - Cd)*Ad + Cs ==> Cs*(Ad + 1) - Cd*Ad
			{ 0                        , OP_ADD          , DST_ALPHA       , INV_DST_ALPHA}   , // 0111: (Cs - Cd)*Ad + Cd ==> Cs*Ad + Cd*(1 - Ad)
			{ 0                        , OP_SUBTRACT     , DST_ALPHA       , DST_ALPHA}       , // 0112: (Cs - Cd)*Ad +  0 ==> Cs*Ad - Cd*Ad
			{ BLEND_A_MAX | BLEND_MIX2 , OP_SUBTRACT     , CONST_ONE       , CONST_COLOR}     , // 0120: (Cs - Cd)*F  + Cs ==> Cs*(F + 1) - Cd*F
			{ BLEND_MIX1               , OP_ADD          , CONST_COLOR     , INV_CONST_COLOR} , // 0121: (Cs - Cd)*F  + Cd ==> Cs*F + Cd*(1 - F)
			{ BLEND_MIX1               , OP_SUBTRACT     , CONST_COLOR     , CONST_COLOR}     , // 0122: (Cs - Cd)*F  +  0 ==> Cs*F - Cd*F
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0200: (Cs -  0)*As + Cs ==> Cs*(As + 1)
			{ BLEND_ACCU               , OP_ADD          , SRC1_ALPHA      , CONST_ONE}       , // 0201: (Cs -  0)*As + Cd ==> Cs*As + Cd
			{ BLEND_NO_REC             , OP_ADD          , SRC1_ALPHA      , CONST_ZERO}      , // 0202: (Cs -  0)*As +  0 ==> Cs*As
			{ BLEND_A_MAX              , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0210: (Cs -  0)*Ad + Cs ==> Cs*(Ad + 1)
			{ BLEND_HW_CLR3            , OP_ADD          , DST_ALPHA       , CONST_ONE}       , // 0211: (Cs -  0)*Ad + Cd ==> Cs*Ad + Cd
			{ BLEND_HW_CLR3            , OP_ADD          , DST_ALPHA       , CONST_ZERO}      , // 0212: (Cs -  0)*Ad +  0 ==> Cs*Ad
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 0220: (Cs -  0)*F  + Cs ==> Cs*(F + 1)
			{ BLEND_ACCU               , OP_ADD          , CONST_COLOR     , CONST_ONE}       , // 0221: (Cs -  0)*F  + Cd ==> Cs*F + Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_COLOR     , CONST_ZERO}      , // 0222: (Cs -  0)*F  +  0 ==> Cs*F
			{ BLEND_MIX3               , OP_ADD          , INV_SRC1_ALPHA  , SRC1_ALPHA}      , // 1000: (Cd - Cs)*As + Cs ==> Cd*As + Cs*(1 - As)
			{ BLEND_A_MAX | BLEND_MIX1 , OP_REV_SUBTRACT , SRC1_ALPHA      , CONST_ONE}       , // 1001: (Cd - Cs)*As + Cd ==> Cd*(As + 1) - Cs*As
			{ BLEND_MIX1               , OP_REV_SUBTRACT , SRC1_ALPHA      , SRC1_ALPHA}      , // 1002: (Cd - Cs)*As +  0 ==> Cd*As - Cs*As
			{ 0                        , OP_ADD          , INV_DST_ALPHA   , DST_ALPHA}       , // 1010: (Cd - Cs)*Ad + Cs ==> Cd*Ad + Cs*(1 - Ad)
			{ BLEND_A_MAX              , OP_REV_SUBTRACT , DST_ALPHA       , CONST_ONE}       , // 1011: (Cd - Cs)*Ad + Cd ==> Cd*(Ad + 1) - Cs*Ad
			{ 0                        , OP_REV_SUBTRACT , DST_ALPHA       , DST_ALPHA}       , // 1012: (Cd - Cs)*Ad +  0 ==> Cd*Ad - Cs*Ad
			{ BLEND_MIX3               , OP_ADD          , INV_CONST_COLOR , CONST_COLOR}     , // 1020: (Cd - Cs)*F  + Cs ==> Cd*F + Cs*(1 - F)
			{ BLEND_A_MAX | BLEND_MIX1 , OP_REV_SUBTRACT , CONST_COLOR     , CONST_ONE}       , // 1021: (Cd - Cs)*F  + Cd ==> Cd*(F + 1) - Cs*F
			{ BLEND_MIX1               , OP_REV_SUBTRACT , CONST_COLOR     , CONST_COLOR}     , // 1022: (Cd - Cs)*F  +  0 ==> Cd*F - Cs*F
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 1100: (Cd - Cd)*As + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 1101: (Cd - Cd)*As + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 1102: (Cd - Cd)*As +  0 ==> 0
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 1110: (Cd - Cd)*Ad + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 1111: (Cd - Cd)*Ad + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 1112: (Cd - Cd)*Ad +  0 ==> 0
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 1120: (Cd - Cd)*F  + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 1121: (Cd - Cd)*F  + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 1122: (Cd - Cd)*F  +  0 ==> 0
			{ 0                        , OP_ADD          , CONST_ONE       , SRC1_ALPHA}      , // 1200: (Cd -  0)*As + Cs ==> Cs + Cd*As
			{ BLEND_HW_CLR1            , OP_ADD          , DST_COLOR       , SRC1_ALPHA}      , // 1201: (Cd -  0)*As + Cd ==> Cd*(1 + As)
			{ BLEND_HW_CLR2            , OP_ADD          , DST_COLOR       , SRC1_ALPHA}      , // 1202: (Cd -  0)*As +  0 ==> Cd*As
			{ 0                        , OP_ADD          , CONST_ONE       , DST_ALPHA}       , // 1210: (Cd -  0)*Ad + Cs ==> Cs + Cd*Ad
			{ BLEND_HW_CLR1            , OP_ADD          , DST_COLOR       , DST_ALPHA}       , // 1211: (Cd -  0)*Ad + Cd ==> Cd*(1 + Ad)
			{ 0                        , OP_ADD          , CONST_ZERO      , DST_ALPHA}       , // 1212: (Cd -  0)*Ad +  0 ==> Cd*Ad
			{ 0                        , OP_ADD          , CONST_ONE       , CONST_COLOR}     , // 1220: (Cd -  0)*F  + Cs ==> Cs + Cd*F
			{ BLEND_HW_CLR1            , OP_ADD          , DST_COLOR       , CONST_COLOR}     , // 1221: (Cd -  0)*F  + Cd ==> Cd*(1 + F)
			{ BLEND_HW_CLR2            , OP_ADD          , DST_COLOR       , CONST_COLOR}     , // 1222: (Cd -  0)*F  +  0 ==> Cd*F
			{ BLEND_NO_REC             , OP_ADD          , INV_SRC1_ALPHA  , CONST_ZERO}      , // 2000: (0  - Cs)*As + Cs ==> Cs*(1 - As)
			{ BLEND_ACCU               , OP_REV_SUBTRACT , SRC1_ALPHA      , CONST_ONE}       , // 2001: (0  - Cs)*As + Cd ==> Cd - Cs*As
			{ BLEND_NO_REC             , OP_REV_SUBTRACT , SRC1_ALPHA      , CONST_ZERO}      , // 2002: (0  - Cs)*As +  0 ==> 0 - Cs*As
			{ 0                        , OP_ADD          , INV_DST_ALPHA   , CONST_ZERO}      , // 2010: (0  - Cs)*Ad + Cs ==> Cs*(1 - Ad)
			{ BLEND_HW_CLR3            , OP_REV_SUBTRACT , DST_ALPHA       , CONST_ONE}       , // 2011: (0  - Cs)*Ad + Cd ==> Cd - Cs*Ad
			{ 0                        , OP_REV_SUBTRACT , DST_ALPHA       , CONST_ZERO}      , // 2012: (0  - Cs)*Ad +  0 ==> 0 - Cs*Ad
			{ BLEND_NO_REC             , OP_ADD          , INV_CONST_COLOR , CONST_ZERO}      , // 2020: (0  - Cs)*F  + Cs ==> Cs*(1 - F)
			{ BLEND_ACCU               , OP_REV_SUBTRACT , CONST_COLOR     , CONST_ONE}       , // 2021: (0  - Cs)*F  + Cd ==> Cd - Cs*F
			{ BLEND_NO_REC             , OP_REV_SUBTRACT , CONST_COLOR     , CONST_ZERO}      , // 2022: (0  - Cs)*F  +  0 ==> 0 - Cs*F
			{ 0                        , OP_SUBTRACT     , CONST_ONE       , SRC1_ALPHA}      , // 2100: (0  - Cd)*As + Cs ==> Cs - Cd*As
			{ 0                        , OP_ADD          , CONST_ZERO      , INV_SRC1_ALPHA}  , // 2101: (0  - Cd)*As + Cd ==> Cd*(1 - As)
			{ 0                        , OP_SUBTRACT     , CONST_ZERO      , SRC1_ALPHA}      , // 2102: (0  - Cd)*As +  0 ==> 0 - Cd*As
			{ 0                        , OP_SUBTRACT     , CONST_ONE       , DST_ALPHA}       , // 2110: (0  - Cd)*Ad + Cs ==> Cs - Cd*Ad
			{ 0                        , OP_ADD          , CONST_ZERO      , INV_DST_ALPHA}   , // 2111: (0  - Cd)*Ad + Cd ==> Cd*(1 - Ad)
			{ 0                        , OP_SUBTRACT     , CONST_ZERO      , DST_ALPHA}       , // 2112: (0  - Cd)*Ad +  0 ==> 0 - Cd*Ad
			{ 0                        , OP_SUBTRACT     , CONST_ONE       , CONST_COLOR}     , // 2120: (0  - Cd)*F  + Cs ==> Cs - Cd*F
			{ 0                        , OP_ADD          , CONST_ZERO      , INV_CONST_COLOR} , // 2121: (0  - Cd)*F  + Cd ==> Cd*(1 - F)
			{ 0                        , OP_SUBTRACT     , CONST_ZERO      , CONST_COLOR}     , // 2122: (0  - Cd)*F  +  0 ==> 0 - Cd*F
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 2200: (0  -  0)*As + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 2201: (0  -  0)*As + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 2202: (0  -  0)*As +  0 ==> 0
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 2210: (0  -  0)*Ad + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 2211: (0  -  0)*Ad + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 2212: (0  -  0)*Ad +  0 ==> 0
			{ BLEND_NO_REC             , OP_ADD          , CONST_ONE       , CONST_ZERO}      , // 2220: (0  -  0)*F  + Cs ==> Cs
			{ BLEND_CD                 , OP_ADD          , CONST_ZERO      , CONST_ONE}       , // 2221: (0  -  0)*F  + Cd ==> Cd
			{ BLEND_NO_REC             , OP_ADD          , CONST_ZERO      , CONST_ZERO}      , // 2222: (0  -  0)*F  +  0 ==> 0
		} };

		const std::array<uint8_t, 16> m_replaceDualSrcBlendMap =
		{ {
			SRC_COLOR,        // SRC_COLOR
			INV_SRC_COLOR,    // INV_SRC_COLOR
			DST_COLOR,        // DST_COLOR
			INV_DST_COLOR,    // INV_DST_COLOR
			SRC_COLOR,        // SRC1_COLOR
			INV_SRC_COLOR,    // INV_SRC1_COLOR
			SRC_ALPHA,        // SRC_ALPHA
			INV_SRC_ALPHA,    // INV_SRC_ALPHA
			DST_ALPHA,        // DST_ALPHA
			INV_DST_ALPHA,    // INV_DST_ALPHA
			SRC_ALPHA,        // SRC1_ALPHA
			INV_SRC_ALPHA,    // INV_SRC1_ALPHA
			CONST_COLOR,      // CONST_COLOR
			INV_CONST_COLOR,  // INV_CONST_COLOR
			CONST_ONE,        // CONST_ONE
			CONST_ZERO        // CONST_ZERO
		} };

		static HWBlend GetBlend(uint32_t index, bool replace_dual_src)
		{
			HWBlend ret = m_blendMap[index];
			if (replace_dual_src)
			{
				ret.src = m_replaceDualSrcBlendMap[ret.src];
				ret.dst = m_replaceDualSrcBlendMap[ret.dst];
			}
			return ret;
		}
	} // Native
} // Renderer


void Renderer::Native::SetBlendingDynamicState(const SimpleTexture* pTexture, const SimpleMesh* pMesh, const VkCommandBuffer& cmd)
{
	static auto pvkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdSetColorBlendEnableEXT");
	assert(pvkCmdSetColorBlendEnableEXT);

	static auto pvkCmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdSetColorBlendEquationEXT");
	assert(pvkCmdSetColorBlendEquationEXT);

	const bool bEnableAlpha = pMesh->GetPrim().ABE;

	VkBool32 bEnableAlphaVk = bEnableAlpha ? VK_TRUE : VK_FALSE;
	pvkCmdSetColorBlendEnableEXT(cmd, 0, 1, &bEnableAlphaVk);

	if (bEnableAlpha) {
		const auto alpha = pTexture->GetTextureRegisters().alpha;

		uint8_t blend_index = static_cast<uint8_t>(((alpha.A * 3 + alpha.B) * 3 + alpha.C) * 3 + alpha.D);
		const HWBlend blend = GetBlend(blend_index, true);

		static constexpr std::array<VkBlendFactor, 16> vk_blend_factors = { {
			VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
			VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_FACTOR_SRC1_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
			VK_BLEND_FACTOR_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
		} };
		static constexpr std::array<VkBlendOp, 3> vk_blend_ops = { {
				VK_BLEND_OP_ADD, VK_BLEND_OP_SUBTRACT, VK_BLEND_OP_REVERSE_SUBTRACT
		} };

		VkColorBlendEquationEXT colorBlendEquation;
		colorBlendEquation.srcColorBlendFactor = vk_blend_factors[blend.src];
		colorBlendEquation.dstColorBlendFactor = vk_blend_factors[blend.dst];
		colorBlendEquation.colorBlendOp = vk_blend_ops[blend.op];

		colorBlendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendEquation.alphaBlendOp = VK_BLEND_OP_ADD;

		pvkCmdSetColorBlendEquationEXT(cmd, 0, 1, &colorBlendEquation);
	}
}