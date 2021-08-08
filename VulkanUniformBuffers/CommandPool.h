#pragma once

#include <memory>
#include <vulkan.h>


class PhysicalDevice;
class Device;


class CommandPool
{
private:
	std::shared_ptr<Device> device;
	VkCommandPool vkCommandPool;

	VkCommandPoolCreateInfo buildCommandPoolCreateInfo(std::shared_ptr<PhysicalDevice> physicalDevice);
	VkResult createCommandPool(std::shared_ptr<Device> device, VkCommandPoolCreateInfo* createInfo);
	void throwIfCreationFailed(VkResult result);

public:
	CommandPool(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device);
	~CommandPool();

	VkCommandPool getHandle() const;
};