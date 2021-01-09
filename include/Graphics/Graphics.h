#pragma once
#include "Graphics/VulkanApi.h"
#include "Base/Window.h"
#include "Log/Logger.h"
#include "DeviceInfo.h"
#include <mutex>
#include <deque>

#ifdef GDF_DEBUG
#define GDF_ENABLE_VALIDATION_LAYER true
#else
#define GDF_ENABLE_VALIDATION_LAYER false
#endif // GDF_DEBUG

namespace gdf
{
GDF_DECLARE_EXPORT_LOG_CATEGORY(GraphicsLog, LogLevel::Info, LogLevel::All);

struct GDF_EXPORT SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class  GDF_EXPORT Graphics : public NonCopyable
{
public:

    Graphics() = default;
    ~Graphics() = default;
    void Initialize(Window *pWindow = nullptr,
                    bool enableValidationLayer = GDF_ENABLE_VALIDATION_LAYER);
    
    void DrawFrame();

    void Cleanup();

    // Initialize Funtion
    void CreateInstance();
    void CreateDebugReporter();
    void CreateDevice(VkPhysicalDeviceFeatures enabledFeatures,
                      std::vector<const char *> enabledExtensions,
                      VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

    void CreateSwapchain();
    void CreateImageViews();

    // Cleanup Funtion
    
    void DestroyImageViews();
    void DestroySwapchain();
    void DestroyDevice();
    void DestroyDebugReporter();
    void DestroyInstance();

    // Tool Funtion
    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice);

    //helpful function
    VkShaderModule CreateShaderModule(const std::vector<char> &code);

    void DeviceWaitIdle();

    SwapChainSupportDetails QuerySwapChainSupport();
    VkSurfaceFormatKHR GetAvailableFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR GetAvailablePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D GetAvailableExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    // get


    VkQueue graphicsQueue()
    {
        return graphicsQueue_;
    }
    VkQueue computeQueue()
    {
        return computeQueue_;
    }
    VkQueue transferQueue()
    {
        return transferQueue_;
    }
    VkQueue presentQueue()
    {
        return presentQueue_;
    }

    // data
    friend class Swapchain;

    VkInstance instance_{VK_NULL_HANDLE};
    VkDebugReportCallbackEXT fpDebugReportCallbackEXT_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};

    //SwapchainInfo
    Window *pWindow_;
    VkSurfaceKHR surfaceKHR_{VK_NULL_HANDLE};
    VkSurfaceFormatKHR surfaceFormatKHR_;
    VkPresentModeKHR PresentModeKHR_;
    VkSwapchainKHR swapchainKHR_{VK_NULL_HANDLE};

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    uint32_t swapahainImageCount{0};
    std::vector<VkImage> swapahainImages;
    std::vector<VkImageView> swapahainImageViews;



    VkQueue graphicsQueue_{VK_NULL_HANDLE};
    VkQueue computeQueue_{VK_NULL_HANDLE};
    VkQueue transferQueue_{VK_NULL_HANDLE};
    VkQueue presentQueue_{VK_NULL_HANDLE};
    

    bool enableValidationLayer_;
    // Device Infomation
    DeviceInfo deviceInfo_;

private:
public:
    static VkBool32 DebugReportCallbackEXT(VkDebugReportFlagsEXT flags,
                                           VkDebugReportObjectTypeEXT objectType,
                                           uint64_t object,
                                           size_t location,
                                           int32_t messageCode,
                                           const char *pLayerPrefix,
                                           const char *pMessage,
                                           void *pUserData);

private:
};
} // namespace gdf



