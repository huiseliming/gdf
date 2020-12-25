#pragma once
#include "Base/Common.h"
#include "gdf_export.h"
#include <chrono>
#include <ctime>

namespace gdf
{

struct GDF_EXPORT ProgramTime {
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<ProgramTime, duration>;
    static const bool is_steady = true;
    static auto now() noexcept -> time_point;
    static std::chrono::steady_clock::time_point programStartTime;
};

} // namespace gdf