#pragma once
#include "Log/LogSink.h"

namespace gdf
{

class CerrSink : public LogSink
{
public:
    virtual void Log(const LogCategory *category, const LogLevel level, const std::string_view message);

    virtual void Exception();
};

class CoutSink : public LogSink
{
public:
    virtual void Log(const LogCategory *category, const LogLevel level, const std::string_view message);

    virtual void Exception();
};

GDF_EXTERN CerrSink cerrSink;
GDF_EXTERN CoutSink coutSink;

} // namespace gdf
