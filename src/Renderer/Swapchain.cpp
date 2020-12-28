#include "Renderer/Swapchain.h"
#include "Base/Window.h"
#include "Renderer/Graphics.h"
#include <cmath>

namespace gdf
{

Swapchain::Swapchain(Window &wnd, Graphics &gfx, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t minImageCount)
    : wnd_(wnd), gfx_(gfx), needRecreate_(false)
{
    VK_ASSERT_SUCCESSED(wnd_.GetVkSurfaceKHR(gfx_.instance(), &surface_));
    if (!gfx.queueFamilyIndices.DetectPresentQueueFamilyIndices(gfx_.physicalDevice_, surface_)) 
        THROW_EXCEPT("Cant find present queue family indices!");
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_.physicalDevice(), surface_, &surfaceCapabilities));

    uint32_t surfaceFormatCount;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_.physicalDevice(), surface_, &surfaceFormatCount, nullptr));
    supportedSurfaceFormats.resize(surfaceFormatCount);
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_.physicalDevice(), surface_, &surfaceFormatCount, supportedSurfaceFormats.data()));
    surfaceFormat_ = supportedSurfaceFormats[0];
    uint32_t presentModeCount;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_.physicalDevice(), surface_, &presentModeCount, nullptr));
    supportedPresentModes.resize(presentModeCount);
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_.physicalDevice(), surface_, &presentModeCount, supportedPresentModes.data()));
    presentMode_ = supportedPresentModes[0];
    minImageCount_ = 3;
    SetSurfaceFormat(surfaceFormat);
    SetPresentMode(presentMode);
    if (minImageCount != UINT32_MAX)
        SetMinImageCount(minImageCount);
    CreateSwapchain();
    CreateSwapchainImageViews();
    CreateSyncObjects();
}

Swapchain::~Swapchain()
{
    DestroySyncObjects();
    DestroyFramebuffer();
    DestroySwapchainImageViews();
    DestroySwapchain();
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(gfx_.instance(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
}

void Swapchain::CreateSwapchain(VkSurfaceFormatKHR surfaceFormat,
                                VkPresentModeKHR presentMode,
                                uint32_t minImageCount)
{
    if (surfaceFormat.format != UINT32_MAX) 
        SetSurfaceFormat(surfaceFormat_);
    if (presentMode != UINT32_MAX) 
        SetPresentMode(presentMode_);
    if (minImageCount != UINT32_MAX) 
        SetMinImageCount(3U);

    VkSwapchainKHR newSwapchain{VK_NULL_HANDLE};
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        extent =
            VkExtent2D{std::min(std::max(static_cast<uint32_t>(wnd_.width()), surfaceCapabilities.minImageExtent.width),
                                surfaceCapabilities.maxImageExtent.width),
                       std::min(std::max(static_cast<uint32_t>(wnd_.height()), surfaceCapabilities.minImageExtent.height),
                                surfaceCapabilities.maxImageExtent.height)};
    } else {
        extent = surfaceCapabilities.currentExtent;
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
        .minImageCount = minImageCount_,
        .imageFormat = surfaceFormat_.format,
        .imageColorSpace = surfaceFormat_.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        //.queueFamilyIndexCount,
        //.pQueueFamilyIndices,
        .preTransform = preTranform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode_,
        .clipped = VK_TRUE,
        .oldSwapchain = swapchain_ != VK_NULL_HANDLE ? swapchain_ : VK_NULL_HANDLE,
    };
    VK_ASSERT_SUCCESSED(vkCreateSwapchainKHR(gfx_.device(), &swapchainCI, nullptr, &newSwapchain));
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(gfx_.device(), swapchain_, nullptr);
    }
    swapchain_ = newSwapchain;
}

void Swapchain::DestroySwapchain()
{
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(gfx_.device(), swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
}

void Swapchain::CreateSwapchainImageViews()
{
    uint32_t imageCount;
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &imageCount, nullptr));
    std::vector<VkImage> images(imageCount);
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &imageCount, images.data()));
    VkImageViewCreateInfo ImageViewCI{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                      .format = surfaceFormat_.format,
                                      .components =
                                        {
                                            VK_COMPONENT_SWIZZLE_R,
                                            VK_COMPONENT_SWIZZLE_G,
                                            VK_COMPONENT_SWIZZLE_B,
                                            VK_COMPONENT_SWIZZLE_A,
                                        },
                                      .subresourceRange = 
                                        {
                                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                            .baseMipLevel = 0,
                                            .levelCount = 1,
                                            .baseArrayLayer = 0,
                                            .layerCount = 1,
                                        }};
    assert(imageViews_.empty());
    imageViews_.resize(imageCount,VK_NULL_HANDLE);
    for (size_t i = 0; i < imageCount; i++) {
        ImageViewCI.image = images[i];
        VK_ASSERT_SUCCESSED(vkCreateImageView(gfx_.device(), &ImageViewCI, nullptr, &imageViews_[i]));
    }
}

