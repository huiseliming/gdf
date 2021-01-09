#include "Git.h"
#include "Graphics\Graphics.h"


#define GIT_UINT32_VERSION                                                                                                     \
    (static_cast<uint32_t>(wcstoul(GIT_VERSION_MAJOR, nullptr, 10) && 0xF) << 28) ||                                           \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_MINOR, nullptr, 10) && 0xFF) << 20) ||                                      \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_PATCH, nullptr, 10) && 0xFFFFF))
#define VK_API_VERSION VK_MAKE_VERSION(1, 0, 0)

namespace gdf
{

GDF_DEFINE_EXPORT_LOG_CATEGORY(GraphicsLog);


void Graphics::Initialize(Window* pWindow, bool enableValidationLayer)
{
    pWindow_ = pWindow;
    enableValidationLayer_ = enableValidationLayer;
    CreateInstance();
    // Setup DebugReportCallback
    if (enableValidationLayer_) CreateDebugReporter();
    if (pWindow) pWindow_->GetVkSurfaceKHR(instance_, &surfaceKHR_);
    CreateDevice({}, {});
}

void Graphics::DrawFrame()
{
}

void Graphics::Cleanup()
{
    DeviceWaitIdle();
    DestroyDevice();
    if (enableValidationLayer_) DestroyDebugReporter();
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
    deviceInfo_.queueFamilyIndices.graphics = GraphicsTools::GetQueueFamilyIndex(deviceInfo_.queueFamilyProperties, VK_QUEUE_GRAPHICS_BIT);
    if (deviceInfo_.queueFamilyIndices.graphics == UINT32_MAX)
        THROW_EXCEPT("No found graphics queue family indices!");
    deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = deviceInfo_.queueFamilyIndices.graphics,
        .queueCount = 1,
        .pQueuePriorities = &defaultQueuePriority,
    });

    // Compute queue
    deviceInfo_.queueFamilyIndices.compute = GraphicsTools::GetQueueFamilyIndex(deviceInfo_.queueFamilyProperties, VK_QUEUE_COMPUTE_BIT);
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
    deviceInfo_.queueFamilyIndices.transfer = GraphicsTools::GetQueueFamilyIndex(deviceInfo_.queueFamilyProperties, VK_QUEUE_TRANSFER_BIT);
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

void Graphics::CreateSwapchain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();
    VkSurfaceFormatKHR surfaceFormat = GetAvailableFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = GetAvailablePresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = GetAvailableExtent(swapChainSupport.capabilities);
    uint32_t swapahainImageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && swapahainImageCount > swapChainSupport.capabilities.maxImageCount)
        swapahainImageCount = swapChainSupport.capabilities.maxImageCount;
    VkSwapchainKHR oldSwapchainKHR = swapchainKHR_;
    VkSurfaceTransformFlagBitsKHR preTranform{};

    if ((swapChainSupport.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0) {
        preTranform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTranform = swapChainSupport.capabilities.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    if (std::find(compositeAlphaFlags.begin(), compositeAlphaFlags.end(), VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != compositeAlphaFlags.end()) {
        compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    } else {
        // TODO: print warn
        for (auto &compositeAlphaFlag : compositeAlphaFlags) {
            if (swapChainSupport.capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
                compositeAlpha = compositeAlphaFlag;
                break;
            };
        }
    }

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surfaceKHR_,
        .minImageCount = swapahainImageCount,
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
        DestroyImageViews();
        vkDestroySwapchainKHR(device_, oldSwapchainKHR, nullptr);
    }
    vkGetSwapchainImagesKHR(device_, swapchainKHR_, &swapahainImageCount, nullptr);
    swapahainImages.resize(swapahainImageCount);
    vkGetSwapchainImagesKHR(device_, swapchainKHR_, &swapahainImageCount, swapahainImages.data());
}

void Graphics::CreateImageViews()
{
    VkImageViewCreateInfo ImageViewCI{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                      .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                      .format = swapChainImageFormat,
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
    swapahainImageViews.resize(swapahainImageCount);
    for (size_t i = 0; i < swapahainImageCount; i++) {
        ImageViewCI.image = swapahainImages[i];
        VK_ASSERT_SUCCESSED(vkCreateImageView(device_, &ImageViewCI, nullptr, &swapahainImageViews[i]));
    }
}

void Graphics::DestroyImageViews()
{
    for (auto imageView : swapahainImageViews)
            vkDestroyImageView(device_, imageView, nullptr);
    swapahainImageViews.clear();
}

void Graphics::DestroySwapchain()
{
    vkDestroySwapchainKHR(device_, swapchainKHR_, nullptr);
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
        vkGetPhysicalDeviceSurfacePresentModesKHR(deviceInfo_.physicalDevice, surfaceKHR_, &presentModeCount, details.presentModes.data());
    }
    return details;
}

VkSurfaceFormatKHR Graphics::GetAvailableFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats) 
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
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
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
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

} // namespace gdf