#pragma once

#include "VulkanIncludes.h"
#include "VulkanReflect.h"
#include <vector>
#include <string>

namespace Shader {
	struct ShaderDefinitions {
		std::string gsDef;
		std::string psDef;
		std::string vsDef;
	};

	void CompileShader(const std::string& configName, const std::string& entryPoint, const std::string& inputName, const std::string& outputName, const std::string& arguments);
	
	namespace PS2 {
		void CompileShaders(const ShaderDefinitions& shaderDefinitions, std::string& outVsFilename, std::string& outPsFilename, std::string& outGsFilename);
	}

	struct ReflectedModule {
		ReflectedModule(const std::string& filename, const VkShaderStageFlagBits stage, bool bFromArchive = false);

		~ReflectedModule();

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
		ReflectData reflectData;
		VkShaderModule shaderModule;
	};
}