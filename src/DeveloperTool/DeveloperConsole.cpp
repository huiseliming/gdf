#include "DeveloperTool/DeveloperConsole.h"
#include "Log/Logger.h"
#include "fmt/core.h"

namespace gdf
{

GDF_DEFINE_EXPORT_LOG_CATEGORY(ConsoleCommand)

DeveloperConsole::DeveloperConsole(size_t maxEntries) : maxEntries_(maxEntries)
{
}

bool DeveloperConsole::RunCommand(std::string_view commandCall)
{
    GDF_LOG(ConsoleCommand, LogLevel::Info, fmt::format("CmdRun: {:s}", commandCall));
    if (!CommandRunner::RunCommand(commandCall)) {
        GDF_LOG(ConsoleCommand, LogLevel::Info, fmt::format("CmdNoFind: {:s}", commandCall));
        return false;
    }
    return true;
}

void DeveloperConsole::Log(const LogCategory &category, const LogLevel level, const std::string_view message)
{
    std::scoped_lock<std::mutex> lock(sync_);
    entries_.emplace_back(Entry{const_cast<LogCategory &>(category), level, message});
    if (entries_.size() > maxEntries_) {
        entries_.pop_front();
    }
}

void DeveloperConsole::Exception()
{
}

void DeveloperConsole::maxEntries(size_t maxEntries)
{
    std::scoped_lock<std::mutex> lock(sync_);
    maxEntries_ = maxEntries;
    while (entries_.size() > maxEntries_) {
        entries_.pop_front();
    }
}

DeveloperConsole &DeveloperConsole::Instance()
{
    static DeveloperConsole instance;
    return instance;
}

} // namespace gdf