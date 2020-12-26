#pragma once
#include <chrono>
namespace gdf
{

namespace Template
{

template <typename ClockType, typename Duration>
struct Timer 
{
    using rep = typename Duration::rep;
    using period = typename Duration::period;
    using duration = Duration;
    using time_point = std::chrono::time_point<Timer, duration>;

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
};

} // namespace Template
using Timer =
    Template::Timer<std::chrono::steady_clock, std::chrono::duration<double, std::ratio<1, 1>>>;


} // namespace gdf