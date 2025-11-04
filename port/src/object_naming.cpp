#include "port.h"

#include <cstdarg>
#include <cstdio>
#include <memory>

namespace ObjectNaming {
	std::optional<std::string> gName;
}

void ObjectNaming::SetNextObjectName(const char* format, ...)
{
	// Use a buffer and vsnprintf to handle printf-style arguments
	va_list args;
	va_start(args, format);

	// Determine the size of the buffer needed
	size_t size = std::vsnprintf(nullptr, 0, format, args) + 1;

	// Allocate a buffer
	std::unique_ptr<char[]> buffer(new char[size]);

	// Format the string into the buffer
	std::vsnprintf(buffer.get(), size, format, args);

	va_end(args);

	// Assign the formatted string to gName
	gName = buffer.get();
}

std::string ObjectNaming::GetNextObjectName()
{
	assert(gName.has_value());
	std::string outName = *gName;
	gName.reset();
	return outName;
}

std::string ObjectNaming::CopyObjectName()
{
	return *gName;
}