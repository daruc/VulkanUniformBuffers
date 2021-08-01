#include "SwapChain.h"
#include "SwapChainSupportDetails.h"
#include "QueueFamilyIndices.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "VulkanSurface.h"
#include <vulkan.h>
#include <memory>
#include <SDL_vulkan.h>
#include <SDL.h>
#include <glm/glm.hpp>


SwapChain::SwapChain(SDL_Window* sdlWindow, std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device, 
	std::shared_ptr<VulkanSurface> vulkanSurface)
{
	this->device = device;

	SwapChainSupportDetails supportDetails = physicalDevice->getSwapChainSupportDetails();
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(supportDetails.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(supportDetails.presentModes);
	VkExtent2D extent = chooseSwapExtent(sdlWindow, supportDetails.capabilities);

	uint32_t imageCount = calculateImageCount(&supportDetails);

	VkSwapchainCreateInfoKHR swapChainCreateInfo = createSwapChainCreateInfo(physicalDevice, vulkanSurface, imageCount,
		surfaceFormat, extent, supportDetails, presentMode);

	VkResult result = createSwapChain(device, &swapChainCreateInfo);
	throwIfCreationFailed(result);

	initSwapChainImages(device);

	vkSwapChainImageFormat = surfaceFormat.format;
	vkSwapChainExtent = extent;

	createSwapChainImageViews(device);
}

SwapChain::~SwapChain()
{
	for (VkImageView swapchainImageView : vkSwapChainImageViews)
	{
		vkDestroyImageView(device->getHandle(), swapchainImageView, nullptr);
	}

	vkDestroySwapchainKHR(device->getHandle(), vkSwapChain, nullptr);
}


uint32_t SwapChain::calculateImageCount(SwapChainSupportDetails* supportDetails) const
{
	uint32_t imageCount = supportDetails->capabilities.minImageCount + 1;

	if (supportDetails->capabilities.maxImageCount > 0 &&
		imageCount > supportDetails->capabilities.maxImageCount)
	{
		imageCount = supportDetails->capabilities.maxImageCount;
	}

	return imageCount;
}


VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	return formats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(SDL_Window* sdlWindow, const VkSurfaceCapabilitiesKHR& capabilities)
{
	VkExtent2D extent;

	if (capabilities.currentExtent.width == UINT32_MAX)
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(sdlWindow, &width, &height);
		height = clampHeight(height, &capabilities);
		width = clampWidth(width, &capabilities);
	}
	else
	{
		extent = capabilities.currentExtent;
	}

	return extent;
}

int SwapChain::clampHeight(int height, const VkSurfaceCapabilitiesKHR* capabilities)
{
	return glm::clamp(static_cast<uint32_t>(height), capabilities->minImageExtent.height,
		capabilities->maxImageExtent.height);
}

int SwapChain::clampWidth(int width, const VkSurfaceCapabilitiesKHR* capabilities)
{
	return glm::clamp(static_cast<uint32_t>(width), capabilities->minImageExtent.width, 
		capabilities->maxImageExtent.width);
}

VkSwapchainKHR SwapChain::getHandle() const
{
	return vkSwapChain;
}

void SwapChain::createSwapChainImageViews(std::shared_ptr<Device> device)
{
	vkSwapChainImageViews.resize(vkSwapChainImages.size());

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = vkSwapChainImageFormat;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	for (size_t i = 0; i < vkSwapChainImageViews.size(); ++i)
	{
		createInfo.image = vkSwapChainImages[i];

		VkResult result = createImageView(device, &createInfo, &vkSwapChainImageViews[i]);
		throwIfCreateImageViewFailed(result);
	}
}

VkResult SwapChain::createImageView(std::shared_ptr<Device> device, VkImageViewCreateInfo* createInfo,
	VkImageView* imageView)
{
	return vkCreateImageView(device->getHandle(), createInfo, nullptr, imageView);
}

void SwapChain::throwIfCreateImageViewFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain image view.");
	}
}

std::vector<VkImageView>* SwapChain::getSwapChainImageViews()
{
	return &vkSwapChainImageViews;
}

std::vector<VkImage>* SwapChain::initSwapChainImages()
{
	return &vkSwapChainImages;
}

VkExtent2D SwapChain::getSwapChainExtent()
{
	return vkSwapChainExtent;
}

VkFormat SwapChain::getSwapChainImageFormat()
{
	return vkSwapChainImageFormat;
}

VkSwapchainCreateInfoKHR SwapChain::createSwapChainCreateInfo(std::shared_ptr<PhysicalDevice> physicalDevice,
	std::shared_ptr<VulkanSurface> vulkanSurface, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat,
	VkExtent2D extent, SwapChainSupportDetails supportDetails, VkPresentModeKHR presentMode)
{
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = vulkanSurface->getHandle();
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices queueFamilyIndices = physicalDevice->getQueueFamilyIndices();
	std::vector<uint32_t> indices;
	indices.push_back(queueFamilyIndices.graphics.value());
	indices.push_back(queueFamilyIndices.presentation.value());

	if (queueFamilyIndices.graphics != queueFamilyIndices.presentation)
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
		swapChainCreateInfo.pQueueFamilyIndices = indices.data();
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapChainCreateInfo.preTransform = supportDetails.capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	return swapChainCreateInfo;
}

VkResult SwapChain::createSwapChain(std::shared_ptr<Device> device, VkSwapchainCreateInfoKHR* swapChainCreateInfo)
{
	return vkCreateSwapchainKHR(device->getHandle(), swapChainCreateInfo, nullptr, &vkSwapChain);
}

void SwapChain::throwIfCreationFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain.");
	}
}

void SwapChain::initSwapChainImages(std::shared_ptr<Device> device)
{
	uint32_t finalImageCount;
	vkGetSwapchainImagesKHR(device->getHandle(), vkSwapChain, &finalImageCount, nullptr);
	vkSwapChainImages.resize(finalImageCount);
	vkGetSwapchainImagesKHR(device->getHandle(), vkSwapChain, &finalImageCount, vkSwapChainImages.data());
}