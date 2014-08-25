#ifndef MATERIAL_H
#define MATERIAL_H

#include "Texture.h"

#include <map>
#include <iostream>


class Material 
{
private:
	std::map <std::string, Texture*> m_textures;
	std::map <std::string, float > m_attributes;
public:
	Material();
	virtual ~Material();

	void setTexture(std::string name, Texture* texture);
	Texture* getTexture(std::string name);

	void setAttribute(std::string name, float value);
	float getAttribute(std::string name);

	std::map <std::string, Texture* > getTextures();
	std::map <std::string, Texture* >* getTexturesPtr();
	std::map <std::string, float > getAttributes();
};

#endif
