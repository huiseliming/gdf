#pragma once
#include "Base/NonCopyable.h"
#include "VulkanApi.h"
#include <vector>

namespace gdf
{

class Graphics;

struct GDF_EXPORT QueueFamily {
    VkQueueFlags flag;
    uint32_t family;
    std::vector<VkQueue> queue;

    friend class Graphics;

private:
    void Attach(VkDevice device,
                VkQueueFlags queueFlags,
                uint32_t queueFamilyIndex,
                uint32_t queueNum)
    {
        this->flag = queueFlags;
        this->family = queueFamilyIndex;
        queue.resize(queueNum);
        for (uint32_t i = 0; i < queueNum; i++)
            vkGetDeviceQueue(device, queueFamilyIndex, i, &queue[i]);
    }
};

class GDF_EXPORT Graphics : public NonCopyable
{
public:
    bool Initialize();

    void Cleanup();

    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice);

    VkInstance instance()
    {
        return instance_;
    }

    VkPhysicalDevice physicalDevice()
    {
        return physicalDevice_;
    }

    VkDevice device()
    {
        return device_;
    }

private:
    VkPhysicalDeviceFeatures physicalDeviceFeatures_ = {};
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::vector<QueueFamily> queueFamilies;
};
} // namespace gdf