#include <vector>
#include <memory>
#include "Device.h"
#include "QueueFamilyIndices.h"


Device::Device(std::shared_ptr<PhysicalDevice> physicalDevice)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(2);
	QueueFamilyIndices queueFamilyIndices = physicalDevice->getQueueFamilyIndices();
	buildGraphicsQueueCreateInfo(&queueFamilyIndices, &queueCreateInfos[0]);
	buildPresentationQueueCreateInfo(&queueFamilyIndices, &queueCreateInfos[1]);
	VkDeviceCreateInfo createInfo = buildDeviceCreateInfo(physicalDevice, &queueCreateInfos);

	VkResult result = createDevice(physicalDevice, &createInfo);
	throwIfCreationFailed(result);

	initGraphicsQueueHandle(&queueFamilyIndices);
	initPresentationQueueHandle(&queueFamilyIndices);
}

Device::~Device()
{
	vkDestroyDevice(vkDevice, nullptr);
}

void Device::buildGraphicsQueueCreateInfo(QueueFamilyIndices* queueFamilyIndices, VkDeviceQueueCreateInfo* outQueueCreateInfo)
{
	*outQueueCreateInfo = {};
	outQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	outQueueCreateInfo->queueFamilyIndex = *queueFamilyIndices->graphics;
	outQueueCreateInfo->queueCount = 1;
	outQueueCreateInfo->pQueuePriorities = &queuePriority;
}

void Device::buildPresentationQueueCreateInfo(QueueFamilyIndices* queueFamilyIndices, VkDeviceQueueCreateInfo* outQueueCreateInfo)
{
	*outQueueCreateInfo = {};
	outQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	outQueueCreateInfo->queueFamilyIndex = *queueFamilyIndices->presentation;
	outQueueCreateInfo->queueCount = 1;
	outQueueCreateInfo->pQueuePriorities = &queuePriority;
}

VkDeviceCreateInfo Device::buildDeviceCreateInfo(std::shared_ptr<PhysicalDevice> physicalDevice,
	std::vector<VkDeviceQueueCreateInfo>* queueCreateInfos)
{
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos->data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos->size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDevice->getDeviceExtensions()->size());
	createInfo.ppEnabledExtensionNames = physicalDevice->getDeviceExtensions()->data();

	return createInfo;
}

VkResult Device::createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo* createInfo)
{
	return vkCreateDevice(physicalDevice->getHandle(), createInfo, nullptr, &vkDevice);
}

void Device::throwIfCreationFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create device.");
	}
}

void Device::initGraphicsQueueHandle(QueueFamilyIndices* queueFamilyIndices)
{
	vkGetDeviceQueue(vkDevice, *queueFamilyIndices->graphics, 0, &vkGraphicsQueue);
}

void Device::initPresentationQueueHandle(QueueFamilyIndices* queueFamilyIndices)
{
	vkGetDeviceQueue(vkDevice, *queueFamilyIndices->presentation, 0, &vkPresentationQueue);
}

VkDevice Device::getHandle() const
{
	return vkDevice;
}

VkQueue Device::getGraphicsQueueHandle() const
{
	return vkGraphicsQueue;
}

VkQueue Device::getPresentationQueueHandle() const
{
	return vkPresentationQueue;
}