#include "VkAppOffscreen.h"

#include <fstream>
#include <array>
#include <glm/gtc/matrix_transform.hpp>


using namespace glm;
using namespace std;

void VulkanAppOffscreen::prepare()
{
	// キューブ用のレンダーパス作成
	createOffscreenRenderPass();

	// リソースの確保など
	prepareUniformBuffers();
	prepareOffscreenTexture();

	// 確保したリソースを使ってディスクリプタを作成
	prepareDescriptorSetLayout();
	prepareDescriptorPool();
	prepareDescriptorSet();

	// 共通のパイプラインステート
	// ブレンディングの設定
	const auto colorWriteAll = \
		VK_COLOR_COMPONENT_R_BIT | \
		VK_COLOR_COMPONENT_G_BIT | \
		VK_COLOR_COMPONENT_B_BIT | \
		VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = VK_TRUE;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.colorWriteMask = colorWriteAll;
	VkPipelineColorBlendStateCreateInfo cbCI{};
	cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cbCI.attachmentCount = 1;
	cbCI.pAttachments = &blendAttachment;

	// ビューポートの設定
	VkViewport viewport;
	{
		viewport.x = 0.0f;
		viewport.y = float(m_swapchainExtent.height);
		viewport.width = float(m_swapchainExtent.width);
		viewport.height = -1.0f * float(m_swapchainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
	}
	VkRect2D scissor = {
	  { 0,0},// offset
	  m_swapchainExtent
	};
	VkPipelineViewportStateCreateInfo viewportCI{};
	viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCI.viewportCount = 1;
	viewportCI.pViewports = &viewport;
	viewportCI.scissorCount = 1;
	viewportCI.pScissors = &scissor;

	// プリミティブトポロジー設定
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
	inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


	// ラスタライザーステート設定
	VkPipelineRasterizationStateCreateInfo rasterizerCI{};
	rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCI.cullMode = VK_CULL_MODE_NONE;
	rasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCI.lineWidth = 1.0f;

	// マルチサンプル設定
	VkPipelineMultisampleStateCreateInfo multisampleCI{};
	multisampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// デプスステンシルステート設定
	VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
	depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCI.depthTestEnable = VK_TRUE;
	depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilCI.depthWriteEnable = VK_TRUE;
	depthStencilCI.stencilTestEnable = VK_FALSE;
	// 三角形のパイプラインステートを作成
	{
		const vec3 red(1.0f, 0.0f, 0.0f);
		const vec3 green(0.0f, 1.0f, 0.0f);
		const vec3 blue(0.0f, 0.0f, 1.0f);
		Vertex vertices[] = {
			{ vec3(-1.0f, 0.0f, 0.0f), red },
			{ vec3(+1.0f, 0.0f, 0.0f), blue },
			{ vec3(0.0f, 1.0f, 0.0f), green },
		};
		uint32_t indices[] = { 0, 1, 2 };

		m_vertexBuffer = createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_indexBuffer = createBuffer(sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		// 頂点データの書き込み
		{
			void* p;
			vkMapMemory(m_device, m_vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, vertices, sizeof(vertices));
			vkUnmapMemory(m_device, m_vertexBuffer.memory);
		}
		// インデックスデータの書き込み
		{
			void* p;
			vkMapMemory(m_device, m_indexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, indices, sizeof(indices));
			vkUnmapMemory(m_device, m_indexBuffer.memory);
		}
		m_indexCount = _countof(indices);

		// 頂点の入力設定
		VkVertexInputBindingDescription inputBinding{
		  0,                          // binding
		  sizeof(Vertex),          // stride
		  VK_VERTEX_INPUT_RATE_VERTEX // inputRate
		};
		array<VkVertexInputAttributeDescription, 2> inputAttribs{
		  {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
		  }
		};
		VkPipelineVertexInputStateCreateInfo vertexInputCI{};
		vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCI.vertexBindingDescriptionCount = 1;
		vertexInputCI.pVertexBindingDescriptions = &inputBinding;
		vertexInputCI.vertexAttributeDescriptionCount = uint32_t(inputAttribs.size());
		vertexInputCI.pVertexAttributeDescriptions = inputAttribs.data();


		// シェーダーバイナリの読み込み
		vector<VkPipelineShaderStageCreateInfo> shaderStages
		{
		  loadShaderModule("triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		  loadShaderModule("triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		// パイプラインレイアウト
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout);

		// パイプラインの構築
		VkGraphicsPipelineCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		ci.stageCount = uint32_t(shaderStages.size());
		ci.pStages = shaderStages.data();
		ci.pInputAssemblyState = &inputAssemblyCI;
		ci.pVertexInputState = &vertexInputCI;
		ci.pRasterizationState = &rasterizerCI;
		ci.pDepthStencilState = &depthStencilCI;
		ci.pMultisampleState = &multisampleCI;
		ci.pViewportState = &viewportCI;
		ci.pColorBlendState = &cbCI;
		ci.renderPass = m_offscreenRenderPass;
		ci.layout = m_pipelineLayout;
		vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_pipeline);

		// ShaderModule はもう不要のため破棄
		for (const auto& v : shaderStages)
		{
			vkDestroyShaderModule(m_device, v.module, nullptr);
		}
	}

	// キューブのパイプラインステートを作成
	{
		const float k = 1.0f;
		const vec3 red(1.0f, 0.0f, 0.0f);
		const vec3 green(0.0f, 1.0f, 0.0f);
		const vec3 blue(0.0f, 0.0f, 1.0f);
		const vec3 white(1.0f);
		const vec3 black(0.0f);
		const vec3 yellow(1.0f, 1.0f, 0.0f);
		const vec3 magenta(1.0f, 0.0f, 1.0f);
		const vec3 cyan(0.0f, 1.0f, 1.0f);

		const vec2 lb(0.0f, 0.0f);
		const vec2 lt(0.0f, 1.0f);
		const vec2 rb(1.0f, 0.0f);
		const vec2 rt(1.0f, 1.0f);
		Vertex vertices[] = {
			// front
			// 正面.
			{ vec3(-k, k, k), yellow,  lb },
			{ vec3(-k,-k, k), red,     lt },
			{ vec3(k, k, k), white,   rb },
			{ vec3(k,-k, k), magenta, rt },
			// 右.
			{ vec3(k, k, k), white,   lb },
			{ vec3(k,-k, k), magenta, lt },
			{ vec3(k, k,-k), cyan,    rb },
			{ vec3(k,-k,-k), blue,    rt },
			// 左
			{ vec3(-k, k,-k), green,  lb },
			{ vec3(-k,-k,-k), black,  lt },
			{ vec3(-k, k, k), yellow, rb },
			{ vec3(-k,-k, k), red,    rt },
			// 裏.
			{ vec3(k, k,-k), cyan, lb },
			{ vec3(k,-k,-k), blue, lt },
			{ vec3(-k, k,-k), green, rb },
			{ vec3(-k,-k,-k), black, rt },
			// 上.
			{ vec3(-k, k,-k), green, lb },
			{ vec3(-k, k, k), yellow, lt },
			{ vec3(k, k,-k), cyan, rb },
			{ vec3(k, k, k), white, rt },
			// 底.
			{ vec3(-k,-k, k), red, lb },
			{ vec3(-k,-k,-k), black, lt },
			{ vec3(k,-k, k), magenta, rb },
			{ vec3(k,-k,-k), blue, rt },
		};
		uint32_t indices[] = {
		  0, 2, 1, 1, 2, 3, // front
		  4, 6, 5, 5, 6, 7, // right
		  8,10, 9, 9,10,11, // left

		  12, 14, 13, 13, 14, 15, // back
		  16, 18, 17, 17, 18, 19, // top
		  20, 22, 21, 21, 22, 23, // bottom
		};

		m_cubeVertexBuffer = createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_cubeIndexBuffer = createBuffer(sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		// 頂点データの書き込み
		{
			void* p;
			vkMapMemory(m_device, m_cubeVertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, vertices, sizeof(vertices));
			vkUnmapMemory(m_device, m_cubeVertexBuffer.memory);
		}
		// インデックスデータの書き込み
		{
			void* p;
			vkMapMemory(m_device, m_cubeIndexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, indices, sizeof(indices));
			vkUnmapMemory(m_device, m_cubeIndexBuffer.memory);
		}
		m_cubeIndexCount = _countof(indices);

		// 頂点の入力設定
		VkVertexInputBindingDescription inputBinding{
		  0,                          // binding
		  sizeof(Vertex),          // stride
		  VK_VERTEX_INPUT_RATE_VERTEX // inputRate
		};
		array<VkVertexInputAttributeDescription, 3> inputAttribs{
		  {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
			{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, uv)},
		  }
		};
		VkPipelineVertexInputStateCreateInfo vertexInputCI{};
		vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCI.vertexBindingDescriptionCount = 1;
		vertexInputCI.pVertexBindingDescriptions = &inputBinding;
		vertexInputCI.vertexAttributeDescriptionCount = uint32_t(inputAttribs.size());
		vertexInputCI.pVertexAttributeDescriptions = inputAttribs.data();

		


		// シェーダーバイナリの読み込み
		vector<VkPipelineShaderStageCreateInfo> shaderStages
		{
		  loadShaderModule("texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		  loadShaderModule("texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		// パイプラインレイアウト
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
		vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_cubePipelineLayout);


		// パイプラインの構築
		VkGraphicsPipelineCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		ci.stageCount = uint32_t(shaderStages.size());
		ci.pStages = shaderStages.data();
		ci.pInputAssemblyState = &inputAssemblyCI;
		ci.pVertexInputState = &vertexInputCI;
		ci.pRasterizationState = &rasterizerCI;
		ci.pDepthStencilState = &depthStencilCI;
		ci.pMultisampleState = &multisampleCI;
		ci.pViewportState = &viewportCI;
		ci.pColorBlendState = &cbCI;
		ci.renderPass = m_renderPass;
		ci.layout = m_cubePipelineLayout;
		vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_cubePipeline);

		// ShaderModule はもう不要のため破棄
		for (const auto& v : shaderStages)
		{
			vkDestroyShaderModule(m_device, v.module, nullptr);
		}
	}
}

void VulkanAppOffscreen::prepareUniformBuffers()
{
	// ユニフォームバッファの作成
	m_uniformBuffers.resize(m_swapchainViews.size());
	for (auto& v : m_uniformBuffers)
	{
		VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		v = createBuffer(sizeof(ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
	}
}

void VulkanAppOffscreen::prepareOffscreenTexture()
{

	// オフスクリーンのレンダーターゲットを作成
	{
		VkImageCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ci.extent = { getSwapchainExtent().width, getSwapchainExtent().height, 1 };
		ci.format = m_surfaceFormat.format;
		ci.imageType = VK_IMAGE_TYPE_2D;
		ci.arrayLayers = 1;
		ci.mipLevels = 1;
		ci.samples = VK_SAMPLE_COUNT_1_BIT;
		ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		vkCreateImage(m_device, &ci, nullptr, &m_texture.image);

		VkMemoryRequirements reqs;
		vkGetImageMemoryRequirements(m_device, m_texture.image, &reqs);
		VkMemoryAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = reqs.size;
		// メモリタイプの判定
		info.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		// メモリの確保
		vkAllocateMemory(m_device, &info, nullptr, &m_texture.memory);
		// メモリのバインド
		vkBindImageMemory(m_device, m_texture.image, m_texture.memory, 0);

		VkImageViewCreateInfo viewCi{};
		viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCi.image = m_texture.image;
		viewCi.format = m_surfaceFormat.format;
		viewCi.components = {
		  VK_COMPONENT_SWIZZLE_R,
		  VK_COMPONENT_SWIZZLE_G,
		  VK_COMPONENT_SWIZZLE_B,
		  VK_COMPONENT_SWIZZLE_A,
		};
		viewCi.subresourceRange = {
		  VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
		};
		vkCreateImageView(m_device, &viewCi, nullptr, &m_texture.view);
	}

	// オフスクリーン用のフレームバッファを作成
	{
		VkImageView attachments[2];
		attachments[0] = m_texture.view;
		attachments[1] = m_depthBufferView;

		VkFramebufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.renderPass = m_offscreenRenderPass;
		ci.width = getSwapchainExtent().width;
		ci.height = getSwapchainExtent().height;
		ci.layers = 1;
		ci.attachmentCount = 2;
		ci.pAttachments = attachments;

		auto result = vkCreateFramebuffer(m_device, &ci, nullptr, &m_offscreenFrameBuffer);
		checkResult(result);

	}

	//サンプラーも作成
	{
		VkSamplerCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		ci.minFilter = VK_FILTER_LINEAR;
		ci.magFilter = VK_FILTER_LINEAR;
		ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		ci.maxAnisotropy = 1.0f;
		ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vkCreateSampler(m_device, &ci, nullptr, &m_sampler);
	}
}


void VulkanAppOffscreen::prepareDescriptorSetLayout()
{
	vector<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayoutBinding bindingUBO{}, bindingTex{};

	bindingUBO.binding = 0;
	bindingUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindingUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindingUBO.descriptorCount = 1;
	bindings.push_back(bindingUBO);

	bindingTex.binding = 1;
	bindingTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindingTex.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindingTex.descriptorCount = 1;
	bindings.push_back(bindingTex);

	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = uint32_t(bindings.size());
	ci.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(m_device, &ci, nullptr, &m_descriptorSetLayout);
}

void VulkanAppOffscreen::prepareDescriptorPool()
{
	array<VkDescriptorPoolSize, 2> descPoolSize;
	descPoolSize[0].descriptorCount = 1;
	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[1].descriptorCount = 1;
	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.maxSets = uint32_t(m_uniformBuffers.size());
	ci.poolSizeCount = uint32_t(descPoolSize.size());
	ci.pPoolSizes = descPoolSize.data();
	vkCreateDescriptorPool(m_device, &ci, nullptr, &m_descriptorPool);
}

void VulkanAppOffscreen::prepareDescriptorSet()
{
	vector<VkDescriptorSetLayout> layouts;
	for (int i = 0; i<int(m_uniformBuffers.size()); ++i)
	{
		layouts.push_back(m_descriptorSetLayout);
	}

	VkDescriptorSetAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = m_descriptorPool;
	ai.descriptorSetCount = m_uniformBuffers.size();
	ai.pSetLayouts = layouts.data();
	m_descriptorSet.resize(m_uniformBuffers.size());
	vkAllocateDescriptorSets(m_device, &ai, m_descriptorSet.data());

	for (int i = 0; i<int(m_uniformBuffers.size()); ++i)
	{
		VkDescriptorBufferInfo descUBO{};
		descUBO.buffer = m_uniformBuffers[i].buffer;
		descUBO.offset = 0;
		descUBO.range = VK_WHOLE_SIZE;

		VkDescriptorImageInfo descImage{};
		descImage.imageView = m_texture.view;
		descImage.sampler = m_sampler;
		descImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet ubo{};
		ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubo.dstBinding = 0;
		ubo.descriptorCount = 1;
		ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo.pBufferInfo = &descUBO;
		ubo.dstSet = m_descriptorSet[i];

		VkWriteDescriptorSet tex{};
		tex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		tex.dstBinding = 1;
		tex.descriptorCount = 1;
		tex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		tex.pImageInfo = &descImage;
		tex.dstSet = m_descriptorSet[i];

		vector<VkWriteDescriptorSet> writeSets = {
			ubo, tex
		};
		vkUpdateDescriptorSets(m_device, uint32_t(writeSets.size()), writeSets.data(), 0, nullptr);
	}

}

void VulkanAppOffscreen::createOffscreenRenderPass()
{
	VkRenderPassCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	array<VkAttachmentDescription, 2> attachments;
	auto& colorTarget = attachments[0];
	auto& depthTarget = attachments[1];

	colorTarget = VkAttachmentDescription{};
	colorTarget.format = m_surfaceFormat.format;
	colorTarget.samples = VK_SAMPLE_COUNT_1_BIT;
	colorTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorTarget.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	depthTarget = VkAttachmentDescription{};
	depthTarget.format = VK_FORMAT_D32_SFLOAT;
	depthTarget.samples = VK_SAMPLE_COUNT_1_BIT;
	depthTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthTarget.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference{}, depthReference{};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc{};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &colorReference;
	subpassDesc.pDepthStencilAttachment = &depthReference;

	ci.attachmentCount = uint32_t(attachments.size());
	ci.pAttachments = attachments.data();
	ci.subpassCount = 1;
	ci.pSubpasses = &subpassDesc;

	auto result = vkCreateRenderPass(m_device, &ci, nullptr, &m_offscreenRenderPass);
	checkResult(result);
}

void VulkanAppOffscreen::cleanup()
{
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_pipeline, nullptr);

	vkFreeMemory(m_device, m_vertexBuffer.memory, nullptr);
	vkFreeMemory(m_device, m_indexBuffer.memory, nullptr);
	vkDestroyBuffer(m_device, m_vertexBuffer.buffer, nullptr);
	vkDestroyBuffer(m_device, m_indexBuffer.buffer, nullptr);

	vkDestroyImage(m_device, m_texture.image, nullptr);
	vkDestroyImageView(m_device, m_texture.view, nullptr);
	vkFreeMemory(m_device, m_texture.memory, nullptr);
}




void VulkanAppOffscreen::makeCommand(VkCommandBuffer command)
{

	array<VkClearValue, 2> clearValue = {
		{ {1.0f, 1.0f, 1.0f, 0.0f},
		  {1.0f, 0}
		}
	};

	VkRenderPassBeginInfo renderPassBI{};
	renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBI.renderPass = m_offscreenRenderPass;
	renderPassBI.framebuffer = m_offscreenFrameBuffer;
	renderPassBI.renderArea.offset = VkOffset2D{ 0, 0 };
	renderPassBI.renderArea.extent = getSwapchainExtent();
	renderPassBI.pClearValues = clearValue.data();
	renderPassBI.clearValueCount = uint32_t(clearValue.size());


	vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

	{
		// 作成したパイプラインをセット
		vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		// 各バッファオブジェクトのセット
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command, 0, 1, &m_vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(command, m_indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);

		// 3角形描画
		vkCmdDrawIndexed(command, m_indexCount, 1, 0, 0, 0);
	}
	// end commandbuffer and renderpass
	vkCmdEndRenderPass(command);

	// clear value 更新
	clearValue = {
		{ {0.25f, 0.5f, 0.25f, 0.0f},
		  {1.0f, 0}
		}
	};

	renderPassBI.framebuffer = m_framebuffers[m_imageIndex];
	renderPassBI.renderPass = m_renderPass;

	

	vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
	{
		// ユニフォームバッファの中身を更新
		ShaderParameters shaderParam{};
		shaderParam.mtxWorld = glm::rotate(glm::identity<glm::mat4>(), glm::radians(45.0f), glm::vec3(0, 1, 0));
		shaderParam.mtxView = lookAtRH(vec3(0.0f, 3.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		shaderParam.mtxProj = perspective(glm::radians(60.0f), 640.0f / 480, 0.01f, 100.0f);

		{
			auto memory = m_uniformBuffers[m_imageIndex].memory;
			void *p;
			vkMapMemory(m_device, memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, &shaderParam, sizeof(shaderParam));
			vkUnmapMemory(m_device, memory);
		}

		// キューブのパイプラインセット
		vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_cubePipeline);

		// バッファオブジェクトのセット
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command, 0, 1, &m_cubeVertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(command, m_cubeIndexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);
	
		// ディスクリプタセットのセット
		VkDescriptorSet descriptorsets[] = {
			m_descriptorSet[m_imageIndex]
		};
		vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_cubePipelineLayout,0, 1, descriptorsets, 0, nullptr);

		// キューブを描画
		vkCmdDrawIndexed(command, m_cubeIndexCount, 1, 0, 0, 0);

	}
	vkCmdEndRenderPass(command);
}


VkPipelineShaderStageCreateInfo VulkanAppOffscreen::loadShaderModule(const char* fileName, VkShaderStageFlagBits stage)
{
	ifstream infile(fileName, std::ios::binary);
	if (!infile)
	{
		OutputDebugStringA("file not found.\n");
		DebugBreak();
	}
	vector<char> filedata;
	filedata.resize(uint32_t(infile.seekg(0, ifstream::end).tellg()));
	infile.seekg(0, ifstream::beg).read(filedata.data(), filedata.size());

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.pCode = reinterpret_cast<uint32_t*>(filedata.data());
	ci.codeSize = filedata.size();
	vkCreateShaderModule(m_device, &ci, nullptr, &shaderModule);

	VkPipelineShaderStageCreateInfo shaderStageCI{};
	shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCI.stage = stage;
	shaderStageCI.module = shaderModule;
	shaderStageCI.pName = "main";
	return shaderStageCI;
}


VulkanAppOffscreen::BufferObject VulkanAppOffscreen::createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
{
	BufferObject obj;
	VkBufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.usage = usage;
	ci.size = size;
	auto result = vkCreateBuffer(m_device, &ci, nullptr, &obj.buffer);
	checkResult(result);

	// メモリ量の算出
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements(m_device, obj.buffer, &reqs);
	VkMemoryAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.allocationSize = reqs.size;
	// メモリタイプの判定
	info.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits, flags);
	// メモリの確保
	vkAllocateMemory(m_device, &info, nullptr, &obj.memory);

	// メモリのバインド
	vkBindBufferMemory(m_device, obj.buffer, obj.memory, 0);
	return obj;
}