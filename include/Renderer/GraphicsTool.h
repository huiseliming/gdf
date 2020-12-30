#pragma once
#include "Renderer/VulkanApi.h"
#include <vector>

namespace gdf
{
namespace GraphicsTool
{
VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char> &code);

};
}




















