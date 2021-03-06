#pragma once
#include <cstdint>
namespace gdf
{
#define LOG_LEVEL_FATAL LogLevel::Fatal
#define LOG_LEVEL_ERROR LogLevel::Error
#define LOG_LEVEL_WARNING LogLevel::Warning
#define LOG_LEVEL_INFO LogLevel::Info
#define LOG_LEVEL_VERBOSE LogLevel::Verbose
#define LOG_LEVEL_DEBUG LogLevel::Debug
#define LOG_LEVEL_ALL LogLevel::All

enum class LogLevel : uint8_t
{
    None,
    Fatal,
    Error,
    Warning,
    Info,
    Verbose,
    Debug = Verbose,
    All = Verbose,
    MaxCount,
};

} // namespace gdf