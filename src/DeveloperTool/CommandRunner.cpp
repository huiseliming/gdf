#include "DeveloperTool/CommandRunner.h"
#include <cctype>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace gdf
{

bool CommandRunner::RegisterCommand(std::string_view commandName, std::function<void(std::string_view)> &&function)
{
    std::scoped_lock<std::mutex> lock{syncMutex_};
    std::string cmdString = std::string{commandName};
    if (commandMap_.contains(cmdString))
        return false;
    commandMap_.insert({std::move(cmdString), std::forward<std::function<void(std::string_view)>>(function)});
    return true;
}

bool CommandRunner::DeregisterCommand(std::string_view commandName)
{
    std::scoped_lock<std::mutex> lock{syncMutex_};
    auto searchIt = commandMap_.find(std::string{commandName});
    if (searchIt != commandMap_.end()) {
        commandMap_.erase(searchIt);
        return true;
    }
    return false;
}

bool CommandRunner::RunCommand(std::string_view commandCall)
{
    auto firstNonSpace = std::string::npos;
    for (size_t i = 0; i < commandCall.size(); i++) {
        if (std::isspace(commandCall[i]) == 0) {
            firstNonSpace = i;
            break;
        }
    }
    if (firstNonSpace == std::string::npos)
        return false;
    auto secondNonSpace = std::string::npos;
    for (size_t i = firstNonSpace; i < commandCall.size(); i++) {
        if (std::isspace(commandCall[i]) != 0) {
            secondNonSpace = i;
            break;
        }
    }
    if (secondNonSpace == std::string::npos)
        secondNonSpace = commandCall.size();
    std::string cmdSearchStr = std::string{commandCall.substr(firstNonSpace, secondNonSpace - firstNonSpace)};
    std::scoped_lock<std::mutex> lock{syncMutex_};
    auto searchIt = commandMap_.find(cmdSearchStr);
    auto isFound = searchIt != commandMap_.end();
    if (isFound)
        searchIt->second(commandCall);
    return isFound;
}

} // namespace gdf
