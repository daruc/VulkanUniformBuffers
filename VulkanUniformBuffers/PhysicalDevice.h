#pragma once

#include <vulkan.h>
#include <memory>
#include <vector>
#include "SwapChainSupportDetails.h"
#include "QueueFamilyIndices.h"


class VulkanInstance;
class VulkanSurface;


class PhysicalDevice
{
private:
	std::shared_ptr<VulkanInstance> vulkanInstance;
	std::shared_ptr<VulkanSurface> vulkanSurface;

	VkPhysicalDevice vkPhysicalDevice;
	std::vector<const char*> deviceExtensions;
	SwapChainSupportDetails swapChainSupportDetails;
	QueueFamilyIndices queueFamilyIndices;

	void buildDeviceExtensions();
	std::vector<VkPhysicalDevice> listAvailableDevices();
	void throwIfNotFoundDevices(unsigned int deviceCount) const;
	void findSuitableDevice(std::vector<VkPhysicalDevice>* availableDevices);
	bool isPhysicalDeviceSuitable(VkPhysicalDeviceProperties properties, VkPhysicalDevice availableDevice);
	bool isDiscreteGpu(VkPhysicalDeviceType deviceType) const;
	void throwIfNotFoundDiscreteGpu(VkPhysicalDevice vkPhysicalDevice) const;
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	bool checkSwapchainSupport(VkPhysicalDevice physicalDevice);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice) const;
	bool checkQueueFamiliesSupport(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice physicalDevice);
	void assignGraphicsOrPresentationIndex(VkQueueFamilyProperties queueFamily, VkPhysicalDevice physicalDevice, QueueFamilyIndices* queueFamilyIndices, unsigned int index);
	bool isGraphicsQueue(VkQueueFlags queueFlags) const;
	void assignGraphicsIndexIfNotSet(QueueFamilyIndices* queueFamilyIndices, unsigned int index);
	bool isGraphicsNotSet(QueueFamilyIndices queueFamilyIndices) const;
	bool isPresentationNotSet(QueueFamilyIndices queueFamilyIndices) const;
	void assignPresentationIndexIfSurfaceSupport(VkPhysicalDevice physicalDevice, QueueFamilyIndices* queueFamilyIndices, unsigned int index);
	bool isSurfaceSupport(VkPhysicalDevice physicalDevice, int index, VkSurfaceKHR surface) const;
	void throwQueueFamilyPropertiesNotFound() const;
	bool areQueueFamiliesSet(QueueFamilyIndices queueFamilyIndices) const;
	std::vector<VkQueueFamilyProperties> listQueueFamilyProperties(VkPhysicalDevice physicalDevice) const;

public:
	PhysicalDevice(std::shared_ptr<VulkanInstance> vulkanInstance, std::shared_ptr<VulkanSurface> vulkanSurface);
	std::vector<const char*>* getDeviceExtensions();
	SwapChainSupportDetails getSwapChainSupportDetails() const;
	QueueFamilyIndices getQueueFamilyIndices() const;
	VkPhysicalDevice getHandle() const;
};