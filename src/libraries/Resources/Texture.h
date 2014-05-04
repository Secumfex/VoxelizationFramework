#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GL/glew.h>

#include "Resources/Resource.h"

class Texture : public Resource{
protected:
	GLuint m_texturehandle;
	int m_activeUnit;
public:
	Texture(std::string path = "");
	virtual ~Texture();

	GLuint getTextureHandle();
	void setTextureHandle(GLuint textureHandle);

	void bindToTextureUnit(int unit);
	int getActiveUnit();
	void unbindFromActiveUnit();
};

#endif
