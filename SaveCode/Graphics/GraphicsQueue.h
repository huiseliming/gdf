#pragma once
#include "VulkanApi.h"
#include <vector>
#include <map>
namespace gdf
{

class GDF_EXPORT GraphicsQueue 
{
public:
    GraphicsQueue(VkQueue queues_);
    GraphicsQueue(const GraphicsQueue &) = delete;
    GraphicsQueue &operator=(const GraphicsQueue &) = delete;
    GraphicsQueue( GraphicsQueue&& ) = delete;
    GraphicsQueue& operator=( GraphicsQueue &&) = delete;

    bool WaitFence(VkFence &fence, std::uint64_t timeout);

    void WaitIdle() ;

    operator VkQueue() const
    {
        return queue_;
    };


private:
    VkQueue queue_;
};

} // namespace gdf