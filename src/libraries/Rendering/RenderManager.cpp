#include "RenderManager.h"

RenderManager::RenderManager()
{

}

RenderManager::~RenderManager()
{
	
}

void RenderManager::render()
{
	for( unsigned int i = 0; i < m_renderpasses.size(); i++)
	{
		m_renderpasses[i]->render();
	}
}

void RenderManager::addRenderPass(RenderPass* renderPass)
{
	m_renderpasses.push_back(renderPass);
}

std::vector<RenderPass* > RenderManager::getRenderPasses()
{
	return m_renderpasses;
}

void RenderManager::setActiveWindow(GLFWwindow* window)
{
	m_activeWindow = window;
}
