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
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "CommandBuffer.h"


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
	uniformBuffer = std::make_shared<UniformBuffer>(physicalDevice, device, swapChain, descriptorSetLayout);
}

void Engine::createDescriptorPool()
{
	uniformBuffer->createDescriptorPool();
}

void Engine::createDescriptorSets()
{
	uniformBuffer->createDescriptorSets();
}

void Engine::createVertexBuffer()
{
	vertexBuffer = std::make_shared<VertexBuffer>(physicalDevice, device, commandPool);
}

void Engine::createIndexBuffer()
{
	indexBuffer = std::make_shared<IndexBuffer>(physicalDevice, device, commandPool);
}

void Engine::createCommandPool()
{
	commandPool = std::make_shared<CommandPool>(physicalDevice, device);
}

void Engine::createCommandBuffers()
{
	commandBuffer = std::make_shared<CommandBuffer>(device, renderPass, framebuffer, commandPool, swapChain, 
		graphicsPipeline, vertexBuffer, indexBuffer, uniformBuffer);
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
	uniformBuffer->initScene();
}

void Engine::updateUniformBuffer(uint32_t imageIndex)
{
	uniformBuffer->updateUniformBuffer(imageIndex);
}

void Engine::updateUniformBufferObject(float deltaSec)
{
	uniformBuffer->updateUniformBufferObject(m_inputState, deltaSec);

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
	submitInfo.pCommandBuffers = commandBuffer->getHandlePtr(imageIndex);
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

	vertexBuffer.reset();
	indexBuffer.reset();
	uniformBuffer.reset();

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
	vulkanSurface.reset();
	device.reset();
	vulkanInstance.reset();
}
