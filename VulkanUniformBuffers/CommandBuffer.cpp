#include "CommandBuffer.h"
#include <array>
#include "Framebuffer.h"
#include "Device.h"
#include "CommandPool.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"


CommandBuffer::CommandBuffer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> renderPass, 
	std::shared_ptr<Framebuffer> frameBuffer, std::shared_ptr<CommandPool> commandPool,
	std::shared_ptr<SwapChain> swapChain, std::shared_ptr<GraphicsPipeline> graphicsPipeline,
	std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer,
	std::shared_ptr<UniformBuffer> uniformBuffer)
{
	vkCommandBuffers.resize(frameBuffer->getCount());

	VkCommandBufferAllocateInfo commandBufferInfo = buildCommandBufferAllocateInfo(commandPool);

	VkResult result = vkAllocateCommandBuffers(device->getHandle(), &commandBufferInfo, vkCommandBuffers.data());
	throwAllocateCommandBufferFailed(result);

	for (size_t i = 0; i < vkCommandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		result = vkBeginCommandBuffer(vkCommandBuffers[i], &beginInfo);
		throwBeginCommandBufferFailed(result);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass->getHandle();
		renderPassInfo.framebuffer = frameBuffer->getHandle(i);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues;
		clearValues[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1] = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getHandle());

		VkBuffer buffers[] = { vertexBuffer->getHandle() };
		VkDeviceSize bufferOffsets[] = { 0 };
		vkCmdBindVertexBuffers(vkCommandBuffers[i], 0, 1, buffers, bufferOffsets);
		vkCmdBindIndexBuffer(vkCommandBuffers[i], indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getLayoutHandle(),
			0, 1, uniformBuffer->getDescriptorSetHandlePtr(i), 0, nullptr);

		vkCmdDrawIndexed(vkCommandBuffers[i], static_cast<uint32_t>(indexBuffer->getIndicesCount()), 1, 0, 0, 0);
		vkCmdEndRenderPass(vkCommandBuffers[i]);

		result = vkEndCommandBuffer(vkCommandBuffers[i]);
		throwEndCommandBufferFailed(result);
	}
}

VkCommandBufferAllocateInfo CommandBuffer::buildCommandBufferAllocateInfo(std::shared_ptr<CommandPool> commandPool)
{
	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = commandPool->getHandle();
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = static_cast<uint32_t>(vkCommandBuffers.size());

	return commandBufferInfo;
}


void CommandBuffer::throwEndCommandBufferFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer.");
	}
}

void CommandBuffer::throwBeginCommandBufferFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin command buffer.");
	}
}

void CommandBuffer::throwAllocateCommandBufferFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}
}

VkCommandBuffer* CommandBuffer::getHandlePtr(uint32_t index)
{
	return &vkCommandBuffers[index];
}