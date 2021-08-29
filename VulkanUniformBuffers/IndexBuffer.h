#pragma once

#include "Buffer.h"
#include <memory>
#include <vector>

class PhysicalDevice;
class Device;
class CommandPool;

class IndexBuffer : public Buffer
{
private:
	std::vector<uint32_t> indices;
	VkBuffer vkIndexBuffer;
	VkDeviceMemory vkIndexDeviceMemory;

	std::vector<uint32_t> buildIndices() const;
	void throwIfMapMemoryFailed(VkResult result) const;

public:
	IndexBuffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
		std::shared_ptr<CommandPool> commandPool);

	VkBuffer getHandle() const;
	uint32_t getIndicesCount() const;

	~IndexBuffer();
};