#include "pointer_conv.h"
#include <assert.h>

#ifdef PLATFORM_WIN
namespace PointerConv
{
	std::unordered_map<int, void*> TextureSectionMap;
	int KeyGenerator = 0;
}

int PointerConv::AddTextureSectionValue(void* value)
{
	TextureSectionMap.try_emplace(++KeyGenerator, value);
	return KeyGenerator;
}

std::optional<void*> PointerConv::ResolveTextureSectionKey(int key)
{
	auto iter = TextureSectionMap.find(key);
	if (iter == TextureSectionMap.end()) {
		return std::nullopt; // key not found, return empty optional
	}
	else {
		return iter->second; // key found, return optional containing value
	}
}

void* PointerConv::ResolveTextureSectionKeyChecked(int key)
{
	std::optional<void*> value = ResolveTextureSectionKey(key);
	assert(value.has_value());
	return *value;
}
#endif
