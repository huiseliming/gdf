#pragma once
#include "Base/Singleton.h"
#include "Log/LogSink.h"
#include "LogCategory.h"
#include <fmt/core.h>
#include <iostream>
#include <mutex>
#include <vector>

namespace std
{
GDF_EXPORT std::string to_string(gdf::LogLevel);
}

#define GDF_LOG(CATEGOTY, LOG_LEVEL, MESSAGE, ...)                                                 \
    if constexpr (static_cast<int>(LOG_LEVEL) <= static_cast<int>(CATEGOTY::compilerLevel)) {      \
        if (LOG_LEVEL <= CATEGOTY::instance().runtimeLevel_) {                                     \
            ::gdf::Logger::instance().Log(                                                         \
                CATEGOTY::instance(), LOG_LEVEL, MESSAGE, ##__VA_ARGS__);                          \
        }                                                                                          \
    }

namespace gdf
{
class GDF_EXPORT Logger : public Singleton<Logger>
{
public:
    ~Logger();

    void Log(const LogCategory &category, const LogLevel level, const std::string_view message);

    template <typename... Args>
    void Log(const LogCategory &category,
             const LogLevel level,
             const std::string_view message,
             Args &&...args)
    {
        Log(category, level, fmt::format(message, std::forward<Args>(args)...));
    }

    bool RegisterSink(LogSink *pSink);
    bool DeregisterSink(LogSink *pSink);

private:
    std::mutex sync;
    std::vector<LogSink *> sinks;
};

template class GDF_EXPORT Singleton<Logger>;

} // namespace gdf