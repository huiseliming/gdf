#pragma once
#include "Base/Singleton.h"
#include "LogCategory.h"
#include <fmt/core.h>
#include <iostream>

namespace std
{
std::string to_string(gdf::LogLevel);
}

#define LOG(CATEGOTY, LOG_LEVEL, MESSAGE, ...)                                                     \
    if constexpr (static_cast<int>(LOG_LEVEL) <= static_cast<int>(CATEGOTY::compilerLevel))        \
    Logger::instance().Log(CATEGOTY::instance(), LOG_LEVEL, MESSAGE, ##__VA_ARGS__)

namespace gdf
{
class Logger : public Singleton<Logger>
{
public:
    ~Logger();
    template <typename... Args>
    void Log(const LogCategory &category,
             LogLevel level,
             const std::string_view message,
             Args &&...args)
    {
        std::cerr << "[" << category.displayName_ << "][" << std::to_string(level) << "]"
                  << fmt::format(message, std::forward<Args>(args)...) << "\n";
    }
};
} // namespace gdf