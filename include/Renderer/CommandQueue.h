#pragma once
#include <vector>
#include "Renderer/VulkanObject.h"
#include <map>
#include "Renderer/CommandContext.h"

namespace gdf
{

class CommandContext;

class GDF_EXPORT CommandQueue : public VulkanObject
{
public:
    CommandQueue(VkDevice device, uint32_t graphicsQueueFamilyIndex, uint32_t queueNum);
    CommandQueue( CommandQueue&& ) = delete;
    CommandQueue& operator=( CommandQueue &&) = delete;
    ~CommandQueue();


    std::unique_ptr<CommandContext> CommandBegin(uint32_t commandBufferCount, VkCommandBufferLevel commandBufferLevel)
    {
        VkCommandBufferAllocateInfo commandBufferAI{};
        commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAI.commandPool = commandPool_;
        commandBufferAI.level = commandBufferLevel;
        commandBufferAI.commandBufferCount = commandBufferCount;
        VkCommandBuffer commandbuffer;
        VK_ASSERT_SUCCESSED(vkAllocateCommandBuffers(device_, &commandBufferAI, &commandbuffer));
        return std::make_unique<CommandContext>(device_, this, commandbuffer);
    }



    bool WaitFence(VkFence &fence, std::uint64_t timeout);
    void WaitIdle() ;

    VkQueue queue()
    {
        return queues_[0];
    }

    VkCommandPool commandPool()
    {
        return commandPool_;
    }

private:
    std::vector<VkQueue> queues_;
    VkCommandPool commandPool_;
};

} // namespace gdf