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

class CommandContext : public VulkanObject
{
public:
    CommandContext(VkDevice device, CommandQueue *commandQueue, VkCommandBuffer commandBuffer);
    CommandContext(CommandContext &&rhs);
    CommandContext &operator=(CommandContext &&rhs);
    ~CommandContext();




    void AddWaitSemaphore(VkSemaphore semaphore);
    void AddSignalSemaphore(VkSemaphore semaphore);
    void SetWaitDstStageMask(VkPipelineStageFlags pepilineStage);
    void SetFence(VkFence fence);
    void Submit();

private:
    CommandQueue *pCommandQueue_;
    VkCommandBuffer commandBuffer_;

    //submit data
    std::vector<VkSemaphore> waitSemaphores_;
    std::vector<VkSemaphore> signalSemaphores_;
    VkPipelineStageFlags waitDstStageMask_;
    VkFence fence_;
};

} // namespace gdf