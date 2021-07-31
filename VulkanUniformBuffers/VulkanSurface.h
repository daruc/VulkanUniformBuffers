#pragma once

#include <memory>
#include "VulkanInstance.h"


class VulkanSurface
{
private:
	VkSurfaceKHR vkSurface;
	std::shared_ptr<VulkanInstance> vulkanInstance;

	SDL_bool buildSurface(SDL_Window* sdlWindow, VkInstance vkInstance);
	void throwIfCreationFailed(SDL_bool result) const;

public:
	VulkanSurface(SDL_Window* sdlWindow, std::shared_ptr<VulkanInstance> vulkanInstance);
	~VulkanSurface();
	VkSurfaceKHR getHandle() const;
};