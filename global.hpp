#pragma once

#include <iomanip>

#define NONE(TEXT)              TEXT
#define BLUE(TEXT)   "\033[34m" TEXT "\033[39m"
#define YELLOW(TEXT) "\033[33m" TEXT "\033[39m"
#define RED(TEXT)    "\033[31m" TEXT "\033[39m"

enum log_level_e : uint8_t
{
	DEBUG = 0,
	WARNING,
	INFO,
	ERROR
};

constexpr uint8_t const LOG_LEVEL = log_level_e::WARNING;

FILE* logfile = stdout;
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOGGER(NAME, ...) if (logfile){ fprintf(logfile, "[" NAME "] " __VA_ARGS__); fprintf(logfile, "\n"); fflush(logfile); }
#define DEBUG(...) { if constexpr (LOG_LEVEL <= log_level_e::DEBUG)   LOGGER("DEBUG",    __VA_ARGS__); }
#define WARN(...)  { if constexpr (LOG_LEVEL <= log_level_e::WARNING) LOGGER(YELLOW("WARNING"),  __VA_ARGS__); }
#define INFO(...)  { if constexpr (LOG_LEVEL <= log_level_e::INFO)    LOGGER(BLUE("INFO"),    __VA_ARGS__);    }
#define ERR(...)   { if constexpr (LOG_LEVEL <= log_level_e::ERROR)   LOGGER(RED("ERROR"),    __VA_ARGS__);   }
#define INT(...)   { LOGGER(RED("ERROR"),    __VA_ARGS__);   exit(1); }

class trace_func {
	const char* m_file;	const char* m_name;
public:
	trace_func(const char* file, const char* name) : m_file{file}, m_name{name}
	{
		// std::time_t time_now = std::time(nullptr);
		// std::stringstream ss;
		// ss << std::put_time(std::localtime(&time_now), "%OH:%OM:%OS");
		DEBUG("%s -> %s:[%d] \\", m_file, m_name, __LINE__);
	}
	~trace_func()
	{
		// std::time_t time_now = std::time(nullptr);
		// std::stringstream ss;
		// ss << std::put_time(std::localtime(&time_now), "%OH:%OM:%OS");
		DEBUG("%s -> %s:[%d] /", m_file, m_name, __LINE__);
	}
};

#define TRACE_FUNCTION() trace_func TRACE_FUNCTION{ __FILENAME__, __PRETTY_FUNCTION__};


constexpr double const   SCALE_FACTOR      = 0.65;
constexpr uint32_t const MAX_RESET_COUNTER = 5;
constexpr uint32_t       DETECTION_PERIOD = 20;