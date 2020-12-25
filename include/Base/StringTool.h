#pragma once
#include "gdf_export.h"
#include <string>

namespace gdf
{
namespace String
{

GDF_EXPORT std::wstring ConvertWstring(const std::string &in);
GDF_EXPORT std::string ConvertString(const std::wstring &in);

} // namespace String
} // namespace gdf