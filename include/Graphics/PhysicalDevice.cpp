#include "PhysicalDevice.h"
namespace gdf
{
PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice) : physicalDevice(physicalDevice)
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
}
VkResult PhysicalDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
											std::vector<const char *> enabledExtensions,
											void *pNextChain,
											bool useSwapChain,
											VkQueueFlags requestedQueueTypes)
{
	// Desired queues need to be requested upon logical device creation
	// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
	// requests different queue types

	std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs;

	// Get queue family indices for the requested queue family types
	// Note that the indices may overlap depending on the implementation

	const float defaultQueuePriority(0.0f);
	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        if (queueFamilyIndices.graphics == UINT32_MAX) 
            THROW_EXCEPT("No found graphics queue family indices!");
        deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilyIndices.graphics,
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority,
        });
	}
	else
	{
		queueFamilyIndices.graphics = UINT32_MAX;
	}

	// Dedicated compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
	{
		queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (queueFamilyIndices.graphics == UINT32_MAX)
            THROW_EXCEPT("No found compute queue family indices!");
		if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
		{
            deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamilyIndices.compute,
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority,
            });
		}
	}
	else
	{
		// Else we use the same queue
		queueFamilyIndices.compute = queueFamilyIndices.graphics;
	}

	// Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
	{
		queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if (queueFamilyIndices.transfer == UINT32_MAX)
            THROW_EXCEPT("No found transfer queue family indices!");
		if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
		{
            deviceQueueCIs.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamilyIndices.transfer,
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority,
            });
		}
	}
	else
	{
		// Else we use the same queue
		queueFamilyIndices.transfer = queueFamilyIndices.graphics;
	}

	// Create the logical device representation
	std::vector<const char*> deviceExtensions(enabledExtensions);
	if (useSwapChain)
	{
		// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (ExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        enableDebugMarkers = true;
    }

    if (deviceExtensions.size() > 0) {
        for (const char *enabledExtension : deviceExtensions) {
            if (!ExtensionSupported(enabledExtension)) {
                THROW_EXCEPT(std::string("Enabled device extension \"") + enabledExtension + "\" is not present at device level");
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

	// If a pNext(Chain) has been passed, we need to add it to the device creation info
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
	if (pNextChain) {
		physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		physicalDeviceFeatures2.features = enabledFeatures;
		physicalDeviceFeatures2.pNext = pNextChain;
		deviceCI.pEnabledFeatures = nullptr;
		deviceCI.pNext = &physicalDeviceFeatures2;
	}

	VkResult result = vkCreateDevice(physicalDevice, &deviceCI, nullptr, &logicalDevice);
	if (result != VK_SUCCESS) 
		return result;

	// Create a default command pool for graphics command buffers
    commandPool = CreateCommandPool(queueFamilyIndices.graphics);

	return result;
}

uint32_t PhysicalDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const
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

bool PhysicalDevice::ExtensionSupported(std::string extension)
{
    return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkCommandPool PhysicalDevice::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
    VkCommandPoolCreateInfo commandPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = createFlags,
        .queueFamilyIndex = queueFamilyIndex,
    };
    VkCommandPool commandPool;
    VK_ASSERT_SUCCESSED(vkCreateCommandPool(logicalDevice, &commandPoolCI, nullptr, &commandPool));
    return commandPool;
}

} // namespace gdf
