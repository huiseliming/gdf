#pragma once
#include "Base/Common.h"
#include "gdf_export.h"
#include <chrono>
namespace gdf
{

#ifndef _WIN32

template <typename ClockType, typename Duration>
struct Clock {
    using rep = typename Duration::rep;
    using period = typename Duration::period;
    using duration = Duration;
    using time_point = std::chrono::time_point<Clock, duration>;
    void Update() noexcept
    {
        lastClockTime = currentClockTime;
        currentClockTime = ClockType::now();
    }

    void Reset() noexcept
    {
        currentClockTime = ClockType::now();
        lastClockTime = currentClockTime;
    }

    rep Elapsed() noexcept
    {
        return time_point(currentClockTime - lastClockTime).time_since_epoch().count();
    }

    rep CurrentTime() const noexcept
    {
        return time_point(currentClockTime.time_since_epoch()).time_since_epoch().count();
    }

private:
    typename ClockType::time_point lastClockTime;
    typename ClockType::time_point currentClockTime;

public:
    static void SetProgramStartTime() noexcept;
    static constexpr bool is_steady = ClockType::is_steady;
    static time_point now() noexcept;
    static typename ClockType::time_point programStartTime;
};

template <typename ClockType, typename Duration>
void Clock<ClockType, Duration>::SetProgramStartTime() noexcept
{
    programStartTime = ClockType::now();
}

template <typename ClockType, typename Duration>
typename Clock<ClockType, Duration>::time_point Clock<ClockType, Duration>::now() noexcept
{
    return time_point(ClockType::now() - programStartTime);
}

template <typename ClockType, typename Duration>
typename ClockType::time_point Clock<ClockType, Duration>::programStartTime;

using ProgramClock =
    Clock<std::chrono::steady_clock, std::chrono::duration<double, std::ratio<1, 1>>>;

#else

#endif
} // namespace gdf