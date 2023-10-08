#include "VulkanShader.h"
#include <fstream>
#include <stdexcept>
#include "VulkanRenderer.h"
#include "archive.h"
#include "hash.h"
#include <filesystem>

namespace Shader_Internal {
	std::vector<char> ReadFile(const std::string& filename, bool bFromArchive /*= false*/) {
		//if (bFromArchive) {
		//	if (filename._Starts_with("shaders/ps2/")) {
		//		std::vector<char> buffer;
		//		int a = strlen("shaders/ps2/");
		//		if (Archive::ExtractFile("shaders/ps2/Shaders_PS2.pack", filename.substr(a, filename.length() - a), buffer)) {
		//
		//			return buffer;
		//		}
		//	}
		//}

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

	// Function to convert HLSL to SPIR-V using the DXC compiler
	void ConvertToSPIRV(const std::string& configName, const std::string& entryPoint, const std::string& outputName, const std::string& arguments) {

		std::string command = std::string(DXC_COMPILER) + " ";
		command += "-spirv " + std::string(PS2_SHADER_SRC_DIR) + " ";
		command += "-DSHADER_MODEL=1" + std::string(" ");
		command += "-T " + configName + " ";
		command += "-E " + entryPoint + " ";
		command += "-Fo " + outputName + " ";
		command += "-fspv-debug=vulkan-with-source" + std::string(" ");
		command += arguments;

		int result = std::system(command.c_str());

		if (result == 0) {
			//gParsedFiles.push_back(outputName);
		}

		// For this demonstration, let's assume the conversion is successful
		//std::cout << "Converting HLSL to SPIR-V for config: " << configName << " and arguments: " << arguments<< std::endl;
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

void Shader::CompileShaders(const ShaderDefinitions& shaderDefinitions, std::string& outVsFilename, std::string& outPsFilename, std::string& outGsFilename)
{
	const std::string gsHash = GetMD5String(shaderDefinitions.gsDef);
	const std::string psHash = GetMD5String(shaderDefinitions.psDef);
	const std::string vsHash = GetMD5String(shaderDefinitions.vsDef);

	outGsFilename = "shaders/ps2/" + gsHash + ".gs.spv";
	outPsFilename = "shaders/ps2/" + psHash + ".ps.spv";
	outVsFilename = "shaders/ps2/" + vsHash + ".vs.spv";

	if (!std::filesystem::exists(outGsFilename)) {
		Shader_Internal::ConvertToSPIRV("gs_6_0", "gs_main", outGsFilename, shaderDefinitions.gsDef.c_str());
	}

	if (!std::filesystem::exists(outPsFilename)) {
		Shader_Internal::ConvertToSPIRV("ps_6_0", "ps_main", outPsFilename, shaderDefinitions.psDef.c_str());
	}

	if (!std::filesystem::exists(outVsFilename)) {
		Shader_Internal::ConvertToSPIRV("vs_6_0", "vs_main", outVsFilename, shaderDefinitions.vsDef.c_str());
	}
}
