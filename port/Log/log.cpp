#include "log.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/include/spdlog/sinks/stdout_color_sinks.h"

const std::string logPath = "logs/";

//LogPtr Log::asyncLog = spdlog::stdout_color_mt("console");
LogPtr Log::asyncLog = spdlog::basic_logger_mt<spdlog::async_factory_nonblock>("async_file_logger", "logs/async_log.txt", true);

void Log::ForceFlush()
{
	asyncLog->flush();

	for (auto& [cat, log] : logs) {
		log->flush();
	}
}

LogPtr Log::CreateLog(const std::string& category)
{
	const std::string path = logPath + category + ".txt";
	return spdlog::basic_logger_mt<spdlog::async_factory_nonblock>(category.c_str(), path.c_str(), true);
}

Log::Log()
{
	asyncLog->set_pattern("%v");
}
