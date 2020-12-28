#include "Renderer/Graphics.h"
#include "Base/Window.h"
#include "Git.h"
#include "Renderer/Swapchain.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#define GIT_UINT32_VERSION                                                                                                     \
    (static_cast<uint32_t>(wcstoul(GIT_VERSION_MAJOR, nullptr, 10) && 0xF) << 28) ||                                           \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_MINOR, nullptr, 10) && 0xFF) << 20) ||                                      \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_PATCH, nullptr, 10) && 0xFFFFF))
#define VK_API_VERSION VK_MAKE_VERSION(1, 0, 0)

namespace gdf
{
GDF_DEFINE_EXPORT_LOG_CATEGORY(GraphicsLog);

bool Graphics::Initialize(bool enableValidationLayer)
{
    enableValidationLayer_ = enableValidationLayer;
    // instance extensions 
    std::vector<const char *> instanceExtensions;
    if (!Window::GetRequiredInstanceExtensions(instanceExtensions))
        return false;
    // instance layers
    std::vector<const char *> instanceLayers;
    if (enableValidationLayer_) {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        instanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    // device extensions 
    std::vector<const char *> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // Instance
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

    // Setup DebugReportCallback
    if (enableValidationLayer_) {
        VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCI{
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
            .flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT |
                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
            //| VK_DEBUG_REPORT_INFORMATION_BIT_EXT
            ,
            .pfnCallback = Graphics::DebugReportCallbackEXT,
        };
        auto fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT"));
        fpCreateDebugReportCallbackEXT(instance_, &DebugReportCallbackCI, nullptr, &fpDebugReportCallbackEXT_);
    }

    // Physical Device
    uint32_t physicalDeviceCount;
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data()));

    std::vector<VkPhysicalDevice> availablePhysicalDevices;
    for (auto physicalDevice : physicalDevices) {
        if (IsPhysicalDeviceSuitable(physicalDevice))
            physicalDevice_ = physicalDevice;
    }
    if (physicalDevice_ == VK_NULL_HANDLE)
        return false;
    // Queues
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilyProperties.data());

    queueFamilyIndices.DetectQueueFamilyIndices(queueFamilyProperties);

    std::vector<VkDeviceQueueCreateInfo> queuesCI;
    uint32_t maxQueueCount = 0;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queuesCI.push_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = queueFamilyProperties[i].queueCount,
        });
        maxQueueCount = std::max(maxQueueCount, queueFamilyProperties[i].queueCount);
    }

    std::vector<float> queuePriorities(maxQueueCount, 0.0f);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queuesCI[i].pQueuePriorities = queuePriorities.data();
    }

    // Logical Device
    VkDeviceCreateInfo deviceCI{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                .queueCreateInfoCount = static_cast<uint32_t>(queuesCI.size()),
                                .pQueueCreateInfos = queuesCI.data(),
                                .enabledLayerCount = 0,
                                .ppEnabledLayerNames = nullptr,
                                .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                                .ppEnabledExtensionNames = deviceExtensions.data(),
                                .pEnabledFeatures = &physicalDeviceFeatures_};
    VK_ASSERT_SUCCESSED(vkCreateDevice(physicalDevice_, &deviceCI, nullptr, &device_));
    // 
    queueFamilies_.resize(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++)
        queueFamilies_[i].Attach(device_, i, queueFamilyProperties[i].queueCount);

    //CommandPool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    VK_ASSERT_SUCCESSED(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_))
    return true;
}

void Graphics::Cleanup()
{
    swapchain_.reset();
    vkDestroyCommandPool(device_,commandPool_, nullptr);
    vkDestroyDevice(device_, nullptr);
    device_ = VK_NULL_HANDLE;
    if (enableValidationLayer_) {
        auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT"));
        vkDestroyDebugReportCallbackEXT(instance_, fpDebugReportCallbackEXT_, nullptr);
    }
    vkDestroyInstance(instance_, nullptr);
    instance_ = VK_NULL_HANDLE;
}

void Graphics::SetSwapchain(std::unique_ptr<Swapchain> &&swapchain)
{
    swapchain_ = std::move(swapchain);
}

bool Graphics::IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return true;
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

void Graphics::DeviceWaitIdle()
{
    VK_ASSERT_SUCCESSED(vkDeviceWaitIdle(device_));
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

void Graphics::QueueFamilyIndices::DetectQueueFamilyIndices(std::vector<VkQueueFamilyProperties> &queueFamilyProperties)
{
    this->queueFamilyProperties = queueFamilyProperties;
    graphics = GetQueueFamilyIndex(queueFamilyProperties, VK_QUEUE_GRAPHICS_BIT);
    compute = GetQueueFamilyIndex(queueFamilyProperties, VK_QUEUE_COMPUTE_BIT);
    transfer = GetQueueFamilyIndex(queueFamilyProperties, VK_QUEUE_TRANSFER_BIT);
}

bool Graphics::QueueFamilyIndices::DetectPresentQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkBool32 supported = VK_FALSE;
    // if graphics queue support present, use it
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphics, surface, &supported);
    // else detect support queue
    if (supported == VK_FALSE) {
        for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supported);
            if (supported == VK_TRUE) {
                present = i;
                return supported;
            }
        }
        return supported;
    }
    present = graphics;
    return supported;
}
uint32_t Graphics::QueueFamilyIndices::GetQueueFamilyIndex(
    std::vector<VkQueueFamilyProperties> &queueFamilyProperties, VkQueueFlagBits queueFlags)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested
    // flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if (queueFamilyProperties[i].queueFlags & queueFlags) {
            return i;
        }
    }
    return UINT32_MAX;
}
} // namespace gdf