#include "VulkanShader.h"
#include <fstream>
#include <stdexcept>
#include "VulkanRenderer.h"
#include "archive.h"

namespace Shader_Internal {
	std::vector<char> ReadFile(const std::string& filename, bool bFromArchive /*= false*/) {
		if (bFromArchive) {
			if (filename._Starts_with("shaders/ps2/")) {
				std::vector<char> buffer;
				int a = strlen("shaders/ps2/");
				if (Archive::ExtractFile("shaders/ps2/Shaders_PS2.pack", filename.substr(a, filename.length() - a), buffer)) {

					return buffer;
				}
			}
		}

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule CreateModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}
}

Shader::ReflectedModule::ReflectedModule(const std::string& filename, const VkShaderStageFlagBits stage, bool bFromArchive /*= false*/)
{
	auto shaderCode = Shader_Internal::ReadFile(filename, bFromArchive);
	shaderModule = Shader_Internal::CreateModule(shaderCode);
	reflectData = reflectDescriptorSetLayout(shaderCode);

	shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo.stage = stage;
	shaderStageCreateInfo.module = shaderModule;
	shaderStageCreateInfo.pName = reflectData.entryPointname.c_str();
}

Shader::ReflectedModule::~ReflectedModule()
{
	vkDestroyShaderModule(GetDevice(), shaderModule, nullptr);
}
