#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vector>
#include "Rendering/Renderable.h"
#include "Rendering/FramebufferObject.h"
#include "Rendering/Shader.h"

class RenderPass
{
private: 
	FramebufferObject* m_fbo;
	Shader* m_shader;
	std::vector< Renderable* > m_renderables;
public:
	RenderPass(Shader* shader, FramebufferObject* fbo = 0);
	~RenderPass();
	void render();

	void addRenderable(Renderable* renderable);

	std::vector< Renderable* > getRenderables();
};

#endif
