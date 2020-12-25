#pragma once
#include "Base/Common.h"
#include "Log/LogLevel.h"
#include <string>

namespace gdf
{

class GDF_EXPORT LogCategory
{
public:
    std::string_view displayName_;
    LogLevel runtimeLevel_;

protected:
    LogCategory(std::string_view displayName, LogLevel runtimeLevel)
        : displayName_(displayName), runtimeLevel_(runtimeLevel)
    {
    }
};

#define DECLARE_INTERNAL_LOG_CATEGORY(CATEGORY_NAME, RUNTIME_DEFAULT_LEVLE, COMPILER_LEVEL)        \
    class GDF_EXPORT CATEGORY_NAME : public ::gdf::LogCategory                                     \
    {                                                                                              \
    public:                                                                                        \
        static constexpr LogLevel compilerLevel = (COMPILER_LEVEL);                                \
        CATEGORY_NAME(std::string_view displayName = #CATEGORY_NAME,                               \
                      LogLevel runtimeLevel = (RUNTIME_DEFAULT_LEVLE));                            \
        static CATEGORY_NAME &instance();                                                          \
    };
#define DEFINE_INTERNAL_LOG_CATEGORY(CATEGORY_NAME)                                                \
    CATEGORY_NAME &CATEGORY_NAME::instance()                                                       \
    {                                                                                              \
        static CATEGORY_NAME logCategory;                                                          \
        return logCategory;                                                                        \
    }                                                                                              \
    CATEGORY_NAME::CATEGORY_NAME(std::string_view displayName, LogLevel runtimeLevel)              \
        : LogCategory(displayName, runtimeLevel)                                                   \
    {                                                                                              \
    }

#define DECLARE_LOG_CATEGORY(CATEGORY_NAME, RUNTIME_DEFAULT_LEVLE, COMPILER_LEVEL)                 \
    class CATEGORY_NAME : public ::gdf::LogCategory                                                \
    {                                                                                              \
    public:                                                                                        \
        static constexpr LogLevel compilerLevel = (COMPILER_LEVEL);                                \
        CATEGORY_NAME(std::string_view displayName = #CATEGORY_NAME,                               \
                      LogLevel runtimeLevel = (RUNTIME_DEFAULT_LEVLE));                            \
        static CATEGORY_NAME &instance();                                                          \
    };
#define DEFINE_LOG_CATEGORY(CATEGORY_NAME) DEFINE_INTERNAL_LOG_CATEGORY(CATEGORY_NAME)

} // namespace gdf