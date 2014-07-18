#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include <Utility/Singleton.h>

#include <GL/glew.h>

#include <vector>
#include <map>

/**
 * the global Renderstate of the Application, storing currently bound Textures and everyting
 * You better always use this thing when using gl Commands
 */
class RenderState : public Singleton< RenderState >
{
	friend class Singleton< RenderState >;

private:
	RenderState();
public:
	std::map<GLenum, GLuint> m_boundVariables;

	~RenderState();

	bool activeStateDiffers( GLenum state, const GLuint& compare );

	void refreshBoundVariables();

	bool bindVertexArrayObjectIfDifferent( GLuint value );

};

#endif
