#include "Renderer/CommandContext.h"
#include "Renderer/CommandQueue.h"
namespace gdf
{
CommandBuffer::CommandBuffer(VkDevice device,
                             CommandQueue &commandQueue, 
                             VkCommandBuffer commandBuffer)
    : VulkanObject(device), commandQueue_(commandQueue), commandBuffer_(commandBuffer)
{
}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(device_, commandQueue_.commandPool(),1,&commandBuffer_);
}

void CommandBuffer::AddWaitSemaphore(VkSemaphore semaphore)
{
    waitSemaphores_.push_back(semaphore);
}

void CommandBuffer::AddSignalSemaphore(VkSemaphore semaphore)
{
    signalSemaphores_.push_back(semaphore);
}

void CommandBuffer::SetWaitDstStageMask(VkPipelineStageFlags pepilineStage)
{
    waitDstStageMask_ = pepilineStage;
}

void CommandBuffer::SetFence(VkFence fence)
{
    fence_ = fence;
}

void CommandBuffer::Submit()
{
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores_.size()),
        .pWaitSemaphores = waitSemaphores_.data(),
        .pWaitDstStageMask = &waitDstStageMask_,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer_,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores_.size()),
        .pSignalSemaphores = signalSemaphores_.data(),
    };
    vkQueueSubmit(commandQueue_.queue(), 1, &submitInfo, fence_);
}

} // namespace gdf
