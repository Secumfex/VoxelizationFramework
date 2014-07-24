#include "RenderPass.h"

#include "Application/Application.h"

RenderPass::RenderPass(Shader* shader, FramebufferObject* fbo)
{
	m_shader = shader;
	m_fbo = fbo;
	m_viewport = glm::vec4(0,0,Application::static_newWindowWidth,Application::static_newWindowHeight);
	m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	if (fbo)
	{
		m_viewport.z = (float) fbo->getWidth();
		m_viewport.w = (float) fbo->getHeight();
	}
}

void RenderPass::setClearColor(float r, float g, float b, float a)
{
	m_clearColor = glm::vec4(r,g,b,a);
}

RenderPass::~RenderPass()
{
	
}

void RenderPass::setShader(Shader* shader)
{
	m_shader = shader;
}

void RenderPass::setFramebufferObject( FramebufferObject* fbo)
{
	m_fbo = fbo;
}

FramebufferObject* RenderPass::getFramebufferObject()
{
	return m_fbo;
}

Shader* RenderPass::getShader()
{
	return m_shader;
}

void RenderPass::clearBits()
{
	call("PRECLEARBITS");
	for (unsigned int i = 0; i < m_clearBits.size(); i++)
	{
		if (m_clearBits[i] == GL_COLOR_BUFFER_BIT)
		{
			glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		}
		glClear( m_clearBits[i] );
	}
}

void RenderPass::preRender()
{
	call("PRERENDER");
}

void RenderPass::uploadUniforms()
{
	for(unsigned int i = 0; i < m_uniforms.size(); i++)
	{
		m_uniforms[i]->uploadUniform( m_shader );
	}
	call("UPLOADUNIFORMS");
}

void RenderPass::render()
{
	if (m_fbo)
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_fbo->getFramebufferHandle( ) );
	}
	else{
		glBindFramebuffer( GL_FRAMEBUFFER, 0);
	}

	m_shader->useProgram();
	glViewport( (GLint) m_viewport.x, (GLint) m_viewport.y, (GLsizei) m_viewport.z, (GLsizei) m_viewport.w);

	clearBits();

	enableStates();
	disableStates();

	preRender();
	for(unsigned int i = 0; i < m_renderables.size(); i++)
	{
		m_renderables[i]->uploadUniforms(m_shader);
		uploadUniforms();
		m_renderables[i]->render();
	}
	postRender();

	restoreStates();

	call("POSTRENDERPASS");	// this renderpass is finished
}

void RenderPass::postRender()
{
	call("POSTRENDER");
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
	m_viewport.x = (float) x;
	m_viewport.y = (float) y;
	m_viewport.z = (float) width;
	m_viewport.w = (float) height;
}

void RenderPass::addClearBit(GLbitfield clearBit)
{
	m_clearBits.push_back(clearBit);
}

void RenderPass::addEnable(GLenum state)
{
	m_enable.push_back(state);
	m_enableTEMP.push_back( 0 );
}

void RenderPass::addDisable(GLenum state)
{
	m_disable.push_back(state);
	m_disableTEMP.push_back( 0 );
}

void RenderPass::enableStates()
{
	for (unsigned int i = 0; i < m_enable.size(); i++)
	{
		m_enableTEMP[i] = glIsEnabled( m_enable [i] );
		if ( !m_enableTEMP[i] )	// is not enabled but should be
		{
			glEnable( m_enable[i] );
		}
	}
}

void RenderPass::disableStates()
{
	for (unsigned int i = 0; i < m_disable.size(); i++)
		{
			m_disableTEMP[i] = glIsEnabled( m_disable [i] );
			if ( m_disableTEMP[i] )	// is enabled but should not be
			{
				glDisable( m_disable[i] );
			}
		}
}

void RenderPass::restoreStates()
{
	for (unsigned int i = 0; i < m_enableTEMP.size(); i++)
	{
		if ( !m_enableTEMP[i] )	// was not enabled but got enabled
		{
			glDisable(m_enable[i]);
		}
	}
	for (unsigned int i = 0; i < m_disableTEMP.size(); i++)
	{
		if ( m_disableTEMP[i] )	// was enabled but got disabled
		{
			glEnable(m_disable[i]);
		}
	}
}

void RenderPass::addUniform(Uploadable* uniform) {
	m_uniforms.push_back( uniform );
}

void RenderPass::removeRenderable(Renderable* renderable) {
	for ( std::vector<Renderable* >::iterator it = m_renderables.begin(); it != m_renderables.end(); ++it)
	{
		if ( (*it) == renderable )
		{
			m_renderables.erase( it );
			return;
		}
	}
}

void RenderPass::clearRenderables() {
	m_renderables.clear();
}
