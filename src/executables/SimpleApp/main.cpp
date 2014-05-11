#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "CustomRenderPasses.h"
#include "SliceMapRendering.h"

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>

class RotatingNode : public Updatable, public RenderableNode
{
	private:
	glm::vec3 m_rotationAxis;
	float m_angle;

	public:
	RotatingNode(Node* parent = 0)
	{
		setParent(parent);
		m_angle = 0.0f;
	}
	void update(float d_t = 0.1f)
	{
		rotate( glm::rotate (glm::mat4(1.0f), m_angle, m_rotationAxis ) );
	}
	void setRotationAxis(glm::vec3 rotationAxis)
	{
		m_rotationAxis = rotationAxis;
	}
	void setAngle(float angle)
	{
		m_angle = angle;
	}

};

class ObjectLoadingApp : public Application
{
	public:
	virtual ~ObjectLoadingApp()
	{

	}
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading cube dae file");
			DEBUGLOG->indent();
				std::vector< Object* > daeCube = m_resourceManager.loadObjectsFromFile(RESOURCES_PATH "/cube.dae");
				DEBUGLOG->log("Loading custom Texture for dae Cube");
				daeCube[0]->getMaterial()->setTexture("diffuseTexture", m_resourceManager.loadTexture("cvlogo.png", RESOURCES_PATH "/"));
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading cube obj file");
			DEBUGLOG->indent();
				std::vector< Object* > objCube =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/cube.obj" );
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading background dae file");
			DEBUGLOG->indent();
				std::vector< Object* > daeBackground=  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/background.dae" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( daeCube );
			scene->addObjects( objCube );
			scene->addObjects( daeBackground );

			DEBUGLOG->log("Creating scene graph nodes");
			DEBUGLOG->indent();

				DEBUGLOG->log("Creating renderable node for cube 1");
				RenderableNode* cubeNode1 = new RenderableNode( scene->getSceneGraph()->getRootNode( ) );
				cubeNode1->rotate( glm::rotate (glm::mat4(1.0f), 60.0f, glm::vec3(0.0f,1.0f,0.0f) ) );
				cubeNode1->translate( glm::translate( glm::mat4(1.0f), glm::vec3(1.5f, 0.5f,-1.0f) ) );
				cubeNode1->setObject(daeCube[0]);

				DEBUGLOG->log("Creating node tree for cube 2");
				Node* positionNode = new Node(scene->getSceneGraph()->getRootNode());
				positionNode->translate( glm::translate(glm::mat4(1.0f),  glm::vec3(-1.0f, 2.0f, -1.0f) ) );

				RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
				yAxisRotationNode->setAngle(0.005f);
				yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );

				RotatingNode* rotatingCubeNode = new RotatingNode(yAxisRotationNode);
				rotatingCubeNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
				rotatingCubeNode->setAngle(0.01f);
				rotatingCubeNode->setObject(objCube[0]);

				DEBUGLOG->log("Adding updatable rotation nodes of cube 2 to scene");
				scene->addUpdatable(yAxisRotationNode);
				scene->addUpdatable(rotatingCubeNode);

				DEBUGLOG->log("Creating renderable node for background");
				RenderableNode* backgroundNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				backgroundNode->setObject(daeBackground[0]);

			DEBUGLOG->outdent();

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
			CameraRenderPass* renderPass = new CameraRenderPass(shader, 0);
			renderPass->setViewport(0,0,800,600);
			renderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
			renderPass->addEnable(GL_DEPTH_TEST);
			renderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
			renderPass->addClearBit(GL_COLOR_BUFFER_BIT);

			renderPass->setCamera( new Camera() );
			renderPass->getCamera()->setPosition(glm::vec3(1.0f,3.0f,7.0f));
			renderPass->getCamera()->setCenter(glm::vec3(0.0f,2.0f,0.0f));
			renderPass->getCamera()->setProjectionMatrix(glm::perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f));

			DEBUGLOG->indent();
				DEBUGLOG->log("Setting an Orthographic Camera for fun...");
				Camera* orthocam = new Camera();
				glm::mat4 ortho = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 10.0f);
				orthocam->setProjectionMatrix(ortho);
				orthocam->setPosition(2.5f,2.5f,5.0f);
				renderPass->setCamera(orthocam);
			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating SliceMapRenderpass");
			DEBUGLOG->indent();
			SliceMap::SliceMapRenderPass* sliceMapRenderPass = SliceMap::getSliceMapRenderPass();
				sliceMapRenderPass->getCamera()->setPosition(2.4f,5.4f,2.4f);
				sliceMapRenderPass->getCamera()->setCenter( glm::vec3( -3.0f, 0.0f, -3.0f ));
				sliceMapRenderPass->setFramebufferObject(0);
			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding Objects to Renderpasses");
			sliceMapRenderPass->addRenderable(cubeNode1);
			sliceMapRenderPass->addRenderable(rotatingCubeNode);
			sliceMapRenderPass->addRenderable(backgroundNode);
			renderPass->addRenderable(cubeNode1);
			renderPass->addRenderable(rotatingCubeNode);
			renderPass->addRenderable(backgroundNode);

			DEBUGLOG->log("Adding Renderpasses to Application");
			m_renderManager.addRenderPass(sliceMapRenderPass);
//			m_renderManager.addRenderPass(renderPass);
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
