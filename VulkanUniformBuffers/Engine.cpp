#include "Engine.h"
#include "SDL.h"
#include "SDL_vulkan.h"
#include <vector>
#include <set>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"


void Engine::initVkInstance()
{
	vulkanInstance = std::make_shared<VulkanInstance>(sdlWindow);
}

void Engine::createVkSurface()
{
	vulkanSurface = std::make_shared<VulkanSurface>(sdlWindow, vulkanInstance);
}

void Engine::pickPhysicalDevice()
{
	physicalDevice = std::make_shared<PhysicalDevice>(vulkanInstance, vulkanSurface);
}

void Engine::createDevice()
{
	device = std::make_shared<Device>(physicalDevice);
}

void Engine::createSwapChain()
{
	swapChain = std::make_shared<SwapChain>(sdlWindow, physicalDevice, device, vulkanSurface);
}

void Engine::createRenderPass()
{
	renderPass = std::make_shared<RenderPass>(physicalDevice, device, swapChain);
}

void Engine::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &uboLayoutBinding;

	VkResult result = vkCreateDescriptorSetLayout(device->getHandle(), &descriptorSetLayoutCreateInfo,
		nullptr, &m_vkDescriptorSetLayout);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

void Engine::createGraphicsPipeline()
{
	VkShaderModule vertexShader = loadShader("vertex.spv");
	VkShaderModule fragmentShader = loadShader("fragment.spv");

	VkPipelineShaderStageCreateInfo vertexStageCreateInfo = {};
	vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module = vertexShader;
	vertexStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo = {};
	fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageCreateInfo.module = fragmentShader;
	fragmentStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStageInfos[] = { vertexStageCreateInfo, fragmentStageCreateInfo };

	const VkVertexInputBindingDescription vertexBindingDesc =
		buildVertexBindingDescription();

	const std::array<VkVertexInputAttributeDescription, 2> vertexAttributeDesc =
		buildVertexAttributeDescription();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDesc.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDesc.size());

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent = swapChain->getSwapChainExtent();
	scissor.offset = { 0, 0 };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.lineWidth = 1.0f;
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo = {};
	multisamplingStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingStateCreateInfo.minSampleShading = 1.0f;
	multisamplingStateCreateInfo.pSampleMask = nullptr;
	multisamplingStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;
	colorBlendState.blendConstants[0] = 0.0f;
	colorBlendState.blendConstants[1] = 0.0f;
	colorBlendState.blendConstants[2] = 0.0f;
	colorBlendState.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;

	VkResult result = vkCreatePipelineLayout(device->getHandle(), &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout.");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;


	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStageInfos;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineInfo.pViewportState = &viewportStateCreateInfo;
	pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
	pipelineInfo.pMultisampleState = &multisamplingStateCreateInfo;
	pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_vkPipelineLayout;
	pipelineInfo.renderPass = renderPass->getHandle();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(device->getHandle(), vertexShader, nullptr);
	vkDestroyShaderModule(device->getHandle(), fragmentShader, nullptr);
}

void Engine::createFramebuffers()
{
	m_vkSwapchainFramebuffers.resize(swapChain->getSwapChainImageViews()->size());

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass->getHandle();
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.width = swapChain->getSwapChainExtent().width;
	framebufferCreateInfo.height = swapChain->getSwapChainExtent().height;
	framebufferCreateInfo.layers = 1;

	for (int i = 0; i < swapChain->getSwapChainImageViews()->size(); ++i)
	{
		std::array<VkImageView, 2> attachments = {
			swapChain->getSwapChainImageViews()->at(i),
			m_vkDepthImageView
		};

		framebufferCreateInfo.pAttachments = attachments.data();

		VkResult result = vkCreateFramebuffer(device->getHandle(), &framebufferCreateInfo, nullptr, &m_vkSwapchainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain frame buffer.");
		}
	}
}

