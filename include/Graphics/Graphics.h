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

#define MAX_FRAMES_IN_FLIGHT 2

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
    void CreateCommandPool();
    void CreateSwapchain();
    void CreateSwapchainImageViews();
    void CreateDepthResources();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandBuffers();
    void CreateSyncObjects();


    // Cleanup Funtion
    void DestroySyncObjects();
    void DestroyCommandBuffers();
    void DestroyFramebuffers();
    void DestroyGraphicsPipeline();
    void DestroyRenderPass();
    void DestroyDepthResources();
    void DestroySwapchainImageViews();
    void DestroySwapchain();
    void DestroyCommandPool();
    void DestroyDevice();
    void DestroyDebugReporter();
    void DestroyInstance();

    // Tool Funtion
    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice);

    //helpful function
    VkShaderModule CreateShaderModule(const std::string &code);
    VkShaderModule CreateShaderModule(const std::vector<char> &code);

    void CreateImage(uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage &image,
                     VkDeviceMemory &imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);



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
    VkCommandPool commandPool_;


    //SwapchainInfo
    Window *pWindow_;
    VkSurfaceKHR surfaceKHR_{VK_NULL_HANDLE};
    VkSurfaceFormatKHR surfaceFormatKHR_;
    VkPresentModeKHR PresentModeKHR_;
    VkSwapchainKHR swapchainKHR_{VK_NULL_HANDLE};

    // Swapchain Infomation
    VkFormat swapchainImageFormat_;
    VkExtent2D swapchainExtent_;
    uint32_t swapahainImageCount_{0};
    std::vector<VkImage> swapahainImages_;
    std::vector<VkImageView> swapahainImageViews_;
    std::vector<VkFramebuffer> swapahainFramebuffers_;

    //Depth Resource
    VkImage depthImage_{VK_NULL_HANDLE};
    VkImageView depthImageView_{VK_NULL_HANDLE};
    VkDeviceMemory depthImageMemory_{VK_NULL_HANDLE};
    
    // Render Objects
    VkRenderPass renderPass_{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline_{VK_NULL_HANDLE};
    VkPipelineLayout graphicsPipelineLayout_{VK_NULL_HANDLE};

    std::vector<VkCommandBuffer> commandBuffers_;

    // Sync Objects
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    // ref Fence Object wait render finished
    std::vector<VkFence> imagesInFlight; 


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
    static std::string GetShadersPath();

private:
};
} // namespace gdf



