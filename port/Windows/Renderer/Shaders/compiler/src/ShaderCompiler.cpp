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
#include <assert.h>
#include "hash.h"
#include "archive.h"
#include "ShaderConfig.h"

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class JobPool {
public:
	JobPool(int numThreads) : stop(false) {
		for (int i = 0; i < numThreads; ++i) {
			workers.emplace_back([this] {
				while (true) {
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(queueMutex);

						// Wait for a task if the queue is empty
						condition.wait(lock, [this] { return stop || !tasks.empty(); });

						// Check if we should stop
						if (stop && tasks.empty()) {
							return;
						}

						// Get the next task from the queue
						task = std::move(tasks.front());
						tasks.pop();
					}

					// Execute the task
					task();
				}
				});
		}
	}

	// Add a task to the job pool
	template <typename Func>
	void addTask(Func&& func) {
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			tasks.emplace(std::forward<Func>(func));
		}

		// Notify one worker thread to pick up the task
		condition.notify_one();
	}

	// Wait for all tasks to finish and stop the worker threads
	void waitAndStop() {
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			stop = true;
		}

		// Notify all worker threads to stop
		condition.notify_all();

		// Join all worker threads
		for (std::thread& worker : workers) {
			worker.join();
		}
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop;
};


std::vector<std::string> gParsedFiles;

std::string shaderOutputDirectoryPS2 = std::string(SHADER_OUTPUT_FOLDER) + "\\ps2";

std::string convertToValidPath(const std::string& path) {
	std::string validPath = path;
	std::replace(validPath.begin(), validPath.end(), '/', '\\');
	std::replace(validPath.begin(), validPath.end(), '\\', '\\');
	return validPath;
}

// Function to convert HLSL to SPIR-V using the DXC compiler
void ConvertToSPIRV(const std::string& configName, const std::string& entryPoint, const std::string& inputName, const std::string& outputName, const std::string& arguments) {

	std::string command = std::string(DXC_COMPILER) + " ";
	command += "-spirv " + std::string(SHADER_SRC_DIR) + " ";
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

class ShaderCompiler {
public:
	MacroSet macroSet;
	std::string type;
	std::string entry;
	std::string extension;
	std::string macroPrefix;

private:
	struct Config {
		std::string name;
		std::string definition;
	};

	void GeneratePermutations(std::vector<Config>& currentConfig,
		std::vector<std::vector<Config>>& allConfigs,
		size_t macroIndex) {
		if (macroIndex == macroSet.size()) {
			// Base case: We have a complete configuration
			allConfigs.push_back(currentConfig);
			return;
		}

		const auto& macro = macroSet[macroIndex];
		for (int value : macro.second) {
			Config config;
			config.name = macroPrefix + "_" + macro.first + "_" + std::to_string(value);
			config.definition = "-D" + macroPrefix + "_" + macro.first + "=" + std::to_string(value);
			currentConfig.push_back(config);

			// Recursively generate configurations for the next macro
			GeneratePermutations(currentConfig, allConfigs, macroIndex + 1);

			// Remove the last added config for backtracking
			currentConfig.pop_back();
		}
	}

	void GeneratePermutations(std::vector<std::vector<Config>>& allConfigs) {
		std::vector<Config> currentConfig;
		GeneratePermutations(currentConfig, allConfigs, 0);
	}

public:
	void Compile() {
		assert(!macroSet.empty());
		assert(!type.empty());
		assert(!entry.empty());
		assert(!extension.empty());
		assert(!macroPrefix.empty());

		JobPool pool(16);

		std::vector<std::vector<Config>> allConfigs;
		GeneratePermutations(allConfigs);

		for (auto& configs : allConfigs) {
			std::stringstream configName;
			std::stringstream configDefinition;

			for (auto& config : configs) {
				configName << config.name;
				configDefinition << config.definition << " ";
			}

			// Compute the MD5 hash for the configuration
			std::string shaderHash = GetMD5String(configName.str());

			std::string configDefString = configDefinition.str();

			pool.addTask([=]() {
				// Convert HLSL to SPIR-V for the current configuration
				ConvertToSPIRV(type.c_str(), entry.c_str(), shaderHash + extension.c_str(), configDefString);
				});

			gParsedFiles.push_back(shaderHash + extension.c_str());
		}

		pool.waitAndStop();

		// Print the generated configurations
		//for (const auto& config : allConfigs) {
		//	for (const Config& entry : config) {
		//		std::cout << "Config name: " << entry.name << ", Definition: " << entry.definition << std::endl;
		//	}
		//	std::cout << "---------------------" << std::endl;
		//}
	}
};

int main() {
	DeleteDirectory(shaderOutputDirectoryPS2);
	CreateDirectory(shaderOutputDirectoryPS2);

	// Vertex
	{
		ShaderCompiler compiler = { VS_MACROS, "vs_6_0", "vs_main", ".vs.spv", "VS" };
		compiler.Compile();
	}

	// Fragment
	{
		ShaderCompiler compiler = { PS_MACROS, "ps_6_0", "ps_main", ".ps.spv", "PS" };
		compiler.Compile();
	}

	// Geometry
	{
		ShaderCompiler compiler = { GS_MACROS, "gs_6_0", "gs_main", ".gs.spv", "GS" };
		compiler.Compile();
	}

	Archive::PackFiles(gParsedFiles, shaderOutputDirectoryPS2 + "\\Shaders_PS2.pack");
	std::vector<char> a;
	Archive::ExtractFile(shaderOutputDirectoryPS2 + "\\Shaders_PS2.pack", gParsedFiles[0], a);

	return 0;
}