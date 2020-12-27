#pragma once
#include <chrono>
namespace gdf
{

namespace Template
{

template <typename ClockType, typename Duration>
struct TimeManager {
    using rep = typename Duration::rep;
    using period = typename Duration::period;
    using duration = Duration;
    using time_point = std::chrono::time_point<TimeManager, duration>;

    void Update() noexcept
    {
        previousActualTimePoint_ = currentActualTimePoint_;
        currentActualTimePoint_ = ClockType::now();
        actualElapsedTime_ = (currentActualTimePoint_ - previousActualTimePoint_);
        // calc dilated time
        if (dilation_ == 1.f) {
            dilatedElapsedTime_ = actualElapsedTime_;
            previousDilatedTimePoint_ = currentDilatedTimePoint_;
            currentDilatedTimePoint_ += dilatedElapsedTime_;
        } else if (dilation_ == 0.f) {
            dilatedElapsedTime_ = Duration(0);
        } else {
            if () {
                statements
            }
            dilatedElapsedTime_ = Duration(actualElapsedTime_ * dilation_);
            currentDilatedTimePoint_ += dilatedElapsedTime_;
            bool modifiedTime = false;
        }
    }

    void Reset() noexcept
    {
        startTimePoint_ = ClockType::now();
        currentActualTimePoint_ = startTimePoint_;
        previousActualTimePoint_ = currentActualTimePoint_;
        actualElapsedTime_ = Duration(0);
        previousDilatedTimePoint_ = startTimePoint_;
        currentDilatedTimePoint_ = previousDilatedTimePoint_;
        dilatedElapsedTime_ = Duration(0);
        modifiedDilation = false;
        dilation_ = 1.f;
    }

    rep Elapsed() noexcept
    {
        return time_point(currentActualTimePoint_ - previousActualTimePoint_)
            .time_since_epoch()
            .count();
    }

    rep CurrentTime() const noexcept
    {
        return time_point(currentActualTimePoint_.time_since_epoch()).time_since_epoch().count();
    }

    float dilation()
    {
        return dilation_;
    }

    void dilation(float dilation)
    {
        dilation_ = dilation, modifiedDilation = true;
    }

private:
    typename ClockType::time_point startTimePoint_;
    typename ClockType::time_point previousDilatedTimePoint_;
    typename ClockType::time_point currentDilatedTimePoint_;
    Duration dilatedElapsedTime_;
    bool modifiedDilation;
    float dilation_;

    typename ClockType::time_point previousActualTimePoint_;
    typename ClockType::time_point currentActualTimePoint_;
    Duration actualElapsedTime_;
};

} // namespace Template
// using Timer =
//    Template::Timer<std::chrono::steady_clock, std::chrono::duration<double, std::ratio<1, 1>>>;

} // namespace gdf