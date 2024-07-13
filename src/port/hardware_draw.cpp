#include "hardware_draw.h"
#include "port.h"
#include "renderer.h"

namespace HardwareRenderer
{
}

void HardwareRenderer::ProcessVertices(char* vtxStart)
{
	Gif_Tag gifTag;
	gifTag.setTag((u8*)vtxStart, true);

	const uint primReg = gifTag.tag.PRIM;
	const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);
	Renderer::SetPrim(primPacked);

	vtxStart += 0x10;

	for (int i = 0; i < gifTag.nLoop; i++) {
		Renderer::GSVertexUnprocessed vtx;
		memcpy(&vtx.STQ, vtxStart, sizeof(vtx.STQ));
		memcpy(&vtx.RGBA, vtxStart + 0x10, sizeof(vtx.RGBA));
		memcpy(&vtx.XYZFlags, vtxStart + 0x20, sizeof(vtx.XYZFlags));

		const uint vtxAnimMatrix = ((vtx.XYZFlags.flags & 0x7ff) - 0x3dc) / 4;

		const uint skip = vtx.XYZFlags.flags & 0x8000;

		const uint shiftedStripIndex = /*stripIndex*/ 0 << 16;

		vtx.XYZFlags.flags |= shiftedStripIndex;

		Renderer::KickVertex(vtx, primPacked, skip, Renderer::GetHardwareDrawBuffer());

		vtxStart += 0x30;
	};
}
