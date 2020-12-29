#pragma once 
#include "VulkanObject.h"
#include <vector>
#include "Renderer/RenderPass.h"

namespace gdf
{

class RenderPass : public VulkanObject
{
public:
    RenderPass(VkDevice device) : VulkanObject(device)
    {
    }

    RenderPass(RenderPass &&rhs) :VulkanObject(std::forward<RenderPass>(rhs))
    {
        flags = rhs.flags;
        rhs.flags = 0;
        attachments = std::move(rhs.attachments);
        subpasses = std::move(rhs.subpasses);
        dependencies = std::move(rhs.dependencies);
        renderPass_ = rhs.renderPass_; 
        rhs.renderPass_ = VK_NULL_HANDLE;

    }
    RenderPass& operator=(RenderPass &&rhs)
    {
        if (std::addressof(rhs) != this) 
            return *this;
        flags = rhs.flags;
        rhs.flags = 0;
        attachments = std::move(rhs.attachments);
        subpasses = std::move(rhs.subpasses);
        dependencies = std::move(rhs.dependencies);
        renderPass_ = rhs.renderPass_;
        rhs.renderPass_ = VK_NULL_HANDLE;
        VulkanObject::operator =(std::forward<RenderPass>(rhs));
    }
    ~RenderPass()
    {
        if (renderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_, renderPass_,nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
    }

    void SetRenderPassCreateFlags(VkRenderPassCreateFlags flags)
    {
        this->flags = flags;
    }
    void AddAttachmentDescription(VkAttachmentDescription attachmentDescription)
    {
        attachments.push_back(attachmentDescription);
    }
    void AddSubpassDescription(VkSubpassDescription subpassDescription)
    {
        subpasses.push_back(subpassDescription);
    }
    void AddSubpassDependency(VkSubpassDependency subpassDependency)
    {
        dependencies.push_back(subpassDependency);
    }

    void Reset()
    {
        if (renderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_, renderPass_, nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
    }

    VkRenderPass Get()
    {
        if (renderPass_ == VK_NULL_HANDLE) {
            VkRenderPassCreateInfo renderPassCI_{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = static_cast<uint32_t>(subpasses.size()),
                .pSubpasses = subpasses.data(),
                .dependencyCount =static_cast<uint32_t>(dependencies.size()),
                .pDependencies = dependencies.data(),
            };
            VK_ASSERT_SUCCESSED(vkCreateRenderPass(device_, &renderPassCI_, nullptr, &renderPass_));
        }
        return renderPass_;
    }

private:
    //AttachmentReference
    //std::vector<VkAttachmentReference> inputAttachments;
    //std::vector<VkAttachmentReference> colorAttachments;
    //VkAttachmentReference resolveAttachments;
    //VkAttachmentReference depthStencilAttachment;
    //
    VkRenderPassCreateFlags flags;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;
    VkRenderPass renderPass_{VK_NULL_HANDLE};
};

} // namespace gdf

//uint32_t inputAttachmentCount;
//const VkAttachmentReference *pInputAttachments;
//uint32_t colorAttachmentCount;
//const VkAttachmentReference *pColorAttachments;
//const VkAttachmentReference *pResolveAttachments;
//const VkAttachmentReference *pDepthStencilAttachment;
//uint32_t preserveAttachmentCount;
//const uint32_t *pPreserveAttachments;





