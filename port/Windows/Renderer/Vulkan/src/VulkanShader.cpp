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
		if (vkCreateShaderModule(GetDevice(), &createInfo, GetAllocator(), &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	// Function to convert HLSL to SPIR-V using the DXC compiler
	void ConvertToSPIRV(const std::string& configName, const std::string& entryPoint, const std::string& inputName, const std::string& outputName, const std::string& arguments) {

		std::string command = std::string(DXC_COMPILER) + " ";
		command += "-spirv " + inputName + " ";
		command += "-DSHADER_MODEL=1" + std::string(" ");
		command += "-T " + configName + " ";
		command += "-E " + entryPoint + " ";
		command += "-Fo " + outputName + " ";
		command += "-fspv-debug=vulkan-with-source" + std::string(" ");
		command += arguments + " ";
		command += "> shader_out.txt";

		const int result = std::system(command.c_str());
		assert(result == 0);
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
	vkDestroyShaderModule(GetDevice(), shaderModule, GetAllocator());
}

namespace {
	long long ReadLastWriteTimeFromFile(const std::string& filePath) {
		std::ifstream file(filePath + ".lastWriteTime");
		if (file.is_open()) {
			long long lastWriteTime;
			file >> lastWriteTime;
			file.close();

			std::chrono::seconds durationInSeconds(lastWriteTime);
			return lastWriteTime;
		}
		else {
			// Return a very early time if the file doesn't exist yet
			return 0;
		}
	}

	void SaveLastWriteTime(const std::string& filePath, long long seconds) {
		std::ofstream file(filePath + ".lastWriteTime");
		file << seconds;
		file.close();
	}
}

void Shader::CompileShader(const std::string& configName, const std::string& entryPoint, const std::string& inputName, const std::string& outputPath, const std::string& arguments)
{
	const std::string inputPath = std::string(SHADER_SRC_DIR) + inputName;

	if (!std::filesystem::exists(outputPath)) {
		Shader_Internal::ConvertToSPIRV(configName, entryPoint, inputPath, outputPath, arguments);
	}
	else {
		// File exists, check if it has been modified
		auto lastWriteTime = std::filesystem::last_write_time(inputPath);
		auto timePoint = lastWriteTime.time_since_epoch();
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();

		auto prevLastWriteTime = ReadLastWriteTimeFromFile(outputPath);

		// Compare the last modification time with the current time
		if (seconds > prevLastWriteTime) {
			// The file has been modified since the last compilation
			Shader_Internal::ConvertToSPIRV(configName, entryPoint, inputPath, outputPath, arguments);
			SaveLastWriteTime(outputPath, seconds);
		}
	}
}

void Shader::PS2::CompileShaders(const ShaderDefinitions& shaderDefinitions, std::string& outVsFilename, std::string& outPsFilename, std::string& outGsFilename)
{
	const std::string gsHash = GetMD5String(shaderDefinitions.gsDef);
	const std::string psHash = GetMD5String(shaderDefinitions.psDef);
	const std::string vsHash = GetMD5String(shaderDefinitions.vsDef);

	outGsFilename = "shaders/ps2/" + gsHash + ".gs.spv";
	outPsFilename = "shaders/ps2/" + psHash + ".ps.spv";
	outVsFilename = "shaders/ps2/" + vsHash + ".vs.spv";

	CompileShader("gs_6_0", "gs_main", "ps2/ps2.hlsl", outGsFilename, shaderDefinitions.gsDef);
	CompileShader("ps_6_0", "ps_main", "ps2/ps2.hlsl", outPsFilename, shaderDefinitions.psDef);
	CompileShader("vs_6_0", "vs_main", "ps2/ps2.hlsl", outVsFilename, shaderDefinitions.vsDef);
}
