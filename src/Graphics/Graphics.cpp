#include "Graphics/Graphics.h"
#include "Base/File.h"
#include "Git.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <array>

#define GIT_UINT32_VERSION                                                                                                     \
    (static_cast<uint32_t>(wcstoul(GIT_VERSION_MAJOR, nullptr, 10) && 0xF) << 28) ||                                           \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_MINOR, nullptr, 10) && 0xFF) << 20) ||                                      \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_PATCH, nullptr, 10) && 0xFFFFF))
#define VK_API_VERSION VK_MAKE_VERSION(1, 0, 0)

namespace gdf
{

GDF_DEFINE_EXPORT_LOG_CATEGORY(GraphicsLog);

void Graphics::Initialize(Window *pWindow, bool enableValidationLayer)
{
    pWindow_ = pWindow;
    enableValidationLayer_ = enableValidationLayer;
    CreateInstance();
    // Setup DebugReportCallback
    if (enableValidationLayer_)
        CreateDebugReporter();
    if (pWindow)
        pWindow_->GetVkSurfaceKHR(instance_, &surfaceKHR_);
    CreateDevice({}, {});
    CreateCommandPool();
    CreateSwapchain();
    CreateDepthResources();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
    CreateSyncObjects();

    ImGuiCreate();
}

void Graphics::FrameBegin()
{
    ImGuiFrameBegin();
}

void Graphics::DrawFrame()
{
    if (RequireRecreateSwapchain_)
        RecreateSwapchain();
    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device_, swapchainKHR_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        GDF_LOG(GraphicsLog, LogLevel::Warning, "vkAcquireNextImageKHR -> VK_ERROR_OUT_OF_DATE_KHR");
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        THROW_EXCEPT("Failed to acquire swap chain image!");
    }

    if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device_, 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight_[imageIndex] = inFlightFences_[currentFrame_];
    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

