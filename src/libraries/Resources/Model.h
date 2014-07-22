#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include "Resources/Resource.h"

class Model : public Resource
{
private:
	GLuint m_VAOHandle;

	GLuint m_indexBufferHandle;
	GLuint m_vertexBufferHandle;
	GLuint m_normalBufferHandle;
	GLuint m_uvBufferHandle;
	GLuint m_tangentBufferHandle;

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
	GLuint getIndexBufferHandle() const;
	void setIndexBufferHandle(GLuint indexBufferHandle);
	GLuint getNormalBufferHandle() const;
	void setNormalBufferHandle(GLuint normalBufferHandle);
	GLuint getPositionBufferHandle() const;
	void setVertexBufferHandle(GLuint positionBufferHandle);
	GLuint getUvBufferHandle() const;
	void setUvBufferHandle(GLuint uvBufferHandle);
	GLuint getVaoHandle() const;
	void setVaoHandle(GLuint vaoHandle);
	GLuint getTangentBufferHandle() const;
	void setTangentBufferHandle(GLuint tangentBufferHandle);
};

#endif
