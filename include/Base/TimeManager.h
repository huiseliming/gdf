#pragma once
#include <chrono>

#define GDF_TIMEMANAGER_PRESTORE_DATA

namespace gdf
{

namespace Template
{

template <typename ClockType, typename OutputDataType = uint64_t, typename Ratio = std::nano>
struct TimeManager {
    using duration = typename ClockType::duration;
    using rep = typename ClockType::duration::rep;
    using period = typename ClockType::duration::period;
    using time_point = std::chrono::time_point<TimeManager, duration>;

    void Update() noexcept
    {
        previousActualTimePoint_ = currentActualTimePoint_;
        currentActualTimePoint_ = ClockType::now();
        actualElapsedTime_ = (currentActualTimePoint_ - previousActualTimePoint_);
        // calc dilated time
        if (dilation_ == 1.f) {
            dilatedElapsedTime_ = actualElapsedTime_;
            //previousDilatedTimePoint_ = currentDilatedTimePoint_;
            //currentDilatedTimePoint_ += dilatedElapsedTime_;
        } else if (dilation_ == 0.f) {
            dilatedElapsedTime_ = duration(0);
        } else {
            // if modified dilation, start time dilation form previous time point
            if (modifiedDilation_) {
                dilatedStartActualTimePoint_ = previousActualTimePoint_; 
                previousDilatedFromStart = std::chrono::duration_cast<duration>((previousActualTimePoint_ - dilatedStartActualTimePoint_) * dilation_);
                currentDilatedFromStart = std::chrono::duration_cast<duration>((currentActualTimePoint_ - dilatedStartActualTimePoint_) * dilation_);
            } else {
                previousDilatedFromStart = currentDilatedFromStart;
                currentDilatedFromStart = std::chrono::duration_cast<duration>((currentActualTimePoint_ - dilatedStartActualTimePoint_) * dilation_);
            }
            dilatedElapsedTime_ = currentDilatedFromStart - previousDilatedFromStart;
        }
        previousTimePoint_ = currentTimePoint_;
        currentTimePoint_ += dilatedElapsedTime_;
        modifiedDilation_ = false;
#ifdef GDF_TIMEMANAGER_PRESTORE_DATA
        elapsed_ = std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentTimePoint_ - previousTimePoint_) .count();
        currentTime_ = std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentTimePoint_ - startTimePoint_) .count();
        realElapsed_ = std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentActualTimePoint_ - previousActualTimePoint_) .count();
        realCurrentTime_ = std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentActualTimePoint_ - actualStartTimePoint_) .count();
#endif // GDF_TIMEMANAGER_PRESTORE_DATA
    }

    void Reset() noexcept
    {
        actualStartTimePoint_ = ClockType::now();
        currentActualTimePoint_ = actualStartTimePoint_;
        previousActualTimePoint_ = currentActualTimePoint_;
        actualElapsedTime_ = duration(0);

        dilatedStartActualTimePoint_ = actualStartTimePoint_;
        //previousDilatedTimePoint_ = previousActualTimePoint_;
        //currentDilatedTimePoint_ = currentActualTimePoint_;
        dilatedElapsedTime_ = duration(0);
        modifiedDilation_ = false;
        dilation_ = 1.f;

        startTimePoint_ = actualStartTimePoint_;
        previousTimePoint_ = previousActualTimePoint_;
        currentTimePoint_ = currentActualTimePoint_;
        elapsedTime_ = duration(0);
    }

    OutputDataType Elapsed() noexcept
    {
#ifdef GDF_TIMEMANAGER_PRESTORE_DATA
        return elapsed_;
#else
        return std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentTimePoint_ - previousTimePoint_) .count();
#endif // GDF_TIMEMANAGER_PRESTORE_DATA
    }

    OutputDataType CurrentTime() const noexcept
    {
#ifdef GDF_TIMEMANAGER_PRESTORE_DATA
        return currentTime_;
#else
        return std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentTimePoint_ - startTimePoint_) .count();
#endif // GDF_TIMEMANAGER_PRESTORE_DATA
    }

    OutputDataType RealElapsed() noexcept
    {
#ifdef GDF_TIMEMANAGER_PRESTORE_DATA
        return realElapsed_;
#else
        return std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentActualTimePoint_ - previousActualTimePoint_) .count();
#endif // GDF_TIMEMANAGER_PRESTORE_DATA
    }

    OutputDataType RealCurrentTime() const noexcept
    {
#ifdef GDF_TIMEMANAGER_PRESTORE_DATA
        return realCurrentTime_;
#else
        return std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( currentActualTimePoint_ - actualStartTimePoint_) .count();
#endif // GDF_TIMEMANAGER_PRESTORE_DATA
    }
    OutputDataType RealCurrentTimePoint()
    {
        return std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>(  currentActualTimePoint_).count();
    }

    OutputDataType RealPreviousTimePoint()
    {
        return std::chrono::duration_cast<std::chrono::duration<OutputDataType,Ratio>>( actualStartTimePoint_) .count();
    }

    float dilation()
    {
        return dilation_;
    }

    void dilation(float dilation)
    {
        dilation_ = dilation;
        modifiedDilation_ = true;
    }

private:
    typename ClockType::time_point dilatedStartActualTimePoint_; 
    duration previousDilatedFromStart;
    duration currentDilatedFromStart;
    duration dilatedElapsedTime_;
    bool modifiedDilation_;
    float dilation_;

    typename ClockType::time_point actualStartTimePoint_; 
    typename ClockType::time_point previousActualTimePoint_;
    typename ClockType::time_point currentActualTimePoint_;
    duration actualElapsedTime_;

        
    typename ClockType::time_point startTimePoint_;
    typename ClockType::time_point previousTimePoint_;
    typename ClockType::time_point currentTimePoint_;
    duration elapsedTime_;
#ifdef GDF_TIMEMANAGER_PRESTORE_DATA
    OutputDataType elapsed_;
    OutputDataType currentTime_;
    OutputDataType realElapsed_;
    OutputDataType realCurrentTime_;
#endif // GDF_TIMEMANAGER_PRESTORE_DATA
};

} // namespace Template

using TimeManager = Template::TimeManager<std::chrono::steady_clock, double, std::ratio<1>>;

} // namespace gdf