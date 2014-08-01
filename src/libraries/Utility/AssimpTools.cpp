#include "Utility/AssimpTools.h"

#include <iostream>
#include <fstream>

#include "Resources/Object.h"
#include "Resources/Material.h"
#include "Resources/Model.h"

#include "Utility/DebugLog.h"

namespace AssimpTools
{
	using namespace std;

	std::string directory;

	std::string getDirectoryPath( std::string file )
	{
		// load the desired file
		std::string directory = file.substr( file.find_last_of( '/' ) + 1 );
		directory = file.substr(0, file.length() - directory.length());
		return directory;
	}

	/**
	 *	load an assimp scene from a file
	 *  @param path to the file
	 *  @param importer reference to be used to import scene ( due to scope issues )
	 */
	const aiScene* loadScene(std::string path, Assimp::Importer& Importer)
	{
		
		// load the desired file
		directory = getDirectoryPath(path);


		std::ifstream fin(path.c_str());
		if(!fin.fail()) {
			fin.close();
		}

		else{

			DEBUGLOG->log(std::string ("ERROR : Couldn't open file: ") + path.c_str() );
			cout<<Importer.GetErrorString()<<endl;
			return 0;
		}

		// load "scene" from file
		const aiScene* pScene = Importer.ReadFile( path,
				aiProcess_Triangulate |
				aiProcess_GenSmoothNormals|
//				aiProcess_GenNormals|
				aiProcess_GenUVCoords |
				aiProcess_FlipUVs|
				aiProcess_ValidateDataStructure |
				aiProcess_CalcTangentSpace
		);

		if( !pScene)
		{
			DEBUGLOG->log( "ERROR : import of scene failed." );
			cout<<Importer.GetErrorString()<<endl;
			return 0;
		}

		else{
			DEBUGLOG->log( std::string( "import of scene " ) + path.c_str() + std::string( " succeeded." ) );
			// DEBUGLOG->indent();
			// {
			// 	DEBUGLOG->log("Scene HasMeshes: ", pScene->HasMeshes());
			// 	DEBUGLOG->log("Scene Meshes   : ", pScene->mNumMeshes);
			// 	DEBUGLOG->log("Scene Textures : ", pScene->mNumTextures);
			// }
			// DEBUGLOG->outdent();
			
			return pScene;
		}
	}

	/**
	 * extract all assimp mesh objects from an assimp scene object
	 */
	std::vector<const aiMesh* > extractMeshesFromScene( const aiScene* scene )
	{
		std::vector <const aiMesh* > meshes;

		for (unsigned int n = 0; n < scene->mNumMeshes; ++n)
		{
			meshes.push_back ( scene->mMeshes[n] );
		}
		return meshes;
	}

	/**
	 * check whether an aiMaterial has a certain texture type and save the path, if true
	 */
	bool materialHasTexture( aiMaterial* mtl, aiTextureType type, aiString* path)
	{
		if(AI_SUCCESS == mtl->GetTexture(type, 0, path)){
			return true;
		}
		else{
			return false;
		}
	}

	/**
	 * load an assimp material object from an assimp mesh object and scene
	 */	
	aiMaterial* loadMaterial(const aiScene* scene, const aiMesh* mesh)
	{
		// create material uniform buffer
		aiMaterial *mtl = scene->mMaterials[mesh->mMaterialIndex];

		return mtl;
	}


