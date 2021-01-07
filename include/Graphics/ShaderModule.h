#pragma once
#include "Base/Common.h"
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

class ShaderModule
{
public:
    operator VkShaderModule()
    {
        return shaderModule_;
    }

private:
    uint64_t hash_;
    VkShaderModule shaderModule_;

public:
    VkShaderModule RequiredShaderModule()
    {
    }

private:
    static void CreateShaderModule()
    {
    }

    static void FindShaderModule()
    {
    }

    static std::unordered_map<uint64_t, ShaderModule> shaderModulePool_;
};