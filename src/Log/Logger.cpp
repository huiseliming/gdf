#include "Log/Logger.h"
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
Logger::~Logger()
{
    std::cerr << std::flush;
}
} // namespace gdf