void Swapchain::DestroySwapchainImageViews()
{
    for (auto imageView : imageViews_)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(gfx_.device(), imageView, nullptr);
    imageViews_.clear();
}

void Swapchain::CreateFramebuffer(VkRenderPass renderPass)
{
    framebuffers.resize(imageViews_.size());
    for (size_t i = 0; i < framebuffers.size(); i++) {
        VkImageView attachments[] = {imageViews_[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;
        VK_ASSERT_SUCCESSED(vkCreateFramebuffer(gfx_.device(), &framebufferInfo, nullptr, &framebuffers[i]))
    }
}

void Swapchain::DestroyFramebuffer()
{
    for (auto framebuffer : framebuffers) 
        vkDestroyFramebuffer(gfx_.device(), framebuffer, nullptr);
    framebuffers.clear();
}

void Swapchain::CreateSyncObjects()
{
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(imageViews_.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(gfx_.device(), &semaphoreCI, nullptr, &renderFinishedSemaphores[i]));
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(gfx_.device(), &semaphoreCI, nullptr, &imageAvailableSemaphores_[i]));
        VK_ASSERT_SUCCESSED(vkCreateFence(gfx_.device(), &fenceCI, nullptr, &inFlightFences[i]));
    }
}

void Swapchain::DestroySyncObjects()
{
    for (auto semaphore : imageAvailableSemaphores_)
        if (semaphore != VK_NULL_HANDLE) 
            vkDestroySemaphore(gfx_.device(), semaphore, nullptr);
    for (auto fence : inFlightFences)
        if (fence != VK_NULL_HANDLE) 
            vkDestroyFence(gfx_.device(), fence, nullptr);
    imageAvailableSemaphores_.clear();
    inFlightFences.clear();
}

 void Swapchain::RequestRecreate()
{
     needRecreate_ = true;
 }

 bool Swapchain::needRecreate()
{
    return needRecreate_;
}

void Swapchain::Recreate()
{
    DestroyFramebuffer();
    DestroySwapchainImageViews();
    CreateSwapchain();
    CreateSwapchainImageViews();
    needRecreate_ = false;
}

VkResult Swapchain::AcquireNextImage(uint32_t &imageIndex)
{
    vkWaitForFences(gfx_.device(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    auto result = vkAcquireNextImageKHR(gfx_.device(), swapchain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        Recreate();
        return result;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        THROW_EXCEPT("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(gfx_.device(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
}

VkSemaphore Swapchain::GetCurrentFrameRenderFinishedSemaphore()
{
    return renderFinishedSemaphores[currentFrame];
}

VkSemaphore Swapchain::GetCurrentFrameImageAvailableSemaphore()
{
    return imageAvailableSemaphores_[currentFrame];
}

VkFence Swapchain::GetCurrentFrameInFlightFence()
{
    return inFlightFences[currentFrame];
}

VkFence *Swapchain::GetCurrentFrameInFlightFencePointer()
{
    return &inFlightFences[currentFrame];
}

VkResult Swapchain::Present(uint32_t &imageIndex)
{
    VkSemaphore waitSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    VkSwapchainKHR swapchains[] = {swapchain_};
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
    };
    auto result = vkQueuePresentKHR(gfx_.presentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        Recreate();
    } else if (result != VK_SUCCESS) {
        THROW_EXCEPT("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return result;

}


bool Swapchain::SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat)
{
    if (surfaceFormat.colorSpace == surfaceFormat_.colorSpace && surfaceFormat.format == surfaceFormat_.format)
        return false;
    for (auto supportedSurfaceFormat : supportedSurfaceFormats) {
        if (surfaceFormat.colorSpace == supportedSurfaceFormat.colorSpace &&
            surfaceFormat.format == supportedSurfaceFormat.format) {
            surfaceFormat_ = surfaceFormat;
            return true;
        }
    }
    surfaceFormat_ = supportedSurfaceFormats[0];
    return false;
}

bool Swapchain::SetPresentMode(VkPresentModeKHR presentMode)
{
    if (presentMode == presentMode_)
        return false;
    for (auto supportedPresentMode : supportedPresentModes) {
        if (presentMode == supportedPresentMode) {
            presentMode_ = presentMode;
            return true;
        }
    }
    presentMode_ = supportedPresentModes[0];
    return false;
}

bool Swapchain::SetMinImageCount(uint32_t  minImageCount)
{
    auto temp = std::min(std::max(minImageCount, surfaceCapabilities.minImageCount), surfaceCapabilities.maxImageCount);
    if (minImageCount_ != temp) {
        minImageCount_ = temp;
        if (temp != minImageCount) {
            return false;
        }
        return true;
    }
    return false;
}

} // namespace gdf