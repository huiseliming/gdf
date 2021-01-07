#pragma once
#include <vector>
#include <string>
namespace gdf
{

namespace File
{
    std::vector<char> ReadBytes(const std::string &filename);
};
} // namespace gdf