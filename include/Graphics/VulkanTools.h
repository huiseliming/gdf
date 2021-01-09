#pragma once
#include "VulkanApi.h"
#include <vector>
#include <string>
namespace gdf
{
namespace GraphicsTools
{
std::string_view PhysicalDeviceTypeString(VkPhysicalDeviceType type);
std::string_view VkResultString(VkResult errorCode);
uint32_t GetQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                             const VkQueueFlagBits queueFlags);
}; // namespace VulkanTools
} // namespace gdf
