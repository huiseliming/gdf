#pragma once
#include "DeveloperTool/CommandRunner.h"
#include "Log/LogCategory.h"
#include "Log/LogLevel.h"
#include "Log/LogSink.h"
#include <deque>

namespace gdf
{

GDF_DECLARE_EXPORT_LOG_CATEGORY(ConsoleCommand, LogLevel::All, LogLevel::All)

class GDF_EXPORT DeveloperConsole : public CommandRunner, public LogSink
{
public:
    DeveloperConsole(size_t maxEntries = 1024);

    bool RunCommand(std::string_view commandCall);

    // Interface class LogSink implement
    virtual void Log(const LogCategory *category, const LogLevel level, const std::string_view message);
    virtual void Exception();

    size_t maxEntries()
    {
        return maxEntries_;
    }

    void maxEntries(size_t maxEntries);

    DeveloperConsole &Instance();

private:
    struct Entry {
        const LogCategory *category;
        LogLevel level;
        std::string_view message;
    };

    size_t maxEntries_;
    std::mutex sync_;
    std::deque<Entry> entries_;
};

} // namespace gdf
