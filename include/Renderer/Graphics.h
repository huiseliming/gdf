#pragma once
#include "Base/NonCopyable.h"
#include "Log/Logger.h"
#include "VulkanApi.h"
#include <vector>
#include <optional>

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
    uint32_t familyIndex;
    std::vector<VkQueue> queue;

    friend class Graphics;
private:
    void Attach(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueNum)
    {
        this->familyIndex = queueFamilyIndex;
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

    //helpful function
    VkShaderModule CreateShaderModule(const std::vector<char> &code);

    void DeviceWaitIdle();

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

    VkQueue graphicsQueue()
    {
        return queueFamilies_[queueFamilyIndices_.graphics].queue[0];
    }

    VkQueue computeQueue()
    {
        return queueFamilies_[queueFamilyIndices_.compute].queue[0];
    }

    VkQueue transferQueue()
    {
        return queueFamilies_[queueFamilyIndices_.transfer].queue[0];
    }

    VkQueue presentQueue()
    {
        return queueFamilies_[queueFamilyIndices_.present].queue[0];
    }

    Swapchain &swapchain()
    {
        return *swapchain_;
    }

    VkCommandPool commandPool()
    {
        return commandPool_;
    }

    static VkBool32 DebugReportCallbackEXT(VkDebugReportFlagsEXT flags,
                                           VkDebugReportObjectTypeEXT objectType,
                                           uint64_t object,
                                           size_t location,
                                           int32_t messageCode,
                                           const char *pLayerPrefix,
                                           const char *pMessage,
                                           void *pUserData);
    friend class Swapchain;

private:
    VkPhysicalDeviceFeatures physicalDeviceFeatures_ = {};
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::unique_ptr<Swapchain> swapchain_;
    VkCommandPool commandPool_;

    std::vector<QueueFamily> queueFamilies_;
    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
        uint32_t present;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        bool HasGraphics()
        {
            return graphics != UINT32_MAX;
        }
        bool HasCompute()
        {
            return compute != UINT32_MAX;
        }
        bool HasTransfer()
        {
            return transfer != UINT32_MAX;
        }
        bool HasPresent()
        {
            return present != UINT32_MAX;
        }

        void DetectQueueFamilyIndices(std::vector<VkQueueFamilyProperties> &queueFamilyProperties);
        bool DetectPresentQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        uint32_t GetQueueFamilyIndex(std::vector<VkQueueFamilyProperties> &queueFamilyProperties, VkQueueFlagBits queueFlags);
    } queueFamilyIndices_;

    VkDebugReportCallbackEXT fpDebugReportCallbackEXT_ = VK_NULL_HANDLE;
    bool enableValidationLayer_;
};
} // namespace gdf