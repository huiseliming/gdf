#pragma once
#include "Renderer/VulkanApi.h"
#include "Base/NonCopyable.h"

namespace gdf
{

class GDF_EXPORT VulkanObject : public NonCopyable
{
public:
    VulkanObject() = delete;
    VulkanObject(VkDevice device) : device_(device)
    {
    }
    VulkanObject(VulkanObject &&rhs)
    {
        device_ = rhs.device_;
        rhs.device_ = VK_NULL_HANDLE;
    }

    VulkanObject& operator=(VulkanObject &&rhs)
    {
        device_ = rhs.device_;
        rhs.device_ = VK_NULL_HANDLE;
        return *this;
    }

protected:
    VkDevice device_;
};


}
