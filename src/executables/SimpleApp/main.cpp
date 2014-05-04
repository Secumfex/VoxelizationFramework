#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>

class ObjectLoadingApp : public Application
{
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading cube dae file");
			DEBUGLOG->indent();
			std::vector< Object* > daeCube = m_resourceManager.loadObjectsFromFile(RESOURCES_PATH "/cube.dae");
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading cube obj file");
			DEBUGLOG->indent();
			std::vector< Object* > objCube =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/cube.obj" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( daeCube );
			scene->addObjects( objCube );

			DEBUGLOG->log("Creating scene graph nodes");
			Node* cubeNode1 = new Node();
			cubeNode1->setObject(daeCube[0]);
			Node* cubeNode2 = new Node();

			cubeNode2->setObject(objCube[0]);
			glm::vec3 translate(0.0f,0.0f,-1.0f);
			cubeNode2->setModelMatrix( glm::translate(cubeNode2->getModelMatrix(), translate) );

			DEBUGLOG->log("Attaching Nodes to Root Node");
			scene->getSceneGraph()->getRootNode()->addChild(cubeNode2);
			scene->getSceneGraph()->getRootNode()->addChild(cubeNode1);

		DEBUGLOG->outdent();
		
		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);	

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();
			DEBUGLOG->log("Compiling simple Shader");
			Shader* shader = new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong.frag");

			DEBUGLOG->log("Creating FramebufferObject");
			FramebufferObject* fbo = new FramebufferObject(800,600);
			fbo->addColorAttachments(1);

			DEBUGLOG->log("Creating Renderpass");
			RenderPass* renderPass = new RenderPass(shader, fbo);

			DEBUGLOG->log("Adding Renderpass to Application");
		DEBUGLOG->outdent();
	}
};

int main(){

	ObjectLoadingApp myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();

	
	return 0;
}
