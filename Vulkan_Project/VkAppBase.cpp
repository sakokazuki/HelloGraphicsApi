#include "vkappbase.h"
#include <sstream>
#include <algorithm>
#include <array>

#define GetInstanceProcAddr(FuncName) \
  m_##FuncName = reinterpret_cast<PFN_##FuncName>(vkGetInstanceProcAddr(m_instance, #FuncName))

using namespace std;


void VulkanAppBase::checkResult(VkResult result)
{
	
}

VulkanAppBase::VulkanAppBase()
{
}

void VulkanAppBase::initialize(GLFWwindow* window, const char* appName)
{
	
}

void VulkanAppBase::terminate()
{

}


void VulkanAppBase::initializeInstance(const char* appName)
{
	
}

void VulkanAppBase::selectPhysicalDevice()
{
	
}

uint32_t VulkanAppBase::searchGraphicsQueueIndex()
{
	return 0;
}
void VulkanAppBase::createDevice()
{
	
}

void VulkanAppBase::prepareCommandPool()
{
	
}

void VulkanAppBase::selectSurfaceFormat(VkFormat format)
{
	
}

void VulkanAppBase::createSwapchain(GLFWwindow* window)
{
	
}
void VulkanAppBase::createDepthBuffer()
{
	
}

void VulkanAppBase::createViews()
{
	
}

void VulkanAppBase::createRenderPass()
{
	
}

void VulkanAppBase::createFramebuffer()
{

}
void VulkanAppBase::prepareCommandBuffers()
{
	
}

void VulkanAppBase::prepareSemaphores()
{

}


uint32_t VulkanAppBase::getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps)const
{
	return 0;
}


void VulkanAppBase::enableDebugReport()
{
	
}

void VulkanAppBase::render()
{
	
}