    //VK_ASSERT_SUCCESSED(vkResetCommandPool(device_, commandPool_, 0));
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_ASSERT_SUCCESSED(vkBeginCommandBuffer(commandBuffers_[imageIndex], &beginInfo))

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = swapchainFramebuffers_[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainExtent_;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers_[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers_[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

    vkCmdDraw(commandBuffers_[imageIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers_[imageIndex]);

    VK_ASSERT_SUCCESSED(vkEndCommandBuffer(commandBuffers_[imageIndex]));

    ImGuiFrameRender(imageIndex);
    VkCommandBuffer commandBuffers[] = {commandBuffers_[imageIndex], imguiCommandBuffers_[imageIndex]};
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    auto submitInfo = GraphicsTools::MakeSubmitInfo(1, waitSemaphores, waitStages, 2, commandBuffers, 1, signalSemaphores);
    VK_ASSERT_SUCCESSED(vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame_]))

    auto presentInfoKHR = GraphicsTools::MakePresentInfoKHR(1, signalSemaphores, &swapchainKHR_, &imageIndex);

    result = vkQueuePresentKHR(presentQueue_, &presentInfoKHR);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        GDF_LOG(GraphicsLog, LogLevel::Warning, "vkQueuePresentKHR -> {}", GraphicsTools::VkResultString(result));
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS) {
        THROW_EXCEPT("Failed to present swap chain image!");
    }
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Graphics::FrameEnd()
{
    ImGuiFrameEnd();
}

void Graphics::Cleanup()
{
    DeviceWaitIdle();
    ImGuiDestroy();

    DestroySyncObjects();
    FreeCommandBuffers();
    DestroyFramebuffers();
    DestroyGraphicsPipeline();
    DestroyRenderPass();
    DestroyDepthResources();
    DestroySwapchain();
    DestroyCommandPool();
    DestroyDevice();
    if (surfaceKHR_ != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance_, surfaceKHR_, nullptr);
    if (enableValidationLayer_)
        DestroyDebugReporter();
    DestroyInstance();
}

void Graphics::CreateInstance()
{
    std::vector<const char *> instanceExtensions;
    if (!Window::GetRequiredInstanceExtensions(instanceExtensions))
        THROW_EXCEPT("Required window instance extensions faild!");
    // Instance
    std::vector<const char *> instanceLayers;
    if (enableValidationLayer_) {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        instanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    }
    VkApplicationInfo appinfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                              .pApplicationName = APPLICATION_NAME,
                              .applicationVersion = GIT_UINT32_VERSION,
                              .pEngineName = APPLICATION_NAME,
                              .engineVersion = GIT_UINT32_VERSION,
                              .apiVersion = VK_API_VERSION};
    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

    VK_ASSERT_SUCCESSED(vkCreateInstance(&instanceCI, nullptr, &instance_));
}

void Graphics::CreateDebugReporter()
{
    VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCI{
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_WARNING_BIT_EXT
        //| VK_DEBUG_REPORT_INFORMATION_BIT_EXT
        ,
        .pfnCallback = Graphics::DebugReportCallbackEXT,
    };
    auto fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT"));
    assert(fpCreateDebugReportCallbackEXT != nullptr);
    VK_ASSERT_SUCCESSED(fpCreateDebugReportCallbackEXT(instance_, &DebugReportCallbackCI, nullptr, &fpDebugReportCallbackEXT_));
}

void Graphics::CreateDevice(VkPhysicalDeviceFeatures enabledFeatures,
                            std::vector<const char *> enabledExtensions,
                            VkQueueFlags requestedQueueTypes)
{
    // Physical Device
    uint32_t physicalDeviceCount;
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data()));

    std::vector<VkPhysicalDevice> availablePhysicalDevices;
    for (auto physicalDevice : physicalDevices) {
        if (IsPhysicalDeviceSuitable(physicalDevice))
            deviceInfo_.Parse(physicalDevice);
    }

    // Logical Device
    assert(device_ == VK_NULL_HANDLE);
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs;
    const float defaultQueuePriority(0.0f);
    // Graphics queue
    deviceInfo_.queueFamilyIndices.graphics =
        GraphicsTools::GetQueueFamilyIndex(deviceInfo_.queueFamilyProperties, VK_QUEUE_GRAPHICS_BIT);
    if (deviceInfo_.queueFamilyIndices.graphics == UINT32_MAX)
        THROW_EXCEPT("No found graphics queue family indices!");
    deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = deviceInfo_.queueFamilyIndices.graphics,
        .queueCount = 1,
        .pQueuePriorities = &defaultQueuePriority,
    });

    // Compute queue
    deviceInfo_.queueFamilyIndices.compute =
        GraphicsTools::GetQueueFamilyIndex(deviceInfo_.queueFamilyProperties, VK_QUEUE_COMPUTE_BIT);
    if (deviceInfo_.queueFamilyIndices.compute == UINT32_MAX)
        THROW_EXCEPT("No found compute queue family indices!");
    if (deviceInfo_.queueFamilyIndices.compute != deviceInfo_.queueFamilyIndices.graphics) {
        deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = deviceInfo_.queueFamilyIndices.compute,
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority,
        });
    }

    // Dedicated transfer queue
    deviceInfo_.queueFamilyIndices.transfer =
        GraphicsTools::GetQueueFamilyIndex(deviceInfo_.queueFamilyProperties, VK_QUEUE_TRANSFER_BIT);
    if (deviceInfo_.queueFamilyIndices.transfer == UINT32_MAX)
        THROW_EXCEPT("No found transfer queue family indices!");
    if ((deviceInfo_.queueFamilyIndices.transfer != deviceInfo_.queueFamilyIndices.graphics) &&
        (deviceInfo_.queueFamilyIndices.transfer != deviceInfo_.queueFamilyIndices.compute)) {
        deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = deviceInfo_.queueFamilyIndices.transfer,
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority,
        });
    }

    // Create the logical device representation
    std::vector<const char *> deviceExtensions(enabledExtensions);
    if (surfaceKHR_ != VK_NULL_HANDLE) {
        // Dedicated present queue
        uint32_t queueFamilyCount = deviceInfo_.queueFamilyProperties.size();
        std::vector<VkBool32> supportsPresent(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; i++)
            vkGetPhysicalDeviceSurfaceSupportKHR(deviceInfo_.physicalDevice, i, surfaceKHR_, &supportsPresent[i]);

        // Search for a graphics and a present queue in the array of queue
        // families, try to find one that supports both
        uint32_t graphicsQueueIndex = UINT32_MAX;
        uint32_t presentQueueIndex = UINT32_MAX;
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if ((deviceInfo_.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
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
        if (presentQueueIndex == UINT32_MAX) {
            THROW_EXCEPT("Could not find a presenting queue!");
        }

        deviceInfo_.queueFamilyIndices.present = presentQueueIndex;

        // if present queue not createinfo push createinfo
        if ((deviceInfo_.queueFamilyIndices.present != deviceInfo_.queueFamilyIndices.graphics) &&
            (deviceInfo_.queueFamilyIndices.present != deviceInfo_.queueFamilyIndices.compute) &&
            (deviceInfo_.queueFamilyIndices.present != deviceInfo_.queueFamilyIndices.transfer)) {
            deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = deviceInfo_.queueFamilyIndices.present,
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority,
            });
        }
        // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    // Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (deviceInfo_.ExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        deviceInfo_.enableDebugMarkers = true;
    }

    if (deviceExtensions.size() > 0) {
        for (const char *enabledExtension : deviceExtensions) {
            if (!deviceInfo_.ExtensionSupported(enabledExtension)) {
                THROW_EXCEPT(std::string("Enabled device extension \"") + enabledExtension +
                             "\" is not present at device level");
            }
        }
    }

    VkDeviceCreateInfo deviceCI = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCIs.size()),
        .pQueueCreateInfos = deviceQueueCIs.data(),
        .enabledLayerCount = static_cast<uint32_t>(0),
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledFeatures,
    };

    VK_ASSERT_SUCCESSED(vkCreateDevice(deviceInfo_.physicalDevice, &deviceCI, nullptr, &device_));

    // GetQueue
    std::vector<uint32_t> allIndices{deviceInfo_.queueFamilyIndices.graphics,
                                     deviceInfo_.queueFamilyIndices.compute,
                                     deviceInfo_.queueFamilyIndices.transfer,
                                     deviceInfo_.queueFamilyIndices.present};
    std::vector<VkQueue *> queues{&graphicsQueue_, &computeQueue_, &transferQueue_, &presentQueue_};

    std::vector<uint32_t> queueIndices(allIndices.size(), UINT32_MAX);
    for (size_t i = 0; i < allIndices.size(); i++) {
        if (allIndices[i] != UINT32_MAX) {
            bool exist = false;
            for (size_t j = 0; j < queueIndices.size(); j++) {
                if (queueIndices[j] == allIndices[i]) {
                    exist = true;
                    *queues[i] = *queues[j];
                }
            }
            if (!exist) {
                queueIndices[i] = allIndices[i];
                vkGetDeviceQueue(device_, allIndices[i], 0, queues[i]);
            } else {
            }
        }
    }
}

