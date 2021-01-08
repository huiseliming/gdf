#include "Graphics/Shader.h"
#include "Graphics/Device.h"
#include <filesystem>
namespace gdf
{

Shader *ShaderPool::Find(std::string_view name)
{
    std::scoped_lock<std::mutex> lock(sync_);
    auto it = shaderPool_.find(name);
    if (it != shaderPool_.end()) {
        return it->second.get();
    }
    return nullptr;
}

Shader *ShaderPool::Create(std::string_view name)
{
    auto pShader = Find(name);
    if (pShader != nullptr)
        return pShader;
    pShader = new Shader;
    shaderPool_.insert(std::make_pair(name, std::unique_ptr<Shader>(pShader)));
    return pShader;
}

void ShaderPool::Load(Shader *pShader)
{
    auto expected = Shader::LoadState::Unloaded;
    if (!pShader->loadState_.compare_exchange_strong(expected, Shader::LoadState::Loading)) {
        // wait other thead to load
    }

    assert(pShader->shaderModule_ == VK_NULL_HANDLE);
    pShader->shaderModule_ = device_.CreateShaderModuleFromFile(pShader->name_);
}

bool ShaderPool::Unload(Shader *pShader)
{
    auto useCount = pShader->useCount_.load();
    if (useCount > 0) {

    }
    auto expected = Shader::LoadState::Loaded;
    if (!pShader->loadState_.compare_exchange_strong(expected, Shader::LoadState::Unloaded)) {
        if (expected != Shader::LoadState::Unloaded) {
            return false;
        }
    }
    if (pShader->shaderModule_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device_, pShader->shaderModule_, nullptr);
        pShader->shaderModule_ = VK_NULL_HANDLE;
    }
}

void Shader::incrementUseCount()
{
    useCount_++;
    if (loadState_ == Unloaded) {
        shaderPool_->Load(this);
    }
}

void Shader::decrementUseCount()
{
    useCount_--;
}

} // namespace gdf