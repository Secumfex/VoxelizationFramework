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
	m_shader->uploadUniform(m_camera->getViewMatrix(),"uniformView");
	m_shader->uploadUniform(m_camera->getProjectionMatrix(), "uniformProjection");

	// some other information
	m_shader->uploadUniform(m_camera->getPosition(), "uniformCameraPosition");
	m_shader->uploadUniform(m_camera->getViewDirection(), "uniformCameraDirection");
	RenderPass::uploadUniforms();
}
