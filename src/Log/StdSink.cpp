#include "Log/StdSink.h"
#include "Log/Logger.h"
#include <fmt/core.h>
#include <iostream>

namespace gdf
{

CerrSink cerrSink;
CoutSink coutSink;

void CerrSink::Log(const LogCategory &category, const LogLevel level, const std::string_view message)
{
    std::cerr << fmt::format("[{:s}][{:s}] {:s}\n", category.displayName_, std::to_string(level), message);
}

void CerrSink::Exception()
{
    std::cerr << std::flush;
}

void CoutSink::Log(const LogCategory &category, const LogLevel level, const std::string_view message)
{
    std::cout << fmt::format("[{:s}][{:s}] {:s}\n", category.displayName_, std::to_string(level), message);
}

void CoutSink::Exception()
{
    std::cout << std::flush;
}

} // namespace gdf
