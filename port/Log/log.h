#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <iostream>
#include <fstream>  // Added for file writing

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
};

struct LogMessageCategory
{
	LogMessage message;
	std::string category;
};

using MessageCategoryQueue = std::deque<LogMessageCategory>;
using MessageQueue = std::deque<LogMessage>;
using MessageMap = std::unordered_map<std::string, MessageQueue>;

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
		if (verboseLevels[level]) {
			MessageQueue& msgQueue = logMessages[category];

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

			{
				consoleLogMessagesTo.push_back({ {message, level}, category });

				if (logMutex.try_lock()) {

					while (!consoleLogMessagesTo.empty()) {
						auto msg = consoleLogMessagesTo.front();
						consoleLogMessagesFrom.push_back(msg);
						consoleLogMessagesTo.pop_front();
					}

					logMutex.unlock();
				}
			}

			msgQueue.push_back({ message, level });
			
			//printf("%s\n", message.c_str());

			if (msgQueue.size() > maxBufferSize) {
				msgQueue.pop_front(); // Remove the oldest log message if the buffer exceeds the maximum size
			}
		}
	}

	const MessageMap& GetLogMessages() const
	{
		return logMessages;
	}

	std::unordered_map<LogLevel, bool>& GetLogVerboseLevels()
	{
		return verboseLevels;
	}

private:
	static constexpr int maxBufferSize = 0x10000; // Maximum size of the circular buffer

	MessageMap logMessages;
	MessageCategoryQueue consoleLogMessagesTo;
	MessageCategoryQueue consoleLogMessagesFrom;

	std::unordered_map<LogLevel, bool> verboseLevels = {
	{ LogLevel::VeryVerbose, true },
	{ LogLevel::Verbose, true },
	{ LogLevel::Info, true },
	{ LogLevel::Warning, true },
	{ LogLevel::Error, true },
	};

	std::mutex logMutex;  // Mutex to synchronize access to log data
	std::unordered_map<std::string, std::unique_ptr<std::ofstream>> logFiles;  // File stream for writing logs
	std::ofstream fullLogFile;  // Single log file stream
	std::thread logThread;  // Thread for flushing logs

	std::atomic<bool> bShutdown = false;

	Log()
	{
		logThread = std::thread(&Log::FlushLogs, this);

		fullLogFile.open("log.txt", std::ios::out | std::ios::trunc);  // Open log file for writing, append mode
		if (!fullLogFile.is_open())
		{
			std::cerr << "Failed to open log file." << std::endl;
		}
	}

	~Log()
	{
		// Notify the log thread to stop and wait for it to join
		{
			std::unique_lock<std::mutex> lock(logMutex);
		}
		bShutdown = true;
		logThread.join();
	}

	void FlushLogs()
	{
		while (true && !bShutdown) {
			// Wait for the log condition to be notified
			std::unique_lock<std::mutex> lock(logMutex);

			// Flush logs
			if (!consoleLogMessagesFrom.empty()) {
				const LogMessageCategory& message = consoleLogMessagesFrom.front();
				//printf("%s\n", message.message.c_str());

				std::string logFileName = message.category + ".txt";

				auto itr = logFiles.find(logFileName);

				if (itr != logFiles.end()) {
					if (itr->second->is_open()) {
						*itr->second << message.message.message << std::endl;  // Write to file
					}
				}
				else {
					auto logFile = std::make_unique<std::ofstream>(logFileName.c_str(), std::ios::out | std::ios::trunc);
					if (logFile->is_open()) {
						*logFile << message.message.message << std::endl;  // Write to file
						logFiles.emplace(logFileName, std::move(logFile));
					}
				}

				if (fullLogFile.is_open()) {
					fullLogFile << message.category << ": " << message.message.message << std::endl;  // Write to file
				}

				consoleLogMessagesFrom.pop_front();
			}
		}
	}

	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
};

#endif // LOG_H
