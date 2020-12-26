#include "Base/Clock.h"
#include <chrono>
namespace gdf
{
#ifdef _WIN32

std::chrono::steady_clock::time_point ProgramClock::programStartTime;

void ProgramClock::SetProgramStartTime() noexcept
{
    std::cout << &programStartTime << std::endl;
    programStartTime = std::chrono::steady_clock::now();
}

ProgramClock::time_point ProgramClock::now() noexcept
{
    std::cout << &programStartTime << std::endl;
    return time_point(std::chrono::steady_clock::now() - programStartTime);
}

#endif

} // namespace gdf
