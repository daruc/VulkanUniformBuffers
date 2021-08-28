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
#include "Buffer.h"


struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class SwapChain;
class DescriptorSetLayout;
struct InputState;


class UniformBuffer : public Buffer
{
private:
	const float angleSpeed = 0.01f;
	const float speed = 1.0f;
	const glm::vec3 xUnit;
	const glm::vec3 yUnit;
	const glm::vec3 zUnit;

	std::vector<VkBuffer> vkUniformBuffers;
	std::vector<VkDeviceMemory> vkUniformDeviceMemory;
	UniformBufferObject uniformBufferObject;
	std::shared_ptr<SwapChain> swapChain;
	std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
	VkDescriptorPool vkUniformDescriptorPool;
	std::vector<VkDescriptorSet> vkUniformDescriptorSets;
	glm::vec3 viewPosition;
	glm::vec3 viewRotation;

	void throwIfAllocateDescriptorSetsFailed(VkResult result) const;
	void throwIfCreateDescriptorPoolFailed(VkResult result) const;
	void moveMouse(const InputState & inputState);
	void moveBackward(float deltaSec);
	void moveForward(float deltaSec);
	void rotateRight(float deltaSec);
	void rotateLeft(float deltaSec);

public:
	UniformBuffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device, 
		std::shared_ptr<SwapChain> swapChain, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout);

	~UniformBuffer();

	void updateUniformBufferObject(const InputState& inputState, float deltaSec);
	void createDescriptorSets();
	void createDescriptorPool();
	void initScene();
	void updateUniformBuffer(uint32_t imageIndex);
	VkDescriptorSet* getDescriptorSetHandlePtr(uint32_t index);
};