#include "pointer_conv.h"
#include <cassert>
#include <vector>

#ifdef PLATFORM_WIN
namespace PointerConv
{
	std::vector<void*> Pointers = { 0 }; // One element to start with, so null lookups don't need special handling
}

int PointerConv::AddPointer(void* value)
{
	if (value == nullptr) {
		return 0; // Return 0 for null values
	}

	Pointers.push_back(value);

	if ((Pointers.size() - 1) == 0x0002b79a) {
		// break the debugger
		//__debugbreak();
	}

	return Pointers.size() - 1;
}

std::optional<void*> PointerConv::ResolvePointer(int key)
{
	if (key < Pointers.size())
	{
		return Pointers[key];
	}

	return std::nullopt; // key not found, return empty optional
}

void* PointerConv::ResolvePointerChecked(int key)
{
	if (key == 0)
	{
		return (void*)0x0;
	}

	std::optional<void*> value = ResolvePointer(key);
	assert(value.has_value());
	return *value;
}
#endif
