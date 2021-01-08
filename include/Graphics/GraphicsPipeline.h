#pragma once
#include "VulkanApi.h"
#include <vector>
#include <glm/glm.hpp>

namespace gdf
{
class Device;


struct Vertex {
    enum class Component 
    { 
        Position,
        Normal, 
        UV, 
        Color, 
        Tangent,
        Joint0, 
        Weight0
    };

    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 color;
    glm::vec4 tangent;
    glm::vec4 joint0;
    glm::vec4 weight0;
    
    std::vector<VkVertexInputAttributeDescription> InputAttributeDescriptions(uint32_t binding, const std::vector<Component> components)
    {
        std::vector<VkVertexInputAttributeDescription> result;
        uint32_t location = 0;
        for (Component component : components) {
            result.push_back(Vertex::InputAttributeDescription(binding, location, component));
            location++;
        }
        return result;
    }

    VkVertexInputAttributeDescription InputAttributeDescription(uint32_t location, uint32_t binding, Component component)
    {
        switch (component) {
        case Component::Position:
            return VkVertexInputAttributeDescription({location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)});
        case Component::Normal:
            return VkVertexInputAttributeDescription({location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        case Component::UV:
            return VkVertexInputAttributeDescription({location, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
        case Component::Color:
            return VkVertexInputAttributeDescription(
                {location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)});
        case Component::Tangent:
            return VkVertexInputAttributeDescription(
                {location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)});
        case Component::Joint0:
            return VkVertexInputAttributeDescription(
                {location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, joint0)});
        case Component::Weight0:
            return VkVertexInputAttributeDescription(
                {location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight0)});
        default:
            return VkVertexInputAttributeDescription({});
        }
    }

    VkVertexInputBindingDescription InputBindingDescription(uint32_t binding)
    {
        return VkVertexInputBindingDescription({binding, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX});
    }
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

        rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        //rasterizationStateCI.pNext;
        //rasterizationStateCI.flags;
        rasterizationStateCI.depthClampEnable = VK_FALSE;
        rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCI.depthBiasEnable = VK_FALSE;
        rasterizationStateCI.depthBiasConstantFactor = 0.0f;
        rasterizationStateCI.depthBiasClamp = 0.0f;
        rasterizationStateCI.depthBiasSlopeFactor = 0.0f;
        rasterizationStateCI.lineWidth = 1.0f;

        multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        //multisampleStateCI.pNext;
        //multisampleStateCI.flags;
        multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCI.sampleShadingEnable = VK_FALSE;
        multisampleStateCI.minSampleShading = 1.0f;
        multisampleStateCI.pSampleMask = nullptr;
        multisampleStateCI.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCI.alphaToOneEnable = VK_FALSE;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {
            VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor =VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            }
        };
        VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
        colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;;
        colorBlendStateCI.pNext;
        colorBlendStateCI.flags;
        colorBlendStateCI.logicOpEnable = VK_FALSE;
        colorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCI.attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates.size());
        colorBlendStateCI.pAttachments = colorBlendAttachmentStates.data();
        colorBlendStateCI.blendConstants[0] = 0.f;
        colorBlendStateCI.blendConstants[1] = 0.f;
        colorBlendStateCI.blendConstants[2] = 0.f;
        colorBlendStateCI.blendConstants[3] = 0.f;

    }

    // ShaderStage
    void AddShaderStage(std::string spvShaderPath, VkShaderStageFlagBits shaderStageFlag, std::string_view mainName = "main");

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