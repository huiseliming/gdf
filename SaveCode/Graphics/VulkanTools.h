#pragma once
#include "VulkanApi.h"
#include <vector>
#include <string>
namespace gdf
{
namespace VulkanTools
{
std::string_view PhysicalDeviceTypeString(VkPhysicalDeviceType type);
std::string_view VkResultString(VkResult errorCode);
}; // namespace VulkanTools
} // namespace gdf
