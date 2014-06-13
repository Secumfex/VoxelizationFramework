#include "Rendering/FramebufferObject.h"

#include "Utility/DebugLog.h"

GLenum FramebufferObject::internalFormat = GL_RGBA;	// default

FramebufferObject::FramebufferObject(int width, int height)
{
	glGenFramebuffers(1, &m_framebufferHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

	m_width = width;
	m_height = height;

	createDepthTexture();

	m_numColorAttachments = 0;
}

void FramebufferObject::createDepthTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

	glGenTextures(1, &m_depthTextureHandle);
	glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureHandle, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint FramebufferObject::createFramebufferTexture()
{
	GLuint textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureHandle;
}

void FramebufferObject::addColorAttachments(int amount)
{
	int maxColorAttachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	if ( m_numColorAttachments + amount <=  maxColorAttachments)
	{
		DEBUGLOG->log("max color attachments: ", maxColorAttachments);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

		DEBUGLOG->log("Creating Color Attachments: ", amount);
		DEBUGLOG->indent();
		for (int i = 0; i < amount; i ++)
		{
			GLuint textureHandle = createFramebufferTexture();
			
			glBindTexture(GL_TEXTURE_2D, textureHandle);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_numColorAttachments + i, GL_TEXTURE_2D, textureHandle, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			m_colorAttachments[GL_COLOR_ATTACHMENT0 + m_numColorAttachments + i] = textureHandle;
			m_drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + m_numColorAttachments + i);
		}
		DEBUGLOG->outdent();

		m_numColorAttachments = m_colorAttachments.size();
		glDrawBuffers(m_drawBuffers.size(), &m_drawBuffers[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

GLuint FramebufferObject::getColorAttachmentTextureHandle(GLenum attachment)
{
	if ( m_colorAttachments.find(attachment) != m_colorAttachments.end())
	{
		return m_colorAttachments[attachment];
	}
	else{
		return 0;
	}
}

GLuint FramebufferObject::getFramebufferHandle()
{
	return m_framebufferHandle;
}

int FramebufferObject::getWidth()
{
	return m_width;
}

const std::map<GLenum, GLuint>& FramebufferObject::getColorAttachments() const {
	return m_colorAttachments;
}

void FramebufferObject::setColorAttachments(
		const std::map<GLenum, GLuint>& colorAttachments) {
	m_colorAttachments = colorAttachments;
}

GLuint FramebufferObject::getDepthTextureHandle() const {
	return m_depthTextureHandle;
}

void FramebufferObject::setDepthTextureHandle(GLuint depthTextureHandle) {
	m_depthTextureHandle = depthTextureHandle;
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureHandle, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const std::vector<GLenum>& FramebufferObject::getDrawBuffers() const {
	return m_drawBuffers;
}

void FramebufferObject::setDrawBuffers(const std::vector<GLenum>& drawBuffers) {
	m_drawBuffers = drawBuffers;
}

void FramebufferObject::setFramebufferHandle(GLuint framebufferHandle) {
	m_framebufferHandle = framebufferHandle;
}

void FramebufferObject::setHeight(int height) {
	m_height = height;
}

int FramebufferObject::getNumColorAttachments() const {
	return m_numColorAttachments;
}

void FramebufferObject::setNumColorAttachments(int numColorAttachments) {
	m_numColorAttachments = numColorAttachments;
}

FramebufferObject::~FramebufferObject() {
	// TODO free OpenGL textures etc.
}

void FramebufferObject::setWidth(int width) {
	m_width = width;
}

int FramebufferObject::getHeight()
{
	return m_height;
}
