#include "Object.h"

#include "Utility/DebugLog.h"

Object::Object( Model* model, Material* material)
{
	m_model = model;
	m_material = material;	
	m_renderMode = GL_TRIANGLES;
}

Object::~Object()
{

}

void Object::setMaterial( Material* material )
{
	m_material = material;
}

void Object::setModel( Model* model )
{
	m_model = model;
}

Model* Object::getModel()
{
	return m_model;
}

Material* Object::getMaterial()
{
	return m_material;
}

void Object::render()
{
	glBindVertexArray(m_model->getVAOHandle());
	glDrawElements(m_renderMode, ( m_model != 0 ) ? m_model->getNumIndices() : 0, GL_UNSIGNED_INT, 0);
}

void Object::uploadUniforms(Shader* shader)
{
	// upload textures
	std::map<std::string, Texture*> textures = (m_material != 0) ? m_material->getTextures() : std::map<std::string, Texture*>();
	int unit = 0;
	for (std::map<std::string, Texture* >::iterator it = textures.begin(); it != textures.end(); ++it)
	{
		(*it).second->bindToTextureUnit(unit);
		shader->uploadUniform(unit, (*it).first);
		unit++;
	}

	// upload attributes
	std::map<std::string, float> attributes = (m_material != 0) ? m_material->getAttributes() : std::map<std::string, float>();
	for (std::map<std::string, float >::iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		shader->uploadUniform( (*it).second, (*it).first);
	}
}

void Object::setRenderMode(GLenum renderMode) {
	m_renderMode = renderMode;
}
