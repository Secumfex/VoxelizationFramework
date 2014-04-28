#include "ResourceManager.h"

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	
}

Texture* ResourceManager::loadTexture(std::string path)
{
	Texture* texture = new Texture(path);
	GLuint texturehandle = TextureTools::loadTexture(path);

	result->setTextureHandle(texturehandle);
	return texture;
}

Model* ResourceManager::loadModel(std::string path)
{
	model = AssimpTools::loadModel(path);

	return model;
}

void ResourceManager::deleteAll()
{
	for (std::map<std::string,Object*>::iterator it = m_loadedModels.begin(); it != m_loadedModels.end(); ++it)
	{
		delete it->second;	
	}
	for (std::map<std::string,Texture*>::iterator it = m_loadedTextures.begin(); it != m_loadedTextures.end(); ++it)
	{
		delete it->second;	
	}
}