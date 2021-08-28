#include "Buffer.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include <stdexcept>


Buffer::Buffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device)
{
	this->device = device;
	this->physicalDevice = physicalDevice;
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
	VkBuffer* outBuffer, VkDeviceMemory* outDeviceMemory)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device->getHandle(), &bufferCreateInfo, nullptr, outBuffer);
	throwIfCreateBufferFailed(result);

	VkMemoryRequirements vertexBufferMemRequirements;
	vkGetBufferMemoryRequirements(device->getHandle(), *outBuffer, &vertexBufferMemRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = vertexBufferMemRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = physicalDevice->findMemoryType(vertexBufferMemRequirements.memoryTypeBits,
		propertyFlags);

	result = vkAllocateMemory(device->getHandle(), &memoryAllocateInfo, nullptr, outDeviceMemory);
	throwIfAllocateMemoryFailed(result);

	vkBindBufferMemory(device->getHandle(), *outBuffer, *outDeviceMemory, 0);
}

void Buffer::throwIfCreateBufferFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer.");
	}
}

void Buffer::throwIfAllocateMemoryFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory.");
	}

}