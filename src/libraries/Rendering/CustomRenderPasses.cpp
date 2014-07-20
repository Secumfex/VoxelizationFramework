#include "CustomRenderPasses.h"

CameraRenderPass::CameraRenderPass()
{
	m_shader = 0;
	m_fbo = 0;
	m_viewport = glm::vec4(0,0,800,600);
	m_camera = 0;
}

CameraRenderPass::CameraRenderPass(Shader* shader, FramebufferObject* fbo)
  : RenderPass(shader, fbo)
{
	m_camera = 0;
}

CameraRenderPass::~CameraRenderPass()
{

}

void CameraRenderPass::setCamera(Camera* camera)
{
	m_camera = camera;
}

Camera* CameraRenderPass::getCamera()
{
	return m_camera;
}

void CameraRenderPass::uploadUniforms()
{
	if(m_camera)
	{
		m_shader->uploadUniform(m_camera->getViewMatrix(),"uniformView");
		m_shader->uploadUniform(m_camera->getProjectionMatrix(), "uniformProjection");

		// some other information
		m_shader->uploadUniform(m_camera->getPosition(), "uniformCameraPosition");
		m_shader->uploadUniform(m_camera->getViewDirection(), "uniformCameraDirection");
	}
	RenderPass::uploadUniforms();
}

TriangleRenderPass::TriangleRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* screenFillingTriangle)
	: RenderPass(shader, fbo)
{
	m_screenFillingTriangle = screenFillingTriangle;
	addRenderable(screenFillingTriangle);
	addDisable(GL_DEPTH_TEST);
}

void TriangleRenderPass::addUniformTexture(Texture* texture, std::string uniformTarget)
{
	DEBUGLOG->log("TRIANGLERENDERPASS : added uniform texture:" + uniformTarget);

	m_uniformTextures.push_back( pair<Texture*, std::string> (texture, uniformTarget) );
}

void TriangleRenderPass::removeUniformTexture(Texture* texture)
{
	for ( std::vector<std::pair<Texture*, std::string > >::iterator it = m_uniformTextures.begin(); it != m_uniformTextures.end(); ++it)
	{
		if ( ( *it ).first == texture)
		{
			DEBUGLOG->log("TRIANGLERENDERPASS : removed uniform texture: " + (*it).second );
			m_uniformTextures.erase( it );
			return;
		}
	}
}

void TriangleRenderPass::removeUniformTexture(std::string uniformTarget)
{
	for ( std::vector<std::pair<Texture*, std::string > >::iterator it = m_uniformTextures.begin(); it != m_uniformTextures.end(); ++it)
	{
		if ( ( *it ).second == uniformTarget)
		{
			DEBUGLOG->log("TRIANGLERENDERPASS : removed uniform texture: " + (*it).second );
			m_uniformTextures.erase( it );
			return;
		}
	}
}

void TriangleRenderPass::uploadUniforms()
{
	for (unsigned int i = 0; i < m_uniformTextures.size(); i++)
	{
		m_uniformTextures[i].first->bindToTextureUnit(i);
		m_shader->uploadUniform( (int) i,  m_uniformTextures[i].second);
	}
	RenderPass::uploadUniforms();
}

void TriangleRenderPass::setUniformTexture(Texture* texture,
		std::string uniformTarget) {
	for ( unsigned int i = 0; i < m_uniformTextures.size(); i++)
	{
		// find uniform string
		if ( m_uniformTextures[i].second == uniformTarget)
		{
			DEBUGLOG->log("TRIANGLERENDERPASS : replaced uniform texture: " + uniformTarget);
			// overwrite handle
			m_uniformTextures[i].first = texture;
			return;
		}
	}

	// if method reaches this position, add as usual
	addUniformTexture( texture, uniformTarget );

}
