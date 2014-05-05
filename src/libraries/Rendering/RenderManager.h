#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include "RenderPass.h"

#include <vector>

class RenderManager 
{
protected:
	std::vector<RenderPass* > m_renderpasses;
	GLFWwindow* m_activeWindow;
public:
	RenderManager();
	~RenderManager();
	void render();

	void addRenderPass(RenderPass* renderPass);

	std::vector<RenderPass* > getRenderPasses();

	void setActiveWindow(GLFWwindow* window);
};

#endif
