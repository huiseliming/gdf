#pragma once
#include"Renderer/VulkanObject.h"
#include <vector>


namespace gdf
{

class CommandQueue;

enum class CommandBufferState {
    kRecording,
    kPending,
};

class CommandBuffer : public VulkanObject
{
public:
    CommandBuffer(VkDevice device, CommandQueue &commandQueue, VkCommandBuffer commandBuffer);
    CommandBuffer(CommandBuffer &&rhs) = delete;
    CommandBuffer &operator=(CommandBuffer &&rhs) = delete;
    ~CommandBuffer();

    void AddWaitSemaphore(VkSemaphore semaphore);
    void AddSignalSemaphore(VkSemaphore semaphore);
    void SetWaitDstStageMask(VkPipelineStageFlags pepilineStage);
    void SetFence(VkFence fence);

    void Submit();

private:
    CommandQueue &commandQueue_;
    VkCommandBuffer commandBuffer_;

    //submit data
    std::vector<VkSemaphore> waitSemaphores_;
    std::vector<VkSemaphore> signalSemaphores_;
    VkPipelineStageFlags waitDstStageMask_;
    VkFence fence_;
};

} // namespace gdf