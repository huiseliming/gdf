#include "Renderer/CommandQueue.h"

namespace gdf
{

CommandQueue::CommandQueue(VkDevice device, uint32_t graphicsQueueFamilyIndex, uint32_t queueNum) : VulkanObject(device)
{
    // request queue
    queues_.resize(queueNum);
    for (uint32_t i = 0; i < queueNum; i++)
        vkGetDeviceQueue(device_, graphicsQueueFamilyIndex, i, &queues_[i]);

    // create command pool
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = graphicsQueueFamilyIndex,
    };
    VK_ASSERT_SUCCESSED(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_));
}

CommandQueue::~CommandQueue()
{
    // if (commandPool_!= VK_NULL_HANDLE) {
    vkDestroyCommandPool(device_, commandPool_, nullptr);
    //    commandPool_ = VK_NULL_HANDLE;
    //}
}

} // namespace gdf
