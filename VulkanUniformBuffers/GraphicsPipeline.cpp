#include "GraphicsPipeline.h"
#include "SwapChain.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include "RenderPass.h"
#include "Vertex.h"
#include <fstream>
#include <vector>


GraphicsPipeline::GraphicsPipeline(std::shared_ptr<Device> device, std::shared_ptr<SwapChain> swapChain,
	std::shared_ptr<RenderPass> renderPass, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout)
{
	this->device = device;
	this->renderPass = renderPass;

	VkShaderModule vertexShader = loadShader("vertex.spv");
	VkShaderModule fragmentShader = loadShader("fragment.spv");

	VkPipelineShaderStageCreateInfo vertexStageCreateInfo = buildVertexStageCreateInfo(vertexShader);
	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo = buildFragmentStageCreateInfo(fragmentShader);
	shaderStageInfos[0] = vertexStageCreateInfo;
	shaderStageInfos[1] = fragmentStageCreateInfo;

	const VkVertexInputBindingDescription vertexBindingDesc = buildVertexBindingDescription();
	const std::array<VkVertexInputAttributeDescription, 2> vertexAttributeDesc = buildVertexAttributeDescription();

	vertexInputInfo = buildVertexInputStateCreateInfo(&vertexBindingDesc, &vertexAttributeDesc);
	inputAssemblyCreateInfo = buildInputAssemblyStateCreateInfo();
	
	VkViewport viewport = buildViewport(swapChain);
	VkRect2D scissor = buildScissor(swapChain);
	viewportStateCreateInfo = buildViewportStateCreateInfo(&viewport, &scissor);

	rasterizationStateCreateInfo = buildRasterizationStateCreateInfo();
	multisamplingStateCreateInfo = buildMultisampleStateCreateInfo();
	VkPipelineColorBlendAttachmentState colorBlendAttachment = buildColorBlendAttachmentState();

	colorBlendState = buildColorBlendAttachmentStateCreateInfo(&colorBlendAttachment);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = buildPipelineLayoutCreateInfo(descriptorSetLayout);
	VkResult result = createPipelineLayout(&pipelineLayoutInfo);
	throwIfCreatePipelineLayoutFailed(result);

	depthStencilStateCreateInfo = buildDepthStencilStateCreateInfo();

	VkGraphicsPipelineCreateInfo pipelineInfo = buildPipelineCreateInfo();

	result = createPipeline(&pipelineInfo);
	throwIfCreatePipelineFailed(result);

	destroyShader(vertexShader);
	destroyShader(fragmentShader);
}

VkPipelineShaderStageCreateInfo GraphicsPipeline::buildVertexStageCreateInfo(VkShaderModule vertexShader) const
{
	VkPipelineShaderStageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	createInfo.module = vertexShader;
	createInfo.pName = "main";
	
	return createInfo;
}

VkPipelineShaderStageCreateInfo GraphicsPipeline::buildFragmentStageCreateInfo(VkShaderModule fragmentShader) const
{
	VkPipelineShaderStageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	createInfo.module = fragmentShader;
	createInfo.pName = "main";

	return createInfo;
}

VkPipelineInputAssemblyStateCreateInfo GraphicsPipeline::buildInputAssemblyStateCreateInfo() const
{
	VkPipelineInputAssemblyStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.primitiveRestartEnable = VK_FALSE;

	return createInfo;
}

VkViewport GraphicsPipeline::buildViewport(std::shared_ptr<SwapChain> swapChain) const
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	return viewport;
}

VkRect2D GraphicsPipeline::buildScissor(std::shared_ptr<SwapChain> swapChain) const
{
	VkRect2D scissor = {};
	scissor.extent = swapChain->getSwapChainExtent();
	scissor.offset = { 0, 0 };

	return scissor;
}

VkPipelineVertexInputStateCreateInfo GraphicsPipeline::buildVertexInputStateCreateInfo(
	const VkVertexInputBindingDescription* vertexBindingDesc, 
	const std::array<VkVertexInputAttributeDescription, 2>* vertexAttributeDesc) const
{
	VkPipelineVertexInputStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	createInfo.pVertexBindingDescriptions = vertexBindingDesc;
	createInfo.vertexBindingDescriptionCount = 1;
	createInfo.pVertexAttributeDescriptions = vertexAttributeDesc->data();
	createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDesc->size());

	return createInfo;
}

VkPipelineViewportStateCreateInfo GraphicsPipeline::buildViewportStateCreateInfo(VkViewport* viewport,
	VkRect2D* scissor) const
{
	VkPipelineViewportStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	createInfo.viewportCount = 1;
	createInfo.pViewports = viewport;
	createInfo.scissorCount = 1;
	createInfo.pScissors = scissor;

	return createInfo;
}

