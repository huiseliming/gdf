#include "Base/ProgramTime.h"
#include <chrono>
namespace gdf
{
std::chrono::steady_clock::time_point ProgramTime::programStartTime;

auto ProgramTime::now() noexcept -> time_point
{
    return time_point(std::chrono::steady_clock::now() - programStartTime);
}
} // namespace gdf
