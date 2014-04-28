#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "Resources/Object.h"
#include "Resources/Texture.h"

#include <string>
#include <map>

#include "Utility/TextureTools.h"
#include "Utility/AssimpTools.h"

class ResourceManager
{
protected:
	std::map<std::string, Model*> m_loadedModels;
	std::map<std::string, Texture* > m_loadedTextures;
public:
	ResourceManager();
	~ResourceManager();

	Model* loadModel(std::string path);
	Texture* loadTexture(std::string path);

	void deleteAll();
};

#endif
