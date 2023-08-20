#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include "hash.h"
#include "archive.h"
#include "ShaderConfig.h"

std::vector<std::string> gParsedFiles;

std::string shaderOutputDirectoryPS2 = std::string(SHADER_OUTPUT_FOLDER) + "\\ps2";

std::string convertToValidPath(const std::string& path) {
	std::string validPath = path;
	std::replace(validPath.begin(), validPath.end(), '/', '\\');
	std::replace(validPath.begin(), validPath.end(), '\\', '\\');
	return validPath;
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
		gParsedFiles.push_back(outputName);
	}

	// For this demonstration, let's assume the conversion is successful
	std::cout << "Converting HLSL to SPIR-V for config: " << configName << std::endl;
}

void CreateDirectory(std::string name) {
	try {
		if (std::filesystem::create_directory(name)) {
			std::cout << "Folder created successfully." << std::endl;
		}
		else {
			std::cout << "Folder creation failed. It might already exist." << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error creating the folder: " << e.what() << std::endl;
	}
}

void DeleteDirectory(std::string name) {
	try {
		if (std::filesystem::remove_all(name)) {
			std::cout << "Folder deleted successfully." << std::endl;
		}
		else {
			std::cout << "Folder deletion failed. It might already exist." << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error deleting the folder: " << e.what() << std::endl;
	}
}

int main() {
	DeleteDirectory(shaderOutputDirectoryPS2);
	CreateDirectory(shaderOutputDirectoryPS2);

	ConvertToSPIRV("vs_6_0", "vs_main", "ps2.vs.spv", "");

	for (int PS_ATST : PS_ATST_VALUES) {
		std::stringstream configName;
		configName << "PS_ATST_" << PS_ATST;
		std::string configDefinitions = "-DPS_ATST=" + std::to_string(PS_ATST);

		// Compute the MD5 hash for the configuration
		std::string shaderHash = GetMD5String(configName.str());

		// Convert HLSL to SPIR-V for the current configuration
		ConvertToSPIRV("ps_6_0", "ps_main", shaderHash + ".ps.spv", configDefinitions);
	}

	//ConvertToSPIRV("ps_6_0", "ps_main", "ps2.ps.spv", "");

	// Iterate through all possible configurations
	for (int GS_PRIM : GS_PRIM_VALUES) {
		for (int GS_POINT : GS_POINT_VALUES) {
			for (int GS_LINE : GS_LINE_VALUES) {
				for (int GS_IIP : GS_IIP_VALUES) {
					std::stringstream configName;
					configName << "GS_PRIM_" << GS_PRIM << "_POINT_" << GS_POINT << "_LINE_" << GS_LINE << "_IIP_" << GS_IIP;
					std::string configDefinitions = "-DGS_PRIM=" + std::to_string(GS_PRIM) +
						" -DGS_POINT=" + std::to_string(GS_POINT) +
						" -DGS_LINE=" + std::to_string(GS_LINE) +
						" -DGS_IIP=" + std::to_string(GS_IIP);

					// Compute the MD5 hash for the configuration
					std::string shaderHash = GetMD5String(configName.str());

					// Convert HLSL to SPIR-V for the current configuration
					ConvertToSPIRV("gs_6_0", "gs_main", shaderHash + ".gs.spv", configDefinitions);
				}
			}
		}
	}

	Archive::PackFiles(gParsedFiles, shaderOutputDirectoryPS2 + "\\Shaders_PS2.pack");
	std::vector<char> a;
	Archive::ExtractFile(shaderOutputDirectoryPS2 + "\\Shaders_PS2.pack", gParsedFiles[0], a);

	return 0;
}