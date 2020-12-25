#pragma once
#include "Base/Common.h"
namespace gdf
{
class GDF_EXPORT NonCopyable
{
private:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;

protected:
    NonCopyable() = default;
    virtual ~NonCopyable() = default;
};
} // namespace gdf