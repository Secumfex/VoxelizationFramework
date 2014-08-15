#include "ResourceManager.h"

#include "Utility/DebugLog.h"

ResourceManager::ResourceManager()
{
	m_screenFillingTriangle = 0;
	m_cube = 0;
}

ResourceManager::~ResourceManager()
{
	
}

std::vector<Object* > ResourceManager::loadObjectsFromFile(std::string path)
{
	std::vector<Object* > loadedObjects;

	if ( checkFile(path) )
	{
		// retrieve file
		DEBUGLOG->log("ERROR : File already loaded before.");
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

/*save the vertex list of a mesh in the map as a corresponding vector to the model*/
void ResourceManager::saveVertexList(Model* model, const aiMesh* mesh)
{
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		m_loadedMeshes[model].push_back(glm::vec4 ( mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f));
	}
}

/*save the vertex list of a mesh in the map as a corresponding vector to the model*/
void ResourceManager::saveFacesList(Model* model, const aiMesh* mesh)
{
	// for every face
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		// current Face
		aiFace currentFace = mesh->mFaces[i];

		// read face vertices indices
		std::vector< unsigned int > faceIndices;
		for (unsigned int k = 0; k < currentFace.mNumIndices; k++)
		{
			faceIndices.push_back( currentFace.mIndices[k] );
		}

		// push back a vector of indices for this face
		m_loadedMeshesFaces[model].push_back( faceIndices );
	}
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

		saveVertexList ( model, mesh );
		saveFacesList	(model, mesh );

		DEBUGLOG->outdent();
		return model;
	}
}

const std::vector<glm::vec4>& ResourceManager::getAssimpMeshForModel(Model* model)
{
	if( m_loadedMeshes.find(model) != m_loadedMeshes.end())
		{
			return m_loadedMeshes[model];
		}
	else{
		return std::vector<glm::vec4>();
	}
}

const std::vector<std::vector< unsigned int > >& ResourceManager::getAssimpMeshFacesForModel(Model* model)
{
	if( m_loadedMeshesFaces.find(model) != m_loadedMeshesFaces.end())
		{
			return m_loadedMeshesFaces[model];
		}
	else{
		return std::vector< std::vector< unsigned int> >();
	}
}

const std::map<std::string, std::string>& ResourceManager::getLoadedFiles() const {
	return m_loadedFiles;
}

const std::map<Model*, std::vector <glm::vec4> >& ResourceManager::getLoadedMeshes() const {
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
		triangle->setIndexBufferHandle( indexBufferHandle );

		GLint indices[] = {0, 1, 2};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		GLuint vertexBufferHandle;
		glGenBuffers(1, &vertexBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);
		triangle->setVertexBufferHandle(vertexBufferHandle);

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

/**
 * get a quad with side length = 1.0
 */
Object* ResourceManager::getQuad() {
	if ( m_quad == 0)
	{
		Model *quad = new Model();
		Material *mat = new Material();

		GLuint quadVertexArrayHandle;

		glGenVertexArrays( 1, &quadVertexArrayHandle );
		glBindVertexArray( quadVertexArrayHandle );

		GLuint indexBufferHandle;
		glGenBuffers(1, &indexBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

		GLint indices[] = { 0, 1, 2, 2, 3, 0 };
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ) , indices,
				GL_STATIC_DRAW);

		GLuint vertexBufferHandle;
		glGenBuffers(1, &vertexBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);

		GLfloat vertices[] = {
				-0.5f, -0.5f, 0.0f,	    //0
				-0.5f, 0.5f, 0.0f,		//1
				0.5f, 0.5f, 0.0f,		//2
				0.5f, -0.5f, 0.0f,		//3
				};

		glBufferData(GL_ARRAY_BUFFER, sizeof( vertices ), vertices,
				GL_STATIC_DRAW);

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

		GLuint uvBufferHandle;
		glGenBuffers(1, &uvBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferHandle);

		GLfloat uvCoords[] = {
				0.0f, 0.0f,	      //0
				0.0f, 1.0f,	      //1
				1.0f, 1.0f,	      //2
				1.0f, 0.0f,       //3
				};

		glBufferData(GL_ARRAY_BUFFER, sizeof( uvCoords ), uvCoords,
				GL_STATIC_DRAW);

		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );

		quad->setVertexBufferHandle(vertexBufferHandle);
		quad->setIndexBufferHandle(indexBufferHandle);
		quad->setVAOHandle(quadVertexArrayHandle);
		quad->setNumIndices(sizeof( indices ) / sizeof( GLint ));
		quad->setNumVertices(sizeof( vertices )/ sizeof( GLfloat ));
		quad->setNumFaces(2);

		Object* quadObject = new Object(quad, mat);
		quadObject->setRenderMode( GL_TRIANGLES );
		m_quad = quadObject;

	}
	return m_quad;
}

/**
 *	get a cube with side length = 1.0
 */
