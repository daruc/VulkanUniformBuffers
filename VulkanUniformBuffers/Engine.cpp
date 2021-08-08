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
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "Depth.h"


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
	descriptorSetLayout = std::make_shared<DescriptorSetLayout>(device);
}

void Engine::createGraphicsPipeline()
{
	graphicsPipeline = std::make_shared<GraphicsPipeline>(device, swapChain, renderPass, descriptorSetLayout);
}

void Engine::createFramebuffers()
{
	framebuffer = std::make_shared<Framebuffer>(device, swapChain, renderPass, depth->getImageViewHandle());
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
	std::vector<VkDescriptorSetLayout> layouts(swapChain->getSwapChainImageViews()->size(), descriptorSetLayout->getHandle());

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
	memoryAllocateInfo.memoryTypeIndex = physicalDevice->findMemoryType(vertexBufferMemRequirements.memoryTypeBits,
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
	commandBufferAllocateInfo.commandPool = commandPool->getHandle();
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
	vkFreeCommandBuffers(device->getHandle(), commandPool->getHandle(), 1, &commandBuffer);
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
	commandPool = std::make_shared<CommandPool>(physicalDevice, device);
}

void Engine::createCommandBuffers()
{
	m_vkCommandBuffers.resize(framebuffer->getCount());

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = commandPool->getHandle();
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
		renderPassInfo.framebuffer = framebuffer->getHandle(i);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues;
		clearValues[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1] = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getHandle());

		VkBuffer buffers[] = { m_vkVertexBuffer };
		VkDeviceSize bufferOffsets[] = { 0 };
		vkCmdBindVertexBuffers(m_vkCommandBuffers[i], 0, 1, buffers, bufferOffsets);
		vkCmdBindIndexBuffer(m_vkCommandBuffers[i], m_vkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getLayoutHandle(),
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
	depth = std::make_shared<Depth>(physicalDevice, device, swapChain);
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

bool Engine::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	return formats[0];
}

VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
	return VK_PRESENT_MODE_FIFO_KHR;
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
	
	depth.reset();
	commandPool.reset();
	graphicsPipeline.reset();
	swapChain.reset();
	framebuffer.reset();
	descriptorSetLayout.reset();
	vkDestroyDescriptorPool(device->getHandle(), m_vkUniformDescriptorPool, nullptr);
	vulkanSurface.reset();
	device.reset();
	vulkanInstance.reset();
}