void Graphics::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = deviceInfo_.queueFamilyIndices.graphics,
    };
    VK_ASSERT_SUCCESSED(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_))
}

void Graphics::CreateSwapchain()
{
    SwapChainSupportDetails swapchainSupport = QuerySwapChainSupport();
    VkSurfaceFormatKHR surfaceFormat = GetAvailableFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = GetAvailablePresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = GetAvailableExtent(swapchainSupport.capabilities);
    swapchainMinImageCount_ = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        swapchainMinImageCount_ > swapchainSupport.capabilities.maxImageCount)
        swapchainMinImageCount_ = swapchainSupport.capabilities.maxImageCount;
    VkSwapchainKHR oldSwapchainKHR = swapchainKHR_;
    VkSurfaceTransformFlagBitsKHR preTranform{};

    if ((swapchainSupport.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0) {
        preTranform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTranform = swapchainSupport.capabilities.currentTransform;
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
            if (swapchainSupport.capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
                compositeAlpha = compositeAlphaFlag;
                break;
            };
        }
    }

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surfaceKHR_,
        .minImageCount = swapchainMinImageCount_,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = preTranform,
        .compositeAlpha = compositeAlpha,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchainKHR,
    };
    uint32_t queueFamilyIndices[] = {deviceInfo_.queueFamilyIndices.graphics, deviceInfo_.queueFamilyIndices.present};
    if (deviceInfo_.queueFamilyIndices.graphics != deviceInfo_.queueFamilyIndices.present) {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = 2;
        swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VK_ASSERT_SUCCESSED(vkCreateSwapchainKHR(device_, &swapchainCI, nullptr, &swapchainKHR_));
    if (oldSwapchainKHR != VK_NULL_HANDLE) {
        DestroySwapchainImageViews();
        vkDestroySwapchainKHR(device_, oldSwapchainKHR, nullptr);
    }
    swapchainImageFormat_ = surfaceFormat.format;
    swapchainExtent_ = extent;
    vkGetSwapchainImagesKHR(device_, swapchainKHR_, &swapchainImageCount_, nullptr);
    swapchainImages_.resize(swapchainImageCount_);
    vkGetSwapchainImagesKHR(device_, swapchainKHR_, &swapchainImageCount_, swapchainImages_.data());
    CreateSwapchainImageViews();
}

