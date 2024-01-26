#include "log.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/include/spdlog/sinks/stdout_color_sinks.h"

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
		log->flush();
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
