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

	GLuint getFramebufferHandle();

	int getWidth();
	int getHeight();

	const std::map<GLenum, GLuint>& getColorAttachments() const;
	void setColorAttachments(const std::map<GLenum, GLuint>& colorAttachments);
	GLuint getDepthTextureHandle() const;
	void setDepthTextureHandle(GLuint depthTextureHandle);
	const std::vector<GLenum>& getDrawBuffers() const;
	void setDrawBuffers(const std::vector<GLenum>& drawBuffers);
	void setFramebufferHandle(GLuint framebufferHandle);
	void setHeight(int height);
	int getNumColorAttachments() const;
	void setNumColorAttachments(int numColorAttachments);
	void setWidth(int width);
};

#endif
