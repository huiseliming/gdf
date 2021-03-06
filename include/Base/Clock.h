#pragma once
#include "Base/Common.h"
#include <chrono>
#include <iostream>

namespace gdf
{

#ifndef _WIN32

template <typename ClockType, typename Duration>
struct Clock {
    using rep = typename Duration::rep;
    using period = typename Duration::period;
    using duration = Duration;
    using time_point = std::chrono::time_point<Clock, duration>;

public:
    static void Initialize() noexcept;
    static time_point now() noexcept;
    static rep CurrentTime();
    static constexpr bool is_steady = ClockType::is_steady;
    static typename ClockType::time_point programStartTime;
};

template <typename ClockType, typename Duration>
void Clock<ClockType, Duration>::Initialize() noexcept
{
    programStartTime = ClockType::now();
}

template <typename ClockType, typename Duration>
typename Clock<ClockType, Duration>::time_point Clock<ClockType, Duration>::now() noexcept
{
    return time_point(ClockType::now() - programStartTime);
}
template <typename ClockType, typename Duration>
typename Clock<ClockType, Duration>::rep Clock<ClockType, Duration>::CurrentTime()
{
    return std::chrono::duration_cast<std::chrono::duration<rep, std::ratio<1, 1>>>(std::chrono::steady_clock::now() -
                                                                                    programStartTime)
        .count();
}

template <typename ClockType, typename Duration>
typename ClockType::time_point Clock<ClockType, Duration>::programStartTime;

using ProgramClock = Clock<std::chrono::steady_clock, std::chrono::duration<double, std::ratio<1, 1>>>;

#else
struct ProgramClock {
    using rep = double;
    using period = std::ratio<1, 1>;
    using duration = std::chrono::duration<double, std::ratio<1, 1>>;
    using time_point = std::chrono::time_point<ProgramClock, duration>;

public:
    static void Initialize() noexcept;
    static time_point now() noexcept;
    static rep CurrentTime();

    static constexpr bool is_steady = std::chrono::steady_clock::is_steady;
    static typename std::chrono::steady_clock::time_point programStartTime;
};
#endif

} // namespace gdf