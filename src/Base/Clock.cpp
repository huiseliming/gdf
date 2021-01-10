#include "Base/Clock.h"
#include <chrono>
namespace gdf
{
#ifdef _WIN32

std::chrono::steady_clock::time_point ProgramClock::programStartTime;

void ProgramClock::Initialize() noexcept
{
    programStartTime = std::chrono::steady_clock::now();
}

ProgramClock::time_point ProgramClock::now() noexcept
{
    return time_point(std::chrono::steady_clock::now() - programStartTime);
}

ProgramClock::rep ProgramClock::CurrentTime()
{
    return std::chrono::duration_cast<std::chrono::duration<rep, std::ratio<1, 1>>>(std::chrono::steady_clock::now() - programStartTime).count();
}

#endif

} // namespace gdf
