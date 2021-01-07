#pragma once
#include "Graphics/VulkanApi.h"
#include "Base/Common.h"
#include <cstddef>
#include <map>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>

namespace gdf
{

class Device;

class ShaderPool
{
    friend class Shader;

public:
    ShaderPool(Device &device) : device_(device)
    {
    }
    ~ShaderPool()
    {
        std::mutex sync_;
        shaderPool_.clear();
    }
    Shader *Find(std::string_view name);

    Shader *Create(std::string_view name);

private:
    void Load(Shader *pShader);

    bool Unload(Shader *pShader);
    Device &device_;
    std::mutex sync_;
    std::map<std::string_view, std::unique_ptr<Shader>> shaderPool_;
};

struct Shader
{
private:
    friend class ShaderPool;
    enum LoadState {
        Unloaded,
        Loading,
        Loaded,
    };

public:
    std::atomic<LoadState> loadState_;
    std::atomic_size_t refCount_;
    std::atomic_size_t useCount_;

    std::string name_;
    ShaderPool *shaderPool_;

    VkShaderModule shaderModule_;
    VkDescriptorSetLayout descriptorSetLayout_;
    VkPushConstantRange pushConstantRange_;

    void incrementUseCount();

    void decrementUseCount();
};

} // namespace gdf