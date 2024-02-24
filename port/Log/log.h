#ifndef LOG_H
#define LOG_H

#include <string>
#include <memory>
#include "spdlog/logger.h"

using LogPtr = std::shared_ptr<spdlog::logger>;

using LogMap = std::vector<std::pair<std::string, LogPtr>>;

enum class LogLevel
{
	VeryVerbose,
	Verbose,
	Info,
	Warning,
	Error,
	Max
};

inline std::string LogLevelToString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::VeryVerbose:
		return "Very Verbose";
	case LogLevel::Verbose:
		return "Verbose";
	case LogLevel::Info:
		return "Info";
	case LogLevel::Warning:
		return "Warning";
	case LogLevel::Error:
		return "Error";
	default:
		return "";
	}
}

inline spdlog::level::level_enum LogLevelToSpdLog(LogLevel level)
{
	switch (level)
	{
	case LogLevel::VeryVerbose:
		return spdlog::level::trace;
	case LogLevel::Verbose:
		return spdlog::level::debug;
	case LogLevel::Info:
		return spdlog::level::info;
	case LogLevel::Warning:
		return spdlog::level::warn;
	case LogLevel::Error:
		return spdlog::level::err;
	default:
		return spdlog::level::debug;
	}
}

class Log
{
public:
	static Log& GetInstance()
	{
		static Log instance;
		return instance;
	}

	static LogPtr asyncLog;
	LogMap logs;

	//void AddLog(LogLevel level, const std::string& category, const char* format, ...);

	template <typename... Args>
	void AddLog(const LogLevel level, const std::string& category, const char* format, Args &&...args) {
		//if (category == "ed3D") {
		//	return;
		//}

		asyncLog->log(LogLevelToSpdLog(level), format, std::forward<Args>(args)...);

		for (auto& [cat, log] : logs) {
			if (cat == category) {
				log->log(LogLevelToSpdLog(level), format, std::forward<Args>(args)...);
				return;
			}
		}

		logs.emplace_back(category, CreateLog(category)).second->log(LogLevelToSpdLog(level), format, std::forward<Args>(args)...);
	}

	void ForceFlush();

	LogPtr CreateLog(const std::string& category);


private:
	Log();
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
};

#endif // LOG_H
