#include "Graphics/Swapchain.h"
#include "Base/File.h"
#include "Base/Window.h"
#include "Graphics/Graphics.h"
#include <algorithm>
#include <cmath>


namespace gdf
{

Swapchain::Swapchain(Window &window, Device &device, bool VSync)
    : window_(window), device_(device), needRecreate_(false), renderPass_(device), graphicsPipeline(device)
{
    VK_ASSERT_SUCCESSED(window_.GetVkSurfaceKHR(Graphics::vulkanInstance(), &surface_));

    // found supports presenting:
    uint32_t queueFamilyCount = device_.queueFamilyProperties.size();
    std::vector<VkBool32> supportsPresent(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++)
        vkGetPhysicalDeviceSurfaceSupportKHR(device_.physicalDevice, i, surface_, &supportsPresent[i]);

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueIndex = UINT32_MAX;
    uint32_t presentQueueIndex = UINT32_MAX;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if ((device_.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueIndex = i;
                presentQueueIndex = i;
                break;
            }
        }
    }
    // If there's no queue that supports both present and graphics
    // try to find a separate present queue
    if (presentQueueIndex == UINT32_MAX) {
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueIndex = i;
                break;
            }
        }
    }

    // Exit if either a graphics or a presenting queue hasn't been found
    if (graphicsQueueIndex == UINT32_MAX || presentQueueIndex == UINT32_MAX) {
        THROW_EXCEPT("Could not find a graphics and/or presenting queue!");
    }

    //if (graphicsQueueIndex != presentQueueIndex) {
    //    THROW_EXCEPT("Separate graphics and presenting queues are not supported yet!");
    //}

    presentQueueIndex_ = presentQueueIndex;

    // found color format
    uint32_t surfaceFormatCount;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceFormatsKHR(device_.physicalDevice, surface_, &surfaceFormatCount, nullptr));
    supportedSurfaceFormats_.resize(surfaceFormatCount);
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceFormatsKHR(
        device_.physicalDevice, surface_, &surfaceFormatCount, supportedSurfaceFormats_.data()));

    surfaceFormat_ = supportedSurfaceFormats_[0];
    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((surfaceFormatCount == 1) && (supportedSurfaceFormats_[0].format == VK_FORMAT_UNDEFINED)) {
        surfaceFormat_.format = VK_FORMAT_B8G8R8A8_UNORM;
        surfaceFormat_.colorSpace = supportedSurfaceFormats_[0].colorSpace;
        // TODO: print warn
    } else {
        // iterate over the list of available surface format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool found_B8G8R8A8_UNORM = false;
        for (auto &&surfaceFormat : supportedSurfaceFormats_) {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
                surfaceFormat_.format = surfaceFormat.format;
                surfaceFormat_.colorSpace = surfaceFormat.colorSpace;
                found_B8G8R8A8_UNORM = true;
                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color format
        if (!found_B8G8R8A8_UNORM) {
            surfaceFormat_.format = supportedSurfaceFormats_[0].format;
            surfaceFormat_.colorSpace = supportedSurfaceFormats_[0].colorSpace;
            // TODO: print warn
        }
    }

    uint32_t presentModeCount;
    VK_ASSERT_SUCCESSED(
        vkGetPhysicalDeviceSurfacePresentModesKHR(device_.physicalDevice, surface_, &presentModeCount, nullptr));
    supportedPresentModes_.resize(presentModeCount);
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfacePresentModesKHR(
        device_.physicalDevice, surface_, &presentModeCount, supportedPresentModes_.data()));
    SetVSyncEnable(VSync);

    //CreateSwapchain();
    // CreateImageViews();
    // CreateRenderPass();
    // CreateGraphicsPipeline();
    // CreateFramebuffer();
    // CreateCommandBuffers();
    // CreateSyncObjects();
}

