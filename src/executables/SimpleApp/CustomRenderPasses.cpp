#include "CustomRenderPasses.h"
TestRenderPass::TestRenderPass(Shader* shader, FramebufferObject* fbo)
{
	m_shader = shader;
	m_fbo = fbo;
	m_viewport = glm::vec4(0,0,800,600);
	if (fbo)
	{
		m_viewport.z = fbo->getWidth();
		m_viewport.w = fbo->getHeight();
	}
	m_camera = 0;
}

TestRenderPass::~TestRenderPass()
{

}

void TestRenderPass::setCamera(Camera* camera)
{
	m_camera = camera;
}

Camera* TestRenderPass::getCamera()
{
	return m_camera;
}

void TestRenderPass::preRender()
{
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TestRenderPass::uploadUniforms()
{
	m_shader->uploadUniform(m_camera->getViewMatrix(),"uniformView");
	m_shader->uploadUniform(m_camera->getProjectionMatrix(), "uniformProjection");
}
