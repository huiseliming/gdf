#pragma once
#include "LogCategory.h"

namespace gdf
{
class LogSink
{
public:
    virtual ~LogSink() = default;
    virtual void Log(const LogCategory *category, const LogLevel level, const std::string_view message) = 0;
    virtual void Exception() = 0;
};

} // namespace gdf