void Engine::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	m_vkUniformBuffers.resize(swapChain->initSwapChainImages()->size());
	m_vkUniformDeviceMemory.resize(swapChain->initSwapChainImages()->size());

	for (size_t i = 0; i < m_vkUniformBuffers.size(); ++i)
	{
		const VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		const VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createBuffer(bufferSize, usageFlags, memoryFlags, &m_vkUniformBuffers[i], &m_vkUniformDeviceMemory[i]);
	}
}

void Engine::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.descriptorCount = static_cast<uint32_t>(swapChain->getSwapChainImageViews()->size());
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	poolCreateInfo.maxSets = static_cast<uint32_t>(swapChain->getSwapChainImageViews()->size());

	VkResult result = vkCreateDescriptorPool(device->getHandle(), &poolCreateInfo, nullptr, &m_vkUniformDescriptorPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool for uniform buffer.");
	}
}

void Engine::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChain->getSwapChainImageViews()->size(), m_vkDescriptorSetLayout);

	VkDescriptorSetAllocateInfo setAllocateInfo = {};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = m_vkUniformDescriptorPool;
	setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	setAllocateInfo.pSetLayouts = layouts.data();

	m_vkUniformDescriptorSets.resize(layouts.size());

	VkResult result = vkAllocateDescriptorSets(device->getHandle(), &setAllocateInfo,
		m_vkUniformDescriptorSets.data());

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set for uniform buffer.");
	}

	for (size_t i = 0; i < m_vkUniformBuffers.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = m_vkUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = m_vkUniformDescriptorSets[i];
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
	}
}

void Engine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
	VkBuffer* outBuffer, VkDeviceMemory* outDeviceMemory)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device->getHandle(), &bufferCreateInfo, nullptr, outBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer.");
	}

	VkMemoryRequirements vertexBufferMemRequirements;
	vkGetBufferMemoryRequirements(device->getHandle(), *outBuffer, &vertexBufferMemRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = vertexBufferMemRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(vertexBufferMemRequirements.memoryTypeBits,
		propertyFlags);

	result = vkAllocateMemory(device->getHandle(), &memoryAllocateInfo, nullptr, outDeviceMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory.");
	}

	vkBindBufferMemory(device->getHandle(), *outBuffer, *outDeviceMemory, 0);
}

void Engine::copyBuffer(VkDeviceSize size, VkBuffer srcBuffer, VkBuffer dstBuffer)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.commandPool = m_vkCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->getHandle(), &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device->getGraphicsQueueHandle(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->getGraphicsQueueHandle());
	vkFreeCommandBuffers(device->getHandle(), m_vkCommandPool, 1, &commandBuffer);
}

void Engine::createVertexBuffer()
{
	m_vertices.resize(8);

	m_vertices[0].color = { 1.0f, 0.0f, 0.0f };
	m_vertices[0].position = { -0.5f, -0.5f, 0.5f };
	
	m_vertices[1].color = { 0.0f, 1.0f, 0.0f };
	m_vertices[1].position = { 0.5f, -0.5f, 0.5f };
	
	m_vertices[2].color = { 0.0f, 0.0f, 1.0f };
	m_vertices[2].position = { 0.5f, 0.5f, 0.5f };

	m_vertices[3].color = { 1.0f, 0.0f, 1.0f };
	m_vertices[3].position = { -0.5f, 0.5f, 0.5f };

	m_vertices[4].color = { 1.0f, 0.0f, 0.0f };
	m_vertices[4].position = { -0.5f, -0.5f, -0.5f };

	m_vertices[5].color = { 0.0f, 1.0f, 0.0f };
	m_vertices[5].position = { 0.5f, -0.5f, -0.5f };

	m_vertices[6].color = { 0.0f, 0.0f, 1.0f };
	m_vertices[6].position = { 0.5f, 0.5f, -0.5f };

	m_vertices[7].color = { 1.0f, 0.0f, 1.0f };
	m_vertices[7].position = { -0.5f, 0.5f, -0.5f };

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	const VkDeviceSize bufferSize = sizeof(Vertex) * m_vertices.size();
	const VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	const VkMemoryPropertyFlags stagingMemPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	createBuffer(bufferSize, stagingBufferUsageFlags, stagingMemPropertyFlags,
		&stagingBuffer, &stagingMemory);

	void* mappedMemory;
	VkResult result = vkMapMemory(device->getHandle(), stagingMemory, 0, bufferSize, 0, &mappedMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map vertex memory.");
	}

	memcpy(mappedMemory, m_vertices.data(), bufferSize);
	vkUnmapMemory(device->getHandle(), stagingMemory);

	const VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	const VkMemoryPropertyFlags vertexMemPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createBuffer(bufferSize, vertexBufferUsageFlags, vertexMemPropertyFlags, &m_vkVertexBuffer,
		&m_vkVertexDeviceMemory);

	copyBuffer(bufferSize, stagingBuffer, m_vkVertexBuffer);

	vkDestroyBuffer(device->getHandle(), stagingBuffer, nullptr);
	vkFreeMemory(device->getHandle(), stagingMemory, nullptr);
}

