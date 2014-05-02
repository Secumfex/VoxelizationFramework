#include "Resources/Model.h"

Model::Model()
{
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