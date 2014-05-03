#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "Resources/Object.h"
#include "Resources/Texture.h"

#include <string>
#include <map>
#include <vector>

#include "Utility/AssimpTools.h"
#include "Utility/TextureTools.h"

class ResourceManager
{
protected:
	std::map<const aiMesh*, Model* > m_loadedModels;
	std::map<std::string, Texture* > m_loadedTextures;
	std::map<std::string, std::string > m_loadedFiles;
public:
	ResourceManager();
	~ResourceManager();

	std::vector< Object* > loadObjectsFromFile(std::string path);
	Model* loadModel(const aiScene* scene, const aiMesh* mesh);
	Material* loadMaterial(const aiScene* scene, const aiMesh* mesh, std::string directory);
	Texture* loadTexture(std::string file, std::string directory);

	bool checkModel(const aiMesh* mesh);
	bool checkTexture(std::string path);
	bool checkFile(std::string file);

	void deleteAll();
};

#endif
