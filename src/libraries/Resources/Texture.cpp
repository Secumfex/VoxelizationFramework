#include "Resources/Texture.h"

Texture::Texture(std::string path) {
	Resource::setPath(path);
	m_texturehandle = 0;
	m_activeUnit = -1;
}

Texture::Texture(GLuint textureHandle)
{
	Resource::setPath("");
	m_activeUnit = -1;
	m_texturehandle = textureHandle;
}

Texture::~Texture() {
}

GLuint Texture::getTextureHandle()
{
	return m_texturehandle;
}

void Texture::setTextureHandle(GLuint textureHandle)
{
	m_texturehandle = textureHandle;
}

void Texture::bindToTextureUnit( int unit )
{
	if ( unit != m_activeUnit)
	{
		unbindFromActiveUnit();
	}

	glActiveTexture(GL_TEXTURE0 + unit);	//bind to unit
	glBindTexture(GL_TEXTURE_2D, m_texturehandle);
	m_activeUnit = unit;
	glActiveTexture(GL_TEXTURE0);
}

void Texture::unbindFromActiveUnit()
{
	if (m_activeUnit != -1)
	{
		glActiveTexture(GL_TEXTURE0 + m_activeUnit);
		// test whether this is really this texture
		GLint handle;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &handle);
		if ( handle == m_texturehandle)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glActiveTexture(GL_TEXTURE0);
		m_activeUnit = -1;
	}
}

int Texture::getActiveUnit()
{
	return m_activeUnit;
}

Texture1D::Texture1D(std::string path)
	: Texture(path){
}

Texture1D::Texture1D(GLuint textureHandle)
    : Texture(textureHandle){
}

Texture1D::~Texture1D() {
}

void Texture1D::bindToTextureUnit(int unit) {
	if ( unit != m_activeUnit)
	{
		Texture1D::unbindFromActiveUnit();
	}

	glActiveTexture(GL_TEXTURE0 + unit);	//bind to unit
	glBindTexture(GL_TEXTURE_1D, m_texturehandle);
	m_activeUnit = unit;
	glActiveTexture(GL_TEXTURE0);
}

void Texture1D::unbindFromActiveUnit() {
	if (m_activeUnit != -1)
	{
		glActiveTexture(GL_TEXTURE0 + m_activeUnit);
		// test whether this is really this texture
		GLint handle;
		glGetIntegerv(GL_TEXTURE_BINDING_1D, &handle);
		if ( handle == m_texturehandle)
		{
			glBindTexture(GL_TEXTURE_1D, 0);
		}
		glActiveTexture(GL_TEXTURE0);
		m_activeUnit = -1;
	}
}
