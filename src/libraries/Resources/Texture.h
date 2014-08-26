#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GL/glew.h>

#include "Resources/Resource.h"

// a 2D Texture
class Texture : public Resource{
protected:
	GLuint m_texturehandle;
	int m_activeUnit;
public:
	Texture(std::string path = "");
	Texture(GLuint textureHandle);
	virtual ~Texture();

	GLuint getTextureHandle();
	void setTextureHandle(GLuint textureHandle);

	virtual void bindToTextureUnit(int unit);
	int getActiveUnit();
	virtual void unbindFromActiveUnit();
};

// a 1D Texture, will bind to GL_TEXTURE_1D instead of GL_TEXTURE_2D
class Texture1D : public Texture
{
private:

public:
	Texture1D( std::string path = "");
	Texture1D( GLuint textureHandle);
	virtual ~Texture1D();

	void bindToTextureUnit( int unit );
	void unbindFromActiveUnit();

};

#endif
