#include "VulkanInstance.h"
#include "SDL_vulkan.h"
#include <vector>


VulkanInstance::VulkanInstance(SDL_Window* sdlWindow)
{
	VkApplicationInfo vkApplicationInfo = buildVkApplicationInfo();
	VkInstanceCreateInfo vkInstanceCreateInfo = buildVkInstanceCreateInfo(sdlWindow, &vkApplicationInfo);
	VkResult result = vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance);
	throwIfCreationFailed(result);
}

VulkanInstance::~VulkanInstance()
{
	vkDestroyInstance(vkInstance, nullptr);
}

VkApplicationInfo VulkanInstance::buildVkApplicationInfo() const
{
	VkApplicationInfo result = {};
	result.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	result.pApplicationName = APPLICATION_NAME;
	result.applicationVersion = APPLICATION_VERSION;
	result.pEngineName = ENGINE_NAME;
	result.engineVersion = ENGINE_VERSION;
	result.apiVersion = API_VERSION;

	return result;
}

VkInstanceCreateInfo VulkanInstance::buildVkInstanceCreateInfo(SDL_Window* sdlWindow, VkApplicationInfo* vkApplicationInfo)
{
	buildExtensions(sdlWindow, &extensions);

	VkInstanceCreateInfo vkInstanceCreateInfo = {};
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pApplicationInfo = vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	vkInstanceCreateInfo.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
	addValidationLayers(&vkInstanceCreateInfo);
#endif

	return vkInstanceCreateInfo;
}

void VulkanInstance::addValidationLayers(VkInstanceCreateInfo* outVkInstanceCreateInfo)
{
	validationLayers.push_back(VALIDATION_LAYER_NAME);

	outVkInstanceCreateInfo->enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	outVkInstanceCreateInfo->ppEnabledLayerNames = validationLayers.data();
}

std::vector<const char*> VulkanInstance::buildExtensions(SDL_Window* sdlWindow, std::vector<const char*>* outExtensions) const
{
	unsigned int extensionCount;
	SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, nullptr);
	outExtensions->resize(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(sdlWindow, &extensionCount, outExtensions->data());
	return extensions;
}

void VulkanInstance::throwIfCreationFailed(VkResult result) const
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create VkInstance.");
	}
}

VkInstance VulkanInstance::getHandle() const
{
	return vkInstance;
}