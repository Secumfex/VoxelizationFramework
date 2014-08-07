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

int RenderManager::addRenderPass(RenderPass* renderPass)
{
	m_renderpasses.push_back(renderPass);
	return m_renderpasses.size()-1;
}

std::vector<RenderPass* > RenderManager::getRenderPasses()
{
	return m_renderpasses;
}

std::vector<RenderPass* >* RenderManager::getRenderPassesPtr()
{
	return &m_renderpasses;
}

int RenderManager::getRenderPassIndex(RenderPass* renderPass) {
	for ( unsigned int i = 0; i < m_renderpasses.size(); i++ )
	{
		if ( m_renderpasses[i] == renderPass )
		{
			return i;
		}
	}
	return -1;
}

void RenderManager::setActiveWindow(GLFWwindow* window)
{
	m_activeWindow = window;
}
