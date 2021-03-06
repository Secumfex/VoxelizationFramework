#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vector>
#include "Rendering/Renderable.h"
#include "Rendering/FramebufferObject.h"
#include "Rendering/Shader.h"
#include "Rendering/Uniform.h"
#include "Utility/SubjectListenerPattern.h"

template<typename T>
struct vecUniform : public std::vector< Uniform<T>* > {};


class RenderPass : public Subject
{
protected:
	glm::vec4 m_viewport;
	glm::vec4 m_clearColor;
	FramebufferObject* m_fbo;
	Shader* m_shader;
	std::vector< Renderable* > m_renderables;

	std::vector< GLbitfield > m_clearBits;
	std::vector< GLenum > m_enable;
	std::vector< GLenum > m_disable;
	std::vector< bool > m_enableTEMP;
	std::vector< bool > m_disableTEMP;

	std::vector< Uploadable* > m_uniforms;

public:
	RenderPass(Shader* shader = 0, FramebufferObject* fbo = 0);
	virtual ~RenderPass();

	virtual void clearBits();
	virtual void enableStates();
	virtual void disableStates();

	virtual void preRender();
	virtual void uploadUniforms();
	virtual void render();
	virtual void postRender();
	virtual void restoreStates();

	void setViewport(int x, int y, int width, int height);
	void setClearColor(float r, float g, float b, float a = 1.0f);

	void setFramebufferObject(FramebufferObject* fbo);
	void setShader(Shader* shader);

	void addRenderable(Renderable* renderable);
	void removeRenderable( Renderable* renderable );
	void clearRenderables();

	std::vector< Renderable* > getRenderables();

	FramebufferObject* getFramebufferObject();
	Shader* getShader();

	void addClearBit(GLbitfield clearBit);
	void addEnable(GLenum state);
	void addDisable(GLenum state);

	void addUniform(Uploadable* uniform);

	void removeEnable(GLenum state);
	void removeDisable(GLenum state);
	void removeClearBit(GLbitfield clearBit);
};

#endif
