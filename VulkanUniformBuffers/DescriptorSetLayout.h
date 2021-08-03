#pragma once

#include <memory>
#include <vulkan.h>


class Device;


class DescriptorSetLayout
{
private:
	std::shared_ptr<Device> device;
	VkDescriptorSetLayout vkDescriptorSetLayout;

	VkDescriptorSetLayoutBinding buildBinding();
	VkDescriptorSetLayoutCreateInfo buildCreateInfo(VkDescriptorSetLayoutBinding* bindings);

	VkResult createDescriptorSetLayout(std::shared_ptr<Device> device,
		VkDescriptorSetLayoutCreateInfo* createInfo);

	void throwIfCreationFailed(VkResult result);

public:
	DescriptorSetLayout(std::shared_ptr<Device> device);
	~DescriptorSetLayout();
	VkDescriptorSetLayout* getHandlePtr();
	VkDescriptorSetLayout getHandle() const;
};