void Graphics::CreateSwapchainImageViews()
{
    VkImageViewCreateInfo ImageViewCI{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                      .format = swapchainImageFormat_,
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
    swapchainImageViews_.resize(swapchainImageCount_);
    for (size_t i = 0; i < swapchainImageCount_; i++) {
        ImageViewCI.image = swapchainImages_[i];
        VK_ASSERT_SUCCESSED(vkCreateImageView(device_, &ImageViewCI, nullptr, &swapchainImageViews_[i]));
    }
}

void Graphics::CreateDepthResources()
{
    VkFormat depthFormat = deviceInfo_.FindDepthFormat();

    CreateImage(swapchainExtent_.width,
                swapchainExtent_.height,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImage_,
                depthImageMemory_);
    depthImageView_ = CreateImageView(depthImage_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Graphics::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{
        .format = swapchainImageFormat_,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = deviceInfo_.FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto colorAttachmentRef = GraphicsTools::MakeAttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    auto depthAttachmentRef = GraphicsTools::MakeAttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    auto subpass = GraphicsTools::MakeSubpassDescription(1, &colorAttachmentRef, &depthAttachmentRef);
    auto dependency = GraphicsTools::MakeSubpassDependency(
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    std::array<VkSubpassDescription, 1> subpasses = {subpass};
    std::array<VkSubpassDependency, 1> dependencies = {dependency};
    VkRenderPassCreateInfo renderPassCI{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = static_cast<uint32_t>(subpasses.size()),
        .pSubpasses = subpasses.data(),
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data(),
    };
    VK_ASSERT_SUCCESSED(vkCreateRenderPass(device_, &renderPassCI, nullptr, &renderPass_));
}

void Graphics::CreateGraphicsPipeline()
{

    auto vertShaderModule = CreateShaderModule(GetShadersPath() + "test.vert.spv");
    auto fragShaderModule = CreateShaderModule(GetShadersPath() + "test.frag.spv");
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCIs = {
        GraphicsTools::MakePipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT),
        GraphicsTools::MakePipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    auto vertexInputStateCI = GraphicsTools::MakePipelineVertexInputStateCreateInfo();
    auto inputAssemblyStateCI = GraphicsTools::MakePipelineInputAssemblyStateCreateInfo();
    auto tessellationStateCI = GraphicsTools::MakePipelineTessellationStateCreateInfo();
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = float(swapchainExtent_.width),
        .height = float(swapchainExtent_.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchainExtent_,
    };
    auto viewportStateCI = GraphicsTools::MakePipelineViewportStateCreateInfo(1, &viewport, 1, &scissor);
    auto rasterizationStateCI = GraphicsTools::MakePipelineRasterizationStateCreateInfo();
    auto multisampleStateCI = GraphicsTools::MakePipelineMultisampleStateCreateInfo();
    auto depthStencilStateCI = GraphicsTools::MakePipelineDepthStencilStateCreateInfo();
    auto colorBlendAttachmentState = GraphicsTools::MakePipelineColorBlendAttachmentState();
    auto colorBlendStateCI =
        GraphicsTools::MakePipelineColorBlendStateCreateInfo(VK_FALSE, VK_LOGIC_OP_COPY, 1, &colorBlendAttachmentState);
    auto dynamicStateCI = GraphicsTools::MakePipelineDynamicStateCreateInfo();

    auto pipelineLayoutCI = GraphicsTools::MakePipelineLayoutCreateInfo();
    VK_ASSERT_SUCCESSED(vkCreatePipelineLayout(device_, &pipelineLayoutCI, nullptr, &graphicsPipelineLayout_));

    VkGraphicsPipelineCreateInfo GraphicsPipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStageCIs.size()),
        .pStages = shaderStageCIs.data(),
        .pVertexInputState = &vertexInputStateCI,
        .pInputAssemblyState = &inputAssemblyStateCI,
        //.pTessellationState = &tessellationStateCI,
        .pViewportState = &viewportStateCI,
        .pRasterizationState = &rasterizationStateCI,
        .pMultisampleState = &multisampleStateCI,
        .pDepthStencilState = &depthStencilStateCI,
        .pColorBlendState = &colorBlendStateCI,
        .pDynamicState = &dynamicStateCI,
        .layout = graphicsPipelineLayout_,
        .renderPass = renderPass_,
        .subpass = 0,
        //.basePipelineHandle = basePipelineHandle,
        //.basePipelineIndex = basePipelineIndex,
    };
    VK_ASSERT_SUCCESSED(
        vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &GraphicsPipelineCI, nullptr, &graphicsPipeline_));
    vkDestroyShaderModule(device_, vertShaderModule, nullptr);
    vkDestroyShaderModule(device_, fragShaderModule, nullptr);
}

void Graphics::CreateFramebuffers()
{
    swapchainFramebuffers_.resize(swapchainImageCount_);
    for (size_t i = 0; i < swapchainImageCount_; i++) {
        std::array<VkImageView, 2> attachments = {swapchainImageViews_[i], depthImageView_};
        VkFramebufferCreateInfo framebufferCI{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass_,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swapchainExtent_.width,
            .height = swapchainExtent_.height,
            .layers = 1,
        };
        VK_ASSERT_SUCCESSED(vkCreateFramebuffer(device_, &framebufferCI, nullptr, &swapchainFramebuffers_[i]))
    }
}

void Graphics::CreateCommandBuffers()
{
    commandBuffers_.resize(swapchainImageCount_);

    VkCommandBufferAllocateInfo CommandBufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = swapchainImageCount_,
    };

    VK_ASSERT_SUCCESSED(vkAllocateCommandBuffers(device_, &CommandBufferAI, commandBuffers_.data()));
    
}

