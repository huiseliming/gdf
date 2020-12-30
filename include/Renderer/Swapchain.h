#pragma once
#include "Base/NonCopyable.h"
#include "VulkanApi.h"
#include <vector>
#include "Renderer/RenderPass.h"
#include "Renderer/GraphicsPipeline.h"

namespace gdf
{

class Window;
class Graphics;

class GDF_EXPORT Swapchain : public NonCopyable
{
public:
    Swapchain() = delete;
    Swapchain(Window &wnd,
              Graphics &gfx,
              VkSurfaceFormatKHR surfaceFormat = {VkFormat(UINT32_MAX), VkColorSpaceKHR(UINT32_MAX)},
              VkPresentModeKHR presentMode = VkPresentModeKHR(UINT32_MAX),
              uint32_t minImageCount = 0);
    ~Swapchain();
    void CreateSwapchain(VkSurfaceFormatKHR surfaceFormat = {VkFormat(UINT32_MAX), VkColorSpaceKHR(UINT32_MAX)},
                         VkPresentModeKHR presentMode = VkPresentModeKHR(UINT32_MAX),
                         uint32_t minImageCount= UINT32_MAX );
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

    bool SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat);
    bool SetPresentMode(VkPresentModeKHR presentMode);
    bool SetMinImageCount(uint32_t minImageCount);

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


private:
    //reuse data
    VkSurfaceFormatKHR surfaceFormat_;
    VkPresentModeKHR presentMode_;
    uint32_t minImageCount_;
    std::vector<VkSurfaceFormatKHR> supportedSurfaceFormats_;
    std::vector<VkPresentModeKHR> supportedPresentModes_;
    VkExtent2D extent_;

    //const define
    constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    
    Window &wnd_;
    Graphics &gfx_;
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