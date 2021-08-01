#pragma once

#include <memory>
#include <vector>
#include <vulkan.h>


class PhysicalDevice;
class Device;
class VulkanSurface;
struct SDL_Window;
struct SwapChainSupportDetails;


class SwapChain
{
private:
	std::shared_ptr<Device> device;

	VkSwapchainKHR vkSwapChain;
	std::vector<VkImage> vkSwapChainImages;
	std::vector<VkImageView> vkSwapChainImageViews;
	VkFormat vkSwapChainImageFormat;
	VkExtent2D vkSwapChainExtent;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
	VkExtent2D chooseSwapExtent(SDL_Window* sdlWindow, const VkSurfaceCapabilitiesKHR& capabilities);
	int clampHeight(int height, const VkSurfaceCapabilitiesKHR* capabilities);
	int clampWidth(int width, const VkSurfaceCapabilitiesKHR* capabilities);
	void createSwapChainImageViews(std::shared_ptr<Device> device);
	VkResult createImageView(std::shared_ptr<Device> device, VkImageViewCreateInfo* createInfo,
		VkImageView* imageView);
	void throwIfCreateImageViewFailed(VkResult result);
	uint32_t calculateImageCount(SwapChainSupportDetails* supportDetails) const;

	VkSwapchainCreateInfoKHR createSwapChainCreateInfo(std::shared_ptr<PhysicalDevice> physicalDevice,
		std::shared_ptr<VulkanSurface> vulkanSurface, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat, VkExtent2D extent,
		SwapChainSupportDetails supportDetails, VkPresentModeKHR presentMode);

	VkResult createSwapChain(std::shared_ptr<Device> device, VkSwapchainCreateInfoKHR* swapChainCreateInfo);
	void throwIfCreationFailed(VkResult result) const;
	void initSwapChainImages(std::shared_ptr<Device> device);

public:
	SwapChain(SDL_Window* sdlWindow, std::shared_ptr<PhysicalDevice> physiclaDevice, std::shared_ptr<Device> device,
		std::shared_ptr<VulkanSurface> vulkanSurface);

	~SwapChain();

	VkSwapchainKHR getHandle() const;
	std::vector<VkImageView>* getSwapChainImageViews();
	std::vector<VkImage>* initSwapChainImages();
	VkExtent2D getSwapChainExtent();
	VkFormat getSwapChainImageFormat();
};