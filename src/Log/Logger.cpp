#include "Log/Logger.h"
#include <mutex>
namespace std
{
std::string to_string(gdf::LogLevel level)
{
    switch (level) {
    case gdf::LogLevel::Fatal:
        return "Fatal";
    case gdf::LogLevel::Error:
        return "Error";
    case gdf::LogLevel::Warning:
        return "Warning";
    case gdf::LogLevel::Info:
        return "Info";
    case gdf::LogLevel::Verbose:
        return "Verbose";
    default:
        break;
    }
    return "None";
}
} // namespace std
namespace gdf
{

void Logger::Log(const LogCategory &category, const LogLevel level, const std::string_view message)
{
    std::scoped_lock<std::mutex> lock{sync};
    for (auto sink : sinks) {
        sink->Log(category, level, message);
    }
    if (level == LogLevel::Fatal) {
        for (auto &sink : sinks) {
            sink->Exception();
        }
    }
}

Logger::~Logger()
{
    std::cerr << std::flush;
}

bool Logger::RegisterSink(LogSink *pSink)
{
    return true;
}

bool Logger::DeregisterSink(LogSink *pSink)
{
    return false;
}

} // namespace gdf