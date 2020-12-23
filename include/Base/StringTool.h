#pragma once
#include <string>

namespace gdf
{
namespace String
{

std::wstring ConvertWstring(const std::string &in);
std::string ConvertString(const std::wstring &in);

} // namespace String
} // namespace gdf