	/**
	 * create a model object from an assimp mesh object
	 */	
	Model* createModelFromMesh(const aiMesh* mesh)
	{
			Model* model = new Model();

			// buffer handle
			GLuint buffer = 0;
			

			model->setNumVertices(	mesh->mNumVertices );
			model->setNumIndices(	mesh->mNumFaces * 3 );
			model->setNumFaces(		mesh->mNumFaces );

			/****************  BUFFER VERTEX INFORMATION  ******************/

			// generate vertex array buffer
			glGenVertexArrays(1,   &buffer );
			glBindVertexArray(		buffer );
			model->setVAOHandle(	buffer );

			// generate index buffer
			vector< GLuint > indices;

			// read index list
			int indicesCounter = 0;
			for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
				unsigned int i=0;
				for (i = 0; i < mesh->mFaces[t].mNumIndices; ++i) {
					indices.push_back( mesh->mFaces[t].mIndices[i]);
					indicesCounter++;
				}
			}
			buffer = 0;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( GLuint ) * indices.size(), &indices[0], GL_STATIC_DRAW);

			DEBUGLOG->log("Buffered indices        : " , indices.size());
			DEBUGLOG->log("Bytes per entry (uint)  : " ,  sizeof( GLuint ) );

			model->setIndexBufferHandle( buffer );

			// generate vertex position buffer
			buffer = 0;
			if (mesh->HasPositions()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D) * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);

				//vertexLoc wurde hier ersetzt
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
				model->setVertexBufferHandle( buffer );
				DEBUGLOG->log("Buffered vertices       : " , mesh->mNumVertices);
				DEBUGLOG->log("Bytes per entry (vec3)  : " ,  sizeof(aiVector3D) );
			}
			else
			{
				DEBUGLOG->log("WARNING : Object has no VERTICES");
			}

			// generate texture coordinates buffer
			vector <float>texCoords;
			float uv_steps = 1.0f / mesh->mNumVertices;

			if (mesh->HasTextureCoords(0)){
				for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
					texCoords.push_back(mesh->mTextureCoords[0][k].x);
					texCoords.push_back(mesh->mTextureCoords[0][k].y);
				}
			}
			else
			{
				DEBUGLOG->log("WARNING : Object has no UV COORDINATES");
				for(unsigned int k = 0; k < mesh->mNumVertices; ++k){
					texCoords.push_back(k * uv_steps);
					texCoords.push_back(k * uv_steps);
				}
			}

			buffer = 0;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->mNumVertices, &texCoords[0], GL_STATIC_DRAW);
	        
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);
			model->setUvBufferHandle( buffer );

			// generate vertex normals buffer
			buffer = 0;
			if (mesh->HasNormals()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D) * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);

				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);
				model->setNormalBufferHandle( buffer );
			}
			else
			{
				DEBUGLOG->log("WARNING : Object has no NORMALS");
			}

			// generate tangent buffer
			buffer = 0;
			if (mesh->HasTangentsAndBitangents()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mTangents, GL_STATIC_DRAW);

				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 3, GL_FLOAT, 0, 0, 0);
				model->setTangentBufferHandle( buffer );
			}
			else
			{
				DEBUGLOG->log("WARNING : Object has no TANGENTS AND BITANGENTS");
			}

			// unbind buffers
			glBindVertexArray(0);

			return model;
	}

	/**
	 *	load Objects from a file
	 */
	std::vector<Object* > loadObjects(std::string path)
	{
 		std::vector<Object* > loadedObjects;

// 		const aiScene* pScene = loadScene(path);

// 		if (!pScene)
// 		{
// 			return loadedObjects;
// 		}

// 		// assimp meshes to be found in file
// 		std::vector< const aiMesh* > meshes = extractMeshesFromScene( pScene );

// 		// For each mesh
// 		for (unsigned int i = 0; i < meshes.size(); i++)
// 		{
// 			const aiMesh* mesh = meshes[i];

// 			Model* model = createModelFromMesh(mesh);
// 			model->setPath( path );

// 			// create material uniform buffer
// 			Material* material = loadMaterialFromMesh(pScene, mesh);
			
// 			Object* object = new Object(model, material);
// 			loadedObjects.push_back(object);
// 		}

// 		/******************************************************/
		
// 		//TODO
// //		std::cout << "BLENDER FILE ... :" << blenderAxes << std::endl;
// //		if (blenderAxes)
// //		{
// //			std::cout << "BLENDER FILE... rotating Object..." << std::endl;
// //		}
// 		/******************************************************/
		return loadedObjects;
	}
}
