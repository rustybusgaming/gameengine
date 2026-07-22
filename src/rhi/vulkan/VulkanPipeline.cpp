// Vulkan Pipeline and Render Pass Implementation
#ifdef NEXUS_VULKAN_ENABLED

#include "RHI/Vulkan/VulkanDevice.h"
#include "RHI/Vulkan/VulkanResource.h"
#include "Logger.h"

namespace Nexus {
namespace RHI {

static VkPrimitiveTopology ToVulkanTopology(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

static VkBlendFactor ToVulkanBlendFactor(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::InvSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::InvSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::InvDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::InvDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        default: return VK_BLEND_FACTOR_ONE;
    }
}

static VkBlendOp ToVulkanBlendOp(BlendOp op) {
    switch (op) {
        case BlendOp::Add: return VK_BLEND_OP_ADD;
        case BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
        case BlendOp::RevSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::Min: return VK_BLEND_OP_MIN;
        case BlendOp::Max: return VK_BLEND_OP_MAX;
        default: return VK_BLEND_OP_ADD;
    }
}

static VkCompareOp ToVulkanCompareOp(CompareFunc func) {
    switch (func) {
        case CompareFunc::Never: return VK_COMPARE_OP_NEVER;
        case CompareFunc::Less: return VK_COMPARE_OP_LESS;
        case CompareFunc::Equal: return VK_COMPARE_OP_EQUAL;
        case CompareFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareFunc::Greater: return VK_COMPARE_OP_GREATER;
        case CompareFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case CompareFunc::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareFunc::Always: return VK_COMPARE_OP_ALWAYS;
        default: return VK_COMPARE_OP_LESS;
    }
}

static VkCullModeFlags ToVulkanCullMode(CullMode mode) {
    switch (mode) {
        case CullMode::None: return VK_CULL_MODE_NONE;
        case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
        default: return VK_CULL_MODE_BACK_BIT;
    }
}

static VkPolygonMode ToVulkanPolygonMode(FillMode mode) {
    switch (mode) {
        case FillMode::Solid: return VK_POLYGON_MODE_FILL;
        case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
        default: return VK_POLYGON_MODE_FILL;
    }
}

RHIPipelinePtr VulkanDevice::CreateGraphicsPipeline(
    const std::vector<RHIShader*>& shaders,
    const BlendStateDesc& blendState,
    const DepthStencilStateDesc& depthStencilState,
    const RasterizerStateDesc& rasterizerState,
    PrimitiveTopology topology) {

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    for (auto* shader : shaders) {
        VulkanShader* vkShader = static_cast<VulkanShader*>(shader);

        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.module = vkShader->GetVkModule();
        stageInfo.pName = "main";

        if (vkShader->GetStage() == ShaderStage::Vertex) {
            stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        } else if (vkShader->GetStage() == ShaderStage::Fragment) {
            stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        } else if (vkShader->GetStage() == ShaderStage::Geometry) {
            stageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        } else if (vkShader->GetStage() == ShaderStage::Compute) {
            stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        }

        shaderStages.push_back(stageInfo);
    }

    // Vertex input (simple position + normal for now)
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(float) * 6; // 3 position + 3 normal
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescs(2);
    attributeDescs[0].binding = 0;
    attributeDescs[0].location = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[0].offset = 0;

    attributeDescs[1].binding = 0;
    attributeDescs[1].location = 1;
    attributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[1].offset = sizeof(float) * 3;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = ToVulkanTopology(topology);
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport state (dynamic)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = !rasterizerState.depthClipEnable;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = ToVulkanPolygonMode(rasterizerState.fillMode);
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = ToVulkanCullMode(rasterizerState.cullMode);
    rasterizer.frontFace = rasterizerState.frontCounterClockwise ?
        VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = rasterizerState.depthBias != 0;
    rasterizer.depthBiasConstantFactor = static_cast<float>(rasterizerState.depthBias);
    rasterizer.depthBiasClamp = rasterizerState.depthBiasClamp;
    rasterizer.depthBiasSlopeFactor = rasterizerState.slopeScaledDepthBias;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = depthStencilState.depthEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = ToVulkanCompareOp(depthStencilState.depthFunc);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = depthStencilState.stencilEnable ? VK_TRUE : VK_FALSE;

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = blendState.blendEnable ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = ToVulkanBlendFactor(blendState.srcBlend);
    colorBlendAttachment.dstColorBlendFactor = ToVulkanBlendFactor(blendState.dstBlend);
    colorBlendAttachment.colorBlendOp = ToVulkanBlendOp(blendState.blendOp);
    colorBlendAttachment.srcAlphaBlendFactor = ToVulkanBlendFactor(blendState.srcBlendAlpha);
    colorBlendAttachment.dstAlphaBlendFactor = ToVulkanBlendFactor(blendState.dstBlendAlpha);
    colorBlendAttachment.alphaBlendOp = ToVulkanBlendOp(blendState.blendOpAlpha);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Dynamic state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline layout (empty for now)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        Logger::Error("Failed to create Vulkan pipeline layout");
        return nullptr;
    }

