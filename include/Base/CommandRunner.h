#pragma once
#include "gdf_export.h"
#include <functional>
#include <mutex>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>


namespace gdf
{

class GDF_EXPORT CommandRunner
{
public:
    bool RegisterCommand(std::string_view commandName, std::function<void (std::string_view commandName)>&& function);

    bool DeregisterCommand(std::string_view commandName);

    bool RunCommand(std::string_view command);

private:
    std::mutex syncMutex_;
    std::unordered_map<std::string, std::function<void (std::string_view)>> commandMap_;
};

}

