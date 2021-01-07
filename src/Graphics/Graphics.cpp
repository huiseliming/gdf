#include "Base/Window.h"
#include "Graphics/Graphics.h"
#include "Git.h"
#include "Graphics/Swapchain.h"
#include "Graphics/GraphicsQueue.h"
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

VkInstance Graphics::vulkanInstance_ = VK_NULL_HANDLE;

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

    VK_ASSERT_SUCCESSED(vkCreateInstance(&instanceCI, nullptr, &vulkanInstance_));

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
            vkGetInstanceProcAddr(vulkanInstance_, "vkCreateDebugReportCallbackEXT"));
        fpCreateDebugReportCallbackEXT(vulkanInstance_, &DebugReportCallbackCI, nullptr, &fpDebugReportCallbackEXT_);
    }
    // Physical Device
    uint32_t physicalDeviceCount;
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(vulkanInstance_, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(vulkanInstance_, &physicalDeviceCount, physicalDevices.data()));
    
    std::vector<VkPhysicalDevice> availablePhysicalDevices;
    for (auto physicalDevice : physicalDevices) {
        if (IsPhysicalDeviceSuitable(physicalDevice))
            pDevice_ = std::make_unique<Device>(physicalDevice);
    }
    if (pDevice_ == nullptr)
        return false;
    // device extensions
    std::vector<const char *> deviceExtensions;
    pDevice_->CreateLogicalDevice(VkPhysicalDeviceFeatures{}, deviceExtensions,nullptr);
    // Get a graphics queue from the device
    VkQueue queue;
    vkGetDeviceQueue(*pDevice_, pDevice_->queueFamilyIndices.graphics, 0, &queue);
    pGraphicsQueue_ = std::make_unique<GraphicsQueue>(queue);
    return true;
}

void Graphics::Cleanup()
{
    DeviceWaitIdle();
    pSwapchain_.reset();
    pDevice_.reset();
    if (enableValidationLayer_) {
        auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(vulkanInstance_, "vkDestroyDebugReportCallbackEXT"));
        vkDestroyDebugReportCallbackEXT(vulkanInstance_, fpDebugReportCallbackEXT_, nullptr);
    }
    vkDestroyInstance(vulkanInstance_, nullptr);
    vulkanInstance_ = VK_NULL_HANDLE;
}

void Graphics::DrawFrame()
{
}

void Graphics::SetSwapchain(std::unique_ptr<Swapchain> &&swapchain)
{
    pSwapchain_ = std::move(swapchain);
}

bool Graphics::IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    //find discrete GPU
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return true;
    return false;
}

//bool Graphics::GetSupportPresentQueue(VkSurfaceKHR surface, VkQueue &queue)
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
    VK_ASSERT_SUCCESSED(vkCreateShaderModule(*pDevice_, &createInfo, nullptr, &shaderModule))
    return shaderModule;
}

void Graphics::DeviceWaitIdle()
{
    VK_ASSERT_SUCCESSED(vkDeviceWaitIdle(*pDevice_));
}

Device &Graphics::device()
{
    return *pDevice_;
}

GraphicsQueue &Graphics::graphicsQueue()
{
    return *pGraphicsQueue_;
}

Swapchain &Graphics::swapchain()
{
    return *pSwapchain_;
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