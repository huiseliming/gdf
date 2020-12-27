#pragma once
#include "Base/NonCopyable.h"
#include "VulkanApi.h"
#include <vector>
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

    void CreateSwapchainImageViews();
    void DestroySwapchainImageViews();

    void CreateFramebuffer(VkRenderPass renderPass);
    void DestroyFramebuffer();

    void CreateSyncObjects();
    void DestroySyncObjects();

    void RequestRecreate();
    bool needRecreate();
    void Recreate();

    VkResult AcquireNextImage(uint32_t &imageIndex);
    VkResult Present();



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
    Window &wnd_;
    Graphics &gfx_;

    std::vector<VkSurfaceFormatKHR> supportedSurfaceFormats;
    std::vector<VkPresentModeKHR> supportedPresentModes;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkExtent2D extent;

    VkSurfaceFormatKHR surfaceFormat_;
    VkPresentModeKHR presentMode_;
    uint32_t minImageCount_;

    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkFence> fences_;

    uint32_t currentFrameIndex{UINT32_MAX};
    uint32_t currentImageIndex{UINT32_MAX};
    std::vector<VkImageView> imageViews_;
    std::vector<VkFramebuffer> framebuffers;


    bool needRecreate_;
};

} // namespace gdf