void Graphics::CreateSyncObjects()
{
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight_.resize(swapchainImageCount_, VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semaphoreCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(device_, &semaphoreCI, nullptr, &imageAvailableSemaphores_[i]));
        VK_ASSERT_SUCCESSED(vkCreateSemaphore(device_, &semaphoreCI, nullptr, &renderFinishedSemaphores_[i]));
        VK_ASSERT_SUCCESSED(vkCreateFence(device_, &fenceCI, nullptr, &inFlightFences_[i]));
    }
}

void Graphics::DestroySyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
        vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
        vkDestroyFence(device_, inFlightFences_[i], nullptr);
    }
}

void Graphics::FreeCommandBuffers()
{
    vkFreeCommandBuffers(device_, commandPool_, static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
}

void Graphics::DestroyFramebuffers()
{
    for (auto swapahainFramebuffer : swapchainFramebuffers_)
        vkDestroyFramebuffer(device_, swapahainFramebuffer, nullptr);
    swapchainFramebuffers_.clear();
}

void Graphics::DestroyGraphicsPipeline()
{
    vkDestroyPipelineLayout(device_, graphicsPipelineLayout_, nullptr);
    vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
}

void Graphics::DestroyRenderPass()
{
    vkDestroyRenderPass(device_, renderPass_, nullptr);
}

void Graphics::DestroyDepthResources()
{
    vkDestroyImageView(device_, depthImageView_, nullptr);
    vkDestroyImage(device_, depthImage_, nullptr);
    vkFreeMemory(device_, depthImageMemory_, nullptr);
}

void Graphics::DestroySwapchainImageViews()
{
    for (auto imageView : swapchainImageViews_)
        vkDestroyImageView(device_, imageView, nullptr);
    swapchainImageViews_.clear();
}

void Graphics::DestroySwapchain()
{
    DestroySwapchainImageViews();
    vkDestroySwapchainKHR(device_, swapchainKHR_, nullptr);
}

void Graphics::DestroyCommandPool()
{
    vkDestroyCommandPool(device_, commandPool_, nullptr);
}

void Graphics::DestroyDevice()
{
    assert(device_ != VK_NULL_HANDLE);
    vkDestroyDevice(device_, nullptr);
}

void Graphics::DestroyDebugReporter()
{
    auto fpDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT"));
    assert(fpDestroyDebugReportCallbackEXT != nullptr);
    fpDestroyDebugReportCallbackEXT(instance_, fpDebugReportCallbackEXT_, nullptr);
}

void Graphics::DestroyInstance()
{
    vkDestroyInstance(instance_, nullptr);
    instance_ = VK_NULL_HANDLE;
}

void Graphics::RecreateSwapchain()
{
    GDF_LOG(GraphicsLog, LogLevel::Info, "RecreateSwapchain");
    RequireRecreateSwapchain_ = false;
    int width = 0, height = 0;
    glfwGetFramebufferSize(pWindow_->pGLFWWindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(pWindow_->pGLFWWindow(), &width, &height);
        glfwWaitEvents();
    }

    // cleanup
    DeviceWaitIdle();

    ImGuiResourceDestroy();

    FreeCommandBuffers();
    DestroyFramebuffers();
    DestroyGraphicsPipeline();
    DestroyRenderPass();
    DestroyDepthResources();

    // create
    CreateSwapchain();
    CreateDepthResources();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();

    ImGuiUpdateMinImageCount(swapchainMinImageCount_);
    ImGuiResourceCreate();
}

void Graphics::ImGuiCreate()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    //ImGuiIO &io = ImGui::GetIO().ConfigFlags|= ImGuiConfigFlags_DockingEnable;

    io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF((File::GetExeDir() + "/fonts/Roboto-Medium.ttf").c_str(), 16.0f);
    io.Fonts->AddFontFromFileTTF((File::GetExeDir() + "/fonts/Cousine-Regular.ttf").c_str(), 15.0f);
    io.Fonts->AddFontFromFileTTF((File::GetExeDir() + "/fonts/DroidSans.ttf").c_str(), 16.0f);
    io.Fonts->AddFontFromFileTTF((File::GetExeDir() + "/fonts/ProggyTiny.ttf").c_str(), 10.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiCreateDescriptorPool();
    ImGuiResourceCreate();

    ImGui_ImplGlfw_InitForVulkan(pWindow_->pGLFWWindow(), true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = instance_;
    initInfo.PhysicalDevice = deviceInfo_.physicalDevice;
    initInfo.Device = device_;
    initInfo.QueueFamily = deviceInfo_.queueFamilyIndices.graphics;
    initInfo.Queue = graphicsQueue_;
    initInfo.PipelineCache = imguiPipelineCache_;
    initInfo.DescriptorPool = imguiDescriptorPool_;
    initInfo.Allocator = imguiAllocator_;
    initInfo.MinImageCount = swapchainMinImageCount_;
    initInfo.ImageCount = swapchainImageCount_;
    initInfo.CheckVkResultFn = ImGuiCheckVkResultCallback;
    ImGui_ImplVulkan_Init(&initInfo, imguiRenderPass_);

    ImGuiUploadFonts();
}

void Graphics::ImGuiDestroy()
{
    ImGuiResourceDestroy();
    vkDestroyDescriptorPool(device_, imguiDescriptorPool_, imguiAllocator_);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Graphics::ImGuiResourceCreate()
{
    ImGuiCreateRenderPass();
    ImGuiCreateFramebuffer();
    ImGuiCreateCommandBuffer();
}

void Graphics::ImGuiResourceDestroy()
{
    vkFreeCommandBuffers(device_, commandPool_, static_cast<uint32_t>(imguiFramebuffers_.size()), imguiCommandBuffers_.data());
    for (size_t i = 0; i < imguiFramebuffers_.size(); i++) {
        vkDestroyFramebuffer(device_, imguiFramebuffers_[i], nullptr);
    }
    vkDestroyRenderPass(device_, imguiRenderPass_, nullptr);
}

void Graphics::ImGuiFrameBegin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    imGuiDrawData_ = ImGui::GetDrawData();
}

void Graphics::ImGuiFrameRender(uint32_t imageIndex)
{

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_ASSERT_SUCCESSED(vkBeginCommandBuffer(imguiCommandBuffers_[imageIndex], &beginInfo));

    VkClearValue clearValues{0.0f, 0.0f, 0.0f, 0.0f};
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = imguiRenderPass_;
    renderPassInfo.framebuffer = imguiFramebuffers_[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainExtent_;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(1);
    renderPassInfo.pClearValues = &clearValues;

    vkCmdBeginRenderPass(imguiCommandBuffers_[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(imGuiDrawData_, imguiCommandBuffers_[imageIndex]);

    vkCmdEndRenderPass(imguiCommandBuffers_[imageIndex]);

    VK_ASSERT_SUCCESSED(vkEndCommandBuffer(imguiCommandBuffers_[imageIndex]));
}

void Graphics::ImGuiFrameEnd()
{
}

void Graphics::ImGuiCreateDescriptorPool()
{
    VkDescriptorPoolSize descriptorPoolSize[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                                 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    VkDescriptorPoolCreateInfo descriptorPoolCI = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000 * IM_ARRAYSIZE(descriptorPoolSize),
        .poolSizeCount = (uint32_t)IM_ARRAYSIZE(descriptorPoolSize),
        .pPoolSizes = descriptorPoolSize,
    };
    VK_ASSERT_SUCCESSED(vkCreateDescriptorPool(device_, &descriptorPoolCI, imguiAllocator_, &imguiDescriptorPool_));
}

void Graphics::ImGuiCreateRenderPass()
{
    VkAttachmentDescription attachmentDesc{
        .format = swapchainImageFormat_,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference attachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpassDesc = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentRef,
    };
    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0, // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo RenderPassCI = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachmentDesc,
        .subpassCount = 1,
        .pSubpasses = &subpassDesc,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency,
    };
    VK_ASSERT_SUCCESSED(vkCreateRenderPass(device_, &RenderPassCI, nullptr, &imguiRenderPass_))
}

void Graphics::ImGuiCreateFramebuffer()
{
    VkImageView attachment[1];
    VkFramebufferCreateInfo FramebufferCI = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = imguiRenderPass_,
    .attachmentCount = 1,
    .pAttachments = attachment,
    .width = swapchainExtent_.width,
    .height = swapchainExtent_.height,
    .layers = 1,
    };
    imguiFramebuffers_.resize(swapchainImageCount_);
    for (uint32_t i = 0; i < swapchainImageCount_; i++) {
        attachment[0] = swapchainImageViews_[i];
        VK_ASSERT_SUCCESSED(vkCreateFramebuffer(device_, &FramebufferCI, nullptr, &imguiFramebuffers_[i]));
    }
}

void Graphics::ImGuiCreateCommandBuffer()
{
    imguiCommandBuffers_.resize(swapchainImageCount_);

    VkCommandBufferAllocateInfo CommandBufferAI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = swapchainImageCount_,
    };
    VK_ASSERT_SUCCESSED(vkAllocateCommandBuffers(device_, &CommandBufferAI, imguiCommandBuffers_.data()));
}

void Graphics::ImGuiUploadFonts()
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommand();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    EndSingleTimeCommand(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Graphics::ImGuiUpdateMinImageCount(uint32_t minImageCount)
{
    ImGui_ImplVulkan_SetMinImageCount(minImageCount);
}

void Graphics::ImGuiCheckVkResultCallback(VkResult result)
{
    VK_ASSERT_SUCCESSED(result);
}

bool Graphics::IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    // find discrete GPU
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return true;
    return false;
}

// bool Graphics::GetSupportPresentQueue(VkSurfaceKHR surface, VkQueue &queue)
//{
//    VkBool32 supported = VK_FALSE;
//    // if graphics queue support present, use it
//    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, queueFamilyIndices_.graphics, surface, &supported);
//    // else detect support queue
//    if (supported == VK_FALSE) {
//        for (size_t i = 0; i < queueFamilyIndices_.queueFamilyProperties.size(); i++) {
//            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface, &supported);
//            if (supported == VK_TRUE) {
//                if (i != queueFamilyIndices_.graphics) {
//                    vkGetDeviceQueue(device_, i, 0, &queue);
//                }
//                return true;
//            }
//        }
//        return false;
//    }
//    queue = graphicsCommandQueue_->queue();
//    return true;
//}

VkShaderModule Graphics::CreateShaderModule(const std::string &shaderPath)
{
    return CreateShaderModule(File::ReadBytes(shaderPath));
}

// helpful function
VkShaderModule Graphics::CreateShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VK_ASSERT_SUCCESSED(vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule))
    return shaderModule;
}