void Engine::createIndexBuffer()
{
	m_indices = { 0, 1, 2, 0, 2, 3, // front
				7, 6, 4, 6, 5, 4, // back
				1, 5, 6, 1, 6, 2,	// right
				4, 0, 3, 4, 3, 7,	// left
				4, 5, 1, 4, 1, 0,	// top
				2, 6, 7, 2, 7, 3	// bottom
	};
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	const VkDeviceSize bufferSize = sizeof(uint32_t) * m_indices.size();
	const VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	const VkMemoryPropertyFlags stagingMemPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	createBuffer(bufferSize, stagingBufferUsageFlags, stagingMemPropertyFlags,
		&stagingBuffer, &stagingMemory);

	void* mappedMemory;
	VkResult result = vkMapMemory(device->getHandle(), stagingMemory, 0, bufferSize, 0, &mappedMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map index memory.");
	}

	memcpy(mappedMemory, m_indices.data(), bufferSize);
	vkUnmapMemory(device->getHandle(), stagingMemory);

	const VkBufferUsageFlags indexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	const VkMemoryPropertyFlags indexMemPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createBuffer(bufferSize, indexBufferUsageFlags, indexMemPropertyFlags, &m_vkIndexBuffer,
		&m_vkIndexDeviceMemory);

	copyBuffer(bufferSize, stagingBuffer, m_vkIndexBuffer);

	vkDestroyBuffer(device->getHandle(), stagingBuffer, nullptr);
	vkFreeMemory(device->getHandle(), stagingMemory, nullptr);
}

void Engine::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = physicalDevice->getQueueFamilyIndices();

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphics.value();

	VkResult result = vkCreateCommandPool(device->getHandle(), &commandPoolCreateInfo, nullptr, &m_vkCommandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool.");
	}
}

