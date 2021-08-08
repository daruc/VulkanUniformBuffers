#include "Depth.h"
#include "SwapChain.h"
#include "Device.h"
#include "PhysicalDevice.h"


Depth::Depth(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
	std::shared_ptr<SwapChain> swapChain)
{
	this->physicalDevice = physicalDevice;
	this->device = device;

	VkFormat format = findDepthFormat();
	VkImageCreateInfo imageInfo = buildImageCreateInfo(swapChain, format);
	VkResult result = createImage(&imageInfo);
	throwIfCreateImageFailed(result);

	VkMemoryAllocateInfo allocInfo = buildMemoryAllocateInfo();
	result = allocateMemory(&allocInfo);
	throwIfAllocateMemoryFailed(result);

	result = bindImageMemory();
	throwIfBindImageMemoryFailed(result);

	VkImageViewCreateInfo imageViewCreateInfo = buildImageViewCreateInfo(format);
	result = createImageView(&imageViewCreateInfo);
	throwIfCreateImageViewFailed(result);
}

Depth::~Depth()
{
	vkDestroyImageView(device->getHandle(), vkImageView, nullptr);
	vkDestroyImage(device->getHandle(), vkImage, nullptr);
	vkFreeMemory(device->getHandle(), vkMemory, nullptr);
}

VkImageCreateInfo Depth::buildImageCreateInfo(std::shared_ptr<SwapChain> swapChain, VkFormat format) const
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.format = format;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = swapChain->getSwapChainExtent().width;
	imageInfo.extent.height = swapChain->getSwapChainExtent().height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	return imageInfo;
}

VkResult Depth::createImage(VkImageCreateInfo* createInfo)
{
	return vkCreateImage(device->getHandle(), createInfo, nullptr, &vkImage);
}

void Depth::throwIfCreateImageFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image.");
	}
}

VkMemoryAllocateInfo Depth::buildMemoryAllocateInfo() const
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device->getHandle(), vkImage, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = physicalDevice->findMemoryType(memoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	return allocInfo;
}

VkResult Depth::allocateMemory(VkMemoryAllocateInfo* allocInfo)
{
	return vkAllocateMemory(device->getHandle(), allocInfo, nullptr, &vkMemory);
}

void Depth::throwIfAllocateMemoryFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate depth device memory.");
	}
}

VkResult Depth::bindImageMemory()
{
	return vkBindImageMemory(device->getHandle(), vkImage, vkMemory, 0);
}

void Depth::throwIfBindImageMemoryFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to bind depth image memory.");
	}
}

VkImageViewCreateInfo Depth::buildImageViewCreateInfo(VkFormat format) const
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = vkImage;
	createInfo.format = format;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = 1;

	return createInfo;
}

VkResult Depth::createImageView(VkImageViewCreateInfo* createInfo)
{
	return vkCreateImageView(device->getHandle(), createInfo, nullptr, &vkImageView);
}

VkResult Depth::throwIfCreateImageViewFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image view.");
	}
}

VkFormat Depth::findDepthFormat() const
{
	std::vector<VkFormat> candidates;
	candidates.push_back(VK_FORMAT_D32_SFLOAT);
	candidates.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	return physicalDevice->findSupportedFormat(candidates, tiling, features);
}

VkImageView Depth::getImageViewHandle() const
{
	return vkImageView;
}
