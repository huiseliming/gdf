#include "Graphics/VulkanTools.h"

namespace gdf
{
namespace GraphicsTools
{
// helpful function
std::string_view PhysicalDeviceTypeString(VkPhysicalDeviceType type)
{
    switch (type) {
#define STR(r)                                                                                                                 \
    case VK_PHYSICAL_DEVICE_TYPE_##r:                                                                                          \
        return #r
        STR(OTHER);
        STR(INTEGRATED_GPU);
        STR(DISCRETE_GPU);
        STR(VIRTUAL_GPU);
#undef STR
    default:
        return "UNKNOWN_DEVICE_TYPE";
    }
}


std::string_view VkResultString(VkResult errorCode)
{
    switch (errorCode) {
#define STR(r)                                                                                                                 \
    case VK_##r:                                                                                                               \
        return #r
        STR(SUCCESS);
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_FRAGMENTED_POOL);
        STR(ERROR_UNKNOWN);
        STR(ERROR_OUT_OF_POOL_MEMORY);
        STR(ERROR_INVALID_EXTERNAL_HANDLE);
        STR(ERROR_FRAGMENTATION);
        STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
        STR(ERROR_INCOMPATIBLE_VERSION_KHR);
        STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
        STR(ERROR_NOT_PERMITTED_EXT);
        STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
        STR(THREAD_IDLE_KHR);
        STR(THREAD_DONE_KHR);
        STR(OPERATION_DEFERRED_KHR);
        STR(OPERATION_NOT_DEFERRED_KHR);
        STR(PIPELINE_COMPILE_REQUIRED_EXT);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
{
    std::vector<VkFormat> depthFormats = {VK_FORMAT_D16_UNORM,
                                          VK_FORMAT_D32_SFLOAT,
                                          VK_FORMAT_D16_UNORM_S8_UINT,
                                          VK_FORMAT_D24_UNORM_S8_UINT,
                                          VK_FORMAT_D32_SFLOAT_S8_UINT};
    for (auto &format : depthFormats) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            *depthFormat = format;
            return VK_TRUE;
        }
    }
    return VK_FALSE;
}

uint32_t GetQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                             const VkQueueFlagBits queueFlags)
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

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if (queueFamilyProperties[i].queueFlags & queueFlags) {
            return i;
        }
    }
    return UINT32_MAX;
}




} // namespace VulkanTools
} // namespace gdf
