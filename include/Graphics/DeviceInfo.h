#pragma once
#include "VulkanApi.h"
#include <vector>

namespace gdf
{

struct DeviceInfo 
{
    /** @brief Physical device representation */
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    /** @brief Properties of the physical device including limits that the application can check against */
    VkPhysicalDeviceProperties properties;
    /** @brief Features of the physical device that an application can use to check if a feature is supported */
    VkPhysicalDeviceFeatures features;
    /** @brief Features that have been enabled for use on the physical device */
    VkPhysicalDeviceFeatures enabledFeatures;
    /** @brief Memory types and heaps of the physical device */
    VkPhysicalDeviceMemoryProperties memoryProperties;
    /** @brief Queue family properties of the physical device */
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    /** @brief List of extensions supported by the device */
    std::vector<std::string> supportedExtensions;
    /** @brief Set to true when the debug marker extension is detected */
    bool enableDebugMarkers = false;
    /** @brief Contains queue family indices */

    struct {
        uint32_t graphics{UINT32_MAX};
        uint32_t compute{UINT32_MAX};
        uint32_t transfer{UINT32_MAX};
        uint32_t present{UINT32_MAX};
    } queueFamilyIndices;

    DeviceInfo() = default;
    void Parse(VkPhysicalDevice physicalDevice);

    uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
    bool ExtensionSupported(std::string extension);



};

} // namespace gdf