VkPipelineRasterizationStateCreateInfo GraphicsPipeline::buildRasterizationStateCreateInfo() const
{
	VkPipelineRasterizationStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	createInfo.depthClampEnable = VK_FALSE;
	createInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.polygonMode = VK_POLYGON_MODE_FILL;
	createInfo.lineWidth = 1.0f;
	createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	createInfo.depthBiasEnable = VK_FALSE;
	createInfo.depthBiasClamp = 0.0f;
	createInfo.depthBiasSlopeFactor = 0.0f;

	return createInfo;
}

VkPipelineMultisampleStateCreateInfo GraphicsPipeline::buildMultisampleStateCreateInfo() const
{
	VkPipelineMultisampleStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	createInfo.sampleShadingEnable = VK_FALSE;
	createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.minSampleShading = 1.0f;
	createInfo.pSampleMask = nullptr;
	createInfo.alphaToCoverageEnable = VK_FALSE;
	createInfo.alphaToOneEnable = VK_FALSE;

	return createInfo;
}

VkPipelineColorBlendAttachmentState GraphicsPipeline::buildColorBlendAttachmentState() const
{
	VkPipelineColorBlendAttachmentState state = {};
	state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	state.blendEnable = VK_FALSE;
	state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	state.alphaBlendOp = VK_BLEND_OP_ADD;

	return state;
}

VkPipelineColorBlendStateCreateInfo GraphicsPipeline::buildColorBlendAttachmentStateCreateInfo(
	VkPipelineColorBlendAttachmentState* colorBlendAttachmentState) const
{
	VkPipelineColorBlendStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	createInfo.logicOpEnable = VK_FALSE;
	createInfo.logicOp = VK_LOGIC_OP_COPY;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = colorBlendAttachmentState;
	createInfo.blendConstants[0] = 0.0f;
	createInfo.blendConstants[1] = 0.0f;
	createInfo.blendConstants[2] = 0.0f;
	createInfo.blendConstants[3] = 0.0f;

	return createInfo;
}

VkPipelineLayoutCreateInfo GraphicsPipeline::buildPipelineLayoutCreateInfo(
	std::shared_ptr<DescriptorSetLayout> descriptorSetLayout) const
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayout->getHandlePtr();

	return pipelineLayoutInfo;
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(device->getHandle(), vkPipeline, nullptr);
	vkDestroyPipelineLayout(device->getHandle(), vkPipelineLayout, nullptr);
}

VkShaderModule GraphicsPipeline::loadShader(const char* fileName)
{
	std::ifstream istr(fileName, std::ios::ate | std::ios::binary);

	if (!istr.is_open())
	{
		throw std::runtime_error("Failed to open shader file.");
	}

	size_t fileSize = static_cast<size_t>(istr.tellg());
	std::vector<char> buffer(fileSize);
	istr.seekg(0);
	istr.read(buffer.data(), fileSize);
	istr.close();

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = fileSize;
	shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device->getHandle(), &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module.");
	}

	return shaderModule;
}

VkVertexInputBindingDescription GraphicsPipeline::buildVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> GraphicsPipeline::buildVertexAttributeDescription()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}

VkResult GraphicsPipeline::createPipelineLayout(VkPipelineLayoutCreateInfo* pipelineLayoutInfo) {
	return vkCreatePipelineLayout(device->getHandle(), pipelineLayoutInfo, nullptr, &vkPipelineLayout);
}

void GraphicsPipeline::throwIfCreatePipelineLayoutFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout.");
	}
}

VkPipelineDepthStencilStateCreateInfo GraphicsPipeline::buildDepthStencilStateCreateInfo() const
{
	VkPipelineDepthStencilStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	createInfo.depthTestEnable = VK_TRUE;
	createInfo.depthWriteEnable = VK_TRUE;
	createInfo.depthCompareOp = VK_COMPARE_OP_LESS;

	return createInfo;
}

VkGraphicsPipelineCreateInfo GraphicsPipeline::buildPipelineCreateInfo() const
{
	VkGraphicsPipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStageInfos;
	createInfo.pVertexInputState = &vertexInputInfo;
	createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	createInfo.pViewportState = &viewportStateCreateInfo;
	createInfo.pRasterizationState = &rasterizationStateCreateInfo;
	createInfo.pMultisampleState = &multisamplingStateCreateInfo;
	createInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	createInfo.pColorBlendState = &colorBlendState;
	createInfo.pDynamicState = nullptr;
	createInfo.layout = vkPipelineLayout;
	createInfo.renderPass = renderPass->getHandle();
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = -1;

	return createInfo;
}

VkResult GraphicsPipeline::createPipeline(VkGraphicsPipelineCreateInfo* pipelineInfo)
{
	return vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, pipelineInfo, nullptr, &vkPipeline);
}

void GraphicsPipeline::throwIfCreatePipelineFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}
}

void GraphicsPipeline::destroyShader(VkShaderModule shader) const
{
	vkDestroyShaderModule(device->getHandle(), shader, nullptr);
}

VkPipelineLayout GraphicsPipeline::getLayoutHandle() const
{
	return vkPipelineLayout;
}

VkPipeline GraphicsPipeline::getHandle() const
{
	return vkPipeline;
}