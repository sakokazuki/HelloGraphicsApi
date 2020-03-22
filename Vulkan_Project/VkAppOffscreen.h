#pragma once

#include "VkAppBase.h"
#include "glm/glm.hpp"

class VulkanAppOffscreen : public VulkanAppBase
{
public:
	VulkanAppOffscreen() : VulkanAppBase() {}

	virtual void prepare() override;
	virtual void cleanup() override;

	virtual void makeCommand(VkCommandBuffer command) override;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};
private:
	struct BufferObject
	{
		VkBuffer buffer;
		VkDeviceMemory  memory;
	};

	struct TextureObject
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	struct ShaderParameters
	{
		glm::mat4 mtxWorld;
		glm::mat4 mtxView;
		glm::mat4 mtxProj;
	};

	BufferObject createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VkPipelineShaderStageCreateInfo loadShaderModule(const char* fileName, VkShaderStageFlagBits stage);
	
	void prepareUniformBuffers();
	void prepareOffscreenTexture();
	void prepareDescriptorSetLayout();
	void prepareDescriptorPool();
	void prepareDescriptorSet();
	void createOffscreenRenderPass();

	TextureObject m_texture;
	VkSampler m_sampler;

	BufferObject m_vertexBuffer;
	BufferObject m_indexBuffer;
	BufferObject m_cubeVertexBuffer;
	BufferObject m_cubeIndexBuffer;
	uint32_t m_cubeIndexCount;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline   m_pipeline;
	uint32_t m_indexCount;

	VkPipelineLayout m_cubePipelineLayout;
	VkPipeline m_cubePipeline;

	VkRenderPass m_offscreenRenderPass;

	std::vector<BufferObject> m_uniformBuffers;
	VkFramebuffer m_offscreenFrameBuffer;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool  m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSet;
};