Swapchain::~Swapchain()
{
    // DestroySyncObjects();
    // DestroyCommandBuffers();
    // DestroyFramebuffer();
    // DestroyGraphicsPipeline();
    // DestroyRenderPass();
    // DestroyImageViews();
    Destroy();
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(Graphics::vulkanInstance(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
}

void Swapchain::SetVSyncEnable(bool enable)
{
    if (enable) {
        for (auto supportedPresentMode : supportedPresentModes_) {
            if (supportedPresentMode == VK_PRESENT_MODE_FIFO_KHR) {
                presentMode_ = VK_PRESENT_MODE_FIFO_KHR;
                return;
            }
        }
        // TODO: print warn
    }
    presentMode_ = VK_PRESENT_MODE_MAILBOX_KHR;
}

void Swapchain::Create(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, bool vsync)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_.physicalDevice, surface_, &surfaceCapabilities));

    VkSwapchainKHR newSwapchain{VK_NULL_HANDLE};
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        extent_ =
            VkExtent2D{std::min(std::max(static_cast<uint32_t>(window_.width()), surfaceCapabilities.minImageExtent.width),
                                surfaceCapabilities.maxImageExtent.width),
                       std::min(std::max(static_cast<uint32_t>(window_.height()), surfaceCapabilities.minImageExtent.height),
                                surfaceCapabilities.maxImageExtent.height)};
    } else {
        extent_ = surfaceCapabilities.currentExtent;
    }
    VkSurfaceTransformFlagBitsKHR preTranform{};
    if ((surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0) {
        preTranform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        // TODO: print warn
        preTranform = surfaceCapabilities.currentTransform;
    }

    uint32_t minImageCount = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) && (minImageCount > surfaceCapabilities.maxImageCount)) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    if (std::find(compositeAlphaFlags.begin(), compositeAlphaFlags.end(), VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) !=
        compositeAlphaFlags.end()) {
        compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    } else {
        // TODO: print warn
        for (auto &compositeAlphaFlag : compositeAlphaFlags) {
            if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
                compositeAlpha = compositeAlphaFlag;
                break;
            };
        }
    }
    VkSwapchainKHR oldSwapchain = swapchain_;
    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface_,
        .minImageCount = minImageCount,
        .imageFormat = surfaceFormat_.format,
        .imageColorSpace = surfaceFormat_.colorSpace,
        .imageExtent = extent_,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = preTranform,
        .compositeAlpha = compositeAlpha,
        .presentMode = presentMode_,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain,
    };
    VK_ASSERT_SUCCESSED(vkCreateSwapchainKHR(device_, &swapchainCI, nullptr, &swapchain_));
    if (oldSwapchain != VK_NULL_HANDLE) {
        DestroyImageViews();
        vkDestroySwapchainKHR(device_, oldSwapchain, nullptr);
    }
    CreateImageViews();
}

void Swapchain::Destroy()
{
    DestroyImageViews();
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
}

void Swapchain::CreateImageViews()
{
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(device_, swapchain_, &SwapchainImageCount_, nullptr));
    std::vector<VkImage> images(SwapchainImageCount_);
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(device_, swapchain_, &SwapchainImageCount_, images.data()));
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
                                      .subresourceRange = {
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                          .baseMipLevel = 0,
                                          .levelCount = 1,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1,
                                      }};
    assert(imageViews_.empty());
    imageViews_.resize(SwapchainImageCount_, VK_NULL_HANDLE);
    for (size_t i = 0; i < SwapchainImageCount_; i++) {
        ImageViewCI.image = images[i];
        VK_ASSERT_SUCCESSED(vkCreateImageView(device_, &ImageViewCI, nullptr, &imageViews_[i]));
    }
}

void Swapchain::DestroyImageViews()
{
    for (auto imageView : imageViews_)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(device_, imageView, nullptr);
    imageViews_.clear();
}

