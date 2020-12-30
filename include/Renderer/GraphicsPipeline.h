#pragma once
#include "Renderer/VulkanObject.h"
#include <vector>
#include "Base/File.h"
#include "Renderer/GraphicsTool.h"

namespace gdf
{

class GraphicsPipeline : public VulkanObject
{
public:
    GraphicsPipeline(VkDevice device) : VulkanObject(device)
    {
    }

    GraphicsPipeline(GraphicsPipeline &&rhs) : VulkanObject(std::forward<GraphicsPipeline>(rhs))
    {
    }
    GraphicsPipeline &operator=(GraphicsPipeline &&rhs)
    {
        if (std::addressof(rhs) != this)
            return *this;
    }
    ~GraphicsPipeline()
    {
        Desstroy();
    }

    void Reset()
    {
        Desstroy();
    }

    void Create()
    {
        VkPipelineViewportStateCreateInfo viewportStateCI
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = static_cast<uint32_t>(viewports.size()),
            .pViewports = viewports.data(),
            .scissorCount = static_cast<uint32_t>(scissors.size()),
            .pScissors = scissors.data(),
        };
        VkPipelineLayoutCreateInfo graphicsPipelineLayoutCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };
        VK_ASSERT_SUCCESSED(vkCreatePipelineLayout(device_, &graphicsPipelineLayoutCI, nullptr, &pipelineLayout_));
        VkGraphicsPipelineCreateInfo GraphicsPipelineCI{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shaderStageCI.size()),
            .pStages = shaderStageCI.data(),
            .pVertexInputState = &vertexInputStateCI,
            .pInputAssemblyState = &inputAssemblyStateCI,
            //.pTessellationState,
            .pViewportState = &viewportStateCI,
            .pRasterizationState = &rasterizationStateCI,
            .pMultisampleState = &multisampleStateCI,
            //.pDepthStencilState ,
            .pColorBlendState = &colorBlendStateCI,
            //.pDynamicState,
            .layout = pipelineLayout_,
            .renderPass = renderPass_,
            //.subpass,
            .basePipelineHandle = VK_NULL_HANDLE,
            //.basePipelineIndex,
        };
        VK_ASSERT_SUCCESSED(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &GraphicsPipelineCI, nullptr, &graphicsPipeline_));
    }

    void Desstroy()
    {
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
        if (graphicsPipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
            graphicsPipeline_ = VK_NULL_HANDLE;
        }
    }

    VkPipeline Get()
    {
        if (graphicsPipeline_ == VK_NULL_HANDLE) {
            Create();
        }
        return graphicsPipeline_;
    }


    void AddShaderStage(std::string spvShaderPath, VkShaderStageFlagBits shaderStageFlag, std::string_view mainName = "main")
    {
        auto vertShaderCode = File::ReadBytes(spvShaderPath);
        VkShaderModule vertShaderModule = GraphicsTool::CreateShaderModule(device_,vertShaderCode);
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = shaderStageFlag;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        ShaderModules.push_back(vertShaderModule);
        shaderStageCI.push_back(vertShaderStageInfo);
    }

    void SetInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI)
    {
        this->inputAssemblyStateCI = inputAssemblyStateCI;
    }
    void SetRasterizationState(VkPipelineRasterizationStateCreateInfo rasterizationStateCI)
    {
        this->rasterizationStateCI = rasterizationStateCI;
    }

    void AddVertexInputBindingDescription(VkVertexInputBindingDescription vertexInputBindingDescription)
    {
        vertexInputBindingDescriptions.push_back(vertexInputBindingDescription);
    }

    void AddVertexInputAttributeDescription(VkVertexInputAttributeDescription vertexInputAttributeDescription)
    {
        vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
    }

private:
    //
    std::vector<VkShaderModule> ShaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCI;

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    //VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
    VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};

    VkRenderPass renderPass_;

    VkPipelineLayout pipelineLayout_; 
    VkPipeline graphicsPipeline_;
};

}







































