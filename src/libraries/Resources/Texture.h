#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GL/glew.h>

#include "Resources/Resource.h"

class Texture : public Resource{
protected:
	GLint m_texturehandle;
public:
	Texture(std::string path = "");
	virtual ~Texture();

	GLint getTextureHandle();
};

#endif
