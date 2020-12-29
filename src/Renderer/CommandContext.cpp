#include "Renderer/CommandContext.h"
#include "Renderer/CommandQueue.h"
namespace gdf
{
CommandContext::CommandContext(VkDevice device,
                             CommandQueue *commandQueue, 
                             VkCommandBuffer commandBuffer)
    : VulkanObject(device), pCommandQueue_(commandQueue), commandBuffer_(commandBuffer)
{
}

CommandContext::CommandContext(CommandContext &&rhs) : VulkanObject(std::forward<CommandContext>(rhs))
{

}

CommandContext& CommandContext::operator = (CommandContext && rhs)
{
    return *this;
}

CommandContext::~CommandContext()
{
    vkFreeCommandBuffers(device_, pCommandQueue_->commandPool(),1,&commandBuffer_);
}

void CommandContext::AddWaitSemaphore(VkSemaphore semaphore)
{
    waitSemaphores_.push_back(semaphore);
}

void CommandContext::AddSignalSemaphore(VkSemaphore semaphore)
{
    signalSemaphores_.push_back(semaphore);
}

void CommandContext::SetWaitDstStageMask(VkPipelineStageFlags pepilineStage)
{
    waitDstStageMask_ = pepilineStage;
}

void CommandContext::SetFence(VkFence fence)
{
    fence_ = fence;
}

void CommandContext::Submit()
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
    vkQueueSubmit(pCommandQueue_->queue(), 1, &submitInfo, fence_);
}

} // namespace gdf
