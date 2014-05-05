#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Scene/Camera.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>

class RotatingNode : public Updatable, public RenderableNode
{
	void update(float d_t = 0.1)
	{
		rotate( glm::rotate (glm::mat4(1.0f), 0.01f, glm::vec3(0.0f,1.0f,0.0f) ) );
	}
};

class TestRenderPass : public RenderPass
{
protected:
	Camera* m_camera;
public:
	TestRenderPass(Shader* shader, FramebufferObject* fbo)
	{
		m_shader = shader;
		m_fbo = fbo;
		m_viewport = glm::vec4(0,0,800,600);

		if (fbo)
		{
			m_viewport.z = fbo->getWidth();
			m_viewport.w = fbo->getHeight();
		}
		m_camera = new Camera();
		m_camera->setPosition(glm::vec3(0.0f,0.0f,-5.0f));
		m_camera->setCenter(glm::vec3(0.0f,0.0f,0.0f));
		m_camera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
	}

	void preRender()
	{
		glClearColor(0.1f,0.1f,0.1f,1.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void uploadUniforms()
	{
		m_shader->uploadUniform(m_camera->getViewMatrix(),"uniformView");
		m_shader->uploadUniform(m_camera->getProjectionMatrix(), "uniformProjection");
	}
};

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
			RenderableNode* cubeNode1 = new RenderableNode();
			cubeNode1->setObject(daeCube[0]);
			RenderableNode* cubeNode2 = new RenderableNode();

			cubeNode2->setObject(objCube[0]);
			glm::vec3 translate(1.2f,0.0f,1.0f);
			cubeNode2->setModelMatrix( glm::translate(cubeNode2->getModelMatrix(), translate) );

			DEBUGLOG->log("Attaching Nodes to Root Node");
			cubeNode1->setParent( scene->getSceneGraph()->getRootNode() );
			cubeNode2->setParent( scene->getSceneGraph()->getRootNode() );

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
			TestRenderPass* renderPass = new TestRenderPass(shader, 0);
			renderPass->setViewport(0,0,800,600);

			DEBUGLOG->log("Adding Objects to Renderpass");
			renderPass->addRenderable(cubeNode1);
			renderPass->addRenderable(cubeNode2);

			DEBUGLOG->log("Adding Renderpass to Application");
			m_renderManager.addRenderPass(renderPass);
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
