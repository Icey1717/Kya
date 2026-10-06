#include "port.h"
#include "Types.h"

void Renderer::ApplyOptionDepthState(const edpkt_data* pPkt)
{
	// edDListPatchGifTag3D references a packed A+D tag in the VU option buffer.
	// Apply its depth state at execution time, before the next native mesh.
	constexpr uint32_t dmaRefTag = 0x30000000;
	const uint32_t qwc = pPkt->asU32[0] & 0xffff;
	const uint32_t vifCommand = pPkt->asU32[3];
	if ((pPkt->asU32[0] & 0x70000000) == dmaRefTag && qwc != 0 &&
		(vifCommand & 0xff00ffff) == (SCE_VIF1_SET_UNPACK(0x03dc, 0, UNPACK_V4_32, 0) & 0xff00ffff)) {
		const edpkt_data* pOptions = LOAD_POINTER_CAST(edpkt_data*, pPkt->asU32[1]);
		if (pOptions && ((pOptions->cmdA >> 58) & 3) == SCE_GIF_PACKED &&
			(pOptions->cmdA >> 60) == 1 && pOptions->cmdB == SCE_GIF_PACKED_AD) {
			const uint32_t nbRegisters = static_cast<uint32_t>(pOptions->cmdA & 0x7fff);
			if (nbRegisters < qwc) {
				for (uint32_t i = 1; i <= nbRegisters; i++) {
					if (pOptions[i].cmdB == SCE_GS_TEST_1) {
						GIFReg::GSTest test = {};
						test.CMD = pOptions[i].cmdA;
						Renderer::SetTest(test);
					}
					else if (pOptions[i].cmdB == SCE_GS_ZBUF_1) {
						Renderer::SetZbuf(static_cast<uint32_t>((pOptions[i].cmdA >> 32) & 1));
					}
				}
			}
		}
	}
}
