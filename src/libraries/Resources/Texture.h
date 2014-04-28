#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GL/glew.h>

#include "Resources/Resource.h"

class Texture : public Resource{
protected:
	GLuint m_texturehandle;
public:
	Texture(std::string path = "");
	virtual ~Texture();

	GLuint getTextureHandle();
	void setTextureHandle(GLuint textureHandle);
};

#endif
