#ifndef ASSIMPTOOLS_H
#define ASSIMPTOOLS_H

#include "GL/glew.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <vector>

class Model;

namespace AssimpTools
{
	using namespace std;

	/**
	 *	load an assimp scene from a file
	 */
	const aiScene* loadScene(std::string path);

	/**
	 * extract all assimp mesh objects from an assimp scene object
	 */
	std::vector<const aiMesh* > extractMeshesFromScene( const aiScene* scene );

	/**
	 * check whether an aiMaterial has a certain texture type and save the path, if true
	 */
	bool materialHasTexture( aiMaterial* mtl, aiTextureType type, aiString* path);

	/**
	 * load an assimp material object from an assimp mesh object and scene
	 */	
	aiMaterial* loadMaterialFromMeshAndScene(const aiScene* scene, const aiMesh* mesh);


	/**
	 * create a model object from an assimp mesh object
	 */	
	Model* createModelFromMesh(const aiMesh* mesh);

	/**
	 *	load Objects from a file
	 */
//	std::vector<Object* > loadObjects(std::string path);
}

#endif