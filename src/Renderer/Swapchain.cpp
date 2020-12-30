#include "Renderer/Swapchain.h"
#include "Base/File.h"
#include "Base/Window.h"
#include "Renderer/Graphics.h"
#include <cmath>

namespace gdf
{

Swapchain::Swapchain(
    Window &wnd, Graphics &gfx, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t minImageCount)
    : wnd_(wnd), gfx_(gfx), needRecreate_(false), renderPass_(gfx_.device()), graphicsPipeline(gfx_.device())
{
    VK_ASSERT_SUCCESSED(wnd_.GetVkSurfaceKHR(gfx_.instance(), &surface_));
    if (!gfx.GetSupportPresentQueue(surface_, presentQueue_))
        THROW_EXCEPT("Cant find present queue family indices!");

    uint32_t surfaceFormatCount;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_.physicalDevice(), surface_, &surfaceFormatCount, nullptr));
    supportedSurfaceFormats_.resize(surfaceFormatCount);
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceFormatsKHR(
        gfx_.physicalDevice(), surface_, &surfaceFormatCount, supportedSurfaceFormats_.data()));
    surfaceFormat_ = supportedSurfaceFormats_[0];
    uint32_t presentModeCount;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_.physicalDevice(), surface_, &presentModeCount, nullptr));
    supportedPresentModes_.resize(presentModeCount);
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfacePresentModesKHR(
        gfx_.physicalDevice(), surface_, &presentModeCount, supportedPresentModes_.data()));
    presentMode_ = supportedPresentModes_[0];
    minImageCount_ = 3;
    SetSurfaceFormat(surfaceFormat);
    SetPresentMode(presentMode);
    if (minImageCount != UINT32_MAX)
        SetMinImageCount(minImageCount);
    CreateSwapchain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffer();
    CreateCommandBuffers();
    CreateSyncObjects();
}

Swapchain::~Swapchain()
{
    DestroySyncObjects();
    DestroyCommandBuffers();
    DestroyFramebuffer();
    DestroyGraphicsPipeline();
    DestroyRenderPass();
    DestroyImageViews();
    DestroySwapchain();
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(gfx_.instance(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
}

void Swapchain::CreateSwapchain(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t minImageCount)
{
    if (surfaceFormat.format != UINT32_MAX)
        SetSurfaceFormat(surfaceFormat_);
    if (presentMode != UINT32_MAX)
        SetPresentMode(presentMode_);
    if (minImageCount != UINT32_MAX)
        SetMinImageCount(3U);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_.physicalDevice(), surface_, &surfaceCapabilities));

    VkSwapchainKHR newSwapchain{VK_NULL_HANDLE};
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        extent_ = VkExtent2D{std::min(std::max(static_cast<uint32_t>(wnd_.width()), surfaceCapabilities.minImageExtent.width),
                                      surfaceCapabilities.maxImageExtent.width),
                             std::min(std::max(static_cast<uint32_t>(wnd_.height()), surfaceCapabilities.minImageExtent.height),
                                      surfaceCapabilities.maxImageExtent.height)};
    } else {
        extent_ = surfaceCapabilities.currentExtent;
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
        .imageExtent = extent_,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        //.queueFamilyIndexCount,
        //.pQueueFamilyIndices,
        .preTransform = preTranform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode_,
        .clipped = VK_TRUE,
        .oldSwapchain = swapchain_,
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

void Swapchain::CreateImageViews()
{
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &SwapchainImageCount_, nullptr));
    std::vector<VkImage> images(SwapchainImageCount_);
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &SwapchainImageCount_, images.data()));
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
    imageViews_.resize(SwapchainImageCount_, VK_NULL_HANDLE);
    for (size_t i = 0; i < SwapchainImageCount_; i++) {
        ImageViewCI.image = images[i];
        VK_ASSERT_SUCCESSED(vkCreateImageView(gfx_.device(), &ImageViewCI, nullptr, &imageViews_[i]));
    }
}

void Swapchain::DestroyImageViews()
{
    for (auto imageView : imageViews_)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(gfx_.device(), imageView, nullptr);
    imageViews_.clear();
}

void Swapchain::CreateRenderPass()
{
    renderPass_.AddAttachmentDescription(VkAttachmentDescription{.format = surfaceFormat_.format,
                                                                 .samples = VK_SAMPLE_COUNT_1_BIT,
                                                                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                                 .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR});
    renderPass_.AddSubpassDescriptionHelper(SubpassDescriptionHelper{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachments = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}
        });
    renderPass_.AddSubpassDependency(VkSubpassDependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    });
}

void Swapchain::DestroyRenderPass()
{
    renderPass_.Reset();
}

void Swapchain::CreateGraphicsPipeline()
{
    graphicsPipeline.AddShaderStage("../shaders/test.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    graphicsPipeline.AddShaderStage("../shaders/test.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    graphicsPipeline.SetInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    });

    graphicsPipeline.AddViewport(VkViewport{
        .x = 0.0f,
        .y = 0.0f, 
        .width = float(extent_.width),
        .height = float(extent_.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    });
    graphicsPipeline.AddScissor(VkRect2D{
        .offset = {0,0},
        .extent = extent_
        });
    graphicsPipeline.SetRasterizationState({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
        });
    graphicsPipeline.SetMultisampleState({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    });
    graphicsPipeline.AddColorBlendAttachmentState({ 
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        });
    graphicsPipeline.SetColorBlendState({
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
        });
    graphicsPipeline.SetRenderPass(renderPass_.Get());
}

