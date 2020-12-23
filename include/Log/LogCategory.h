#pragma once
#include "Log/LogLevel.h"
#include <string>

namespace gdf
{

class LogCategory
{
public:
    std::string_view name;
    LogLevel minLevel;

protected:
    LogCategory(std::string_view name, LogLevel minLevel) : name(name), minLevel(minLevel)
    {
    }
};

#define LOG_DEFINE_CATEGORY(CATEGORY_NAME, RUNTIME_LEVEL, COMPILER_LEVEL)                          \
    class CATEGORY_NAME : public LogCategory                                                       \
    {                                                                                              \
    public:                                                                                        \
        inline static constexpr std::string_view name = #CATEGORY_NAME;                            \
        inline static constexpr LogLevel compilerLevel = COMPILER_LEVEL;                           \
        CATEGORY_NAME() : LogCategory(#CATEGORY_NAME, RUNTIME_LEVEL)                               \
        {                                                                                          \
        }                                                                                          \
        inline static CATEGORY_NAME &instance()                                                    \
        {                                                                                          \
            static CATEGORY_NAME logCategory;                                                      \
            return logCategory;                                                                    \
        }                                                                                          \
    };
} // namespace gdf