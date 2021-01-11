#include "Base/File.h"
#include "Base/Common.h"
#include <cstdio>
#include <fstream>
#include <sys/syslimits.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#endif

namespace gdf
{
std::vector<char> File::ReadBytes(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        THROW_EXCEPT("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}
std::string File::GetExePath()
{
#ifdef _WIN32
    char result[MAX_PATH];
    return std::string(result, GetModuleFileNameA(NULL, result, MAX_PATH));
#elif __APPLE__
    char result[PATH_MAX];
    uint32_t size = 0;
    assert(0 != _NSGetExecutablePath(nullptr, &size));
    char *buffer = new char[size + 1];
    assert(0 == _NSGetExecutablePath(buffer, &size));
    char buf[PATH_MAX]; /* PATH_MAX incudes the \0 so +1 is not required */
    realpath(buffer, result);
    if (!realpath(buffer, result)) {
        return "";
    }
    return result;
#else
    char result[PATH_MAX];
    size_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#endif
}

std::string File::GetExeDir()
{
    std::string executeFileDir;
    std::string executeFilePath = GetExePath();
    if (!executeFilePath.empty()) {
        std::size_t pos = executeFilePath.rfind(Separator());
        if (pos != std::string::npos) {
            executeFileDir = executeFilePath.substr(0ul, pos);
        }
    }
    return executeFileDir;
}

std::string_view File::Separator()
{
#ifdef _WIN32
    static std::string_view separator = "\\";
#else
    static std::string_view separator = "/";
#endif
    return separator;
}

} // namespace gdf