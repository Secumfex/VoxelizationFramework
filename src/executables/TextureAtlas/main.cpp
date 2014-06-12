#include <Application/Application.h>

#include <iostream>


#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>
#include <Voxelization/TextureAtlas.h>

#include <Misc/MiscListeners.h>
#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>
#include <Misc/SimpleSceneTools.h>
#include <Utility/Timer.h>

class TextureAtlasBuildingApp : public Application
{
private:

	TexAtlas::TextureAtlasRenderer* m_textureAtlasRenderer;

	CameraRenderPass* createUVRenderPass( )
	{
		DEBUGLOG->indent();
			Shader* uvShader= new Shader(SHADERS_PATH "/textureAtlas/simpleUV.vert", SHADERS_PATH "/textureAtlas/worldPosition.frag");
			FramebufferObject* fbo = new FramebufferObject(512,512);
			fbo->addColorAttachments(1);

			CameraRenderPass* uvRenderPass = new CameraRenderPass(uvShader, fbo);
			uvRenderPass->setViewport(0,0,512,512);
			uvRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
			uvRenderPass->addEnable(GL_DEPTH_TEST);
			uvRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
			uvRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

			Camera* camera = new Camera();
			camera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
			camera->setPosition(0.0f,0.0f,5.0f);
			uvRenderPass->setCamera(camera);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding renderpass to application");
		m_renderManager.addRenderPass(uvRenderPass);

		return uvRenderPass;
	}

	CameraRenderPass* createPhongRenderPass( )
	{
		DEBUGLOG->indent();
		Shader* phongPersp= new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong_backfaceCulling_persp.frag");
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

		DEBUGLOG->log("Adding renderpasses to application");
		m_renderManager.addRenderPass(phongPerspectiveRenderPass);

		return phongPerspectiveRenderPass;
	}

	TexAtlas::TextureAtlasRenderer* createTextureAtlasRenderer( RenderableNode* renderableNode )
	{
		DEBUGLOG->log("Creating TextureAtlasRenderer for provided renderable node");
		DEBUGLOG->indent();
			m_textureAtlasRenderer = new TexAtlas::TextureAtlasRenderer( renderableNode, 512, 512 );
		DEBUGLOG->outdent();

		return m_textureAtlasRenderer;
	}

public:
	virtual ~TextureAtlasBuildingApp()
	{

	}

	void postInitialize()
	{
		Scene* scene = SimpleScene:: createNewScene ( this );

		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
			std::vector<Renderable* > renderables;

			RenderableNode* testRoomNode = SimpleScene::loadTestRoomObject( this );
			renderables.push_back(testRoomNode);

			RenderableNode* someObjectNode = SimpleScene::loadObject("/stanford/bunny/bunny_blender.dae" , this);
			someObjectNode->scale( glm::vec3( 25.0f,25.0f,25.0f ) );
			renderables.push_back(someObjectNode);

			std::pair<Node*, Node*> rotatingNodes = SimpleScene::createRotatingNodes( this );

			DEBUGLOG->log("Attaching objects to scene graph");
			DEBUGLOG->indent();
				testRoomNode->setParent(scene->getSceneGraph()->getRootNode() );

				rotatingNodes.first->setParent( scene->getSceneGraph()->getRootNode() );
	//			overlappingGeometryNode->setParent( rotatingNodes.second );
				someObjectNode->setParent( rotatingNodes.second );
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();
			TexAtlas::TextureAtlasRenderer* textureAtlasRenderer = createTextureAtlasRenderer( someObjectNode );
		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating perspective phong renderpass");

			CameraRenderPass* phongPerspectiveRenderPass = createPhongRenderPass( );

			DEBUGLOG->log("Adding objects to perspective phong render pass");
			for (unsigned int i = 0; i < renderables.size(); i++)
			{
				phongPerspectiveRenderPass->addRenderable( renderables[i] );
			}

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

			CameraRenderPass* UVRenderPass = createUVRenderPass( );

			DEBUGLOG->log("Adding objects to uv render pass");

			UVRenderPass->addRenderable( someObjectNode );

			DEBUGLOG->log("Creating screen filling triangle render passes");
			DEBUGLOG->indent();
				DEBUGLOG->indent();
				DEBUGLOG->log("Creating screen filling triangle rendering for UV render pass");
					TriangleRenderPass* showRenderPassUV = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassUV->setViewport(512,0,512,512);

					Texture* renderPassUVTexture = new Texture();
					renderPassUVTexture->setTextureHandle(UVRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassUV->addUniformTexture(renderPassUVTexture, "uniformTexture");

//TODO				showRenderPassUV->addUniformTexture(textureAtlasRenderer->getTextureAtlas(), "uniformTexture");

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
			m_inputManager.attachListenerOnKeyPress(new DebugPrintDoubleListener( m_cycleTimer.getElapsedTimePtr(), "Elapsed Time: " ), GLFW_KEY_SPACE, GLFW_PRESS);

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
