#include "Resources/Texture.h"

Texture::Texture(std::string path) {
	Resource::setPath(path);
	m_texturehandle = 0;
}

Texture::~Texture() {
	// TODO Auto-generated destructor stub
}

GLint Texture::getTextureHandle()
{
	return m_texturehandle;
}
