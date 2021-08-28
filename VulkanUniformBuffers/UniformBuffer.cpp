#include "UniformBuffer.h"
#include "SwapChain.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DescriptorSetLayout.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "InputState.h"


UniformBuffer::UniformBuffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device, 
	std::shared_ptr<SwapChain> swapChain, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout)
	: Buffer(physicalDevice, device), 
	speed(1.0f),
	xUnit(1.0f, 0.0f, 0.0f),
	yUnit(0.0f, 1.0f, 0.0f),
	zUnit(0.0f, 0.0f, 1.0f)
{
	this->swapChain = swapChain;
	this->descriptorSetLayout = descriptorSetLayout;
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	vkUniformBuffers.resize(swapChain->initSwapChainImages()->size());
	vkUniformDeviceMemory.resize(swapChain->initSwapChainImages()->size());

	for (size_t i = 0; i < vkUniformBuffers.size(); ++i)
	{
		const VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		const VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createBuffer(bufferSize, usageFlags, memoryFlags, &vkUniformBuffers[i], &vkUniformDeviceMemory[i]);
	}
}

UniformBuffer::~UniformBuffer()
{
	for (size_t i = 0; i < vkUniformBuffers.size(); ++i)
	{
		vkDestroyBuffer(device->getHandle(), vkUniformBuffers[i], nullptr);
		vkFreeMemory(device->getHandle(), vkUniformDeviceMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device->getHandle(), vkUniformDescriptorPool, nullptr);
}

void UniformBuffer::updateUniformBuffer(uint32_t imageIndex)
{
	void* data;
	uint32_t memSize = static_cast<uint32_t>(sizeof(UniformBufferObject));
	vkMapMemory(device->getHandle(), vkUniformDeviceMemory[imageIndex], 0.0f, memSize, 0, &data);
	memcpy(data, &uniformBufferObject, memSize);
	vkUnmapMemory(device->getHandle(), vkUniformDeviceMemory[imageIndex]);
}

void UniformBuffer::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.descriptorCount = static_cast<uint32_t>(swapChain->getSwapChainImageViews()->size());
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	poolCreateInfo.maxSets = static_cast<uint32_t>(swapChain->getSwapChainImageViews()->size());

	VkResult result = vkCreateDescriptorPool(device->getHandle(), &poolCreateInfo, nullptr, &vkUniformDescriptorPool);
	throwIfCreateDescriptorPoolFailed(result);
}

void UniformBuffer::throwIfCreateDescriptorPoolFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool for uniform buffer.");
	}
}

void UniformBuffer::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChain->getSwapChainImageViews()->size(), descriptorSetLayout->getHandle());

	VkDescriptorSetAllocateInfo setAllocateInfo = {};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = vkUniformDescriptorPool;
	setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	setAllocateInfo.pSetLayouts = layouts.data();

	vkUniformDescriptorSets.resize(layouts.size());

	VkResult result = vkAllocateDescriptorSets(device->getHandle(), &setAllocateInfo,
		vkUniformDescriptorSets.data());

	throwIfAllocateDescriptorSetsFailed(result);

	for (size_t i = 0; i < vkUniformBuffers.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = vkUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = vkUniformDescriptorSets[i];
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
	}
}

void UniformBuffer::throwIfAllocateDescriptorSetsFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set for uniform buffer.");
	}
}

void UniformBuffer::initScene()
{
	uniformBufferObject.model = glm::mat4(1.0f);

	viewPosition = { 0.0f, 0.0f, 2.0f };
	viewRotation = { 0.0f, 0.0f, 0.0f };

	glm::mat4 viewTranslationMat = glm::translate(glm::mat4(1.0f), viewPosition);
	uniformBufferObject.view = glm::inverse(viewTranslationMat);

	float aspectRatio = swapChain->getSwapChainExtent().width / static_cast<float>(swapChain->getSwapChainExtent().height);
	uniformBufferObject.projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	uniformBufferObject.projection =
		glm::scale(uniformBufferObject.projection, glm::vec3(1.0, -1.0f, 1.0f));
}

void UniformBuffer::updateUniformBufferObject(const InputState& inputState, float deltaSec)
{
	if (inputState.left)
	{
		rotateLeft(deltaSec);
	}

	if (inputState.right)
	{
		rotateRight(deltaSec);
	}

	if (inputState.forward)
	{
		moveForward(deltaSec);
	}

	if (inputState.backward)
	{
		moveBackward(deltaSec);
	}

	if (inputState.mouseRight)
	{
		moveMouse(inputState);
	}
}

void UniformBuffer::rotateLeft(float deltaSec)
{
	glm::mat4 viewRotationMat(1.0f);

	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.y, yUnit);
	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.x, xUnit);

	glm::vec3 rightVec = viewRotationMat * glm::vec4(xUnit, 0.0f);
	glm::vec3 translationVec = -rightVec * speed * deltaSec;

	viewPosition += translationVec;

	glm::mat4 viewTranslationMat = glm::translate(glm::mat4(1.0f), viewPosition);
	uniformBufferObject.view = glm::inverse(viewTranslationMat * viewRotationMat);
}

void UniformBuffer::rotateRight(float deltaSec)
{
	glm::mat4 viewRotationMat(1.0f);

	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.y, yUnit);
	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.x, xUnit);

	glm::vec3 rightVec = viewRotationMat * glm::vec4(xUnit, 0.0f);
	glm::vec3 translationVec = rightVec * speed * deltaSec;

	viewPosition += translationVec;
	uniformBufferObject.view = glm::translate(uniformBufferObject.view, -translationVec);
}

void UniformBuffer::moveForward(float deltaSec)
{
	glm::mat4 viewRotationMat(1.0f);

	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.y, yUnit);
	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.x, xUnit);

	glm::vec3 forwardVec = viewRotationMat * glm::vec4(-zUnit, 0.0f);
	glm::vec3 translationVec = forwardVec * speed * deltaSec;

	viewPosition += translationVec;
	uniformBufferObject.view = glm::translate(uniformBufferObject.view, -translationVec);
}

void UniformBuffer::moveBackward(float deltaSec)
{
	glm::mat4 viewRotationMat(1.0f);

	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.y, yUnit);
	viewRotationMat = glm::rotate(viewRotationMat, viewRotation.x, xUnit);

	glm::vec3 forwardVec = viewRotationMat * glm::vec4(-zUnit, 0.0f);
	glm::vec3 translationVec = -forwardVec * speed * deltaSec;

	viewPosition += translationVec;
	uniformBufferObject.view = glm::translate(uniformBufferObject.view, -translationVec);
}


void UniformBuffer::moveMouse(const InputState & inputState)
{
	float xAngle = -static_cast<float>(inputState.mouseYRel) * angleSpeed;
	float yAngle = -static_cast<float>(inputState.mouseXRel) * angleSpeed;
	viewRotation.x += xAngle;
	viewRotation.y += yAngle;

	glm::mat4 rotationMat(1.0f);
	rotationMat = glm::rotate(rotationMat, viewRotation.y, yUnit);
	rotationMat = glm::rotate(rotationMat, viewRotation.x, xUnit);

	glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), viewPosition);
	uniformBufferObject.view = glm::inverse(translationMat * rotationMat);
}

VkDescriptorSet* UniformBuffer::getDescriptorSetHandlePtr(uint32_t index)
{
	return &vkUniformDescriptorSets[index];
}