#include "ResourceManager.h"

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	
}

Texture* ResourceManager::loadTexture(std::string path)
{
	if ( checkTexture(path) )
	{
		// texture exists already
		return m_loadedTextures[path];
	}
	else{
		// texture does not yet exist and must be buffered
		Texture* texture = new Texture(path);
		GLuint texturehandle = TextureTools::loadTexture(path);
		texture->setTextureHandle(texturehandle);
		m_loadedTextures[path] = texture;
		return texture;
	}
}

Material* ResourceManager::loadMaterial(const aiScene* scene, const aiMesh* mesh)
{
	aiMaterial* mtl = AssimpTools::loadMaterialFromMeshAndScene(scene, mesh);

	aiString texPath;   // temporary variable to save path of texture

	Material* mat = new Material();

	if(AssimpTools::materialHasTexture( mtl,aiTextureType_DIFFUSE, &texPath ) ){
		Texture* diffuseTex = loadTexture( texPath.C_Str() );
        mat->setTexture( "diffuseTexture", diffuseTex );
	}

	if(AssimpTools::materialHasTexture( mtl, aiTextureType_NORMALS, &texPath ) ){
		Texture* normalTex = new Texture( texPath.C_Str() );
        mat->setTexture( "normalTexture", normalTex );
	}
}

/* load a single model object from an assimp mesh*/
Model* ResourceManager::loadModel( const aiScene* scene, aiMesh* mesh )
{
	if ( checkModel(mesh) )
	{
		return m_loadedModels[mesh];
	}
	else{
		Model* model = AssimpTools::createModelFromMesh( mesh );
		m_loadedModels[mesh] = model;
		return model;
	}
}

void ResourceManager::deleteAll()
{
	//TODO delete everything
	// for (std::map<std::string,Object*>::iterator it = m_loadedModels.begin(); it != m_loadedModels.end(); ++it)
	// {
	// 	delete it->second;	
	// }
	// for (std::map<std::string,Texture*>::iterator it = m_loadedTextures.begin(); it != m_loadedTextures.end(); ++it)
	// {
	// 	delete it->second;	
	// }
}

/**
 *	check whether a texture has already been loaded before
 *  returns true if texture already buffered, false if not
 */
bool ResourceManager::checkTexture(std::string path)
{
	if ( m_loadedTextures.find(path) != m_loadedTextures.end())
	{
		return true;
	}
	else{
		return false;
	}
}


/**
 *	check whether a model has already been loaded before
 *  returns true if model already buffered, false if not
 */
bool ResourceManager::checkModel(const aiMesh* mesh)
{
	if ( m_loadedModels.find(mesh) != m_loadedModels.end())
	{
		return true;
	}
	else{
		return false;
	}
}

/**
 *	check whether a file has already been loaded before
 *  returns true if file already buffered, false if not
 */
bool ResourceManager::checkFile(std::string file)
{
	if ( m_loadedFiles.find(file) != m_loadedFiles.end())
	{
		return true;
	}
	else{
		return false;
	}
}

