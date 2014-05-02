#include "Resources/Material.h"

Material::Material()
{

}

Material::~Material()
{

}

void Material::setTexture(std::string name, Texture* texture)
{
	m_textures[name] = texture;
}

Texture* Material::getTexture(std::string name)
{
	if (m_textures.find(name) != m_textures.end())
	{
		return m_textures[name];
	}
	else{
		std::cout << "ERROR: Texture '" << name << "' did not exist in this material." << std::endl;
		return 0;
	}
}

void Material::setAttribute(std::string name, float value)
{
	m_attributes[name] = value;
}

float Material::getAttribute(std::string name)
{
	if (m_attributes.find(name) != m_attributes.end())
	{
		return m_attributes[name];
	}
	else{
		std::cout << "ERROR: Attribute '" << name << "' did not exist in this material."<<std::endl;
		return 0.0f;
	}
}

std::map <std::string, Texture*> Material::getTextures()
{
	return m_textures;
}

std::map <std::string, float> Material::getAttributes()
{
	return m_attributes;
}