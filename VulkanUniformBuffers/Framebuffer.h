#pragma once

#include <memory>
#include <vector>
#include <vulkan.h>


class SwapChain;
class RenderPass;
class Device;


class Framebuffer
{
private:
	std::shared_ptr<Device> device;
	std::vector<VkFramebuffer> vkSwapchainFramebuffers;

	VkFramebufferCreateInfo buildCreateInfo(std::shared_ptr<SwapChain> swapChain, 
		std::shared_ptr<RenderPass> renderPass) const;

	std::array<VkImageView, 2> buildAttachments(std::shared_ptr<SwapChain> swapChain,
		VkImageView vkDepthImageView, size_t index) const;

	VkResult createFramebuffer(std::shared_ptr<Device> device, VkFramebufferCreateInfo* framebufferCreateInfo,
		size_t index);

	void throwIfCreationFailed(VkResult result) const;

public:
	Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<SwapChain> swapChain, 
		std::shared_ptr<RenderPass> renderPass, VkImageView vkDepthImageView);

	~Framebuffer();

	size_t getCount() const;
	VkFramebuffer getHandle(size_t index) const;
};