void Swapchain::DestroyGraphicsPipeline()
{
    graphicsPipeline.Reset();
    //vkDestroyPipeline(gfx_.device(), graphicsPipeline_, nullptr);
    //vkDestroyPipelineLayout(gfx_.device(), pipelineLayout_, nullptr);
}

void Swapchain::CreateFramebuffer()
{
    framebuffers_.resize(SwapchainImageCount_);
    for (size_t i = 0; i < SwapchainImageCount_; i++) {
        VkImageView attachments[] = {imageViews_[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_.Get();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent_.width;
        framebufferInfo.height = extent_.height;
        framebufferInfo.layers = 1;
        VK_ASSERT_SUCCESSED(vkCreateFramebuffer(gfx_.device(), &framebufferInfo, nullptr, &framebuffers_[i]))
    }
}

void Swapchain::DestroyFramebuffer()
{
    for (auto framebuffer : framebuffers_)
        vkDestroyFramebuffer(gfx_.device(), framebuffer, nullptr);
    framebuffers_.clear();
}

void Swapchain::CreateCommandBuffers()
{
    commandBuffers_.resize(SwapchainImageCount_);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gfx_.graphicsCommandQueue().commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

    if (vkAllocateCommandBuffers(gfx_.device(), &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers_.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers_[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass_.Get();
        renderPassInfo.framebuffer = framebuffers_[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent_;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.Get());

        vkCmdDraw(commandBuffers_[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers_[i]);

        if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void Swapchain::DestroyCommandBuffers()
{
    vkFreeCommandBuffers(gfx_.device(),
                         gfx_.graphicsCommandQueue().commandPool(),
                         static_cast<uint32_t>(commandBuffers_.size()),
                         commandBuffers_.data());
}

void Swapchain::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight_.resize(imageViews_.size(), VK_NULL_HANDLE);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(gfx_.device(), &semaphoreCI, nullptr, &renderFinishedSemaphores_[i]));
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(gfx_.device(), &semaphoreCI, nullptr, &imageAvailableSemaphores_[i]));
        VK_ASSERT_SUCCESSED(vkCreateFence(gfx_.device(), &fenceCI, nullptr, &inFlightFences_[i]));
    }
}

void Swapchain::DestroySyncObjects()
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (renderFinishedSemaphores_[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(gfx_.device(), renderFinishedSemaphores_[i], nullptr);
        if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(gfx_.device(), imageAvailableSemaphores_[i], nullptr);
        if (inFlightFences_[i] != VK_NULL_HANDLE)
            vkDestroyFence(gfx_.device(), inFlightFences_[i], nullptr);
    }
    renderFinishedSemaphores_.clear();
    imageAvailableSemaphores_.clear();
    inFlightFences_.clear();
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
    gfx_.DeviceWaitIdle();
    DestroyCommandBuffers();
    DestroyFramebuffer();
    DestroyGraphicsPipeline();
    DestroyRenderPass();
    DestroyImageViews();
    CreateSwapchain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffer();
    CreateCommandBuffers();
    needRecreate_ = false;
}

void Swapchain::DrawFrame()
{
    if (needRecreate()) {
        Recreate();
    }
    // AcquireNextImage
    vkWaitForFences(gfx_.device(), 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(
        gfx_.device(), swapchain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        Recreate();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        THROW_EXCEPT("failed to acquire swap chain image!");
    }

    if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(gfx_.device(), 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight_[imageIndex] = inFlightFences_[currentFrame_];

    // rendering
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(gfx_.device(), 1, &inFlightFences_[currentFrame_]);

    VK_ASSERT_SUCCESSED(vkQueueSubmit(gfx_.graphicsCommandQueue().queue(), 1, &submitInfo, inFlightFences_[currentFrame_]));

    // presentation
    VkSwapchainKHR swapchains[] = {swapchain_};
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores)),
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
    };
    vkQueuePresentKHR(presentQueue_, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RequestRecreate();
    }
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkSemaphore Swapchain::GetCurrentFrameRenderFinishedSemaphore()
{
    return renderFinishedSemaphores_[currentFrame_];
}

VkSemaphore Swapchain::GetCurrentFrameImageAvailableSemaphore()
{
    return imageAvailableSemaphores_[currentFrame_];
}

VkFence Swapchain::GetCurrentFrameInFlightFence()
{
    return inFlightFences_[currentFrame_];
}

VkFence *Swapchain::GetCurrentFrameInFlightFencePointer()
{
    return &inFlightFences_[currentFrame_];
}

std::vector<VkFence> &Swapchain::imageInFlight()
{
    return imagesInFlight_;
}

bool Swapchain::SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat)
{
    if (surfaceFormat.colorSpace == surfaceFormat_.colorSpace && surfaceFormat.format == surfaceFormat_.format)
        return false;
    for (auto supportedSurfaceFormat : supportedSurfaceFormats_) {
        if (surfaceFormat.colorSpace == supportedSurfaceFormat.colorSpace &&
            surfaceFormat.format == supportedSurfaceFormat.format) {
            surfaceFormat_ = surfaceFormat;
            return true;
        }
    }
    surfaceFormat_ = supportedSurfaceFormats_[0];
    return false;
}

bool Swapchain::SetPresentMode(VkPresentModeKHR presentMode)
{
    if (presentMode == presentMode_)
        return false;
    for (auto supportedPresentMode : supportedPresentModes_) {
        if (presentMode == supportedPresentMode) {
            presentMode_ = presentMode;
            return true;
        }
    }
    presentMode_ = supportedPresentModes_[0];
    return false;
}

bool Swapchain::SetMinImageCount(uint32_t minImageCount)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_.physicalDevice(), surface_, &surfaceCapabilities));
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