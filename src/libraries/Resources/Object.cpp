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