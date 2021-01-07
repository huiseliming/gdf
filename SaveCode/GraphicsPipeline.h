#pragma once
#include "Base/File.h"
#include "VulkanObject.h"
#include "VulkanTools.h"
#include <vector>

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
        ShaderModules = std::move(rhs.ShaderModules);
        shaderStageCI = std::move(rhs.shaderStageCI);
        vertexInputBindingDescriptions = std::move(rhs.vertexInputBindingDescriptions);
        vertexInputAttributeDescriptions = std::move(rhs.vertexInputAttributeDescriptions);
        inputAssemblyStateCI = rhs.inputAssemblyStateCI;
        rhs.inputAssemblyStateCI = {};
        tessellationStateCI = rhs.tessellationStateCI;
        rhs.tessellationStateCI = {};
        viewports = std::move(rhs.viewports);
        scissors = std::move(rhs.scissors);
        rasterizationStateCI = rhs.rasterizationStateCI;
        rhs.rasterizationStateCI = {};
        sampleMask = rhs.sampleMask;
        rhs.sampleMask = {};
        multisampleStateCI = rhs.multisampleStateCI;
        rhs.multisampleStateCI = {};
        if (multisampleStateCI.pSampleMask != nullptr)
            multisampleStateCI.pSampleMask = &sampleMask;
        depthStencilStateCI = rhs.depthStencilStateCI;
        rhs.depthStencilStateCI = {};
        colorBlendAttachmentStates = std::move(rhs.colorBlendAttachmentStates);
        colorBlendStateCI = rhs.colorBlendStateCI;
        rhs.colorBlendStateCI = {};
        dynamicStates = std::move(rhs.dynamicStates);
        descriptorSetLayouts = std::move(rhs.descriptorSetLayouts);
        pushConstantRanges = std::move(rhs.pushConstantRanges);
        renderPass_ = rhs.renderPass_;
        rhs.renderPass_ = VK_NULL_HANDLE;
        subpass = rhs.subpass;
        rhs.subpass = {};
        basePipelineHandle = rhs.basePipelineHandle;
        rhs.basePipelineHandle = VK_NULL_HANDLE;
        basePipelineIndex = rhs.basePipelineIndex;
        rhs.basePipelineIndex = {};
        pipelineLayout_ = rhs.pipelineLayout_;
        rhs.pipelineLayout_ = VK_NULL_HANDLE;
        graphicsPipeline_ = rhs.graphicsPipeline_;
        rhs.graphicsPipeline_ = VK_NULL_HANDLE;
    }
    GraphicsPipeline &operator=(GraphicsPipeline &&rhs)
    {
        if (std::addressof(rhs) != this)
            return *this;
        ShaderModules = std::move(rhs.ShaderModules);
        shaderStageCI = std::move(rhs.shaderStageCI);
        vertexInputBindingDescriptions = std::move(rhs.vertexInputBindingDescriptions);
        vertexInputAttributeDescriptions = std::move(rhs.vertexInputAttributeDescriptions);
        inputAssemblyStateCI = rhs.inputAssemblyStateCI;
        rhs.inputAssemblyStateCI = {};
        tessellationStateCI = rhs.tessellationStateCI;
        rhs.tessellationStateCI = {};
        viewports = std::move(rhs.viewports);
        scissors = std::move(rhs.scissors);
        rasterizationStateCI = rhs.rasterizationStateCI;
        rhs.rasterizationStateCI = {};
        sampleMask = rhs.sampleMask;
        rhs.sampleMask = {};
        multisampleStateCI = rhs.multisampleStateCI;
        rhs.multisampleStateCI = {};
        if (multisampleStateCI.pSampleMask != nullptr)
            multisampleStateCI.pSampleMask = &sampleMask;
        depthStencilStateCI = rhs.depthStencilStateCI;
        rhs.depthStencilStateCI = {};
        colorBlendAttachmentStates = std::move(rhs.colorBlendAttachmentStates);
        colorBlendStateCI = rhs.colorBlendStateCI;
        rhs.colorBlendStateCI = {};
        dynamicStates = std::move(rhs.dynamicStates);
        descriptorSetLayouts = std::move(rhs.descriptorSetLayouts);
        pushConstantRanges = std::move(rhs.pushConstantRanges);
        renderPass_ = rhs.renderPass_;
        rhs.renderPass_ = VK_NULL_HANDLE;
        subpass = rhs.subpass;
        rhs.subpass = {};
        basePipelineHandle = rhs.basePipelineHandle;
        rhs.basePipelineHandle = VK_NULL_HANDLE;
        basePipelineIndex = rhs.basePipelineIndex;
        rhs.basePipelineIndex = {};
        pipelineLayout_ = rhs.pipelineLayout_;
        rhs.pipelineLayout_ = VK_NULL_HANDLE;
        graphicsPipeline_ = rhs.graphicsPipeline_;
        rhs.graphicsPipeline_ = VK_NULL_HANDLE;
        return *this;
    }
    ~GraphicsPipeline()
    {
        Desstroy();
    }

    void Reset()
    {
        for (auto ShaderModule : ShaderModules)
            vkDestroyShaderModule(device_, ShaderModule, nullptr);
        ShaderModules.clear();
        shaderStageCI = {};
        vertexInputBindingDescriptions.clear();
        vertexInputAttributeDescriptions.clear();
        inputAssemblyStateCI = {};
        tessellationStateCI = {};
        viewports.clear();
        scissors.clear();
        rasterizationStateCI = {};
        sampleMask = {};
        multisampleStateCI = {};
        depthStencilStateCI = {};
        colorBlendAttachmentStates.clear();
        colorBlendStateCI = {};
        dynamicStates.clear();
        descriptorSetLayouts.clear();
        pushConstantRanges.clear();
        renderPass_ = VK_NULL_HANDLE;
        subpass = {};
        basePipelineHandle = VK_NULL_HANDLE;
        basePipelineIndex = {};
        Desstroy();
    }

    void Create();

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

    // ShaderStage
    void AddShaderStage(std::string spvShaderPath, VkShaderStageFlagBits shaderStageFlag, std::string_view mainName = "main")
    {
        auto vertShaderCode = File::ReadBytes(spvShaderPath);
        VkShaderModule vertShaderModule = device_.CreateShaderModule(vertShaderCode);
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = shaderStageFlag;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        ShaderModules.push_back(vertShaderModule);
        shaderStageCI.push_back(vertShaderStageInfo);
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

private:
    // ShaderStage
    std::vector<VkShaderModule> ShaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCI;
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
};

} // namespace gdf
