#pragma once
#include "Base/Common.h"
#include "gdf_export.h"
#include <chrono>
#include <ctime>

namespace gdf
{
template <typename TimeType, typename Rep, typename Period>
struct GDF_EXPORT ProgramTime {
    using rep = Rep;
    using period = Period;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<ProgramTime, duration>;

    void Update() noexcept
    {
    }

    void Reset() noexcept
    {
    }

    Rep Elapsed() noexcept
    {
    }

    Rep CurrentTime() const noexcept
    {
    }

    typename TimeType::time_point LastTimePoint() const noexcept
    {
    }

    typename TimeType::time_point CurrentTimePoint()const noexcept
    {
        
    }

    static const bool is_steady = true;
    static auto now() noexcept -> time_point;
    static std::chrono::steady_clock::time_point programStartTime;
};

} // namespace gdf