#include "Base/File.h"
#include "Base/Common.h"
#include <fstream>

#ifdef _WIN32
#include <Windows.h>

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
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
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