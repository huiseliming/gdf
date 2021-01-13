#include "Graphics/PhysicalDevice.h"
#include "Base/File.h"
#include <cassert>
namespace gdf
{

void PhysicalDevice::Parse(VkPhysicalDevice physicalDevice, bool enableGetPhysicalDeviceProperty2Extension)
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

uint32_t PhysicalDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
    return GraphicsTools::GetQueueFamilyIndex(queueFamilyProperties, queueFlags);
}

bool PhysicalDevice::ExtensionSupported(std::string extension)
{
    return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkFormat PhysicalDevice::FindDepthFormat()
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

VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat> &candidates,
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

uint32_t PhysicalDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