    // Create a simple render pass for now
    auto renderPass = CreateRenderPass({TextureFormat::RGBA8_SRGB}, TextureFormat::D24_UNORM_S8_UINT);
    VulkanRenderPass* vkRenderPass = static_cast<VulkanRenderPass*>(renderPass.get());

    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = vkRenderPass->GetVkRenderPass();
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        Logger::Error("Failed to create Vulkan graphics pipeline");
        vkDestroyPipelineLayout(device_, pipelineLayout, nullptr);
        return nullptr;
    }

    return std::make_shared<VulkanPipeline>(this, pipeline, pipelineLayout);
}

RHIRenderPassPtr VulkanDevice::CreateRenderPass(
    const std::vector<TextureFormat>& colorFormats,
    TextureFormat depthFormat) {

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorRefs;

    // Color attachments
    for (size_t i = 0; i < colorFormats.size(); ++i) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = ToVulkanFormat(colorFormats[i]);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachments.push_back(colorAttachment);

        VkAttachmentReference colorRef{};
        colorRef.attachment = static_cast<uint32_t>(i);
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRefs.push_back(colorRef);
    }

    // Depth attachment
    VkAttachmentReference depthRef{};
    if (depthFormat != TextureFormat::R8_UNORM) { // Use as "no depth" indicator
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = ToVulkanFormat(depthFormat);
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(depthAttachment);

        depthRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    // Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
    subpass.pColorAttachments = colorRefs.data();
    if (depthFormat != TextureFormat::R8_UNORM) {
        subpass.pDepthStencilAttachment = &depthRef;
    }

    // Subpass dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        Logger::Error("Failed to create Vulkan render pass");
        return nullptr;
    }

    return std::make_shared<VulkanRenderPass>(this, renderPass);
}

RHIFramebufferPtr VulkanDevice::CreateFramebuffer(
    RHIRenderPass* renderPass,
    const std::vector<RHITexture*>& colorAttachments,
    RHITexture* depthAttachment) {

    VulkanRenderPass* vkRenderPass = static_cast<VulkanRenderPass*>(renderPass);

    std::vector<VkImageView> attachments;
    uint32_t width = 0, height = 0;

    for (auto* tex : colorAttachments) {
        VulkanTexture* vkTex = static_cast<VulkanTexture*>(tex);
        attachments.push_back(vkTex->GetVkImageView());
        if (width == 0) {
            width = tex->GetWidth();
            height = tex->GetHeight();
        }
    }

    if (depthAttachment) {
        VulkanTexture* vkDepth = static_cast<VulkanTexture*>(depthAttachment);
        attachments.push_back(vkDepth->GetVkImageView());
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass->GetVkRenderPass();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    VkFramebuffer framebuffer;
    if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        Logger::Error("Failed to create Vulkan framebuffer");
        return nullptr;
    }

    return std::make_shared<VulkanFramebuffer>(this, framebuffer, width, height);
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_VULKAN_ENABLED
