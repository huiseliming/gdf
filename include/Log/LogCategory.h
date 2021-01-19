#pragma once
#include "Base/Common.h"
#include "Log/LogLevel.h"
#include <string>

namespace gdf
{

class LogCategory
{
public:
    std::string_view displayName_;
    LogLevel runtimeLevel_;

protected:
    LogCategory(std::string_view displayName, LogLevel runtimeLevel) : displayName_(displayName), runtimeLevel_(runtimeLevel)
    {
    }
};

#define GDF_DECLARE_EXPORT_LOG_CATEGORY(CATEGORY_NAME, RUNTIME_DEFAULT_LEVLE, COMPILER_LEVEL)                                  \
    class CATEGORY_NAME : public ::gdf::LogCategory                                                                            \
    {                                                                                                                          \
    public:                                                                                                                    \
        static constexpr LogLevel compilerLevel = (COMPILER_LEVEL);                                                            \
        CATEGORY_NAME(std::string_view displayName = #CATEGORY_NAME, LogLevel runtimeLevel = (RUNTIME_DEFAULT_LEVLE));         \
        static CATEGORY_NAME *instance();                                                                                      \
    };
#define GDF_DEFINE_EXPORT_LOG_CATEGORY(CATEGORY_NAME)                                                                          \
    CATEGORY_NAME *CATEGORY_NAME::instance()                                                                                   \
    {                                                                                                                          \
        static CATEGORY_NAME logCategory;                                                                                      \
        return &logCategory;                                                                                                   \
    }                                                                                                                          \
    CATEGORY_NAME::CATEGORY_NAME(std::string_view displayName, LogLevel runtimeLevel) : LogCategory(displayName, runtimeLevel) \
    {                                                                                                                          \
    }

#define GDF_DECLARE_LOG_CATEGORY(CATEGORY_NAME, RUNTIME_DEFAULT_LEVLE, COMPILER_LEVEL)                                         \
    class CATEGORY_NAME : public ::gdf::LogCategory                                                                            \
    {                                                                                                                          \
    public:                                                                                                                    \
        static constexpr LogLevel compilerLevel = (COMPILER_LEVEL);                                                            \
        CATEGORY_NAME(std::string_view displayName = #CATEGORY_NAME, LogLevel runtimeLevel = (RUNTIME_DEFAULT_LEVLE));         \
        static CATEGORY_NAME *instance();                                                                                      \
    };
#define GDF_DEFINE_LOG_CATEGORY(CATEGORY_NAME) GDF_DEFINE_EXPORT_LOG_CATEGORY(CATEGORY_NAME)

} // namespace gdf