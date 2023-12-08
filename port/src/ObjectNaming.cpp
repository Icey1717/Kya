#include "port.h"

namespace ObjectNaming {
	std::optional<std::string> gName;
}

void ObjectNaming::SetNextObjectName(const char* newName)
{
	gName = newName;
}

std::string ObjectNaming::GetNextObjectName()
{
	assert(gName.has_value());
	std::string outName = *gName;
	gName.reset();
	return outName;
}