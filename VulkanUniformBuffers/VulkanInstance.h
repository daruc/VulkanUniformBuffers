#pragma once

#include <vulkan.h>
#include <memory>
#include <vector>
#include "SDL.h"


class VulkanInstance
{
private:
	const char* APPLICATION_NAME = "Vulkan Init";
	const uint32_t APPLICATION_VERSION = VK_MAKE_VERSION(1, 0, 0);
	const char* ENGINE_NAME = "Engine Name";
	const uint32_t ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);
	const uint32_t API_VERSION = VK_API_VERSION_1_0;
	const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

	std::vector<const char*> extensions;
	std::vector<const char*> validationLayers;
	VkInstance vkInstance;

	VkApplicationInfo buildVkApplicationInfo() const;
	std::vector<const char*> buildExtensions(SDL_Window* sdlWindow, std::vector<const char*>* outExtensions) const;
	VkInstanceCreateInfo buildVkInstanceCreateInfo(SDL_Window* sdlWindow, VkApplicationInfo* vkApplicationInfo);
	void addValidationLayers(VkInstanceCreateInfo* outVkInstanceCreateInfo);
	void throwIfCreationFailed(VkResult result) const;

public:
	VulkanInstance(SDL_Window* sdlWindow);
	~VulkanInstance();
	VkInstance getHandle() const;
};