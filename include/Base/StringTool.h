#pragma once
#include <string>

namespace String
{

std::wstring ConvertWstring(const std::string &in);
std::string ConvertString(const std::wstring &in);

} // namespace String
