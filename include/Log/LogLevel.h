#pragma once
#include <cstdint>
#define LOG_LEVEL_FATAL LogLevel::Fatal
#define LOG_LEVEL_ERROR LogLevel::Error
#define LOG_LEVEL_WARNING LogLevel::Warning
#define LOG_LEVEL_INFO LogLevel::Info
#define LOG_LEVEL_VERBOSE LogLevel::Verbose

enum class LogLevel : uint8_t
{
    None,
    Fatal,
    Error,
    Warning,
    Info,
    Verbose,
    All = Verbose,
    MaxCount,
};
