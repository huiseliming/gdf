#include "Renderer/Graphics.h"
#include "Base/Exception.h"
#include "Git.h"
#include "Base/Window.h"
#include <GLFW/glfw3.h>
#include <vector>


#define GIT_UINT32_VERSION                                                                         \
    (static_cast<uint32_t>(wcstoul(GIT_VERSION_MAJOR, nullptr, 10) && 0xF) << 28) ||               \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_MINOR, nullptr, 10) && 0xFF) << 20) ||          \
        (static_cast<uint32_t>(wcstoul(GIT_VERSION_PATCH, nullptr, 10) && 0xFFFFF))
#define VK_API_VERSION VK_MAKE_VERSION(1, 0, 0)

#include <iostream>
bool Graphics::Initialize()
{
    std::vector<std::string> glfwRequiredInstanceExtensions;
    if (!Window::GetRequiredInstanceExtensions(glfwRequiredInstanceExtensions))
        return false;
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
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
    };
    VK_ASSERT_SUCCESSED(vkCreateInstance(&instanceCI, nullptr, &instance_));
    // Physical Device
    uint32_t physicalDeviceCount;
    VK_ASSERT_SUCCESSED(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
    VK_ASSERT_SUCCESSED(
        vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data()));

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
    std::vector<float> queuePriorities(maxQueueCount,0.0f);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queuesCI[i].pQueuePriorities = queuePriorities.data();
    }

    // Logical Device 
    VkDeviceCreateInfo deviceCI{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                .queueCreateInfoCount = static_cast<uint32_t>(queuesCI.size()),
                                .pQueueCreateInfos = queuesCI.data(),
                                .enabledLayerCount = 0,
                                .ppEnabledLayerNames = nullptr,
                                .enabledExtensionCount = 0,
                                .ppEnabledExtensionNames = nullptr,
                                .pEnabledFeatures = &physicalDeviceFeatures_};
    VK_ASSERT_SUCCESSED(vkCreateDevice(physicalDevice_, &deviceCI, nullptr, &device_));
    //
    queueFamilies.resize(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++)
        queueFamilies[i].Attach(device_, queueFamilyProperties[i].queueFlags, i, queueFamilyProperties[i].queueCount);

    return true;
}

void Graphics::Cleanup()
{
    vkDestroyDevice(device_, nullptr);
    device_ = VK_NULL_HANDLE;
    vkDestroyInstance(instance_, nullptr);
    instance_ = VK_NULL_HANDLE;
}

bool Graphics::IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return true;
}
