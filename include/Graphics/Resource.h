#pragma once
#include "Graphics/VulkanApi.h"
#include <atomic>

struct Resource {
public:
    enum Type
    {
        kResourceTypeNone,
        kResourceTypeTexture,
        kResourceTypeMaterial,
        kResourceTypeMax,
    };
    Resource(VkDevice device) : device(device)
    {
        id = nextId++;
        resourceCount++;
    }
    virtual ~Resource()
    {
        resourceCount--;
    }

    virtual bool Create() = 0;
    virtual void Destroy() = 0;

private:
    VkDevice device{VK_NULL_HANDLE};

    uint32_t id;
    static std::atomic<uint32_t> nextId;
    static std::atomic<uint32_t> resourceCount;
};
