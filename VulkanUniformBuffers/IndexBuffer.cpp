#include "IndexBuffer.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "CommandPool.h"

IndexBuffer::IndexBuffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
	std::shared_ptr<CommandPool> commandPool)
	: Buffer(physicalDevice, device)
{
	indices = buildIndices();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	const VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();
	const VkBufferUsageFlags stagingBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	const VkMemoryPropertyFlags stagingMemPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	createBuffer(bufferSize, stagingBufferUsageFlags, stagingMemPropertyFlags,
		&stagingBuffer, &stagingMemory);

	void* mappedMemory;
	VkResult result = vkMapMemory(device->getHandle(), stagingMemory, 0, bufferSize, 0, &mappedMemory);
	throwIfMapMemoryFailed(result);

	memcpy(mappedMemory, indices.data(), bufferSize);
	vkUnmapMemory(device->getHandle(), stagingMemory);

	const VkBufferUsageFlags indexBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	const VkMemoryPropertyFlags indexMemPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	createBuffer(bufferSize, indexBufferUsageFlags, indexMemPropertyFlags, &vkIndexBuffer,
		&vkIndexDeviceMemory);

	copyBuffer(commandPool, bufferSize, stagingBuffer, vkIndexBuffer);

	vkDestroyBuffer(device->getHandle(), stagingBuffer, nullptr);
	vkFreeMemory(device->getHandle(), stagingMemory, nullptr);
}

void IndexBuffer::throwIfMapMemoryFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to map index memory.");
	}
}

std::vector<uint32_t> IndexBuffer::buildIndices() const
{
	return { 0, 1, 2, 0, 2, 3, // front
			7, 6, 4, 6, 5, 4, // back
			1, 5, 6, 1, 6, 2,	// right
			4, 0, 3, 4, 3, 7,	// left
			4, 5, 1, 4, 1, 0,	// top
			2, 6, 7, 2, 7, 3	// bottom
	};
}

VkBuffer IndexBuffer::getHandle() const
{
	return vkIndexBuffer;
}

IndexBuffer::~IndexBuffer()
{
	vkDestroyBuffer(device->getHandle(), vkIndexBuffer, nullptr);
	vkFreeMemory(device->getHandle(), vkIndexDeviceMemory, nullptr);
}

uint32_t IndexBuffer::getIndicesCount() const
{
	return indices.size();
}