#include "Renderer/Swapchain.h"
#include "Base/Window.h"
#include "Renderer/Graphics.h"
#include <cmath>
#include "Base/File.h"

namespace gdf
{

Swapchain::Swapchain(Window &wnd, Graphics &gfx, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t minImageCount)
    : wnd_(wnd), gfx_(gfx), needRecreate_(false)
{
    VK_ASSERT_SUCCESSED(wnd_.GetVkSurfaceKHR(gfx_.instance(), &surface_));
    if (!gfx.queueFamilyIndices.DetectPresentQueueFamilyIndices(gfx_.physicalDevice_, surface_)) 
        THROW_EXCEPT("Cant find present queue family indices!");

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

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_ASSERT_SUCCESSED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_.physicalDevice(), surface_, &surfaceCapabilities));

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
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &SwapchainImageCount, nullptr));
    std::vector<VkImage> images(SwapchainImageCount);
    VK_ASSERT_SUCCESSED(vkGetSwapchainImagesKHR(gfx_.device(), swapchain_, &SwapchainImageCount, images.data()));
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
    imageViews_.resize(SwapchainImageCount, VK_NULL_HANDLE);
    for (size_t i = 0; i < SwapchainImageCount; i++) {
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
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = surfaceFormat_.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_ASSERT_SUCCESSED(vkCreateRenderPass(gfx_.device(), &renderPassInfo, nullptr, &renderPass_))
}

void Swapchain::DestroyRenderPass()
{
    vkDestroyRenderPass(gfx_.device(), renderPass_, nullptr);
}
void Swapchain::CreateGraphicsPipeline()
{
    auto vertShaderCode = File::ReadBytes("../shaders/test.vert.spv");
    auto fragShaderCode = File::ReadBytes("../shaders/test.frag.spv");

    VkShaderModule vertShaderModule = gfx_.CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = gfx_.CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(gfx_.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(gfx_.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(gfx_.device(), fragShaderModule, nullptr);
    vkDestroyShaderModule(gfx_.device(), vertShaderModule, nullptr);
}

void Swapchain::DestroyGraphicsPipeline()
{
    vkDestroyPipeline(gfx_.device(), graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(gfx_.device(), pipelineLayout, nullptr);
}

void Swapchain::CreateFramebuffer()
{
    framebuffers.resize(SwapchainImageCount);
    for (size_t i = 0; i < SwapchainImageCount; i++) {
        VkImageView attachments[] = {imageViews_[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_;
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

void Swapchain::CreateCommandBuffers()
{
    commandBuffers.resize(SwapchainImageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gfx_.commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(gfx_.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass_;
        renderPassInfo.framebuffer = framebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void Swapchain::DestroyCommandBuffers()
{
    vkFreeCommandBuffers(gfx_.device(), gfx_.commandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void Swapchain::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight_.resize(imageViews_.size(), VK_NULL_HANDLE);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(gfx_.device(), &semaphoreCI, nullptr, &renderFinishedSemaphores[i]));
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(gfx_.device(), &semaphoreCI, nullptr, &imageAvailableSemaphores_[i]));
        VK_ASSERT_SUCCESSED(vkCreateFence(gfx_.device(), &fenceCI, nullptr, &inFlightFences[i]));
    }
}

void Swapchain::DestroySyncObjects()
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (renderFinishedSemaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(gfx_.device(), renderFinishedSemaphores[i], nullptr);
        if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(gfx_.device(), imageAvailableSemaphores_[i], nullptr);
        if (inFlightFences[i] != VK_NULL_HANDLE)
            vkDestroyFence(gfx_.device(), inFlightFences[i], nullptr);
    }
    renderFinishedSemaphores.clear();
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

VkResult Swapchain::AcquireNextImage(uint32_t &imageIndex)
{
    auto result = vkAcquireNextImageKHR(gfx_.device(), swapchain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        assert(false);
        RequestRecreate();
    } else if (result == VK_SUBOPTIMAL_KHR) {
        assert(false);
        return VK_SUCCESS;
    }
    return result;
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

std::vector<VkFence> &Swapchain::imageInFlight()
{
    return imagesInFlight_;
}

VkResult Swapchain::Present(uint32_t &imageIndex, std::vector<VkSemaphore> waitSemaphores)
{
    return Present(imageIndex, static_cast<uint32_t>(waitSemaphores.size()), waitSemaphores.data());
}

VkResult Swapchain::Present(uint32_t &imageIndex, uint32_t waitSemaphoreCount, VkSemaphore *waitSemaphores)
{
    VkSwapchainKHR swapchains[] = {swapchain_};
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = waitSemaphoreCount,
        .pWaitSemaphores = waitSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
    };
    auto result = vkQueuePresentKHR(gfx_.presentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        assert(false);
        RequestRecreate();
        return result;
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