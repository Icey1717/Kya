#include "log.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/include/spdlog/sinks/stdout_color_sinks.h"

#include <fstream>

static const std::string gLogPath = "logs/";
//constexpr spdlog::level::level_enum gLogLevel = spdlog::level::trace;
constexpr spdlog::level::level_enum gLogLevel = spdlog::level::info;

//LogPtr Log::asyncLog = spdlog::stdout_color_mt("console");
//LogPtr Log::asyncLog = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt", true);
LogPtr Log::asyncLog = spdlog::basic_logger_st("async_file_logger", "logs/async_log.txt", true);

static inline void InitLog(LogPtr pLog) {
	pLog->set_pattern("%v");
	pLog->set_level(gLogLevel);
}

void Log::ForceFlush()
{
	asyncLog->flush();

	for (auto& [cat, log] : logs) {
		log.logPtr->flush();
	}
}

LogPtr Log::CreateLog(const std::string& category)
{
	const std::string path = gLogPath + category + ".txt";
	auto pNewLog = spdlog::basic_logger_mt<spdlog::async_factory_nonblock>(category.c_str(), path.c_str(), true);
	InitLog(pNewLog);
	return pNewLog;
}

Log::Log()
{
	InitLog(asyncLog);
}

static bool LoadCategoryEnabledStatusFromDisk(const std::string& category) {
	// Assume the category enabled status is stored in a file named "category_config.txt"
	// Format of the file: <category_name> <enabled_status>
	// Example: "category1 true"

	std::ifstream configFile("category_config.txt");
	if (!configFile.is_open()) {
		// Handle error: unable to open file
		return true;
	}

	std::string categoryName;
	bool enabledStatus;

	while (configFile >> categoryName >> std::boolalpha >> enabledStatus) {
		if (categoryName == category) {
			configFile.close();
			return enabledStatus;
		}
	}

	// If the category is not found in the config file, assume it is enabled by default
	configFile.close();
	return true;
}

LogEntry::LogEntry(std::string category)
{
	bEnabled = LoadCategoryEnabledStatusFromDisk(category);

	logPtr = Log::CreateLog(category);
}
