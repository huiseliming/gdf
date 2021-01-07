#include "Base/File.h"
#include "Base/Common.h"
#include <fstream>

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
} // namespace gdf