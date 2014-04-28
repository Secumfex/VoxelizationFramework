#ifndef ASSIMPTOOLS_H
#define ASSIMPTOOLS_H

#include "GL/glew.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace AssimpTools
{
	using namespace std;
	aiScene* loadScene(std::string path)
	{
		// create an importer
		Assimp::Importer Importer;
		
		// load the desired file
		std::string directory = path.substr( path.find_last_of( '/' ) + 1 );
	    std::string objName = directory;
		directory = path.substr(0, path.length() - directory.length());


		std::ifstream fin(path.c_str());
		if(!fin.fail()) {
			fin.close();
		}
		else{

			cout<<"ERROR : Couldn't open file: " << path.c_str()<<endl;
			cout<<Importer.GetErrorString()<<endl;
			return 0;
		}

		// load "scene" from file
		const aiScene* pScene = Importer.ReadFile( path,
				aiProcess_Triangulate |
				aiProcess_GenSmoothNormals|
				aiProcess_GenUVCoords |
				aiProcess_FlipUVs|
				aiProcess_ValidateDataStructure |
				aiProcess_CalcTangentSpace
		);

		if( !pScene)
		{
			cout << "ERROR : import of scene failed."
			cout<<Importer.GetErrorString()<<endl;
			return 0;
		}
		else{
			cout<<"Import of scene " <<path.c_str()<<" succeeded."<<endl;
			return aiScene;
		}
	}

	Model* loadModel(std::string path)
	{
		pScene = loadScene(path);
		if (!pScene)
		{
			return 0;
		}

		glm::vec3 aabb_min = glm::vec3(FLT_MAX,FLT_MAX,FLT_MAX);
		glm::vec3 aabb_max = glm::vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

		// For each mesh
		for (unsigned int n = 0; n < pScene->mNumMeshes; ++n)
		{
			const aiMesh* mesh = pScene->mMeshes[n];

			//material and mesh to be filled
			Model* aModel = new Model(path);
			Material* aMat = new Material(path);

			// buffer handle
			GLuint buffer = 0;
			
			//extends of Axis Aligned Bounding Box
			glm::vec3 aabbMax = glm::vec3(INT_MIN, INT_MIN, INT_MIN);
			glm::vec3 aabbMin = glm::vec3(INT_MAX, INT_MAX, INT_MAX);

			//index list for vertex list
			vector<unsigned int> indices;

			int incidesCounter = 0;

			for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
				unsigned int i=0;
				for (i = 0; i < mesh->mFaces[t].mNumIndices; ++i) {
					indices.push_back(mesh->mFaces[t].mIndices[i]);
					incidesCounter++;
				}
			}

			aModel->setNumVertices(mesh->mNumVertices);
			aModel->setNumIndices(mesh->mNumFaces * 3);
			aModel->setNumFaces(pScene->mMeshes[n]->mNumFaces);

			// generate Vertex Array for mesh
			GLuint temp = 0;
			glGenVertexArrays(1,&temp);
			aModel->setVAOHandle(temp);
			glBindVertexArray(aModel->getVAOHandle());


			// buffer for faces (indices)
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

			// buffer for vertex positions
			if (mesh->HasPositions()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D) * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);

				//vertexLoc wurde hier ersetzt
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);

			}

			// buffer for vertex normals
			if (mesh->HasNormals()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);

				// normalLoc wurde hier ersetzt
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);


			}
			if (mesh->HasTangentsAndBitangents()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mTangents, GL_STATIC_DRAW);

				// normalLoc wurde hier ersetzt
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 3, GL_FLOAT, 0, 0, 0);

			}
			if(mesh->HasBones()){
				cout << "HAT ANIMATION"<< endl;
			}

			// buffer for vertex texture coordinates
			vector <float>texCoords;
			float uv_steps = 1.0 / mesh->mNumVertices;

			if (mesh->HasTextureCoords(0)){
				for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
					texCoords.push_back(mesh->mTextureCoords[0][k].x);
					texCoords.push_back(mesh->mTextureCoords[0][k].y);
				}
			}
			else
			{
				for(unsigned int k = 0; k < mesh->mNumVertices; ++k){
					texCoords.push_back(k * uv_steps);
					texCoords.push_back(k * uv_steps);
				}
			}
			std::vector<glm::vec3> vertexPositions;
			for(unsigned int k = 0; k < mesh->mNumVertices; ++k){
				if(aabbMax.x < mesh->mVertices[k].x)
					aabbMax.x = mesh->mVertices[k].x;
				if(aabbMax.y < mesh->mVertices[k].y)
					aabbMax.y = mesh->mVertices[k].y;
				if(aabbMax.z < mesh->mVertices[k].z)
					aabbMax.z = mesh->mVertices[k].z;
				if(aabbMin.x > mesh->mVertices[k].x)
					aabbMin.x = mesh->mVertices[k].x;
				if(aabbMin.y > mesh->mVertices[k].y)
					aabbMin.y = mesh->mVertices[k].y;
				if(aabbMin.z > mesh->mVertices[k].z)
					aabbMin.z = mesh->mVertices[k].z;
			}

			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->mNumVertices, &texCoords[0], GL_STATIC_DRAW);
	        
			//und texCoordLoc wurde dann auch ersetzt
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);


			// unbind buffers
			glBindVertexArray(0);

			// create material uniform buffer
			aiMaterial *mtl = pScene->mMaterials[mesh->mMaterialIndex];
	    

			aiString texPath;   //contains path of texture

			Texture *tex_temp = new Texture();



			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)){
				cout << "Try to find DiffuseMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
	            aMat->setDiffuseMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_AMBIENT, 0, &texPath)){
				cout << "Try to find AmbientMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setAmbientMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_OPACITY, 0, &texPath)){
				cout << "Try to find OpacityMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setOpacityMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_NORMALS, 0, &texPath)){
				//For some Reason HeightMap and NormalMap are switched in Assimp
				cout << "Try to find NormalMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setNormalMap(tex_temp);
			}
			// @todo : find out whether it really is switched or not
			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_HEIGHT, 0, &texPath)){
				//For some Reason HeightMap and NormalMap are switched in Assimp
				cout << "Try to find HeightMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setHeightMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_EMISSIVE, 0, &texPath)){
				cout << "Try to find EmissiveMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setEmissiveMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_SPECULAR, 0, &texPath)){
				cout << "Try to find SpecularMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setSpecularMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_REFLECTION, 0, &texPath)){
				cout << "Try to find ReflectionMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setReflectionMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_SHININESS, 0, &texPath)){
				cout << "Try to find ShininessMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setShininessMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DISPLACEMENT, 0, &texPath)){
				cout << "Try to find DisplacementMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setDisplacementMap(tex_temp);
			}

			if(AI_SUCCESS == mtl->GetTexture(aiTextureType_LIGHTMAP, 0, &texPath)){
				cout << "Try to find LightMap: " << texPath.C_Str() << endl;
				tex_temp = new Texture(directory + texPath.C_Str());
				aMat->setLightMap(tex_temp);
			}

	        aiString name;
			if(AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_NAME, &name)){
				std::string matName = name.C_Str();
				matName = matName.substr( matName.find_last_of( '/' ) + 1 );


	            std::cout<<"\nName des Materials: "<<matName<<endl;

				aMat->setName(matName);
			}
	        else{
				aMat->setName("genericMaterial");
	        }
	        
	        
	        /* try to generate material by name */
	        
			GraphicsComponent* gc=new GraphicsComponent(aModel
,aMat);
	        MaterialManager* mm= MaterialManager::getInstance();



	        if(aMat->getName().find("custom") != std::string::npos)
	        {
	        	cout<<"\nRead from mtl\n";

		        float c[4];
		        
		 		// diffuse
				set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
				aiColor4D diffuse;
				if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse)){
				color4_to_float4(&diffuse, c);
				}
				aMat->setDiffuse(glm::vec3(c[0], c[1], c[2]));


		        // ambient
				set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
				aiColor4D ambient;
				if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
					color4_to_float4(&ambient, c);
				//memcpy(aMat.ambient, c, sizeof(c));
				aMat->setAmbient(glm::vec3(ambient.r, ambient.g, ambient.b));

		        // specular

				set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);

				aiColor4D specular;
				if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
					color4_to_float4(&specular, c);
				//memcpy(aMat.specular, c, sizeof(c));
				aMat->setSpecular(glm::vec3(specular.r, specular.g, specular.b));

		        // emission
				set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
				aiColor4D emission;
				if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
					color4_to_float4(&emission, c);
		        //memcpy(aMat.emissive, c, sizeof(c));
				aMat->setEmission(glm::vec3(emission.r, emission.g, emission.b));

		        // shininess
		        float shininess = 0.0;
				//unsigned int max;
				if(AI_SUCCESS != mtl->Get(AI_MATKEY_SHININESS, shininess))
					shininess = 50.0;


				aMat->setShininess(1.0f);
				//shininess/1000.0f
	        }
	        else{
		        try {
		            mm->makeMaterial(aMat->getName(),gc);
		            }
		        catch (string param){
		           cout<<"\nFAILED: generate material by name";
		            }
	        }

			//Mesh und Material wird gelesen und in neuer GraphicsComponent gespeichert
			gc->setGhostObject(aabbMin, aabbMax);

			virtualObject->addGraphicsComponent(gc);

			if(aabbMin.x < aabb_min.x)
			 	aabb_min.x = aabbMin.x;
			 if(aabbMin.y < aabb_min.y)
			 	aabb_min.y = aabbMin.y;
			 if(aabbMin.z < aabb_min.z)
			 	aabb_min.z = aabbMin.z;
			 if(aabbMax.x > aabb_max.x)
			 	aabb_max.x = aabbMax.x;
			 if(aabbMax.y > aabb_max.y)
			 	aabb_max.y = aabbMax.y;
			 if(aabbMax.z > aabb_max.z)
			 	aabb_max.z = aabbMax.z;
		}

		glm::vec3 boxValue = aabb_max - aabb_min;
		float width = boxValue.x;
		float height = boxValue.y;
		float depth = boxValue.z;

		float x = aabb_min.x + width / 2.0f;
		float y = aabb_min.y + height / 2.0f;
		float z = aabb_min.z + depth / 2.0f;

		glm::vec3 normal;
		normal.x= aabb_min.y*aabb_max.z - aabb_min.z*aabb_max.y;
		normal.y= aabb_min.z*aabb_max.x - aabb_min.x*aabb_max.z;
		normal.z= aabb_min.x*aabb_max.y - aabb_min.y*aabb_max.x;

	//	std::cout << "max: " << aabb_max.x << " , "<< aabb_max.y << " , "<< aabb_max.z << std::endl;
	//	std::cout << "min: " << aabb_min.x << " , "<< aabb_min.y << " , "<< aabb_min.z << std::endl;

		switch(bodyType)
		{
			case CUBE:		virtualObject->setPhysicsComponent(width, height, depth, x, y, z, mass, collisionFlag);
				break;
			case PLANE:		virtualObject->setPhysicComponent(x, y, z, normal, mass, collisionFlag);
				break;
			case SPHERE:	virtualObject->setPhysicsComponent((aabb_max.x-aabb_min.x)/2.0, (aabb_max.x-aabb_min.x)/2.0+aabb_min.x, (aabb_max.y-aabb_min.y)/2.0+aabb_min.y, (aabb_max.z-aabb_min.z)/2.0+aabb_min.z, mass, collisionFlag);
				break;
			case OTHER:		virtualObject->setPhysicsComponent(aabb_min, aabb_max, mass, collisionFlag);
				break;
		}

		/******************************************************/
		std::cout << "BLENDER FILE ... :" << blenderAxes << std::endl;
		if (blenderAxes)
		{
			std::cout << "BLENDER FILE... rotating Object..." << std::endl;
			btRigidBody* rigidBody = virtualObject->getPhysicsComponent()->getRigidBody();
			btMotionState* motion = rigidBody->getMotionState();

			btTransform worldTrans;
			worldTrans.setIdentity();
			worldTrans.setRotation( btQuaternion( btVector3(1.0f, 0.0f, 0.0f), ( (-1.0f) * PI ) / 2.0f));
			std::cout << "BLENDER FILE... updating ModelMatrix" << std::endl;

			motion->setWorldTransform(worldTrans);

			virtualObject->updateModelMatrixViaPhysics();
		}
		/******************************************************/
		return virtualObject;
	}
}

#endif