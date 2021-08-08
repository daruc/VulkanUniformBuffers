#pragma once

#include <vulkan.h>
#include <memory>
#include <vector>


class PhysicalDevice;
class Device;
class SwapChain;


class Depth
{
private:
	std::shared_ptr<PhysicalDevice> physicalDevice;
	std::shared_ptr<Device> device;
	VkImage vkImage;
	VkImageView vkImageView;
	VkDeviceMemory vkMemory;

	VkImageCreateInfo buildImageCreateInfo(std::shared_ptr<SwapChain> swapChain, VkFormat format) const;
	VkFormat findDepthFormat() const;
	VkResult createImage(VkImageCreateInfo* createInfo);
	void throwIfCreateImageFailed(VkResult result) const;
	VkMemoryAllocateInfo buildMemoryAllocateInfo() const;
	VkResult allocateMemory(VkMemoryAllocateInfo* allocInfo);
	void throwIfAllocateMemoryFailed(VkResult result) const;
	VkResult bindImageMemory();
	void throwIfBindImageMemoryFailed(VkResult result) const;
	VkImageViewCreateInfo buildImageViewCreateInfo(VkFormat format) const;
	VkResult createImageView(VkImageViewCreateInfo* createInfo);
	VkResult throwIfCreateImageViewFailed(VkResult result) const;

public:
	Depth(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
		std::shared_ptr<SwapChain> swapChain);
	~Depth();

	VkImageView getImageViewHandle() const;
};