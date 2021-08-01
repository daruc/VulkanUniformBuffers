#pragma once

#include <memory>
#include <vector>
#include <vulkan.h>


class Device;
class SwapChain;
class PhysicalDevice;


class RenderPass
{
private:
	std::shared_ptr<Device> device;
	VkRenderPass vkRenderPass;

	VkFormat findDepthFormat(std::shared_ptr<PhysicalDevice> physicalDevice);

	VkFormat findSupportedFormat(std::shared_ptr<PhysicalDevice> physicalDevice, const std::vector<VkFormat>& candidates,
		VkImageTiling tiling, VkFormatFeatureFlags features);

	VkAttachmentDescription buildColorAttachmentDescription(std::shared_ptr<SwapChain> swapChain);
	VkAttachmentReference buildColorAttachmentReference();
	VkAttachmentDescription buildDepthAttachmentDescription(std::shared_ptr<PhysicalDevice> physicalDevice);
	VkAttachmentReference buildDepthAttachmentReference();

	VkSubpassDescription buildSubpassDescription(VkAttachmentReference* colorAttachmentRef, 
		VkAttachmentReference* depthAttachmentRef);

	VkSubpassDependency buildSubpassDependency();

	VkRenderPassCreateInfo buildRenderPassCreateInfo(std::vector<VkAttachmentDescription>* attachments,
		VkSubpassDescription* subpass, VkSubpassDependency* dependency);

	VkResult createRenderPass(std::shared_ptr<Device> device, VkRenderPassCreateInfo* renderPassCreateInfo);
	void throwIfCreationFailed(VkResult result);

public:
	RenderPass(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
		std::shared_ptr<SwapChain> swapChain);

	~RenderPass();

	VkRenderPass getHandle() const;
};