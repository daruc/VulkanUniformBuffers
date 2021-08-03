#include "DescriptorSetLayout.h"
#include <vulkan.h>
#include "Device.h"


DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device)
{
	this->device = device;

	VkDescriptorSetLayoutBinding bindings = buildBinding();
	VkDescriptorSetLayoutCreateInfo createInfo = buildCreateInfo(&bindings);

	VkResult result = vkCreateDescriptorSetLayout(device->getHandle(), &createInfo,
		nullptr, &vkDescriptorSetLayout);

	throwIfCreationFailed(result);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device->getHandle(), vkDescriptorSetLayout, nullptr);
}

VkDescriptorSetLayoutBinding DescriptorSetLayout::buildBinding()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	return uboLayoutBinding;
}

VkDescriptorSetLayoutCreateInfo DescriptorSetLayout::buildCreateInfo(VkDescriptorSetLayoutBinding* bindings)
{
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = bindings;

	return createInfo;
}

VkResult DescriptorSetLayout::createDescriptorSetLayout(std::shared_ptr<Device> device,
	VkDescriptorSetLayoutCreateInfo* createInfo)
{
	return vkCreateDescriptorSetLayout(device->getHandle(), createInfo, nullptr, &vkDescriptorSetLayout);
}

void DescriptorSetLayout::throwIfCreationFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

VkDescriptorSetLayout* DescriptorSetLayout::getHandlePtr()
{
	return &vkDescriptorSetLayout;
}

VkDescriptorSetLayout DescriptorSetLayout::getHandle() const
{
	return vkDescriptorSetLayout;
}