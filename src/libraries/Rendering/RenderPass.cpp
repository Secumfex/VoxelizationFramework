#include "RenderPass.h"

RenderPass::RenderPass()
{

}

RenderPass::~RenderPass()
{
	
}

void RenderPass::render()
{
	for(unsigned int i = 0; i < m_renderables.size(); i++)
	{
		m_renderables[i]->render();
	}
}

void RenderPass::addRenderable(Renderable* renderable)
{
	m_renderables.push_back(renderable);
}

std::vector< Renderable* > RenderPass::getRenderables()
{
	return m_renderables;
}
