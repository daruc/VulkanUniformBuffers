#include "VertexBuffer.h"
#include "Vertex.h"
#include "Device.h"
#include "CommandPool.h"


VertexBuffer::VertexBuffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
	std::shared_ptr<CommandPool> commandPool)
	: Buffer(physicalDevice, device)
{
	vertices = buildVertices();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
	const VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	const VkMemoryPropertyFlags stagingMemPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	createBuffer(bufferSize, stagingBufferUsageFlags, stagingMemPropertyFlags,
		&stagingBuffer, &stagingMemory);

	void* mappedMemory;
	VkResult result = vkMapMemory(device->getHandle(), stagingMemory, 0, bufferSize, 0, &mappedMemory);
	throwIfMapMemoryFailed(result);

	memcpy(mappedMemory, vertices.data(), bufferSize);
	vkUnmapMemory(device->getHandle(), stagingMemory);

	const VkBufferUsageFlags vertexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	const VkMemoryPropertyFlags vertexMemPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createBuffer(bufferSize, vertexBufferUsageFlags, vertexMemPropertyFlags, &vkVertexBuffer,
		&vkVertexDeviceMemory);

	copyBuffer(commandPool, bufferSize, stagingBuffer, vkVertexBuffer);

	vkDestroyBuffer(device->getHandle(), stagingBuffer, nullptr);
	vkFreeMemory(device->getHandle(), stagingMemory, nullptr);
}

std::vector<Vertex> VertexBuffer::buildVertices() const
{
	std::vector<Vertex> vertices(8);

	vertices[0].color = { 1.0f, 0.0f, 0.0f };
	vertices[0].position = { -0.5f, -0.5f, 0.5f };

	vertices[1].color = { 0.0f, 1.0f, 0.0f };
	vertices[1].position = { 0.5f, -0.5f, 0.5f };

	vertices[2].color = { 0.0f, 0.0f, 1.0f };
	vertices[2].position = { 0.5f, 0.5f, 0.5f };

	vertices[3].color = { 1.0f, 0.0f, 1.0f };
	vertices[3].position = { -0.5f, 0.5f, 0.5f };

	vertices[4].color = { 1.0f, 0.0f, 0.0f };
	vertices[4].position = { -0.5f, -0.5f, -0.5f };

	vertices[5].color = { 0.0f, 1.0f, 0.0f };
	vertices[5].position = { 0.5f, -0.5f, -0.5f };

	vertices[6].color = { 0.0f, 0.0f, 1.0f };
	vertices[6].position = { 0.5f, 0.5f, -0.5f };

	vertices[7].color = { 1.0f, 0.0f, 1.0f };
	vertices[7].position = { -0.5f, 0.5f, -0.5f };

	return vertices;
}

void VertexBuffer::throwIfMapMemoryFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map vertex memory.");
	}
}

VertexBuffer::~VertexBuffer()
{
	vkDestroyBuffer(device->getHandle(), vkVertexBuffer, nullptr);
	vkFreeMemory(device->getHandle(), vkVertexDeviceMemory, nullptr);
}

VkBuffer VertexBuffer::getHandle() const
{
	return vkVertexBuffer;
}