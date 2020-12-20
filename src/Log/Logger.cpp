#include "Log/Logger.h"
namespace std
{
std::string to_string(LogLevel level)
{
    switch (level) {
    case LogLevel::Fatal:
        return "Fatal  ";
    case LogLevel::Error:
        return "Error  ";
    case LogLevel::Warning:
        return "Warning";
    case LogLevel::Info:
        return "Info   ";
    case LogLevel::Verbose:
        return "Verbose";
    default:
        break;
    }
    return "None";
}
} // namespace std

Logger::~Logger()
{
    std::cerr << std::flush;
}
