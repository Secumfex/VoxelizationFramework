#include <Application/Application.h>

#include <iostream>

#include "Listeners.h"

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>
#include <Voxelization/VoxelGrid.h>

#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>
#include <Misc/SimpleSceneTools.h>
#include <Utility/Timer.h>

class TextureAtlasBuildingApp : public Application
{
private:

	CameraRenderPass* createUVRenderPass( std::vector< Renderable* > renderables )
	{
		DEBUGLOG->indent();
			Shader* uvShader= new Shader(SHADERS_PATH "/textureAtlas/simpleUV.vert", SHADERS_PATH "/textureAtlas/worldPosition.frag");
			FramebufferObject* fbo = new FramebufferObject(512,512);
			fbo->addColorAttachments(1);

			CameraRenderPass* uvRenderPass = new CameraRenderPass(uvShader, fbo);
			uvRenderPass->setViewport(0,0,512,512);
			uvRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
			uvRenderPass->addEnable(GL_DEPTH_TEST);
			uvRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
			uvRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

			Camera* camera = new Camera();
			camera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
			camera->setPosition(0.0f,0.0f,5.0f);
			uvRenderPass->setCamera(camera);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to uv render pass");
		for (unsigned int i = 0; i < renderables.size(); i++)
		{
			uvRenderPass->addRenderable( renderables[i] );
		}

		DEBUGLOG->log("Adding renderpass to application");
		m_renderManager.addRenderPass(uvRenderPass);

		return uvRenderPass;
	}

	CameraRenderPass* createPhongRenderPass(std::vector< Renderable* > renderables)
	{
		DEBUGLOG->indent();
		Shader* phongPersp= new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/textureAtlas/worldPosition_backfaceCulling.frag");
		FramebufferObject* fbo = new FramebufferObject(512,512);
		fbo->addColorAttachments(1);

		CameraRenderPass* phongPerspectiveRenderPass = new CameraRenderPass(phongPersp, fbo);
		phongPerspectiveRenderPass->setViewport(0,0,512,512);
		phongPerspectiveRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
		phongPerspectiveRenderPass->addEnable(GL_DEPTH_TEST);
		phongPerspectiveRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
		phongPerspectiveRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

		Camera* camera = new Camera();
		camera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
		camera->setPosition(0.0f,0.0f,5.0f);
		phongPerspectiveRenderPass->setCamera(camera);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to perspective phong render pass");
		for (unsigned int i = 0; i < renderables.size(); i++)
		{
			phongPerspectiveRenderPass->addRenderable( renderables[i] );
		}

		DEBUGLOG->log("Adding renderpasses to application");
		m_renderManager.addRenderPass(phongPerspectiveRenderPass);

		return phongPerspectiveRenderPass;
	}

public:
	virtual ~TextureAtlasBuildingApp()
	{

	}

	void postInitialize()
	{
		DEBUGLOG->log("Creating a scene instance");
		DEBUGLOG->indent();
		Scene* scene = new Scene();
		DEBUGLOG->outdent();

		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);

		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();

			RenderableNode* testRoomNode = SimpleScene::loadTestRoomObject( this );

			testRoomNode->setParent(scene->getSceneGraph()->getRootNode() );

			RenderableNode* overlappingGeometryNode = SimpleScene::loadOverlappingGeometry( this );
			std::pair<Node*, Node*> rotatingNodes = SimpleScene::createRotatingNodes( this );

			rotatingNodes.first->setParent( scene->getSceneGraph()->getRootNode() );
			overlappingGeometryNode->setParent( rotatingNodes.second );

		DEBUGLOG->outdent();

		DEBUGLOG->log("Attaching objects to scene graph");
		DEBUGLOG->indent();

				std::vector<Renderable* > renderables;
				renderables.push_back(testRoomNode);
				renderables.push_back(overlappingGeometryNode);

		DEBUGLOG->outdent();
		

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating perspective phong renderpass");

			CameraRenderPass* phongPerspectiveRenderPass = createPhongRenderPass( renderables );

			DEBUGLOG->log("Creating screen filling triangle render passes");
			DEBUGLOG->indent();
				Shader* showTexture = new Shader(SHADERS_PATH "/screenspace/screenFill.vert" ,SHADERS_PATH "/screenspace/simpleTexture.frag");

				DEBUGLOG->indent();
				DEBUGLOG->log("Creating screen filling triangle rendering for perspective phong render pass");
					TriangleRenderPass* showRenderPassPerspective = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassPerspective->setViewport(0,0,512,512);

					Texture* renderPassPerspectiveTexture = new Texture();
					renderPassPerspectiveTexture->setTextureHandle(phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassPerspective->addUniformTexture(renderPassPerspectiveTexture, "uniformTexture");

					m_renderManager.addRenderPass(showRenderPassPerspective);
				DEBUGLOG->outdent();
			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating perspective phong renderpass");

			CameraRenderPass* UVRenderPass = createUVRenderPass( renderables );

			DEBUGLOG->log("Creating screen filling triangle render passes");
			DEBUGLOG->indent();
				DEBUGLOG->indent();
				DEBUGLOG->log("Creating screen filling triangle rendering for UV render pass");
					TriangleRenderPass* showRenderPassUV = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassUV->setViewport(512,0,512,512);

					Texture* renderPassUVTexture = new Texture();
					renderPassUVTexture->setTextureHandle(UVRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassUV->addUniformTexture(renderPassUVTexture, "uniformTexture");

					m_renderManager.addRenderPass(showRenderPassUV);
				DEBUGLOG->outdent();
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();
			DEBUGLOG->log("Configuring camera movement");
			Camera* movableCam = phongPerspectiveRenderPass->getCamera();
			SimpleScene::configureSimpleCameraMovement(movableCam, this);

			DEBUGLOG->log("Configuring Turntable for root node");
			Turntable* turntable = new Turntable(scene->getSceneGraph()->getRootNode(), &m_inputManager);
			turntable->setSensitivity(0.05f);
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
