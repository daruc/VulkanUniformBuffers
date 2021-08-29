#pragma once

#include <vulkan.h>
#include <vector>
#include <memory>

class Device;
class RenderPass;
class Framebuffer;
class SwapChain;
class CommandPool;
class VertexBuffer;
class GraphicsPipeline;
class IndexBuffer;
class UniformBuffer;


class CommandBuffer
{
private:
	std::vector<VkCommandBuffer> vkCommandBuffers;

	void throwEndCommandBufferFailed(VkResult result);
	void throwBeginCommandBufferFailed(VkResult result);
	void throwAllocateCommandBufferFailed(VkResult result);

	VkCommandBufferAllocateInfo buildCommandBufferAllocateInfo(std::shared_ptr<CommandPool> commandPool);

public:
	CommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass,
		std::shared_ptr<Framebuffer> frameBuffer, std::shared_ptr<CommandPool> commandPool,
		std::shared_ptr<SwapChain> swapChain, std::shared_ptr<GraphicsPipeline> graphicsPipeline,
		std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer,
		std::shared_ptr<UniformBuffer> uniformBuffer);

	VkCommandBuffer* getHandlePtr(uint32_t index);
};