// void Swapchain::CreateRenderPass()
//{
//    renderPass_.AddAttachmentDescription(VkAttachmentDescription{.format = surfaceFormat_.format,
//                                                                 .samples = VK_SAMPLE_COUNT_1_BIT,
//                                                                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//                                                                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//                                                                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//                                                                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//                                                                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//                                                                 .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR});
//    renderPass_.AddSubpassDescriptionHelper(
//        SubpassDescriptionHelper{.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                 .colorAttachments = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}});
//    renderPass_.AddSubpassDependency(VkSubpassDependency{
//        .srcSubpass = VK_SUBPASS_EXTERNAL,
//        .dstSubpass = 0,
//        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//        .srcAccessMask = 0,
//        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//    });
//}
//
// void Swapchain::DestroyRenderPass()
//{
//    renderPass_.Reset();
//}
//
// void Swapchain::CreateGraphicsPipeline()
//{
//    graphicsPipeline.AddShaderStage("../shaders/test.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
//    graphicsPipeline.AddShaderStage("../shaders/test.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
//    graphicsPipeline.SetInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo{
//        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
//        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
//        .primitiveRestartEnable = VK_FALSE,
//    });
//
//    graphicsPipeline.AddViewport(VkViewport{
//        .x = 0.0f,
//        .y = 0.0f,
//        .width = float(extent_.width),
//        .height = float(extent_.height),
//        .minDepth = 0.0f,
//        .maxDepth = 1.0f,
//    });
//    graphicsPipeline.AddScissor(VkRect2D{.offset = {0, 0}, .extent = extent_});
//    graphicsPipeline.SetRasterizationState({
//        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//        .depthClampEnable = VK_FALSE,
//        .rasterizerDiscardEnable = VK_FALSE,
//        .polygonMode = VK_POLYGON_MODE_FILL,
//        .cullMode = VK_CULL_MODE_BACK_BIT,
//        .frontFace = VK_FRONT_FACE_CLOCKWISE,
//        .depthBiasEnable = VK_FALSE,
//        .lineWidth = 1.0f,
//    });
//    graphicsPipeline.SetMultisampleState({
//        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
//        .sampleShadingEnable = VK_FALSE,
//    });
//    graphicsPipeline.AddColorBlendAttachmentState({
//        .blendEnable = VK_FALSE,
//        .colorWriteMask =
//            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
//    });
//    graphicsPipeline.SetColorBlendState({
//        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//        .logicOpEnable = VK_FALSE,
//        .logicOp = VK_LOGIC_OP_COPY,
//        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
//    });
//    graphicsPipeline.SetRenderPass(renderPass_.Get());
//}
//
// void Swapchain::DestroyGraphicsPipeline()
//{
//    graphicsPipeline.Reset();
//    // vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
//    // vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
//}
//
// void Swapchain::CreateFramebuffer()
//{
//    framebuffers_.resize(SwapchainImageCount_);
//    for (size_t i = 0; i < SwapchainImageCount_; i++) {
//        VkImageView attachments[] = {imageViews_[i]};
//        VkFramebufferCreateInfo framebufferInfo{};
//        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        framebufferInfo.renderPass = renderPass_.Get();
//        framebufferInfo.attachmentCount = 1;
//        framebufferInfo.pAttachments = attachments;
//        framebufferInfo.width = extent_.width;
//        framebufferInfo.height = extent_.height;
//        framebufferInfo.layers = 1;
//        VK_ASSERT_SUCCESSED(vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffers_[i]))
//    }
//}
//
// void Swapchain::DestroyFramebuffer()
//{
//    for (auto framebuffer : framebuffers_)
//        vkDestroyFramebuffer(device_, framebuffer, nullptr);
//    framebuffers_.clear();
//}
//
// void Swapchain::CreateCommandBuffers()
//{
//    commandBuffers_.resize(SwapchainImageCount_);
//
//    VkCommandBufferAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    allocInfo.commandPool = gfx_.graphicsCommandQueue().commandPool();
//    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();
//
//    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate command buffers!");
//    }
//
//    for (size_t i = 0; i < commandBuffers_.size(); i++) {
//        VkCommandBufferBeginInfo beginInfo{};
//        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//        if (vkBeginCommandBuffer(commandBuffers_[i], &beginInfo) != VK_SUCCESS) {
//            throw std::runtime_error("failed to begin recording command buffer!");
//        }
//
//        VkRenderPassBeginInfo renderPassInfo{};
//        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//        renderPassInfo.renderPass = renderPass_.Get();
//        renderPassInfo.framebuffer = framebuffers_[i];
//        renderPassInfo.renderArea.offset = {0, 0};
//        renderPassInfo.renderArea.extent = extent_;
//
//        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
//        renderPassInfo.clearValueCount = 1;
//        renderPassInfo.pClearValues = &clearColor;
//
//        vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//        vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.Get());
//
//        vkCmdDraw(commandBuffers_[i], 3, 1, 0, 0);
//
//        vkCmdEndRenderPass(commandBuffers_[i]);
//
//        if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS) {
//            throw std::runtime_error("failed to record command buffer!");
//        }
//    }
//}
//
// void Swapchain::DestroyCommandBuffers()
//{
//    vkFreeCommandBuffers(device_,
//                         gfx_.graphicsCommandQueue().commandPool(),
//                         static_cast<uint32_t>(commandBuffers_.size()),
//                         commandBuffers_.data());
//}
//
// void Swapchain::CreateSyncObjects()
//{
//    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
//    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
//
//    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
//    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
//    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
//    imagesInFlight_.resize(imageViews_.size(), VK_NULL_HANDLE);
//
//    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        VK_ASSERT_SUCCESSED(vkCreateSemaphore(device_, &semaphoreCI, nullptr, &renderFinishedSemaphores_[i]));
//        VK_ASSERT_SUCCESSED(vkCreateSemaphore(device_, &semaphoreCI, nullptr, &imageAvailableSemaphores_[i]));
//        VK_ASSERT_SUCCESSED(vkCreateFence(device_, &fenceCI, nullptr, &inFlightFences_[i]));
//    }
//}
//
// void Swapchain::DestroySyncObjects()
//{
//    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        if (renderFinishedSemaphores_[i] != VK_NULL_HANDLE)
//            vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
//        if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE)
//            vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
//        if (inFlightFences_[i] != VK_NULL_HANDLE)
//            vkDestroyFence(device_, inFlightFences_[i], nullptr);
//    }
//    renderFinishedSemaphores_.clear();
//    imageAvailableSemaphores_.clear();
//    inFlightFences_.clear();
//}
//
// void Swapchain::RequestRecreate()
//{
//    needRecreate_ = true;
//}
//
// bool Swapchain::needRecreate()
//{
//    return needRecreate_;
//}
//
// void Swapchain::Recreate()
//{
//    gfx_.DeviceWaitIdle();
//    DestroyCommandBuffers();
//    DestroyFramebuffer();
//    DestroyGraphicsPipeline();
//    DestroyRenderPass();
//    DestroyImageViews();
//    CreateSwapchain();
//    CreateImageViews();
//    CreateRenderPass();
//    CreateGraphicsPipeline();
//    CreateFramebuffer();
//    CreateCommandBuffers();
//    needRecreate_ = false;
//}
//
// void Swapchain::DrawFrame()
//{
//    if (needRecreate()) {
//        Recreate();
//    }
//    // AcquireNextImage
//    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
//
//    uint32_t imageIndex;
//    auto result = vkAcquireNextImageKHR(
//        device_, swapchain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
//    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//        Recreate();
//        return;
//    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//        THROW_EXCEPT("failed to acquire swap chain image!");
//    }
//
//    if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE) {
//        vkWaitForFences(device_, 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
//    }
//    imagesInFlight_[imageIndex] = inFlightFences_[currentFrame_];
//
//    // rendering
//    VkSubmitInfo submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
//    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//    submitInfo.waitSemaphoreCount = 1;
//    submitInfo.pWaitSemaphores = waitSemaphores;
//    submitInfo.pWaitDstStageMask = waitStages;
//
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];
//
//    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
//    submitInfo.signalSemaphoreCount = 1;
//    submitInfo.pSignalSemaphores = signalSemaphores;
//
//    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);
//
//    VK_ASSERT_SUCCESSED(vkQueueSubmit(gfx_.graphicsCommandQueue().queue(), 1, &submitInfo, inFlightFences_[currentFrame_]));
//
//    // presentation
//    VkSwapchainKHR swapchains[] = {swapchain_};
//    VkPresentInfoKHR presentInfo{
//        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
//        .waitSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores)),
//        .pWaitSemaphores = signalSemaphores,
//        .swapchainCount = 1,
//        .pSwapchains = swapchains,
//        .pImageIndices = &imageIndex,
//    };
//    vkQueuePresentKHR(presentQueue_, &presentInfo);
//    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
//        RequestRecreate();
//    }
//    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
//}
//
// VkSemaphore Swapchain::GetCurrentFrameRenderFinishedSemaphore()
//{
//    return renderFinishedSemaphores_[currentFrame_];
//}
//
// VkSemaphore Swapchain::GetCurrentFrameImageAvailableSemaphore()
//{
//    return imageAvailableSemaphores_[currentFrame_];
//}
//
// VkFence Swapchain::GetCurrentFrameInFlightFence()
//{
//    return inFlightFences_[currentFrame_];
//}
//
// VkFence *Swapchain::GetCurrentFrameInFlightFencePointer()
//{
//    return &inFlightFences_[currentFrame_];
//}
//
// std::vector<VkFence> &Swapchain::imageInFlight()
//{
//    return imagesInFlight_;
//}
} // namespace gdf