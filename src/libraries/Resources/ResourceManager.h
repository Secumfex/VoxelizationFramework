#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "Resources/Object.h"
#include "Resources/Texture.h"
#include <string>

class ResourceManager
{
protected:
public:
	ResourceManager();
	~ResourceManager();

	Object* loadObject(std::string path);
	Texture* loadTexture(std::string path);

};

#endif
