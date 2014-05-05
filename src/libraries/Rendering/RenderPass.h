#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vector>
#include "Rendering/Renderable.h"
#include "Rendering/FramebufferObject.h"
#include "Rendering/Shader.h"

class RenderPass
{
protected:
	glm::vec4 m_viewport;
	FramebufferObject* m_fbo;
	Shader* m_shader;
	std::vector< Renderable* > m_renderables;
public:
	RenderPass(Shader* shader = 0, FramebufferObject* fbo = 0);
	~RenderPass();

	virtual void preRender();
	virtual void uploadUniforms();
	void render();
	virtual void postRender();

	void setViewport(int x, int y, int width, int height);

	void addRenderable(Renderable* renderable);

	std::vector< Renderable* > getRenderables();
};

#endif
