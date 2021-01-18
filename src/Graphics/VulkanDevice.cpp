#include "Graphics/VulkanDevice.h"
#include "Base/File.h"
#include <cassert>
#include <vector>
namespace gdf
{

void VulkanDevice::AttachPhysicalDevice(VkPhysicalDevice physicalDevice, bool enableGetPhysicalDeviceProperty2Extension)
{
    this->physicalDevice = physicalDevice;
    // Store Properties features, limits and properties of the physical device for later use
    // Device properties also contain limits and sparse properties
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    // Features should be checked by the examples before using them
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    // Memory properties are used regularly for creating all kinds of buffers
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    // Queue family properties, used for setting up requested queues upon device creation
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // Get list of supported extensions
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    if (extensionCount > 0) {
        std::vector<VkExtensionProperties> extensions(extensionCount);
        if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS) {
            for (auto extension : extensions) {
                supportedExtensions.push_back(extension.extensionName);
            }
        }
    }

    // GetPhysicalDeviceFeatures2
    if (enableGetPhysicalDeviceProperty2Extension) {

        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

#ifdef __APPLE__
        features2.pNext = &portabilitySubsetFeaturesKHR;
        portabilitySubsetFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR,
        portabilitySubsetFeaturesKHR.pNext = nullptr;
#else
        features2.pNext = nullptr;
#endif
        vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
        // VkPhysicalDeviceProperties2 p;
        // vkGetPhysicalDeviceProperties2(deviceInfo_.physicalDevice, );
    }
}

void VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
                                       VkSurfaceKHR surface,
                                       std::vector<const char *> enabledExtensions,
                                       std::vector<const char *> instanceExtensions)
{
    // Logical Device
    assert(logicalDevice == VK_NULL_HANDLE);
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs;
    const float defaultQueuePriority(0.0f);
    // Graphics queue
    queueFamilyIndices.graphics = GraphicsTools::GetQueueFamilyIndex(queueFamilyProperties, VK_QUEUE_GRAPHICS_BIT);
    if (queueFamilyIndices.graphics == UINT32_MAX)
        THROW_EXCEPT("No found graphics queue family indices!");
    deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndices.graphics,
        .queueCount = 1,
        .pQueuePriorities = &defaultQueuePriority,
    });

    // Compute queue
    queueFamilyIndices.compute = GraphicsTools::GetQueueFamilyIndex(queueFamilyProperties, VK_QUEUE_COMPUTE_BIT);
    if (queueFamilyIndices.compute == UINT32_MAX)
        THROW_EXCEPT("No found compute queue family indices!");
    if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
        deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilyIndices.compute,
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority,
        });
    }

    // Dedicated transfer queue
    queueFamilyIndices.transfer = GraphicsTools::GetQueueFamilyIndex(queueFamilyProperties, VK_QUEUE_TRANSFER_BIT);
    if (queueFamilyIndices.transfer == UINT32_MAX)
        THROW_EXCEPT("No found transfer queue family indices!");
    if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) &&
        (queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
        deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilyIndices.transfer,
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority,
        });
    }

    // Create the logical device representation
    std::vector<const char *> deviceExtensions(enabledExtensions);
    if (surface != VK_NULL_HANDLE) {
        // Dedicated present queue
        uint32_t queueFamilyCount = queueFamilyProperties.size();
        std::vector<VkBool32> supportsPresent(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; i++)
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);

        // Search for a graphics and a present queue in the array of queue
        // families, try to find one that supports both
        uint32_t graphicsQueueIndex = UINT32_MAX;
        uint32_t presentQueueIndex = UINT32_MAX;
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
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

        queueFamilyIndices.present = presentQueueIndex;

        // if present queue not createinfo push createinfo
        if ((queueFamilyIndices.present != queueFamilyIndices.graphics) &&
            (queueFamilyIndices.present != queueFamilyIndices.compute) &&
            (queueFamilyIndices.present != queueFamilyIndices.transfer)) {
            deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamilyIndices.present,
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority,
            });
        }
        // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    // Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (ExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        enableDebugMarkers = true;
    }

#ifdef __APPLE__
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if ((std::find_if(instanceExtensions.begin(),
                      instanceExtensions.end(),
                      [](const char *extensionName) {
                          return !strcmp(extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
                      }) != instanceExtensions.end()) &&
        ExtensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        enablePortabilitySubsetExtension_ = true;
    }
#endif
    if (deviceExtensions.size() > 0) {
        for (const char *enabledExtension : deviceExtensions) {
            if (!ExtensionSupported(enabledExtension)) {
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

    VK_ASSERT_SUCCESSED(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &logicalDevice));
    std::vector<uint32_t> allIndices{
        queueFamilyIndices.graphics, queueFamilyIndices.compute, queueFamilyIndices.transfer, queueFamilyIndices.present};
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
                vkGetDeviceQueue(logicalDevice, allIndices[i], 0, queues[i]);
            } else {
            }
        }
    }
}

uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
    return GraphicsTools::GetQueueFamilyIndex(queueFamilyProperties, queueFlags);
}

bool VulkanDevice::ExtensionSupported(std::string extension)
{
    return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkFormat VulkanDevice::FindDepthFormat()
{
    return FindSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat VulkanDevice::FindSupportedFormat(const std::vector<VkFormat> &candidates,
                                           VkImageTiling tiling,
                                           VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED;
}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    THROW_EXCEPT("Failed to find suitable memory type!");
}

} // namespace gdf
