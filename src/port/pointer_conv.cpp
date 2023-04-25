#include "pointer_conv.h"
#include <assert.h>

#ifdef PLATFORM_WIN
namespace PointerConv
{
	std::vector<void*> TextureSections = { 0 }; // One element to start with, so null lookups don't
	int KeyGenerator = 0;
}

int PointerConv::AddTextureSectionValue(void* value)
{
	TextureSections.push_back(value);
	return TextureSections.size() - 1;
}

std::optional<void*> PointerConv::ResolveTextureSectionKey(int key)
{
	if (key < TextureSections.size())
	{
		return TextureSections[key];
	}

	return std::nullopt; // key not found, return empty optional
}

void* PointerConv::ResolveTextureSectionKeyChecked(int key)
{
	if (key == 0)
	{
		return (void*)0x0;
	}

	std::optional<void*> value = ResolveTextureSectionKey(key);
	assert(value.has_value());
	return *value;
}
#endif
