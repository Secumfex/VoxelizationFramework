#include "Resources/Texture.h"

Texture::Texture(std::string path) {
	Resource::setPath(path);
	m_texturehandle = 0;
}

Texture::~Texture() {
	// TODO Auto-generated destructor stub
}

GLuint Texture::getTextureHandle()
{
	return m_texturehandle;
}

void Texture::setTextureHandle(GLuint textureHandle)
{
	m_texturehandle = textureHandle;
}