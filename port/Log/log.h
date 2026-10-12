#ifndef LOG_H
#define LOG_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "spdlog/logger.h"

using LogPtr = std::shared_ptr<spdlog::logger>;

struct LogEntry
{
	LogEntry(std::string category);

	LogPtr logPtr;
	bool bEnabled = true;
};

using LogMap = std::unordered_map<std::string, LogEntry>;

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

	LogEntry& GetCategory(const std::string& category) {
		std::lock_guard<std::mutex> lock(categoryMutex);
		return logs.try_emplace(category, category).first->second;
	}

	template <typename... Args>
	void AddLog(const LogLevel level, const std::string& category, const char* format, Args &&...args) {
		AddLog(level, GetCategory(category), format, std::forward<Args>(args)...);
	}

	// The macros check the entry before evaluating arguments and reuse that lookup.
	template <typename... Args>
	void AddLog(const LogLevel level, LogEntry& entry, const char* format, Args &&...args) {
		// Early exit if the log is not enabled.
		if (!entry.bEnabled) {
			return;
		}

		// Log to async log and the specified log.
		// Use fmt::runtime to wrap the runtime format string
		asyncLog->log(LogLevelToSpdLog(level), fmt::runtime(format), std::forward<Args>(args)...);
		entry.logPtr->log(LogLevelToSpdLog(level), fmt::runtime(format), std::forward<Args>(args)...);
	}

	inline LogMap& GetLogs() { return logs; }

	void ForceFlush();
	// Bounded, thread-safe snapshot for the debug UI; existing file logs are unchanged.
	static std::vector<std::string> GetRecentMessages();

	static LogPtr CreateLog(const std::string& category);


private:
	std::mutex categoryMutex;
	Log();
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
};

#endif // LOG_H
