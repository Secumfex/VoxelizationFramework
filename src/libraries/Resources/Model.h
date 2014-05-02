#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include "Resources/Resource.h"

class Model : public Resource
{
private:
	GLuint m_VAOHandle;

	int m_numFaces;
	int m_numVertices;
	int m_numIndices;
public:
	Model();
	virtual ~Model();

	GLuint getVAOHandle();
	void setVAOHandle(GLuint VAOHandle);

	void setNumFaces(int numFaces);
	void setNumIndices(int numIndices);
	void setNumVertices(int numVertices);

	int getNumFaces();
	int getNumIndices();
	int getNumVertices();
};

#endif