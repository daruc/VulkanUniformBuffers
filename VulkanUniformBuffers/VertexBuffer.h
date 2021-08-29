#pragma once

#include <memory>
#include <vector>
#include "Buffer.h"
#include "Vertex.h"

class CommandPool;


class VertexBuffer : public Buffer
{
private:
	std::vector<Vertex> vertices;
	VkBuffer vkVertexBuffer;
	VkDeviceMemory vkVertexDeviceMemory;

	void throwIfMapMemoryFailed(VkResult result) const;
	std::vector<Vertex> buildVertices() const;

public:
	VertexBuffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
		std::shared_ptr<CommandPool> commandPool);

	~VertexBuffer();

	VkBuffer getHandle() const;
};