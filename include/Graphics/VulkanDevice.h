#pragma once
#include "VulkanApi.h"
#include <vector>

namespace gdf
{

struct VulkanDevice {
    /** @brief Logical device representation (application's view of the device) */
    VkDevice logicalDevice{VK_NULL_HANDLE};
    /** @brief Physical device representation */
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    /** @brief Properties of the physical device including limits that the application can check against */
    VkPhysicalDeviceProperties properties;
    /** @brief Features of the physical device that an application can use to check if a feature is supported */
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceFeatures2 features2{};
#ifdef __APPLE__
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilitySubsetFeaturesKHR{};
#endif
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

    /** @brief queues */
    VkQueue graphicsQueue_{VK_NULL_HANDLE};
    VkQueue computeQueue_{VK_NULL_HANDLE};
    VkQueue transferQueue_{VK_NULL_HANDLE};
    VkQueue presentQueue_{VK_NULL_HANDLE};
    /** @brief Contains queue family indices */
    struct {
        uint32_t graphics{UINT32_MAX};
        uint32_t compute{UINT32_MAX};
        uint32_t transfer{UINT32_MAX};
        uint32_t present{UINT32_MAX};
    } queueFamilyIndices;

    VulkanDevice() = default;
    void AttachPhysicalDevice(VkPhysicalDevice physicalDevice, bool enableGetPhysicalDeviceProperty2Extension);
    void CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
                             VkSurfaceKHR surface,
                             std::vector<const char *> enabledExtensions);
    
    
    
    operator VkPhysicalDevice()
    {
        return physicalDevice;
    }

    operator VkDevice()
    {
        return logicalDevice;
    }

    uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
    bool ExtensionSupported(std::string extension);

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

} // namespace gdf
