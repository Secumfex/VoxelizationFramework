#include "ResourceManager.h"

#include "Utility/DebugLog.h"

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	
}

std::vector<Object* > ResourceManager::loadObjectsFromFile(std::string path)
{
	std::vector<Object* > loadedObjects;

	if ( checkFile(path) )
	{
		return loadedObjects;
	}
	else{
		DEBUGLOG->log("Importing scene");
		Assimp::Importer importer;
		const aiScene* scene = AssimpTools::loadScene(path, importer);

		if (!scene)
 		{
 			return loadedObjects;
		}

		DEBUGLOG->log("Extracting assimp meshes");

		std::vector< const aiMesh* > meshes = AssimpTools::extractMeshesFromScene( scene);

		DEBUGLOG->log("Constructing objects from found meshes : ", meshes.size());
		DEBUGLOG->indent();
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			DEBUGLOG->log(std::string("Constructing model    from mesh" ));
			Model* model = loadModel( scene, meshes[i] );
			DEBUGLOG->log(std::string("Constructing material from mesh" ));
		
			std::string directory = AssimpTools::getDirectoryPath( path );
			Material* mat= loadMaterial(scene, meshes[i], directory);

			Object* object= new Object(model, mat);
		
			loadedObjects.push_back(object);
			DEBUGLOG->log(std::string("Constructing object complete."));
		}

		DEBUGLOG->outdent();
		DEBUGLOG->log("Importing scene complete");
		
		return loadedObjects;
	}
}

Texture* ResourceManager::loadTexture(std::string file, std::string directory)
{
	if ( checkTexture(file) )
	{
		// texture exists already
		DEBUGLOG->log("File has already been loaded: " + file);
		return m_loadedTextures[file];
	}
	else{
		// texture does not yet exist and must be buffered
		DEBUGLOG->log("File has NOT been loaded and will be buffered: " + file);
		Texture* texture = new Texture(file);
		GLuint texturehandle = TextureTools::loadTexture( directory + file );
		texture->setTextureHandle(texturehandle);
		m_loadedTextures[file] = texture;
		return texture;
	}
}

Material* ResourceManager::loadMaterial(const aiScene* scene, const aiMesh* mesh, std::string directory)
{
	aiMaterial* mtl = AssimpTools::loadMaterial(scene, mesh);

	aiString texPath;   // temporary variable to save path of texture

	Material* mat = new Material();

	if(AssimpTools::materialHasTexture( mtl,aiTextureType_DIFFUSE, &texPath ) ){
		Texture* diffuseTex = loadTexture( texPath.C_Str(), directory );
        mat->setTexture( "diffuseTexture", diffuseTex );
	}

	if(AssimpTools::materialHasTexture( mtl, aiTextureType_NORMALS, &texPath ) ){
		Texture* normalTex = loadTexture( texPath.C_Str() , directory);
        mat->setTexture( "normalTexture", normalTex );
	}

	return mat;
}

/* load a single model object from an assimp mesh*/
Model* ResourceManager::loadModel( const aiScene* scene, const aiMesh* mesh )
{
	if ( checkModel(mesh) )
	{
		DEBUGLOG->indent();
		DEBUGLOG->log("Mesh already exists... ");
		return m_loadedModels[mesh];
	}
	else{
		DEBUGLOG->indent();
		DEBUGLOG->log("Mesh does NOT exist and will be buffered... ");
		Model* model = AssimpTools::createModelFromMesh( mesh );
		m_loadedModels[mesh] = model;
		DEBUGLOG->outdent();
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

