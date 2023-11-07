#include "log.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/include/spdlog/sinks/stdout_color_sinks.h"

//LogPtr Log::asyncLog = spdlog::stdout_color_mt("console");
LogPtr Log::asyncLog = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt", true);