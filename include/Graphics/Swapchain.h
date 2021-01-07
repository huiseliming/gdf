#pragma once
#include "Base/NonCopyable.h"
#include "Device.h"
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "VulkanApi.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace gdf
{

class Window;
class Graphics;

class GDF_EXPORT Swapchain : public NonCopyable
{
public:
    Swapchain() = delete;
    Swapchain(Window &wnd, Device &device, bool VSync = false);
    ~Swapchain();

    void SetVSyncEnable(bool enable);

    void CreateSwapchain(VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR},
                         VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
                         bool vsync = true);
    void DestroySwapchain();

    void CreateImageViews();
    void DestroyImageViews();

    void CreateRenderPass();
    void DestroyRenderPass();

    void CreateGraphicsPipeline();
    void DestroyGraphicsPipeline();

    void CreateFramebuffer();
    void DestroyFramebuffer();

    void CreateCommandBuffers();
    void DestroyCommandBuffers();

    void CreateSyncObjects();
    void DestroySyncObjects();

    void RequestRecreate();
    bool needRecreate();
    void Recreate();

    void DrawFrame();

    VkSemaphore GetCurrentFrameRenderFinishedSemaphore();
    VkSemaphore GetCurrentFrameImageAvailableSemaphore();
    VkFence GetCurrentFrameInFlightFence();
    VkFence *GetCurrentFrameInFlightFencePointer();
    std::vector<VkFence> &imageInFlight();

    VkSurfaceFormatKHR surfaceFormat()
    {
        return surfaceFormat_;
    }
    VkPresentModeKHR presentMode()
    {
        return presentMode_;
    }
    uint32_t minImageCount()
    {
        return minImageCount_;
    }

    VkSurfaceKHR surface()
    {
        return surface_;
    }

private:
    bool VSyncEnable_ = false;
    uint32_t presentQueueIndex_ = UINT32_MAX;
    // reuse data
    VkSurfaceFormatKHR surfaceFormat_;
    VkPresentModeKHR presentMode_;
    uint32_t minImageCount_;
    std::vector<VkSurfaceFormatKHR> supportedSurfaceFormats_;
    std::vector<VkPresentModeKHR> supportedPresentModes_;
    VkExtent2D extent_;

    // const define
    constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    Window &window_;
    Device &device_;
    // vk object
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
    std::vector<VkImageView> imageViews_;

    RenderPass renderPass_;
    VkPipeline graphicsPipeline_;
    GraphicsPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<VkCommandBuffer> commandBuffers_;

    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkFence> inFlightFences_;
    std::vector<VkFence> imagesInFlight_;
    // state mark
    uint32_t SwapchainImageCount_;
    uint32_t currentFrame_{0};
    VkQueue presentQueue_;
    bool needRecreate_;
};

} // namespace gdf