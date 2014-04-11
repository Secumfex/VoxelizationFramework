#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vector>
#include "Rendering/Renderable.h"

class RenderPass
{
private: 
	std::vector< Renderable* > m_renderables;
public:
	RenderPass();
	~RenderPass();
	void render();

	void addRenderable(Renderable* renderable);

	std::vector< Renderable* > getRenderables();
};

#endif
