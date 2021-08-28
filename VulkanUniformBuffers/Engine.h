#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vulkan.h>
#include <optional>
#include <vector>
#include <array>
#include "glm/common.hpp"

#include "glm/mat4x4.hpp"
#include <chrono>
#include "SDL.h"
#include <memory>
#include "SwapChainSupportDetails.h"
#include "QueueFamilyIndices.h"
#include "Vertex.h"
#include "InputState.h"


class VulkanInstance;
class VulkanSurface;
class PhysicalDevice;
class Device;
class SwapChain;
class RenderPass;
class DescriptorSetLayout;
class GraphicsPipeline;
class Framebuffer;
class CommandPool;
class Depth;
class UniformBuffer;


class Engine
{
private:
	const int MAX_FRAMES_IN_FLIGHT;

	struct SDL_Window* sdlWindow;
	std::shared_ptr<VulkanInstance> vulkanInstance;
	std::shared_ptr<VulkanSurface> vulkanSurface;
	std::shared_ptr<PhysicalDevice> physicalDevice;
	std::shared_ptr<Device> device;
	std::shared_ptr<SwapChain> swapChain;
	std::shared_ptr<RenderPass> renderPass;
	std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<GraphicsPipeline> graphicsPipeline;
	std::shared_ptr<Framebuffer> framebuffer;
	std::shared_ptr<CommandPool> commandPool;
	std::shared_ptr<Depth> depth;
	std::shared_ptr<UniformBuffer> uniformBuffer;

	std::vector<VkCommandBuffer> m_vkCommandBuffers;
	std::vector<VkSemaphore> m_vkImageAvailableSemaphores;
	std::vector<VkSemaphore> m_vkRenderFinishedSemaphores;
	std::vector<VkFence> m_vkFences;
	std::vector<VkFence> m_vkImagesInFlightFences;
	int m_currentFrame;
	std::vector<Vertex> m_vertices;
	VkBuffer m_vkVertexBuffer;
	VkDeviceMemory m_vkVertexDeviceMemory;
	std::vector<uint32_t> m_indices;
	VkBuffer m_vkIndexBuffer;
	VkDeviceMemory m_vkIndexDeviceMemory;

	std::chrono::high_resolution_clock::time_point m_prevTime;
	InputState m_inputState;

	void initVkInstance();
	void createVkSurface();
	void pickPhysicalDevice();
	void createDevice();
	void createSwapChain();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
		VkBuffer* outBuffer, VkDeviceMemory* outDeviceMemory);

	void copyBuffer(VkDeviceSize size, VkBuffer source, VkBuffer destination);

	void createVertexBuffer();
	void createIndexBuffer();
	void createCommandPool();
	void createCommandBuffers();
	void createSemaphores();
	void createFences();
	void createDepthResources();

	void initScene();
	void updateUniformBuffer(uint32_t imageIndex);
	void updateUniformBufferObject(float deltaSec);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
	bool hasStencilComponent(VkFormat format);

	void readMouseButton(bool down, Uint8 button);
	void readMouseMotion(Sint16 xRel, Sint16 yRel);
	void readKey(bool down, SDL_Keycode key);
	
public:
	Engine();

	void init(SDL_Window* sdlWindow);
	void readInput(const SDL_Event& sdlEvent);
	void update();
	void render();
	void cleanUp();
};

