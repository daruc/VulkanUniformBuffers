#pragma once

#include <vulkan.h>
#include "PhysicalDevice.h"


class PhysicalDevice;


class Device
{
private:
	const float queuePriority = 1.0f;
	const VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDevice vkDevice;
	VkQueue vkGraphicsQueue;
	VkQueue vkPresentationQueue;

	void buildGraphicsQueueCreateInfo(QueueFamilyIndices* queueFamilyIndices, VkDeviceQueueCreateInfo* outQueueCreateInfo);
	void buildPresentationQueueCreateInfo(QueueFamilyIndices* queueFamilyIndices, VkDeviceQueueCreateInfo* outQueueCreateInfo);

	VkDeviceCreateInfo buildDeviceCreateInfo(std::shared_ptr<PhysicalDevice> physicalDevice,
		std::vector<VkDeviceQueueCreateInfo>* queueCreateInfos);

	VkResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo* createInfo);
	void throwIfCreationFailed(VkResult result);
	void initGraphicsQueueHandle(QueueFamilyIndices* queueFamilyIndices);
	void initPresentationQueueHandle(QueueFamilyIndices* queueFamilyIndices);

public:
	Device(std::shared_ptr<PhysicalDevice> physicalDevice);
	~Device();
	VkDevice getHandle() const;
	VkQueue getGraphicsQueueHandle() const;
	VkQueue getPresentationQueueHandle() const;
};