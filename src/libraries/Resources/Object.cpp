#include "Object.h"

Object::Object( Model* model, Material* material)
{
	m_model = model;
	m_material = material;	
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
	glDrawElements(GL_TRIANGLES, m_model->getNumIndices(), GL_UNSIGNED_INT, 0);
}

void Object::uploadUniforms(Shader* shader)
{
	std::map<std::string, Texture*> textures = m_material->getTextures();
	int unit = 0;
	for (std::map<std::string, Texture* >::iterator it = textures.begin(); it != textures.end(); ++it)
	{
		(*it).second->bindToTextureUnit(unit);
		shader->uploadUniform(unit, (*it).first);
		unit++;
	}
}