void Graphics::CreateImage(uint32_t width,
                           uint32_t height,
                           VkFormat format,
                           VkImageTiling tiling,
                           VkImageUsageFlags usage,
                           VkMemoryPropertyFlags properties,
                           VkImage &image,
                           VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo imageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent =
            {
                .width = width,
                .height = height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VK_ASSERT_SUCCESSED(vkCreateImage(device_, &imageCI, nullptr, &image))

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = deviceInfo_.FindMemoryType(memRequirements.memoryTypeBits, properties);

    VK_ASSERT_SUCCESSED(vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory))
    VK_ASSERT_SUCCESSED(vkBindImageMemory(device_, image, imageMemory, 0));
}

VkImageView Graphics::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageView imageView;
    VkImageViewCreateInfo imageViewCI{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      .image = image,
                                      .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                      .format = format,
                                      .components = {},
                                      .subresourceRange{
                                          .aspectMask = aspectFlags,
                                          .baseMipLevel = 0,
                                          .levelCount = 1,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1,
                                      }};
    VK_ASSERT_SUCCESSED(vkCreateImageView(device_, &imageViewCI, nullptr, &imageView))
    return imageView;
}

// Command Helper

inline VkCommandBuffer Graphics::BeginSingleTimeCommand()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool_;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_ASSERT_SUCCESSED(vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_ASSERT_SUCCESSED(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    return commandBuffer;
}

inline void Graphics::EndSingleTimeCommand(VkCommandBuffer commandBuffer)
{
    VK_ASSERT_SUCCESSED(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VK_ASSERT_SUCCESSED(vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE));
    VK_ASSERT_SUCCESSED(vkQueueWaitIdle(graphicsQueue_));
    vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
}

void Graphics::DeviceWaitIdle()
{
    VK_ASSERT_SUCCESSED(vkDeviceWaitIdle(device_));
}

SwapChainSupportDetails Graphics::QuerySwapChainSupport()
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceInfo_.physicalDevice, surfaceKHR_, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(deviceInfo_.physicalDevice, surfaceKHR_, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(deviceInfo_.physicalDevice, surfaceKHR_, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(deviceInfo_.physicalDevice, surfaceKHR_, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            deviceInfo_.physicalDevice, surfaceKHR_, &presentModeCount, details.presentModes.data());
    }
    return details;
}

VkSurfaceFormatKHR Graphics::GetAvailableFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    return availableFormats[0];
}

