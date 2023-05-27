#pragma once

#include "VulkanIncludes.h"
#include <vector>
#include <string>

namespace Shader {
	std::vector<char> readFile(const std::string& filename);

	VkShaderModule createShaderModule(const std::vector<char>& code);
}