#include "ResourceManager.h"

#include "Utility/DebugLog.h"

ResourceManager::ResourceManager()
{
	m_screenFillingTriangle = 0;
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
		m_loadedMeshes[model] = mesh;
		DEBUGLOG->outdent();
		return model;
	}
}

const aiMesh* ResourceManager::getAssimpMeshForModel(Model* model)
{
	if( m_loadedMeshes.find(model) != m_loadedMeshes.end())
		{
			return m_loadedMeshes[model];
		}
	else{
		return 0;
	}
}

const std::map<std::string, std::string>& ResourceManager::getLoadedFiles() const {
	return m_loadedFiles;
}

const std::map<Model*, const aiMesh*>& ResourceManager::getLoadedMeshes() const {
	return m_loadedMeshes;
}

const std::map<const aiMesh*, Model*>& ResourceManager::getLoadedModels() const {
	return m_loadedModels;
}

const std::map<std::string, Texture*>& ResourceManager::getLoadedTextures() const {
	return m_loadedTextures;
}

void ResourceManager::setScreenFillingTriangle(
		 Renderable* screenFillingTriangle) {
	m_screenFillingTriangle = screenFillingTriangle;
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

/**
 *	get a screen Filling Triangle Renderable
 */
Renderable* ResourceManager::getScreenFillingTriangle(){
	if(m_screenFillingTriangle == 0){

		Model *triangle = new Model;
		Material *mat = new Material();

		GLuint screenFillVertexArrayHandle;

		glGenVertexArrays(1, &screenFillVertexArrayHandle);
		glBindVertexArray(screenFillVertexArrayHandle);

		GLuint indexBufferHandle;
		glGenBuffers(1, &indexBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

		GLint indices[] = {0, 1, 2};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		GLuint vertexBufferHandle;
		glGenBuffers(1, &vertexBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);

		GLfloat vertices[] = {-1, -1,   3, -1,   -1,  3};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

		triangle->setVAOHandle(screenFillVertexArrayHandle);
		triangle->setNumIndices(3);
		triangle->setNumVertices(3);
		triangle->setNumFaces(1);

		m_screenFillingTriangle = new Object(triangle,mat);

	} return m_screenFillingTriangle;
}

