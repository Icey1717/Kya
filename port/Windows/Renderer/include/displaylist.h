#pragma once

/* Drawing functions for dealing with draws built up from bespoke display lists */

namespace Renderer
{
	struct SimpleTexture;

	namespace DisplayList
	{
		void Begin2D(short viewportWidth, short viewportHeight, uint32_t mode);

		void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, float q);
		void SetTexCoord(float s, float t);
		void SetVertex(float x, float y, float z, uint32_t skip);

		void End2D();

		void BindTexture(SimpleTexture* pNewTexture);
	}
}

#ifdef PLATFORM_WIN
#define DISPLAY_LIST_2D_BEGIN(viewportWidth, viewportHeight, mode) Renderer::DisplayList::Begin2D(viewportWidth, viewportHeight, mode)

#define DISPLAY_LIST_SET_COLOR(r, g, b, a, q) Renderer::DisplayList::SetColor(r, g, b, a, q)
#define DISPLAY_LIST_SET_TEXCOORD(s, t) Renderer::DisplayList::SetTexCoord(s, t)
#define DISPLAY_LIST_SET_VERTEX(x, y, z, skip) Renderer::DisplayList::SetVertex(x, y, z, skip)

#define DISPLAY_LIST_2D_END(...) Renderer::DisplayList::End2D()
#define DISPLAY_LIST_BIND_TEXTURE(pTexture) Renderer::DisplayList::BindTexture(pTexture)

#else
#define DISPLAY_LIST_2D_BEGIN(...)
#define DISPLAY_LIST_SET_COLOR(r, g, b, a, q)
#define DISPLAY_LIST_SET_TEXCOORD(s, t)
#define DISPLAY_LIST_SET_VERTEX(x, y, z, skip)
#define DISPLAY_LIST_2D_END(...)
#define DISPLAY_LIST_BIND_TEXTURE(pTexture)
#endif // PLATFORM_WIN