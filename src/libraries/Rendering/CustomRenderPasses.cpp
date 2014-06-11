#include "CustomRenderPasses.h"

CameraRenderPass::CameraRenderPass()
{
	m_shader = 0;
	m_fbo = 0;
	m_viewport = glm::vec4(0,0,800,600);
	m_camera = 0;
}

CameraRenderPass::CameraRenderPass(Shader* shader, FramebufferObject* fbo)
{
	m_shader = shader;
	m_fbo = fbo;
	if (fbo)
	{
		m_viewport.z = fbo->getWidth();
		m_viewport.w = fbo->getHeight();
	}
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
{
	m_shader = shader;
	m_fbo = fbo;
	m_screenFillingTriangle = screenFillingTriangle;
	addRenderable(screenFillingTriangle);
	addDisable(GL_DEPTH_TEST);
}

void TriangleRenderPass::addUniformTexture(Texture* texture, std::string uniformTarget)
{
	m_uniformTextures.push_back( pair<Texture*, std::string> (texture, uniformTarget) );
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
