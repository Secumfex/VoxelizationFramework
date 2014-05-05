#include "RenderPass.h"

RenderPass::RenderPass(Shader* shader, FramebufferObject* fbo)
{
	m_shader = shader;
	m_fbo = fbo;
	m_viewport = glm::vec4(0,0,800,600);
	if (fbo)
	{
		m_viewport.z = fbo->getWidth();
		m_viewport.w = fbo->getHeight();
	}
}

RenderPass::~RenderPass()
{
	
}

void RenderPass::preRender()
{

}

void RenderPass::uploadUniforms()
{

}

void RenderPass::render()
{
	if (m_fbo)
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_fbo->getFramebufferHandle( ) );
	}
	m_shader->useProgram();
	glViewport(m_viewport.x, m_viewport.y, m_viewport.z, m_viewport.w);
	preRender();
	for(unsigned int i = 0; i < m_renderables.size(); i++)
	{
		m_renderables[i]->uploadUniforms(m_shader);
		uploadUniforms();
		m_renderables[i]->render();
	}
	postRender();
}

void RenderPass::postRender()
{

}

void RenderPass::addRenderable(Renderable* renderable)
{
	m_renderables.push_back(renderable);
}

std::vector< Renderable* > RenderPass::getRenderables()
{
	return m_renderables;
}

void RenderPass::setViewport(int x, int y, int width, int height)
{
	m_viewport.x = x;
	m_viewport.y = y;
	m_viewport.z = width;
	m_viewport.w = height;
}
