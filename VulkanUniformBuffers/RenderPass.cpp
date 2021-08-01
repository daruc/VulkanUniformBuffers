#include "RenderPass.h"
#include "SwapChain.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include <vulkan.h>
#include <memory>
#include <array>


RenderPass::RenderPass(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<Device> device,
	std::shared_ptr<SwapChain> swapChain)
{
	this->device = device;

	// color
	VkAttachmentDescription colorAttachment = buildColorAttachmentDescription(swapChain);
	VkAttachmentReference colorAttachmentRef = buildColorAttachmentReference();

	// depth
	VkAttachmentDescription depthAttachment = buildDepthAttachmentDescription(physicalDevice);
	VkAttachmentReference depthAttachmentRef = buildDepthAttachmentReference();
	VkSubpassDescription subpass = buildSubpassDescription(&colorAttachmentRef, &depthAttachmentRef);
	VkSubpassDependency dependency = buildSubpassDependency();

	std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassCreateInfo = buildRenderPassCreateInfo(&attachments, &subpass, &dependency);
	VkResult result = createRenderPass(device, &renderPassCreateInfo);
	throwIfCreationFailed(result);
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(device->getHandle(), vkRenderPass, nullptr);
}

VkAttachmentDescription RenderPass::buildColorAttachmentDescription(std::shared_ptr<SwapChain> swapChain)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChain->getSwapChainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return colorAttachment;
}

VkAttachmentReference RenderPass::buildColorAttachmentReference()
{
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return colorAttachmentRef;
}

VkAttachmentDescription RenderPass::buildDepthAttachmentDescription(std::shared_ptr<PhysicalDevice> physicalDevice)
{
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat(physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depthAttachment;
}

VkAttachmentReference RenderPass::buildDepthAttachmentReference()
{
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depthAttachmentRef;
}

VkSubpassDescription RenderPass::buildSubpassDescription(VkAttachmentReference* colorAttachmentRef,
	VkAttachmentReference* depthAttachmentRef)
{
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = colorAttachmentRef;
	subpass.pDepthStencilAttachment = depthAttachmentRef;

	return subpass;
}

VkSubpassDependency RenderPass::buildSubpassDependency()
{
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	return dependency;
}

VkRenderPassCreateInfo RenderPass::buildRenderPassCreateInfo(std::vector<VkAttachmentDescription>* attachments,
	VkSubpassDescription* subpass, VkSubpassDependency* dependency)
{
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments->size());
	createInfo.pAttachments = attachments->data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = dependency;

	return createInfo;
}

VkResult RenderPass::createRenderPass(std::shared_ptr<Device> device, VkRenderPassCreateInfo* renderPassCreateInfo)
{
	return vkCreateRenderPass(device->getHandle(), renderPassCreateInfo, nullptr, &vkRenderPass);
}

void RenderPass::throwIfCreationFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	}
}

VkFormat RenderPass::findDepthFormat(std::shared_ptr<PhysicalDevice> physicalDevice)
{
	std::vector<VkFormat> candidates;
	candidates.push_back(VK_FORMAT_D32_SFLOAT);
	candidates.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	return findSupportedFormat(physicalDevice, candidates, tiling, features);
}

VkFormat RenderPass::findSupportedFormat(std::shared_ptr<PhysicalDevice> physicalDevice, 
	const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice->getHandle(), format, &properties);

		if ((tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) ||
			(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features))
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format.");
}

VkRenderPass RenderPass::getHandle() const
{
	return vkRenderPass;
}