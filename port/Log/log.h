#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdarg>
#include <cstdio>

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


struct LogMessage
{
	std::string message;
	LogLevel level;
	std::string category;
};

class Log
{
public:
	static Log& GetInstance()
	{
		static Log instance;
		return instance;
	}

	void AddLog(LogLevel level, const std::string& category, const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		// Format the variadic arguments into a temporary buffer
		const int bufferSize = std::vsnprintf(nullptr, 0, format, args) + 1;
		std::vector<char> buffer(bufferSize);
		std::vsnprintf(buffer.data(), bufferSize, format, args);

		va_end(args);

		// Construct the log message using std::ostringstream
		std::ostringstream oss;
		//oss << "Category: " << category << " | Level: " << static_cast<int>(level) << " | ";
		oss << buffer.data(); // Append the formatted buffer to the std::ostringstream

		std::string message = oss.str();

		logMessages.push_back({ message, level, category });
		logCategories.insert(category);

		if (verboseLevels[level]) {
			printf("%s", message.c_str());
		}
	}

	const std::vector<LogMessage>& GetLogMessages() const
	{
		return logMessages;
	}

	const std::unordered_set<std::string>& GetLogCategories() const
	{
		return logCategories;
	}

	std::unordered_map<LogLevel, bool>& GetLogVerboseLevels()
	{
		return verboseLevels;
	}

private:
	std::vector<LogMessage> logMessages;
	std::unordered_set<std::string> logCategories;
	std::unordered_map<LogLevel, bool> verboseLevels = {
	{ LogLevel::VeryVerbose, false },
	{ LogLevel::Verbose, false },
	{ LogLevel::Info, true },
	{ LogLevel::Warning, true },
	{ LogLevel::Error, true },
	};

	Log() = default;
	~Log() = default;
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
};

#endif // LOG_H
