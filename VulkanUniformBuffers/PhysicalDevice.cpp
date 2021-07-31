#include "PhysicalDevice.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include <set>


PhysicalDevice::PhysicalDevice(std::shared_ptr<VulkanInstance> vulkanInstance, std::shared_ptr<VulkanSurface> vulkanSurface)
{
	this->vulkanInstance = vulkanInstance;
	this->vulkanSurface = vulkanSurface;

	buildDeviceExtensions();
	std::vector<VkPhysicalDevice> availableDevices = listAvailableDevices();
	findSuitableDevice(&availableDevices);
	throwIfNotFoundDiscreteGpu(vkPhysicalDevice);
}

void PhysicalDevice::buildDeviceExtensions()
{
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

std::vector<VkPhysicalDevice> PhysicalDevice::listAvailableDevices()
{
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanInstance->getHandle(), &deviceCount, nullptr);

	throwIfNotFoundDevices(deviceCount);

	std::vector<VkPhysicalDevice> availableDevices(deviceCount);
	vkEnumeratePhysicalDevices(vulkanInstance->getHandle(), &deviceCount, availableDevices.data());

	return availableDevices;
}

void PhysicalDevice::throwIfNotFoundDevices(unsigned int deviceCount) const
{
	if (deviceCount == 0)
	{
		throw std::runtime_error("None physical device is available.");
	}
}

void PhysicalDevice::findSuitableDevice(std::vector<VkPhysicalDevice>* availableDevices)
{
	vkPhysicalDevice = VK_NULL_HANDLE;
	for (VkPhysicalDevice availableDevice : *availableDevices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(availableDevice, &properties);

		if (isPhysicalDeviceSuitable(properties, availableDevice))
		{
			vkPhysicalDevice = availableDevice;
			break;
		}
	}
}

bool PhysicalDevice::isPhysicalDeviceSuitable(VkPhysicalDeviceProperties properties, VkPhysicalDevice availableDevice)
{
	return isDiscreteGpu(properties.deviceType) &&
		checkDeviceExtensionSupport(availableDevice) &&
		checkSwapchainSupport(availableDevice) &&
		checkQueueFamiliesSupport(availableDevice);
}

bool PhysicalDevice::isDiscreteGpu(VkPhysicalDeviceType deviceType) const
{
	return deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

void PhysicalDevice::throwIfNotFoundDiscreteGpu(VkPhysicalDevice vkPhysicalDevice) const
{
	if (vkPhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("None discrete GPU is available.");
	}
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
	uint32_t availableExtensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

	std::set<std::string> unavailableExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (VkExtensionProperties& available : availableExtensions)
	{
		unavailableExtensions.erase(available.extensionName);
	}

	return unavailableExtensions.empty();
}

bool PhysicalDevice::checkSwapchainSupport(VkPhysicalDevice physicalDevice) 
{
	swapChainSupportDetails = querySwapChainSupport(physicalDevice);
	return !swapChainSupportDetails.presentModes.empty() &&
		!swapChainSupportDetails.formats.empty();
}

SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice physicalDevice) const
{
	SwapChainSupportDetails supportDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkanSurface->getHandle(), &supportDetails.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanSurface->getHandle(), &formatCount, nullptr);

	if (formatCount > 0)
	{
		supportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanSurface->getHandle(), &formatCount, supportDetails.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanSurface->getHandle(), &presentModeCount, nullptr);

	if (presentModeCount > 0)
	{
		supportDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, vulkanSurface->getHandle(), &presentModeCount, supportDetails.presentModes.data());
	}

	return supportDetails;
}

bool PhysicalDevice::checkQueueFamiliesSupport(VkPhysicalDevice physicalDevice)
{
	queueFamilyIndices = findQueueFamilyIndices(physicalDevice);
	return queueFamilyIndices.graphics.has_value() && queueFamilyIndices.presentation.has_value();
}

QueueFamilyIndices PhysicalDevice::findQueueFamilyIndices(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices queueFamilyIndices;
	std::vector<VkQueueFamilyProperties> queueFamilies = listQueueFamilyProperties(physicalDevice);

	unsigned int index = 0;
	for (VkQueueFamilyProperties queueFamily : queueFamilies)
	{
		assignGraphicsOrPresentationIndex(queueFamily, physicalDevice, &queueFamilyIndices, index);

		if (areQueueFamiliesSet(queueFamilyIndices))
		{
			return queueFamilyIndices;
		}

		++index;
	}

	throwQueueFamilyPropertiesNotFound();
}


void PhysicalDevice::assignGraphicsOrPresentationIndex(VkQueueFamilyProperties queueFamily, VkPhysicalDevice physicalDevice, QueueFamilyIndices* queueFamilyIndices, unsigned int index)
{
	if (isGraphicsQueue(queueFamily.queueFlags))
	{
		assignGraphicsIndexIfNotSet(queueFamilyIndices, index);
	}
	else if (isPresentationNotSet(*queueFamilyIndices))
	{
		assignPresentationIndexIfSurfaceSupport(physicalDevice, queueFamilyIndices, index);
	}
}

bool PhysicalDevice::isGraphicsQueue(VkQueueFlags queueFlags) const
{
	return queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

void PhysicalDevice::assignGraphicsIndexIfNotSet(QueueFamilyIndices* queueFamilyIndices, unsigned int index)
{
	if (isGraphicsNotSet(*queueFamilyIndices))
	{
		queueFamilyIndices->graphics = index;
	}
}

bool PhysicalDevice::isGraphicsNotSet(QueueFamilyIndices queueFamilyIndices) const
{
	return !queueFamilyIndices.graphics.has_value();
}

bool PhysicalDevice::isPresentationNotSet(QueueFamilyIndices queueFamilyIndices) const
{
	return !queueFamilyIndices.presentation.has_value();
}

void PhysicalDevice::assignPresentationIndexIfSurfaceSupport(VkPhysicalDevice physicalDevice, QueueFamilyIndices* queueFamilyIndices, unsigned int index)
{
	if (isSurfaceSupport(physicalDevice, index, vulkanSurface->getHandle()))
	{
		queueFamilyIndices->presentation = index;
	}
}

bool PhysicalDevice::isSurfaceSupport(VkPhysicalDevice physicalDevice, int index, VkSurfaceKHR surface) const
{
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentSupport);
	
	return presentSupport;
}

bool PhysicalDevice::areQueueFamiliesSet(QueueFamilyIndices queueFamilyIndices) const
{
	return queueFamilyIndices.graphics.has_value() && queueFamilyIndices.presentation.has_value();
}

void PhysicalDevice::throwQueueFamilyPropertiesNotFound() const
{
	throw std::runtime_error("Graphics with presentation queue family not found.");
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::listQueueFamilyProperties(VkPhysicalDevice physicalDevice) const
{
	unsigned int familyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, queueFamilies.data());

	return queueFamilies;
}

std::vector<const char*>* PhysicalDevice::getDeviceExtensions()
{
	return &deviceExtensions;
}

SwapChainSupportDetails PhysicalDevice::getSwapChainSupportDetails() const
{
	return swapChainSupportDetails;
}

QueueFamilyIndices PhysicalDevice::getQueueFamilyIndices() const
{
	return queueFamilyIndices;
}

VkPhysicalDevice PhysicalDevice::getHandle() const
{
	return vkPhysicalDevice;
}