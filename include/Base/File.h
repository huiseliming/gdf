#pragma once
#include <vector>
#include <string>
namespace gdf
{

class File
{
public:
    static std::vector<char> ReadBytes(const std::string &filename);
};
} // namespace gdf