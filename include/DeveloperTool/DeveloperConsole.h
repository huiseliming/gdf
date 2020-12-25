#pragma once
#include "DeveloperTool/CommandRunner.h"
#include "Log/LogSink.h"
#include "Log/LogLevel.h"
#include "Log/LogCategory.h"
#include <deque>

namespace gdf
{

DECLARE_INTERNAL_LOG_CATEGORY(ConsoleCommand, LogLevel::All, LogLevel::All)

class GDF_EXPORT DeveloperConsole : public CommandRunner ,public LogSink
{
public:

    bool RunCommand(std::string_view commandCall);

    // Interface class LogSink implement
    virtual void Log(const LogCategory &category,
                     const LogLevel level,
                     const std::string_view message);
    virtual void Exception();

    DeveloperConsole &Instance();

private:
    struct Entry {
        LogCategory &category;
        LogLevel level;
        std::string_view message;
    };

    std::deque<Entry> entries;
};

}








































