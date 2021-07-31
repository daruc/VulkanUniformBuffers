#include "VulkanSurface.h"
#include "SDL_vulkan.h"


VulkanSurface::VulkanSurface(SDL_Window* sdlWindow, std::shared_ptr<VulkanInstance> vulkanInstance)
{
	this->vulkanInstance = vulkanInstance;
	SDL_bool result = buildSurface(sdlWindow, vulkanInstance->getHandle());
	throwIfCreationFailed(result);
}

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(vulkanInstance->getHandle(), vkSurface, nullptr);
}

SDL_bool VulkanSurface::buildSurface(SDL_Window* sdlWindow, VkInstance vkInstance)
{
	return SDL_Vulkan_CreateSurface(sdlWindow, vkInstance, &vkSurface);
}

void VulkanSurface::throwIfCreationFailed(SDL_bool result) const
{
	if (result == SDL_FALSE)
	{
		throw std::runtime_error("Failed to create VkSurfaceKHR.");
	}
}

VkSurfaceKHR VulkanSurface::getHandle() const
{
	return vkSurface;
}