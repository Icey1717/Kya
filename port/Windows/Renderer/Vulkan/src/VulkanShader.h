#pragma once

#include "VulkanIncludes.h"
#include "VulkanReflect.h"
#include <vector>
#include <string>

namespace Shader {
	struct ReflectedModule {
		ReflectedModule(const std::string& filename, const VkShaderStageFlagBits stage, bool bFromArchive = false);

		~ReflectedModule();

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
		ReflectData reflectData;
		VkShaderModule shaderModule;
	};
}