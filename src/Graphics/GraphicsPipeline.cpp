#include "Graphics/GraphicsPipeline.h"
#include "Graphics/Device.h"
#include "Base/File.h"

namespace gdf{

GraphicsPipeline::operator VkPipeline()
{
    if (!isModify) {
        Destroy();
        Create();
    } else {
        if (graphicsPipeline_ == VK_NULL_HANDLE) {
            Create();
        }
    }
    return graphicsPipeline_;
}

void GraphicsPipeline::AddShaderStage(std::string spvShaderPath,
                                                  VkShaderStageFlagBits shaderStageFlag,
                                                  std::string_view mainName)
{
    VkShaderModule vertShaderModule = device_.CreateShaderModule(File::ReadBytes(spvShaderPath));
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = shaderStageFlag;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    ShaderModules.push_back(vertShaderModule);
    shaderStageCIs.push_back(vertShaderStageInfo);
}

void GraphicsPipeline::Create()
{
    VkPipelineVertexInputStateCreateInfo vertexInputStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
        .pVertexBindingDescriptions = vertexInputBindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data(),
    };
    VkPipelineViewportStateCreateInfo viewportStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = static_cast<uint32_t>(viewports.size()),
        .pViewports = viewports.data(),
        .scissorCount = static_cast<uint32_t>(scissors.size()),
        .pScissors = scissors.data(),
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,            // Optional
        .pSetLayouts = nullptr,         // Optional
        .pushConstantRangeCount = 0,    // Optional
        .pPushConstantRanges = nullptr, // Optional

    };
    VkPipelineDynamicStateCreateInfo dynamicStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };
    vkCreatePipelineLayout(device_, &pipelineLayoutCI, nullptr, &pipelineLayout_);
    VkGraphicsPipelineCreateInfo GraphicsPipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStageCIs.size()),
        .pStages = shaderStageCIs.data(),
        .pVertexInputState = &vertexInputStateCI,
        .pInputAssemblyState = &inputAssemblyStateCI,
        //.pTessellationState = tessellationStateCI.sType ? &tessellationStateCI : nullptr,
        .pViewportState = &viewportStateCI,
        .pRasterizationState = &rasterizationStateCI,
        .pMultisampleState = &multisampleStateCI,
        //.pDepthStencilState = depthStencilStateCI.sType ? &depthStencilStateCI : nullptr,
        .pColorBlendState = &colorBlendStateCI,
        .pDynamicState = &dynamicStateCI,
        .layout = pipelineLayout_,
        .renderPass = renderPass_,
        .subpass = subpass,
        .basePipelineHandle = basePipelineHandle,
        .basePipelineIndex = basePipelineIndex,
    };
}

void GraphicsPipeline::Destroy()
{
    for (auto ShaderModules : ShaderModules) {
        if (ShaderModules != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, ShaderModules, nullptr);
        }
    }
    ShaderModules.clear();
    if (pipelineLayout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
        pipelineLayout_ = VK_NULL_HANDLE;
    }
    if (graphicsPipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
        graphicsPipeline_ = VK_NULL_HANDLE;
    }

}

}