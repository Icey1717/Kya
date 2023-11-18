#ifndef LOG_H
#define LOG_H

#include <string>
#include <memory>
#include "spdlog/logger.h"

using LogPtr = std::shared_ptr<spdlog::logger>;

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
		return spdlog::level::info;
	case LogLevel::Verbose:
		return spdlog::level::info;
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

	//void AddLog(LogLevel level, const std::string& category, const char* format, ...);

	template <typename... Args>
	void AddLog(const LogLevel level, const std::string& category, const char* format, Args &&...args) {
		asyncLog->log(LogLevelToSpdLog(level), format, std::forward<Args>(args)...);
	}

	void ForceFlush();


private:
	Log();
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
};

#endif // LOG_H
