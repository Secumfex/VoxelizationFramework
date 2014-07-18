#include "Rendering/RenderState.h"

#include "Utility/DebugLog.h"

RenderState::RenderState()
{
}

RenderState::~RenderState() {

}

bool RenderState::activeStateDiffers(GLenum state, const GLuint& compare )
{
	std::map<GLenum, GLuint>::iterator it = m_boundVariables.find(state);
	if (it != m_boundVariables.end() )
	{
		if ( (*it).second == compare)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		GLint value;
		glGetIntegerv(state, &value);
		m_boundVariables[state] = value;

		return ( compare != value );
	}
}

void RenderState::refreshBoundVariables() {

	for ( std::map< GLenum, GLuint >::iterator it = m_boundVariables.begin(); it != m_boundVariables.end(); ++it)
	{
		GLint value;
		GLenum enumeration = (*it).first;
		glGetIntegerv( enumeration, &value );

		m_boundVariables[ enumeration ] = value;
	}

}

bool RenderState::bindVertexArrayObjectIfDifferent(GLuint value) {
	if ( m_boundVariables[ GL_VERTEX_ARRAY_BINDING ] != value )
	{
		glBindVertexArray( value );
		return true;
	}
	return false;
}
