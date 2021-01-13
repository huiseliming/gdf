#pragma once
#include "Common.h"
#include <vector>
#include <string>

namespace gdf
{

namespace File
{
GDF_EXPORT std::vector<char> ReadBytes(const std::string &filename);

GDF_EXPORT std::string GetExePath();

GDF_EXPORT std::string GetExeDir();

GDF_EXPORT std::string_view Separator();



} // namespace File
} // namespace gdf