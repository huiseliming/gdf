#include "Renderer/Swapchain.h"
#include "Base/Window.h"
#include "Renderer/Graphics.h"
#include <cmath>

namespace gdf
{

Swapchain::Swapchain(Window &wnd, Graphics &gfx) : wnd_(wnd), gfx_(gfx)
{
    VK_ASSERT_SUCCESSED(wnd_.GetVkSurfaceKHR(gfx_.instance(), &surface_));
    {
        uint32_t surfaceFormatCount;
        VK_ASSERT_SUCCESSED(
            vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_.physicalDevice(), surface_, &surfaceFormatCount, nullptr));
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);

        VK_ASSERT_SUCCESSED(
            vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_.physicalDevice(), surface_, &surfaceFormatCount, surfaceFormats.data()));
        surfaceFormat = surfaceFormats[0];
    }
    {
        uint32_t presentModeCount;
        VK_ASSERT_SUCCESSED(
            vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_.physicalDevice(), surface_, &presentModeCount, nullptr));
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        VK_ASSERT_SUCCESSED(
            vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_.physicalDevice(), surface_, &presentModeCount, presentModes.data()));
        presentMode = presentModes[0];
    }
    CreateSwapchain();
    CreateSwapchainImageViews();
}

Swapchain::~Swapchain()
{
    DestroySwapchainImageViews();
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(gfx_.device(), swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(gfx_.instance(), surface_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
}

void Swapchain::FramebufferResize()
{
    CreateSwapchain();
}

void Swapchain::CreateSwapchain()
{
    VkSwapchainKHR newSwapchain{VK_NULL_HANDLE};
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_.physicalDevice(), surface_, &surfaceCapabilities));
    VkExtent2D imageExtent;
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        imageExtent =
            VkExtent2D{std::min(std::max(static_cast<uint32_t>(wnd_.width()), surfaceCapabilities.minImageExtent.width),
                                surfaceCapabilities.maxImageExtent.width),
                       std::min(std::max(static_cast<uint32_t>(wnd_.height()), surfaceCapabilities.minImageExtent.height),
                                surfaceCapabilities.maxImageExtent.height)};
    } else {
        imageExtent = surfaceCapabilities.currentExtent;
    }
    VkSurfaceTransformFlagBitsKHR preTranform{};
    if ((surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0) {
        preTranform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTranform = surfaceCapabilities.currentTransform;
    }
    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface_,
        .minImageCount = std::min(std::max(3U, surfaceCapabilities.minImageCount), surfaceCapabilities.maxImageCount),
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = imageExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        //.queueFamilyIndexCount,
        //.pQueueFamilyIndices,
        .preTransform = preTranform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = swapchain_ ? swapchain_ : nullptr,
    };
    VK_ASSERT_SUCCESSED(vkCreateSwapchainKHR(gfx_.device(), &swapchainCI, nullptr, &newSwapchain));
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(gfx_.device(), swapchain_, nullptr);
    }
    swapchain_ = newSwapchain;
}

void Swapchain::RecreateSwapchainImageViews()
{
    DestroySwapchainImageViews();
    CreateSwapchainImageViews();
}

void Swapchain::CreateSwapchainImageViews()
{
    uint32_t imageCount;
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &imageCount, nullptr));
    std::vector<VkImage> images(imageCount);
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &imageCount, images.data()));
    VkImageViewCreateInfo ImageViewCI{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                      .format = surfaceFormat.format,
                                      .components =
                                          {
                                              VK_COMPONENT_SWIZZLE_R,
                                              VK_COMPONENT_SWIZZLE_G,
                                              VK_COMPONENT_SWIZZLE_B,
                                              VK_COMPONENT_SWIZZLE_A,
                                          },
                                      .subresourceRange = {
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                          .baseMipLevel = 0,
                                          .levelCount = 1,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1,
                                      }};
    assert(imageViews_.empty());
    imageViews_.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        ImageViewCI.image = images[i];
        VK_ASSERT_SUCCESSED(vkCreateImageView(gfx_.device(), &ImageViewCI, nullptr, &imageViews_[i]));
    }
}

void Swapchain::DestroySwapchainImageViews()
{
    for (auto imageView : imageViews_)
        vkDestroyImageView(gfx_.device(), imageView, nullptr);
    imageViews_.clear();
}

} // namespace gdf