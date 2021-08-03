#pragma once

#include <vulkan.h>
#include <memory>
#include <array>


class SwapChain;
class DescriptorSetLayout;
class Device;
class RenderPass;


class GraphicsPipeline
{
private:
	std::shared_ptr<Device> device;
	std::shared_ptr<RenderPass> renderPass;
	VkPipelineLayout vkPipelineLayout;
	VkPipeline vkPipeline;
	VkPipelineShaderStageCreateInfo shaderStageInfos[2];
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
	VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
	VkPipelineColorBlendStateCreateInfo colorBlendState;

	VkShaderModule loadShader(const char* fileName);

	VkPipelineShaderStageCreateInfo buildVertexStageCreateInfo(VkShaderModule vertexShader) const;
	VkPipelineShaderStageCreateInfo buildFragmentStageCreateInfo(VkShaderModule fragmentShader) const;
	VkVertexInputBindingDescription buildVertexBindingDescription();

	VkPipelineVertexInputStateCreateInfo buildVertexInputStateCreateInfo(
		const VkVertexInputBindingDescription* vertexBindingDesc,
		const std::array<VkVertexInputAttributeDescription, 2>* vertexAttributeDesc) const;

	VkPipelineInputAssemblyStateCreateInfo buildInputAssemblyStateCreateInfo() const;
	VkViewport buildViewport(std::shared_ptr<SwapChain> swapChain) const;
	VkRect2D buildScissor(std::shared_ptr<SwapChain> swapChain) const;

	VkPipelineViewportStateCreateInfo buildViewportStateCreateInfo(VkViewport* viewport,
		VkRect2D* scissor) const;

	VkPipelineRasterizationStateCreateInfo buildRasterizationStateCreateInfo() const;
	VkPipelineMultisampleStateCreateInfo buildMultisampleStateCreateInfo() const;
	VkPipelineColorBlendAttachmentState buildColorBlendAttachmentState() const;

	VkPipelineColorBlendStateCreateInfo buildColorBlendAttachmentStateCreateInfo(
		VkPipelineColorBlendAttachmentState* colorBlendAttachmentState) const;

	VkPipelineLayoutCreateInfo buildPipelineLayoutCreateInfo(
		std::shared_ptr<DescriptorSetLayout> descriptorSetLayout) const;

	std::array<VkVertexInputAttributeDescription, 2> buildVertexAttributeDescription();
	VkResult createPipelineLayout(VkPipelineLayoutCreateInfo* pipelineLayoutInfo);
	void throwIfCreatePipelineLayoutFailed(VkResult result);
	VkPipelineDepthStencilStateCreateInfo buildDepthStencilStateCreateInfo() const;
	VkGraphicsPipelineCreateInfo buildPipelineCreateInfo() const;
	VkResult createPipeline(VkGraphicsPipelineCreateInfo* pipelineInfo);
	void throwIfCreatePipelineFailed(VkResult result);
	void destroyShader(VkShaderModule shader) const;

public:
	GraphicsPipeline(std::shared_ptr<Device> device, std::shared_ptr<SwapChain> swapChain,
		std::shared_ptr<RenderPass> renderPass, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout);

	~GraphicsPipeline();

	VkPipelineLayout getLayoutHandle() const;
	VkPipeline getHandle() const;
};