VkPresentModeKHR Graphics::GetAvailablePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Graphics::GetAvailableExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(pWindow_->pGLFWWindow(), &width, &height);
        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        actualExtent.width =
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height =
            std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

VkBool32 Graphics::DebugReportCallbackEXT(VkDebugReportFlagsEXT flags,
                                          VkDebugReportObjectTypeEXT objectType,
                                          uint64_t object,
                                          size_t location,
                                          int32_t messageCode,
                                          const char *pLayerPrefix,
                                          const char *pMessage,
                                          void *pUserData)
{
    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
        GDF_LOG(GraphicsLog, LogLevel::Error, "[{}] code {} : {}", pLayerPrefix, messageCode, pMessage);
    }
    if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 || (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0) {
        GDF_LOG(GraphicsLog, LogLevel::Warning, "[{}] code {} : {}", pLayerPrefix, messageCode, pMessage);
    }
    if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
        GDF_LOG(GraphicsLog, LogLevel::Info, "[{}] code {} : {}", pLayerPrefix, messageCode, pMessage);
    }
    if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) {
        GDF_LOG(GraphicsLog, LogLevel::Debug, "[{}] code {} : {}", pLayerPrefix, messageCode, pMessage);
    }

    return VK_FALSE;
}

std::string Graphics::GetShadersPath()
{
#ifdef _WIN32
    return File::GetExeDir() + "/../../../../shaders/";
#else
    return File::GetExeDir() + "/../../../shaders/";
#endif
}

} // namespace gdf