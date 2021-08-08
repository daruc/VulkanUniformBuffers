#include "Framebuffer.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Device.h"
#include <array>


Framebuffer::Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<SwapChain> swapChain,
	std::shared_ptr<RenderPass> renderPass, VkImageView vkDepthImageView)
{
	this->device = device;

	vkSwapchainFramebuffers.resize(swapChain->getSwapChainImageViews()->size());
	VkFramebufferCreateInfo framebufferCreateInfo = buildCreateInfo(swapChain, renderPass);

	for (size_t i = 0; i < swapChain->getSwapChainImageViews()->size(); ++i)
	{
		std::array<VkImageView, 2> attachments = buildAttachments(swapChain, vkDepthImageView, i);
		framebufferCreateInfo.pAttachments = attachments.data();

		VkResult result = createFramebuffer(device, &framebufferCreateInfo, i);
		throwIfCreationFailed(result);
	}
}

Framebuffer::~Framebuffer()
{
	for (VkFramebuffer framebuffer : vkSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(device->getHandle(), framebuffer, nullptr);
	}
}

VkFramebufferCreateInfo Framebuffer::buildCreateInfo(std::shared_ptr<SwapChain> swapChain,
	std::shared_ptr<RenderPass> renderPass) const
{
	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass->getHandle();
	createInfo.attachmentCount = 2;
	createInfo.width = swapChain->getSwapChainExtent().width;
	createInfo.height = swapChain->getSwapChainExtent().height;
	createInfo.layers = 1;

	return createInfo;
}

std::array<VkImageView, 2> Framebuffer::buildAttachments(std::shared_ptr<SwapChain> swapChain,
	VkImageView vkDepthImageView, size_t index) const
{
	return {
		swapChain->getSwapChainImageViews()->at(index),
		vkDepthImageView
	};
}

VkResult Framebuffer::createFramebuffer(std::shared_ptr<Device> device, VkFramebufferCreateInfo* framebufferCreateInfo,
	size_t index)
{
	return vkCreateFramebuffer(device->getHandle(), framebufferCreateInfo, nullptr, &vkSwapchainFramebuffers[index]);
}

void Framebuffer::throwIfCreationFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain frame buffer.");
	}
}

size_t Framebuffer::getCount() const
{
	return vkSwapchainFramebuffers.size();
}

VkFramebuffer Framebuffer::getHandle(size_t index) const
{
	return vkSwapchainFramebuffers[index];
}
