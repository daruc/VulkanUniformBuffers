#include "CommandPool.h"
#include <memory>
#include "PhysicalDevice.h"
#include "Device.h"


CommandPool::CommandPool(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device)
{
	this->device = device;

	VkCommandPoolCreateInfo commandPoolCreateInfo = buildCommandPoolCreateInfo(physicalDevice);
	VkResult result = createCommandPool(device, &commandPoolCreateInfo);
	throwIfCreationFailed(result);
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(device->getHandle(), vkCommandPool, nullptr);
}

VkCommandPoolCreateInfo CommandPool::buildCommandPoolCreateInfo(std::shared_ptr<PhysicalDevice> physicalDevice)
{
	QueueFamilyIndices queueFamilyIndices = physicalDevice->getQueueFamilyIndices();

	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndices.graphics.value();

	return createInfo;
}

VkResult CommandPool::createCommandPool(std::shared_ptr<Device> device, VkCommandPoolCreateInfo* createInfo)
{
	return vkCreateCommandPool(device->getHandle(), createInfo, nullptr, &vkCommandPool);;
}

void CommandPool::throwIfCreationFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool.");
	}
}

VkCommandPool CommandPool::getHandle() const
{
	return vkCommandPool;
}