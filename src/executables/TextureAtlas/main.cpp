#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "Listeners.h"

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>
#include <Voxelization/VoxelGrid.h>

#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>
#include <Utility/Timer.h>

class TextureAtlasBuildingApp : public Application
{
	public:
	virtual ~TextureAtlasBuildingApp()
	{

	}

	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading test room dae file");
			DEBUGLOG->indent();
				std::vector< Object* > testRoom=  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/testRoom.dae" );
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading overlapping geometry file");
			DEBUGLOG->indent();
				std::vector< Object* > overlappingGeometry =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/overlappingGeometry.dae" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( testRoom );
			scene->addObjects( overlappingGeometry );

			DEBUGLOG->log("Creating scene graph nodes");
			DEBUGLOG->indent();

				DEBUGLOG->log("Creating node tree for rotating node");
				Node* positionNode = new Node( scene->getSceneGraph()->getRootNode() );
				positionNode->translate( glm::translate(glm::mat4(1.0f),  glm::vec3(0.0f, 0.0f, 0.0f) ) );

				RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
				yAxisRotationNode->setAngle(0.005f);
				yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );

				RotatingNode* rotatingNode = new RotatingNode(yAxisRotationNode);
				rotatingNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
				rotatingNode->setAngle(0.1f);

				DEBUGLOG->log("Adding updatable rotation nodes to scene");
				scene->addUpdatable(yAxisRotationNode);
				scene->addUpdatable(rotatingNode);

				DEBUGLOG->log("Creating renderable node for overlapping geometry attached to rotating node");
				RenderableNode* overlappingGeometryNode = new RenderableNode(rotatingNode);
				overlappingGeometryNode->setObject(overlappingGeometry[0]);

				DEBUGLOG->log("Creating renderable node for test room");
				RenderableNode* testRoomNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				testRoomNode->scale(glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)));
				testRoomNode->setObject(testRoom[0]);

			DEBUGLOG->outdent();
		DEBUGLOG->outdent();
		
		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);	

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating perspective phong renderpass");
			DEBUGLOG->indent();
				Shader* phongPersp= new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong_backfaceCulling_persp.frag");
				FramebufferObject* fbo2 = new FramebufferObject(512,512);
				fbo2->addColorAttachments(1);

				CameraRenderPass* phongPerspectiveRenderPass = new CameraRenderPass(phongPersp, fbo2);
				phongPerspectiveRenderPass->setViewport(0,0,512,512);
				phongPerspectiveRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
				phongPerspectiveRenderPass->addEnable(GL_DEPTH_TEST);
				phongPerspectiveRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
				phongPerspectiveRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

				Camera* perspectiveCamera = new Camera();
				perspectiveCamera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
				perspectiveCamera->setPosition(0.0f,0.0f,5.0f);
				phongPerspectiveRenderPass->setCamera(perspectiveCamera);
			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding objects to perspective phong render pass");
			phongPerspectiveRenderPass->addRenderable(overlappingGeometryNode);
			phongPerspectiveRenderPass->addRenderable(testRoomNode);

			DEBUGLOG->log("Adding renderpasses to application");
			m_renderManager.addRenderPass(phongPerspectiveRenderPass);

			DEBUGLOG->log("Creating screen filling triangle render passes");
			DEBUGLOG->indent();
				Shader* showTexture = new Shader(SHADERS_PATH "/screenspace/screenFill.vert" ,SHADERS_PATH "/screenspace/simpleTexture.frag");

				DEBUGLOG->indent();
				DEBUGLOG->log("Creating screen filling triangle rendering for perspective phong render pass");
					TriangleRenderPass* showRenderPassPerspective = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassPerspective->setViewport(512,0,512,512);

					Texture* renderPassPerspectiveTexture = new Texture();
					renderPassPerspectiveTexture->setTextureHandle(phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassPerspective->addUniformTexture(renderPassPerspectiveTexture, "uniformTexture");

					m_renderManager.addRenderPass(showRenderPassPerspective);
				DEBUGLOG->outdent();
			DEBUGLOG->outdent();
		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();
			DEBUGLOG->log("Configuring camera movement");
			Camera* movableCam = phongPerspectiveRenderPass->getCamera();

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

			DEBUGLOG->log("Configuring Turntable for root node");
			Turntable* turntable = new Turntable(scene->getSceneGraph()->getRootNode(), &m_inputManager);
			turntable->setSensitivity(0.1f);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);

			DEBUGLOG->log("Adding a Timer to the scene for fun");
			GLFWTimer* glfwTimer = new GLFWTimer(true);
			m_inputManager.attachListenerOnKeyPress(new DebugPrintDoubleListener( glfwTimer->getElapsedTimePtr( ), "Elapsed Time: " ), GLFW_KEY_SPACE, GLFW_PRESS);
			scene->addUpdatable( glfwTimer );

			scene->addUpdatable(turntable);
		DEBUGLOG->outdent();

	}
};

int main(){

	TextureAtlasBuildingApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	
	return 0;
}
