#pragma once
#include "Base/NonCopyable.h"
#include "Log/Logger.h"
#include "VulkanApi.h"
#include "GraphicsQueue.h"
#include <vector>
#include <optional>
#include "Device.h"

#ifdef GDF_DEBUG
#define GDF_ENABLE_VALIDATION_LAYER true
#else
#define GDF_ENABLE_VALIDATION_LAYER false
#endif // GDF_DEBUG

namespace gdf
{

GDF_DECLARE_EXPORT_LOG_CATEGORY(GraphicsLog, LogLevel::Info, LogLevel::All);

class Swapchain;

class GDF_EXPORT Graphics : public NonCopyable
{
public:
    bool Initialize(bool enableValidationLayer = GDF_ENABLE_VALIDATION_LAYER);

    void Cleanup();

    void SetSwapchain(std::unique_ptr<Swapchain> &&swapchain);

    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice);

    //helpful function
    bool GetSupportPresentQueue(VkSurfaceKHR surface, VkQueue &queue);
    VkShaderModule CreateShaderModule(const std::vector<char> &code);

    void DeviceWaitIdle();

    Device &device()
    {
        return *pDevice_;
    }

    GraphicsQueue &graphicsQueue()
    {
        return *pGraphicsQueue_;
    }

    Swapchain &swapchain()
    {
        return *pSwapchain_;
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
    std::unique_ptr<Device> pDevice_;
    std::unique_ptr<Swapchain> pSwapchain_;
    std::unique_ptr<GraphicsQueue> pGraphicsQueue_;

    VkDebugReportCallbackEXT fpDebugReportCallbackEXT_ = VK_NULL_HANDLE;
    bool enableValidationLayer_;

public:
    static VkInstance vulkanInstance()
    {
        return vulkanInstance_;
    }

private:
    static VkInstance vulkanInstance_;
};
} // namespace gdf