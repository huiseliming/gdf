#pragma once
#include "Base/Window.h"
#include "Graphics/VulkanApi.h"
#include "Log/Logger.h"
#include "VulkanDevice.h"
#include <deque>
#include <mutex>

#ifdef GDF_DEBUG
#define GDF_ENABLE_VALIDATION_LAYER true
#else
#define GDF_ENABLE_VALIDATION_LAYER false
#endif // GDF_DEBUG

#define MAX_FRAMES_IN_FLIGHT 2
struct ImDrawData;

namespace gdf
{
GDF_DECLARE_EXPORT_LOG_CATEGORY(GraphicsLog, LogLevel::Info, LogLevel::All);

struct GDF_EXPORT SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class GDF_EXPORT Graphics : public NonCopyable
{
public:
    Graphics() = default;
    ~Graphics() = default;
    void Initialize(Window *pWindow = nullptr, bool enableValidationLayer = GDF_ENABLE_VALIDATION_LAYER);

    void FrameBegin();
    void DrawFrame();
    void FrameEnd();

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
    void FreeCommandBuffers();
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

    // Recreate

    void RecreateSwapchain();
    void RequireRecreateSwapchain(bool required)
    {
        RequireRecreateSwapchain_ = required;
    }
    bool RequireRecreateSwapchain_{false};

    // ImGui
    void ImGuiCreate();
    void ImGuiDestroy();
    void ImGuiResourceCreate();
    void ImGuiResourceDestroy();
    void ImGuiFrameBegin();
    void ImGuiFrameRender(uint32_t imageIndex);
    void ImGuiFrameEnd();
    void ImGuiCreateDescriptorPool();
    void ImGuiCreateRenderPass();
    void ImGuiCreateFramebuffer();
    void ImGuiCreateCommandBuffer();
    void ImGuiUploadFonts();
    void ImGuiUpdateMinImageCount(uint32_t minImageCount);
    static void ImGuiCheckVkResultCallback(VkResult result);

    ImDrawData *imGuiDrawData_{nullptr};
    VkAllocationCallbacks *imguiAllocator_{nullptr};
    VkPipelineCache imguiPipelineCache_{VK_NULL_HANDLE};
    VkDescriptorPool imguiDescriptorPool_{VK_NULL_HANDLE};
    VkRenderPass imguiRenderPass_{VK_NULL_HANDLE};
    std::vector<VkFramebuffer> imguiFramebuffers_;
    std::vector<VkCommandBuffer> imguiCommandBuffers_;

    // Tool Funtion
    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice);

    // helpful function
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

    // Command Helper
    VkCommandBuffer BeginSingleTimeCommand();
    void EndSingleTimeCommand(VkCommandBuffer commandBuffer);

    //
    void DeviceWaitIdle();

    SwapChainSupportDetails QuerySwapChainSupport();
    VkSurfaceFormatKHR GetAvailableFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR GetAvailablePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D GetAvailableExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    // get

    VkQueue graphicsQueue()
    {
        return device_.graphicsQueue_;
    }
    VkQueue computeQueue()
    {
        return device_.computeQueue_;
    }
    VkQueue transferQueue()
    {
        return device_.transferQueue_;
    }
    VkQueue presentQueue()
    {
        return device_.presentQueue_;
    }

    friend class Swapchain;
    // data

    VkInstance instance_{VK_NULL_HANDLE};
    VkDebugReportCallbackEXT fpDebugReportCallbackEXT_{VK_NULL_HANDLE};
    VulkanDevice device_;
    VkCommandPool commandPool_;

    // SwapchainInfo
    Window *pWindow_;
    VkSurfaceKHR surfaceKHR_{VK_NULL_HANDLE};
    VkSurfaceFormatKHR surfaceFormatKHR_;
    VkPresentModeKHR presentModeKHR_;
    VkSwapchainKHR swapchainKHR_{VK_NULL_HANDLE};

    // Swapchain Infomation
    VkFormat swapchainImageFormat_;
    VkExtent2D swapchainExtent_;
    uint32_t swapchainMinImageCount_{0};
    uint32_t swapchainImageCount_{0};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> swapchainFramebuffers_;

    // Depth Resource
    VkImage depthImage_{VK_NULL_HANDLE};
    VkImageView depthImageView_{VK_NULL_HANDLE};
    VkDeviceMemory depthImageMemory_{VK_NULL_HANDLE};

    // Render Objects
    VkRenderPass renderPass_{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline_{VK_NULL_HANDLE};
    VkPipelineLayout graphicsPipelineLayout_{VK_NULL_HANDLE};

    std::vector<VkCommandBuffer> commandBuffers_;

    // Sync Objects
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    uint32_t currentFrame_{0};
    // ref Fence Object wait render finished
    std::vector<VkFence> imagesInFlight_;

    // what is enable
    bool enableValidationLayer_;
    bool enableGetPhysicalDeviceProperty2Extension_{false};

    // extensions and layers
    std::vector<const char *> instanceExtensions_;
    std::vector<const char *> instanceLayers_;

    // setting
    float DPI{1.0f};

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
