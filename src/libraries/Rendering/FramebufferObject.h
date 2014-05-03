#ifndef FRAMEBUFFEROBJECT_H
#define FRAMEBUFFEROBJECT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <vector>

class FramebufferObject
{
private:
	GLuint m_framebufferHandle;

	int m_width;
	int m_height;

	int m_numColorAttachments;

	GLuint m_depthTextureHandle;

	std::map< GLenum, GLuint > m_colorAttachments;
	std::vector<GLenum > m_drawBuffers;
public:
	FramebufferObject(int width = 800, int height = 600);
	~FramebufferObject();

	void createDepthTexture();

	GLuint createFramebufferTexture();
	void addColorAttachments(int amount);

	GLuint getColorAttachmentTextureHandle(GLenum attachment);

	int getWidth();
	int getHeight();
};

#endif