#ifndef _POINTER_CONV_H
#define _POINTER_CONV_H

#ifdef PLATFORM_WIN

#include <unordered_map>
#include <optional>
#include <string>

namespace PointerConv
{
	int AddTextureSectionValue(void* value);
	std::optional<void*> ResolveTextureSectionKey(int key);
	void* ResolveTextureSectionKeyChecked(int key);
}

#define STORE_SECTION(a) PointerConv::AddTextureSectionValue(a)
#define LOAD_SECTION(a) PointerConv::ResolveTextureSectionKeyChecked(a)
#else
#define STORE_SECTION(a) (int)(a)
#define LOAD_SECTION(a) a
#endif

#endif //_POINTER_CONV_H
