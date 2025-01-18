#pragma once

/* Drawing functions for dealing with draws built up from bespoke display lists */

namespace Renderer
{
	namespace DisplayList
	{
		void Begin2D();
		void End2D();
	}
}

#ifdef PLATFORM_WIN
#define DISPLAY_LIST_2D_START(...) Renderer::DisplayList::Begin2D()
#define DISPLAY_LIST_2D_END(...) Renderer::DisplayList::End2D()
#else
#define DISPLAY_LIST_2D_START(...)
#define DISPLAY_LIST_2D_END(...)
#endif // PLATFORM_WIN