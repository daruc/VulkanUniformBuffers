#pragma once

#include <vulkan.h>
#include <memory>

class Device;
class PhysicalDevice;
class CommandPool;


class Buffer
{
private:
	void throwIfCreateBufferFailed(VkResult result) const;
	void throwIfAllocateMemoryFailed(VkResult result) const;

protected:
	VkBuffer* outBuffer;
	VkDeviceMemory* outDeviceMemory;

	std::shared_ptr<Device> device;
	std::shared_ptr<PhysicalDevice> physicalDevice;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
		VkBuffer* outBuffer, VkDeviceMemory* outDeviceMemory);

	void copyBuffer(std::shared_ptr<CommandPool> commandPool, VkDeviceSize size, VkBuffer srcBuffer, VkBuffer dstBuffer);

public:
	Buffer(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device);
};