#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "CustomRenderPasses.h"
#include "SliceMapRendering.h"
#include "Listeners.h"

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

			DEBUGLOG->log("Loading test room dae file");
			DEBUGLOG->indent();
				std::vector< Object* > testRoom=  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/testRoom.dae" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( daeCube );
			scene->addObjects( objCube );
			scene->addObjects( daeBackground );
			scene->addObjects( testRoom );

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

				DEBUGLOG->log("Creating renderable node for test room");
				RenderableNode* testRoomNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				testRoomNode->setObject(testRoom[0]);

			DEBUGLOG->outdent();
		DEBUGLOG->outdent();
		
		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);	

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating phong renderpass");
			Shader* shader = new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong.frag");
			FramebufferObject* fbo = new FramebufferObject(800,600);
			fbo->addColorAttachments(1);

			CameraRenderPass* renderPass = new CameraRenderPass(shader, fbo);
			renderPass->setViewport(0,0,800,600);
			renderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
			renderPass->addEnable(GL_DEPTH_TEST);
			renderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
			renderPass->addClearBit(GL_COLOR_BUFFER_BIT);

			DEBUGLOG->log("Creating slice map renderpass");
			DEBUGLOG->indent();
			SliceMap::SliceMapRenderPass* sliceMapRenderPass = SliceMap::getSliceMapRenderPass();
				sliceMapRenderPass->getCamera()->setPosition(0.0f,4.99f,0.0f);
				sliceMapRenderPass->getCamera()->setCenter( glm::vec3( 0.0f, 0.0f, 0.0f ));
			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding objects to phong render pass");
			sliceMapRenderPass->addRenderable(cubeNode1);
			sliceMapRenderPass->addRenderable(rotatingCubeNode);
			sliceMapRenderPass->addRenderable(backgroundNode);
			sliceMapRenderPass->addRenderable(testRoomNode);

			DEBUGLOG->log("Adding objects to slice map render pass");
			renderPass->addRenderable(cubeNode1);
			renderPass->addRenderable(rotatingCubeNode);
			renderPass->addRenderable(backgroundNode);
			renderPass->addRenderable(testRoomNode);

			DEBUGLOG->log("Creating screen filling triangle render pass");
			DEBUGLOG->indent();
				Shader* showTexture = new Shader(SHADERS_PATH "/screenspace/screenFill.vert" ,SHADERS_PATH "/screenspace/simpleTexture.frag");
				TriangleRenderPass* triangleRenderPass = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
				triangleRenderPass->setViewport(0,0,512,512);
				triangleRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

//				Texture* renderPassTexture = new Texture();
//				renderPassTexture->setTextureHandle(renderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
//				triangleRenderPass->addUniformTexture(renderPassTexture, "uniformTexture");

				Texture* sliceMapRenderPassTexture = new Texture();
				sliceMapRenderPassTexture->setTextureHandle(sliceMapRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
				triangleRenderPass->addUniformTexture(sliceMapRenderPassTexture, "uniformTexture");
			DEBUGLOG->outdent();

			DEBUGLOG->indent();
				DEBUGLOG->log("Setting the slicemap's camera in renderpass 1 for fun...");
				renderPass->setCamera(sliceMapRenderPass->getCamera());
			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding renderpasses to application");
			m_renderManager.addRenderPass(sliceMapRenderPass);
			m_renderManager.addRenderPass(renderPass);
			m_renderManager.addRenderPass(triangleRenderPass);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();
			DEBUGLOG->log("Configuring camera movement");
			Camera* movableCam = sliceMapRenderPass->getCamera();
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 1.0f),GLFW_KEY_W, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_W, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 1.0f),GLFW_KEY_D, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_D, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, -1.0f),GLFW_KEY_S, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_S, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, -1.0f),GLFW_KEY_A, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_A, GLFW_RELEASE);
			DEBUGLOG->log("Adding updatable camera to scene");
			scene->addUpdatable(movableCam);
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
