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
	std::map<Model*, const aiMesh*> m_loadedMeshes;
	std::map<std::string, Texture* > m_loadedTextures;
	std::map<std::string, std::string > m_loadedFiles;

	Renderable* m_screenFillingTriangle;
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

	const aiMesh* getAssimpMeshForModel(Model* model);

	Renderable* getScreenFillingTriangle();

	void deleteAll();
	const std::map<std::string, std::string>& getLoadedFiles() const;
	const std::map<Model*, const aiMesh*>& getLoadedMeshes() const;
	const std::map<const aiMesh*, Model*>& getLoadedModels() const;
	const std::map<std::string, Texture*>& getLoadedTextures() const;

	void setScreenFillingTriangle(Renderable* screenFillingTriangle);
};

#endif
