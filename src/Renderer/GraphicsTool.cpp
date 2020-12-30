#include "Renderer/GraphicsTool.h"



namespace gdf
{
namespace GraphicsTool
{
// helpful function
VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VK_ASSERT_SUCCESSED(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule))
    return shaderModule;
}







}
}