void Engine::createCommandBuffers()
{
	m_vkCommandBuffers.resize(m_vkSwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = m_vkCommandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = static_cast<uint32_t>(m_vkCommandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(device->getHandle(), &commandBufferInfo, m_vkCommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}

	for (size_t i = 0; i < m_vkCommandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		result = vkBeginCommandBuffer(m_vkCommandBuffers[i], &beginInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin command buffer.");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass->getHandle();
		renderPassInfo.framebuffer = m_vkSwapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues;
		clearValues[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1] = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);

		VkBuffer buffers[] = { m_vkVertexBuffer };
		VkDeviceSize bufferOffsets[] = { 0 };
		vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0, 1, buffers, bufferOffsets);
		vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout,
			0, 1, &m_vkUniformDescriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(m_vkCommandBuffers[i], static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(m_vkCommandBuffers[i]);

		result = vkEndCommandBuffer(m_vkCommandBuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer.");
		}
	}
}

void Engine::createSemaphores()
{
	m_vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkResult result = vkCreateSemaphore(device->getHandle(), &semaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphores[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image available semaphore.");
		}

		result = vkCreateSemaphore(device->getHandle(), &semaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphores[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render finished semaphore.");
		}
	}

}

void Engine::createFences()
{
	m_vkFences.resize(MAX_FRAMES_IN_FLIGHT);
	m_vkImagesInFlightFences.resize(swapChain->getSwapChainImageViews()->size(), VK_NULL_HANDLE);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkResult result = vkCreateFence(device->getHandle(), &fenceCreateInfo, nullptr, &m_vkFences[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create fance.");
		}
	}
}

void Engine::createDepthResources()
{
	VkFormat format = findDepthFormat();
	
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = format;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = swapChain->getSwapChainExtent().width;
	imageInfo.extent.height = swapChain->getSwapChainExtent().height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateImage(device->getHandle(), &imageInfo, nullptr, &m_vkDepthImage);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image.");
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device->getHandle(), m_vkDepthImage, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkAllocateMemory(device->getHandle(), &allocInfo, nullptr, &m_vkDepthMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate depth device memory.");
	}

	result = vkBindImageMemory(device->getHandle(), m_vkDepthImage, m_vkDepthMemory, 0);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to bind depth image memory.");
	}

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = m_vkDepthImage;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	result = vkCreateImageView(device->getHandle(), &imageViewCreateInfo, nullptr, &m_vkDepthImageView);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image view.");
	}
}

void Engine::initScene()
{
	m_prevTime = std::chrono::high_resolution_clock::now();
	m_uniformBufferObject.model = glm::mat4(1.0f);

	m_viewPosition = { 0.0f, 0.0f, 2.0f };
	m_viewRotation = { 0.0f, 0.0f, 0.0f };

	glm::mat4 viewTranslationMat = glm::translate(glm::mat4(1.0f), m_viewPosition);
	m_uniformBufferObject.view = glm::inverse(viewTranslationMat);

	float aspectRatio = swapChain->getSwapChainExtent().width / static_cast<float>(swapChain->getSwapChainExtent().height);
	m_uniformBufferObject.projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	m_uniformBufferObject.projection =
		glm::scale(m_uniformBufferObject.projection, glm::vec3(1.0, -1.0f, 1.0f));
}

void Engine::updateUniformBuffer(uint32_t imageIndex)
{
	void* data;
	uint32_t memSize = static_cast<uint32_t>(sizeof(UniformBufferObject));
	vkMapMemory(device->getHandle(), m_vkUniformDeviceMemory[imageIndex], 0.0f, memSize, 0, &data);
	memcpy(data, &m_uniformBufferObject, memSize);
	vkUnmapMemory(device->getHandle(), m_vkUniformDeviceMemory[imageIndex]);
}

void Engine::updateUniformBufferObject(float deltaSec)
{
	const float speed = 1.0f;
	const glm::vec3 xUnit(1.0f, 0.0f, 0.0f);
	const glm::vec3 yUnit(0.0f, 1.0f, 0.0f);
	const glm::vec3 zUnit(0.0f, 0.0f, 1.0f);

	if (m_inputState.left)
	{
		glm::mat4 viewRotationMat(1.0f);

		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.y, yUnit);
		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.x, xUnit);

		glm::vec3 rightVec = viewRotationMat * glm::vec4(xUnit, 0.0f);
		glm::vec3 translationVec = -rightVec * speed * deltaSec;

		m_viewPosition += translationVec;

		glm::mat4 viewTranslationMat = glm::translate(glm::mat4(1.0f), m_viewPosition);
		m_uniformBufferObject.view = glm::inverse(viewTranslationMat * viewRotationMat);
	}

	if (m_inputState.right)
	{
		glm::mat4 viewRotationMat(1.0f);

		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.y, yUnit);
		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.x, xUnit);

		glm::vec3 rightVec = viewRotationMat * glm::vec4(xUnit, 0.0f);
		glm::vec3 translationVec = rightVec * speed * deltaSec;

		m_viewPosition += translationVec;
		m_uniformBufferObject.view = glm::translate(m_uniformBufferObject.view, -translationVec);
	}

	if (m_inputState.forward)
	{
		glm::mat4 viewRotationMat(1.0f);

		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.y, yUnit);
		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.x, xUnit);

		glm::vec3 forwardVec = viewRotationMat * glm::vec4(-zUnit, 0.0f);
		glm::vec3 translationVec = forwardVec * speed * deltaSec;

		m_viewPosition += translationVec;
		m_uniformBufferObject.view = glm::translate(m_uniformBufferObject.view, -translationVec);
	}

	if (m_inputState.backward)
	{
		glm::mat4 viewRotationMat(1.0f);

		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.y, yUnit);
		viewRotationMat = glm::rotate(viewRotationMat, m_viewRotation.x, xUnit);

		glm::vec3 forwardVec = viewRotationMat * glm::vec4(-zUnit, 0.0f);
		glm::vec3 translationVec = -forwardVec * speed * deltaSec;

		m_viewPosition += translationVec;
		m_uniformBufferObject.view = glm::translate(m_uniformBufferObject.view, -translationVec);
	}

	if (m_inputState.mouseRight)
	{
		const float angleSpeed = 0.01f;
		float xAngle = -static_cast<float>(m_inputState.mouseYRel) * angleSpeed;
		float yAngle = -static_cast<float>(m_inputState.mouseXRel) * angleSpeed;
		m_viewRotation.x += xAngle;
		m_viewRotation.y += yAngle;

		glm::mat4 rotationMat(1.0f);
		rotationMat = glm::rotate(rotationMat, m_viewRotation.y, yUnit);
		rotationMat = glm::rotate(rotationMat, m_viewRotation.x, xUnit);

		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_viewPosition);

		m_uniformBufferObject.view = glm::inverse(translationMat * rotationMat);
	}

	m_inputState.mouseXRel = 0;
	m_inputState.mouseYRel = 0;
}