Object* ResourceManager::getCube(){
	if(m_cube == 0){

		Model *cube = new Model();
		Material *mat = new Material();

		GLuint cubeVertexArrayHandle;

		glGenVertexArrays(1, &cubeVertexArrayHandle);
		glBindVertexArray(cubeVertexArrayHandle);

		GLuint indexBufferHandle;
		glGenBuffers(1, &indexBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

		GLint indices[] = {
				0, 1,
				1, 2 ,
				2, 3,
				0, 3,
				0, 4,
				3, 7,
				4, 7,
				2, 6,
				4, 5,
				1, 5,
				5, 6,
				6, 7};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		GLuint vertexBufferHandle;
		glGenBuffers(1, &vertexBufferHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);

		GLfloat vertices[] = {
				-0.5f, -0.5f,  0.5f,	//0
				-0.5f, 0.5f,  0.5f,		//1
				0.5f, 0.5f,  0.5f,		//2
				0.5f, -0.5f,  0.5f,		//3

				-0.5f, -0.5f,  -0.5f,	//4
				-0.5f, 0.5f,  -0.5f,	//5
				0.5f, 0.5f,  -0.5f,		//6
				0.5f, -0.5f,  -0.5f		//7
				};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


		cube->setVertexBufferHandle( vertexBufferHandle );
		cube->setIndexBufferHandle( indexBufferHandle );
		cube->setVAOHandle(cubeVertexArrayHandle);
		cube->setNumIndices(sizeof(indices) / sizeof( GLint ) );
		cube->setNumVertices(sizeof(vertices ) / sizeof( GLfloat ) );
		cube->setNumFaces(6);

		Object* cubeObject  = new Object(cube,mat);
		cubeObject->setRenderMode(GL_LINES);
		m_cube = cubeObject;

	} return m_cube;
}

Model* ResourceManager::generateVoxelGridModel( int width, int height, int depth, float cellSize )
{
//	DEBUGLOG->log("Creating Voxel Grid Model with parameters: ");
//	DEBUGLOG->indent();
//		DEBUGLOG->log("width   : ", width);
//		DEBUGLOG->log("height  : ", height);
//		DEBUGLOG->log("depth   : ", depth);
//		DEBUGLOG->log("cellSize: ", cellSize);
//	DEBUGLOG->outdent();

	Model *voxelGrid = new Model();
	Material *mat = new Material();

	GLuint vertexArrayHandle;
	glGenVertexArrays(1, &vertexArrayHandle);
	glBindVertexArray(vertexArrayHandle);

	// create an entire grid
	std::vector< float > vertices;
	std::vector< unsigned int > indices;

	unsigned int index = 0;

	// all vertical lines
	for( int w = 0; w <= width; w++ )
	{
		for ( int d = 0; d <= depth; d++ )
		{
			vertices.push_back(cellSize * (float) w );
			vertices.push_back(0.0f);
			vertices.push_back(cellSize * (float) d );

			indices.push_back( index );	// start vertex index
			index++;

			vertices.push_back( cellSize * (float) w );
			vertices.push_back( cellSize * (float) height);
			vertices.push_back( cellSize * (float) d );

			indices.push_back( index );	// end vertex index
			index++;
		}
	}

	// all horizontal lines
	for(int d = 0; d <= depth; d++)
	{
		for (int h = 0; h <= height; h++)
		{
			vertices.push_back( 0.0f );
			vertices.push_back( cellSize * (float) h );
			vertices.push_back( cellSize * (float) d );

			indices.push_back( index );	// start vertex index
			index++;

			vertices.push_back( cellSize * width );
			vertices.push_back( cellSize * (float) h );
			vertices.push_back( cellSize * (float) d );

			indices.push_back( index );	// end vertex index
			index++;
		}
	}

	// all lines into space lines
	for( int w = 0; w <= width; w++ )
	{
		for ( int h = 0; h <= height; h++ )
		{
			vertices.push_back( cellSize * (float) w );
			vertices.push_back( cellSize * (float) h );
			vertices.push_back( 0.0f );

			indices.push_back( index );	// start vertex index
			index++;

			vertices.push_back( cellSize * (float) w );
			vertices.push_back( cellSize * (float) h );
			vertices.push_back( cellSize * (float) depth );

			indices.push_back( index );	// end vertex index
			index++;
		}
	}

	// buffer indices
	GLuint indexBufferHandle;
	glGenBuffers(1, &indexBufferHandle);
	voxelGrid->setIndexBufferHandle( indexBufferHandle );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW );


	// buffer vertices
	GLuint vertexBufferHandle;
	glGenBuffers(1, &vertexBufferHandle);
	voxelGrid->setVertexBufferHandle( vertexBufferHandle );
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(   0,    // attribute 0. No particular reason for 0, but must match the layout in the shader.
			   3,                  // size
			   GL_FLOAT,           // type
			   GL_FALSE,           // normalized?
			   0,                  // stride
			   0          // array buffer offset
			   );

	glBindVertexArray(0);

	DEBUGLOG->indent();
//		DEBUGLOG->log("generated indices : ", indices.size());
//		DEBUGLOG->log("generated vertices : ", vertices.size() / 3);

		voxelGrid->setNumIndices(indices.size());
		voxelGrid->setNumVertices(vertices.size() / 3);
		voxelGrid->setNumFaces(indices.size() / 2);

		voxelGrid->setVAOHandle(vertexArrayHandle);

	DEBUGLOG->outdent();
	return voxelGrid;
}
