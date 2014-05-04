#include "Resources/Texture.h"

Texture::Texture(std::string path) {
	Resource::setPath(path);
	m_texturehandle = 0;
	m_activeUnit = -1;
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
	unbindFromActiveUnit();

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
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		m_activeUnit = -1;
	}
}

int Texture::getActiveUnit()
{
	return m_activeUnit;
}
