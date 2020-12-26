#pragma once
#include "Base/Common.h"
#include "gdf_export.h"
#include <chrono>

namespace gdf
{

template <typename ClockType, typename Duration> 
struct Clock
{
    using rep = typename Duration::rep;
    using period = typename Duration::period;
    using duration = Duration;
    using time_point = std::chrono::time_point<Clock, duration>;

    void Update() noexcept
    {

    }

    void Reset() noexcept
    {

    }

    rep Elapsed() noexcept
    {

    }

    period CurrentTime() const noexcept
    {
    }

    typename Duration LastTimePoint() const noexcept
    {
    }

    typename Duration CurrentTimePoint() const noexcept
    {
        
    }

    static time_point ProgramStartTime() noexcept;

    static void SetProgramStartTime();
    static constexpr bool is_steady = ClockType::is_steady;
    static time_point now() noexcept;
    static typename ClockType::time_point programStartTime;
};

template <typename ClockType, typename Duration>
typename Clock<ClockType, Duration>::time_point 
Clock<ClockType, Duration>::ProgramStartTime() noexcept
{
    return Clock<ClockType, Duration>::time_point(Clock<ClockType, Duration>::programStartTime -
                                                  ClockType::time_point());
}

template <typename ClockType, typename Duration>
void Clock<ClockType, Duration>::SetProgramStartTime()
{
    Clock<ClockType, Duration>::programStartTime = ClockType::now();
}

template <typename ClockType, typename Duration>
typename Clock<ClockType, Duration>::time_point 
Clock<ClockType, Duration>::now() noexcept
{
    return Clock<ClockType, Duration>::time_point(ClockType::now() -
                                                  Clock<ClockType, Duration>::programStartTime);
}

template <typename ClockType, typename Duration>
typename ClockType::time_point
 Clock<ClockType, Duration>::programStartTime;

using ProgramClock = Clock<std::chrono::steady_clock,std::chrono::duration<double, std::ratio<1,1>>>;
GDF_EXPORT ProgramClock;

} // namespace gdf