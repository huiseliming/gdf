#pragma once
#include "VulkanApi.h"
#include <vector>

namespace gdf
{

struct Device {
    /** @brief Physical device representation */
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    /** @brief Logical device representation (application's view of the device) */
    VkDevice logicalDevice{VK_NULL_HANDLE};
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
    /** @brief Default command pool for the graphics queue family index */
    VkCommandPool commandPool = VK_NULL_HANDLE;
    /** @brief Set to true when the debug marker extension is detected */
    bool enableDebugMarkers = false;
    /** @brief Contains queue family indices */

    struct {
        uint32_t graphics{UINT32_MAX};
        uint32_t compute{UINT32_MAX};
        uint32_t transfer{UINT32_MAX};
        uint32_t present{UINT32_MAX};
    } queueFamilyIndices;

    operator VkDevice() const
    {
        return logicalDevice;
    };

    explicit Device(VkPhysicalDevice physicalDevice);
    ~Device();

    VkResult CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
                                 std::vector<const char *> enabledExtensions,
                                 void *pNextChain,
                                 VkSurfaceKHR surface = VK_NULL_HANDLE,
                                 VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
    bool ExtensionSupported(std::string extension);
    VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex,
                                    VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);





    //tool 
    VkShaderModule CreateShaderModule(const std::vector<char> &code);
    VkShaderModule CreateShaderModuleFromFile(std::string path);
};

} // namespace gdf