VkVertexInputBindingDescription Engine::buildVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Engine::buildVertexAttributeDescription()
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

uint32_t Engine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice->getHandle(), &deviceMemoryProperties);

	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && 
			(deviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}

	throw std::runtime_error("Can't find memory type.");
}

VkFormat Engine::findSupportedFormat(const std::vector<VkFormat>& candidates,
	VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice->getHandle(), format, &properties);

		if ((tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) ||
			(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features))
		{
			return format;
		}
	}
	
	throw std::runtime_error("Failed to find supported format.");
}

VkFormat Engine::findDepthFormat()
{
	std::vector<VkFormat> candidates;
	candidates.push_back(VK_FORMAT_D32_SFLOAT);
	candidates.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
	
	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	return findSupportedFormat(candidates, tiling, features);
}

bool Engine::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkShaderModule Engine::loadShader(const char* fileName)
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

VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	return formats[0];
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	VkExtent2D extent;

	if (capabilities.currentExtent.width == UINT32_MAX)
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(sdlWindow, &width, &height);

		glm::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		glm::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	}
	else
	{
		extent = capabilities.currentExtent;
	}

	return extent;
}


void Engine::readMouseButton(bool down, Uint8 button)
{
	switch (button)
	{
	case SDL_BUTTON_RIGHT:
		m_inputState.mouseRight = down;
		break;
	}
}

void Engine::readMouseMotion(Sint16 xRel, Sint16 yRel)
{
	m_inputState.mouseXRel = xRel;
	m_inputState.mouseYRel = yRel;
}

void Engine::readKey(bool down, SDL_Keycode key)
{
	switch (key)
	{
	case SDLK_a:
		m_inputState.left = down;
		break;
	case SDLK_d:
		m_inputState.right = down;
		break;
	case SDLK_w:
		m_inputState.forward = down;
		break;
	case SDLK_s:
		m_inputState.backward = down;
		break;
	}
}

Engine::Engine()
	: MAX_FRAMES_IN_FLIGHT(2)
{
}

void Engine::init(SDL_Window* sdlWindow)
{
	this->sdlWindow = sdlWindow;
	m_currentFrame = 0;

	m_inputState = {};

	initVkInstance();
	createVkSurface();
	pickPhysicalDevice();
	createDevice();
	createSwapChain();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createDepthResources();
	createFramebuffers();
	createCommandBuffers();
	createSemaphores();
	createFences();

	initScene();
}

