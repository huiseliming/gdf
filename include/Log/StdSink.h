#pragma once
#include "Log/LogSink.h"

namespace gdf
{

class GDF_EXPORT CerrSink : public LogSink
{
public:
    virtual void Log(const LogCategory *category, const LogLevel level, const std::string_view message);

    virtual void Exception();
};

class GDF_EXPORT CoutSink : public LogSink
{
public:
    virtual void Log(const LogCategory *category, const LogLevel level, const std::string_view message);

    virtual void Exception();
};

GDF_EXTERN GDF_EXPORT CerrSink cerrSink;
GDF_EXTERN GDF_EXPORT CoutSink coutSink;

} // namespace gdf
