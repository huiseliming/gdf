#pragma once
#include "Base/NonCopyable.h"
#include "Log/Logger.h"
#include "VulkanApi.h"
#include <vector>

#ifdef GDF_DEBUG
#define GDF_ENABLE_VALIDATION_LAYER true
#else
#define GDF_ENABLE_VALIDATION_LAYER false
#endif // GDF_DEBUG

namespace gdf
{

GDF_DECLARE_EXPORT_LOG_CATEGORY(GraphicsLog, LogLevel::Info, LogLevel::All);

class Graphics;
struct Swapchain;

struct GDF_EXPORT QueueFamily {
    VkQueueFlags flag;
    uint32_t family;
    std::vector<VkQueue> queue;

    friend class Graphics;

private:
    void Attach(VkDevice device, VkQueueFlags queueFlags, uint32_t queueFamilyIndex, uint32_t queueNum)
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
    bool Initialize(bool enableValidationLayer = GDF_ENABLE_VALIDATION_LAYER);

    void Cleanup();

    void SetSwapchain(std::unique_ptr<Swapchain> &&swapchain);

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

    Swapchain &swapchain()
    {
        return *swapchain_;
    }

    static VkBool32 DebugReportCallbackEXT(VkDebugReportFlagsEXT flags,
                                           VkDebugReportObjectTypeEXT objectType,
                                           uint64_t object,
                                           size_t location,
                                           int32_t messageCode,
                                           const char *pLayerPrefix,
                                           const char *pMessage,
                                           void *pUserData);

private:
    VkPhysicalDeviceFeatures physicalDeviceFeatures_ = {};
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::vector<QueueFamily> queueFamilies_;

    std::unique_ptr<Swapchain> swapchain_;
    VkDebugReportCallbackEXT fpDebugReportCallbackEXT_ = VK_NULL_HANDLE;
    bool enableValidationLayer_;
};
} // namespace gdf