void Engine::readInput(const SDL_Event& sdlEvent)
{
	switch (sdlEvent.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		readMouseButton(true, sdlEvent.button.button);
		break;
	case SDL_MOUSEBUTTONUP:
		readMouseButton(false, sdlEvent.button.button);
		break;
	case SDL_KEYDOWN:
		readKey(true, sdlEvent.key.keysym.sym);
		break;
	case SDL_KEYUP:
		readKey(false, sdlEvent.key.keysym.sym);
		break;
	case SDL_MOUSEMOTION:
		readMouseMotion(sdlEvent.motion.xrel, sdlEvent.motion.yrel);
		break;
	}
}

void Engine::update()
{
	using namespace std::chrono;

	time_point currentTime = high_resolution_clock::now();
	float deltaSec = duration<float>(currentTime - m_prevTime).count();
	m_prevTime = currentTime;
	updateUniformBufferObject(deltaSec);
}

void Engine::render()
{
	vkWaitForFences(device->getHandle(), 1, &m_vkFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device->getHandle(), swapChain->getHandle(), UINT64_MAX,
		m_vkImageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire next image.");
	}

	if (m_vkImagesInFlightFences[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device->getHandle(), 1, &m_vkImagesInFlightFences[imageIndex], VK_TRUE, UINT64_MAX);
	}

	VkSemaphore waitSemaphores[] = { m_vkImageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[] = { m_vkRenderFinishedSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	updateUniformBuffer(imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device->getHandle(), 1, &m_vkFences[m_currentFrame]);

	result = vkQueueSubmit(device->getGraphicsQueueHandle(), 1, &submitInfo, m_vkFences[m_currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to queue submit.");
	}

	VkSwapchainKHR swapChains[] = { swapChain->getHandle() };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(device->getPresentationQueueHandle(), &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to queue presentation.");
	}

	vkQueueWaitIdle(device->getPresentationQueueHandle());

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::cleanUp()
{
	vkDeviceWaitIdle(device->getHandle());

	vkDestroyBuffer(device->getHandle(), m_vkVertexBuffer, nullptr);
	vkFreeMemory(device->getHandle(), m_vkVertexDeviceMemory, nullptr);

	vkDestroyBuffer(device->getHandle(), m_vkIndexBuffer, nullptr);
	vkFreeMemory(device->getHandle(), m_vkIndexDeviceMemory, nullptr);

	for (size_t i = 0; i < m_vkUniformBuffers.size(); ++i)
	{
		vkDestroyBuffer(device->getHandle(), m_vkUniformBuffers[i], nullptr);
		vkFreeMemory(device->getHandle(), m_vkUniformDeviceMemory[i], nullptr);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(device->getHandle(), m_vkImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device->getHandle(), m_vkRenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device->getHandle(), m_vkFences[i], nullptr);
	}

	for (VkFence imagesInFlightFence : m_vkImagesInFlightFences)
	{
		vkDestroyFence(device->getHandle(), imagesInFlightFence, nullptr);
	}

	vkDestroyImageView(device->getHandle(), m_vkDepthImageView, nullptr);
	vkDestroyImage(device->getHandle(), m_vkDepthImage, nullptr);
	vkFreeMemory(device->getHandle(), m_vkDepthMemory, nullptr);

	vkDestroyCommandPool(device->getHandle(), m_vkCommandPool, nullptr);
	vkDestroyPipeline(device->getHandle(), m_vkPipeline, nullptr);
	vkDestroyPipelineLayout(device->getHandle(), m_vkPipelineLayout, nullptr);
	swapChain.reset();

	for (VkFramebuffer framebuffer : m_vkSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(device->getHandle(), framebuffer, nullptr);
	}

	vkDestroyDescriptorSetLayout(device->getHandle(), m_vkDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device->getHandle(), m_vkUniformDescriptorPool, nullptr);
	vulkanSurface.reset();
	device.reset();
	vulkanInstance.reset();
}
