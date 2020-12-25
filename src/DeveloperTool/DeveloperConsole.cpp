#include "DeveloperTool\DeveloperConsole.h"
#include "Log/Logger.h"
#include "fmt/core.h"

namespace gdf
{

DEFINE_INTERNAL_LOG_CATEGORY(ConsoleCommand)

bool DeveloperConsole::RunCommand(std::string_view commandCall)
{
    GDF_LOG(ConsoleCommand, LogLevel::Info, commandCall);
    if (!CommandRunner::RunCommand(commandCall)) {
        GDF_LOG(ConsoleCommand, LogLevel::Info, fmt::format("CmdNoRun: {:s}",commandCall));
        return false;
    }
    return true;
}

void DeveloperConsole::Log( const LogCategory &category,
                            const LogLevel level,
                            const std::string_view message)
{
    entries.emplace_back(Entry{const_cast<LogCategory &>(category), level, message});
}

void DeveloperConsole::Exception()
{
}

DeveloperConsole &DeveloperConsole::Instance()
{
    static DeveloperConsole instance;
    return instance;
}

} // namespace gdf