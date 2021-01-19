#pragma once
#include "Common.h"
#include <string>
#include <vector>


namespace gdf
{

namespace File
{
std::vector<char> ReadBytes(const std::string &filename);

std::string GetExePath();

std::string GetExeDir();

std::string_view Separator();

} // namespace File
} // namespace gdf