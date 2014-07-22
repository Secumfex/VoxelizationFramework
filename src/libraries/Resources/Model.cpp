#include "Resources/Model.h"

Model::Model()
{
	m_VAOHandle = 0;
	m_indexBufferHandle = 0;
	m_vertexBufferHandle = 0;
	m_normalBufferHandle = 0;
	m_tangentBufferHandle = 0;
	m_uvBufferHandle = 0;
	m_numFaces = 0;
	m_numVertices = 0;
	m_numFaces = 0;
	m_path = "";
	m_numIndices = 0;
}

Model::~Model()
{

}

GLuint Model::getVAOHandle()
{
	return m_VAOHandle;
}

void Model::setVAOHandle(GLuint VAOhandle)
{
	m_VAOHandle = VAOhandle;
}

void Model::setNumFaces(int numFaces)
{
	m_numFaces = numFaces;
}

void Model::setNumIndices(int numIndices)
{
	m_numIndices = numIndices;
}

void Model::setNumVertices(int numVertices){
	 m_numVertices = numVertices;
}

int Model::getNumFaces()
{
	return m_numFaces;
}

int Model::getNumIndices()
{
	return m_numIndices;
}

int Model::getNumVertices()
{
	return m_numVertices;
}

GLuint Model::getIndexBufferHandle() const {
	return m_indexBufferHandle;
}

void Model::setIndexBufferHandle(GLuint indexBufferHandle) {
	m_indexBufferHandle = indexBufferHandle;
}

GLuint Model::getNormalBufferHandle() const {
	return m_normalBufferHandle;
}

void Model::setNormalBufferHandle(GLuint normalBufferHandle) {
	m_normalBufferHandle = normalBufferHandle;
}

GLuint Model::getPositionBufferHandle() const {
	return m_vertexBufferHandle;
}

void Model::setVertexBufferHandle(GLuint vertexBufferHandle) {
	m_vertexBufferHandle = vertexBufferHandle;
}

GLuint Model::getUvBufferHandle() const {
	return m_uvBufferHandle;
}

void Model::setUvBufferHandle(GLuint uvBufferHandle) {
	m_uvBufferHandle = uvBufferHandle;
}

GLuint Model::getVaoHandle() const {
	return m_VAOHandle;
}

GLuint Model::getTangentBufferHandle() const {
	return m_tangentBufferHandle;
}

void Model::setTangentBufferHandle(GLuint tangentBufferHandle) {
	m_tangentBufferHandle = tangentBufferHandle;
}

void Model::setVaoHandle(GLuint vaoHandle) {
	m_VAOHandle = vaoHandle;
}
