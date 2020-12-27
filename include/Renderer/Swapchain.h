#pragma once
#include "VulkanApi.h"
#include "Base/NonCopyable.h"
#include <vector>
namespace gdf
{

class Window;
class Graphics;

class GDF_EXPORT Swapchain : public NonCopyable
{
public:
    Swapchain() = delete;
    Swapchain(Window &wnd, Graphics &gfx);
    ~Swapchain();

    void FramebufferResize();
    void CreateSwapchain();
    void RecreateSwapchainImageViews();
    void CreateSwapchainImageViews();
    void DestroySwapchainImageViews();

private:
    Window &wnd_;
    Graphics &gfx_;

    VkSurfaceFormatKHR surfaceFormat{};
    VkPresentModeKHR presentMode{};

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    
    uint32_t currentIndex{UINT32_MAX};
    std::vector<VkImageView> imageViews_;
};

} // namespace gdf