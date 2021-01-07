#pragma once
#include "VulkanApi.h"
#include <vector>

namespace gdf
{
class Device;

struct Vertex {

};


struct GraphicsPipeline
{
    GraphicsPipeline(Device &device) : device_(device)
    {
        inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

        multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCI.sampleShadingEnable = VK_FALSE;
        multisampleStateCI.minSampleShading = 1.0f;          // Optional
        multisampleStateCI.pSampleMask = nullptr;            // Optional
        multisampleStateCI.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampleStateCI.alphaToOneEnable = VK_FALSE;      // Optional
    }

    // VertexInputState
    void AddVertexInputBindingDescription(VkVertexInputBindingDescription vertexInputBindingDescription)
    {
        vertexInputBindingDescriptions.push_back(vertexInputBindingDescription);
    }
    void AddVertexInputAttributeDescription(VkVertexInputAttributeDescription vertexInputAttributeDescription)
    {
        vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
    }

    // InputAssemblyState
    void SetInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI)
    {
        this->inputAssemblyStateCI = inputAssemblyStateCI;
    }

    // TessellationState
    void SetTessellationState(VkPipelineTessellationStateCreateInfo tessellationStateCI)
    {
        this->tessellationStateCI = tessellationStateCI;
    }

    // ViewportState
    void AddViewport(VkViewport viewport)
    {
        viewports.push_back(viewport);
    }

    void AddScissor(VkRect2D scissor)
    {
        scissors.push_back(scissor);
    }

    // RasterizationState
    void SetRasterizationState(VkPipelineRasterizationStateCreateInfo rasterizationStateCI)
    {
        this->rasterizationStateCI = rasterizationStateCI;
    }

    // MultisampleState
    void SetSampleMask(uint32_t sampleMask)
    {
        this->sampleMask = sampleMask;
        multisampleStateCI.pSampleMask = &this->sampleMask;
    }
    void SetMultisampleState(VkPipelineMultisampleStateCreateInfo multisampleStateCI)
    {
        this->multisampleStateCI = multisampleStateCI;
    }

    // DepthStencilState
    void SetDepthStencilState(VkPipelineDepthStencilStateCreateInfo depthStencilStateCI)
    {
        this->depthStencilStateCI = depthStencilStateCI;
    }

    // ColorBlendState
    void AddColorBlendAttachmentState(VkPipelineColorBlendAttachmentState colorBlendAttachmentState)
    {
        colorBlendAttachmentStates.push_back(colorBlendAttachmentState);
    }

    void SetColorBlendState(VkPipelineColorBlendStateCreateInfo colorBlendStateCI)
    {
        this->colorBlendStateCI = colorBlendStateCI;
    }

    // DynamicState
    void AddDynamicState(VkDynamicState dynamicState)
    {
        dynamicStates.push_back(dynamicState);
    }

    // PipelineLayout
    void AddDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
    {
        descriptorSetLayouts.push_back(descriptorSetLayout);
    }
    void AddPushConstantRange(VkPushConstantRange pushConstantRange)
    {
        pushConstantRanges.push_back(pushConstantRange);
    }

    // RenderPass
    void SetRenderPass(VkRenderPass renderPass)
    {
        this->renderPass_ = renderPass;
    }

    // Subpass
    void SetSubpass(uint32_t subpass)
    {
        this->subpass = subpass;
    }

    // BasePipelineHandle
    void SetBasePipelineHandle(VkPipeline pipeline)
    {
        basePipelineHandle = pipeline;
    }

    // BasePipelineIndex
    void SetBasePipelineIndex(int32_t index)
    {
        basePipelineIndex = index;
    }

    void Create();
    void Destroy();

    operator VkPipeline();


        // ShaderStage
    std::vector<VkShaderModule> ShaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCIs;
    // VertexInputState
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    // InputAssemblyState
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
    // TessellationState
    VkPipelineTessellationStateCreateInfo tessellationStateCI{};
    // ViewportState
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    // RasterizationState
    VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
    // MultisampleState
    uint32_t sampleMask{};
    VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
    // DepthStencilState
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
    // ColorBlendState
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
    // DynamicState
    std::vector<VkDynamicState> dynamicStates;
    // PipelineLayout
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkPushConstantRange> pushConstantRanges;
    // RenderPass
    VkRenderPass renderPass_;
    // Subpass
    uint32_t subpass{};
    // BasePipelineHandle
    VkPipeline basePipelineHandle{VK_NULL_HANDLE};
    // BasePipelineIndex
    int32_t basePipelineIndex{};
    // VKObject
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline_{VK_NULL_HANDLE};


    bool isModify;
    Device &device_;
};

}