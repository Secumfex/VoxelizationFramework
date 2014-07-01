#include <Application/Application.h>

#include <iostream>

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>

#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>
#include <Voxelization/TextureAtlas.h>
#include <Voxelization/SliceMapRendering.h>

#include <Misc/MiscListeners.h>
#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>
#include <Misc/SimpleSceneTools.h>
#include <Utility/Timer.h>

static bool rotatingBunny = false;

static int voxelGridResolution = 256;

static glm::vec3 lightPosition = glm::vec3(2.5f, 2.5f, 2.5f);

class ComputeShaderApp: public Application {
private:

	CameraRenderPass* createPhongRenderPass( )
	{
		DEBUGLOG->indent();
		Shader* phongPersp= new Shader(SHADERS_PATH "/myShader/phong_uniformLight.vert", SHADERS_PATH "/myShader/phong_backfaceCulling_persp.frag");
		FramebufferObject* fbo = new FramebufferObject(512,512);
		fbo->addColorAttachments(1);

		CameraRenderPass* phongPerspectiveRenderPass = new CameraRenderPass(phongPersp, fbo);
		phongPerspectiveRenderPass->setViewport(0,0,512,512);
		phongPerspectiveRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
		phongPerspectiveRenderPass->addEnable(GL_DEPTH_TEST);
		phongPerspectiveRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
		phongPerspectiveRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);
		phongPerspectiveRenderPass->addUniform( new Uniform<glm::vec3>("uniformLightPos", &lightPosition));

		Camera* camera = new Camera();
		camera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
		camera->setPosition(0.0f,0.0f,5.0f);
		phongPerspectiveRenderPass->setCamera(camera);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding render passes to application");
		m_renderManager.addRenderPass(phongPerspectiveRenderPass);

		return phongPerspectiveRenderPass;
	}

public:
	ComputeShaderApp()
	{
		m_name = "Compute Shader App";
	}
	virtual ~ComputeShaderApp()
	{

	}
	void postInitialize()
	{
		/**************************************************************************************
		 * 								   OBJECT LOADING
		 **************************************************************************************/
		Scene* scene = SimpleScene:: createNewScene ( this );

		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();

			std::vector<Renderable* > renderables;

			RenderableNode* testRoomNode = SimpleScene::loadTestRoomObject( this );
			renderables.push_back(testRoomNode);

			RenderableNode* bunnyNode= SimpleScene::loadObject("/stanford/bunny/blender_bunny.dae" , this);

			DEBUGLOG->log("Scaling bunny up by 25");
			bunnyNode->scale( glm::vec3( 25.0f, 25.0f, 25.0f ) );

			renderables.push_back(bunnyNode);

			DEBUGLOG->log("Attaching objects to scene graph");
			DEBUGLOG->indent();
					if ( rotatingBunny )
					{
						std::pair<Node*, Node*> rotatingNodes = SimpleScene::createRotatingNodes( this, 0.1f, 0.1f);
						rotatingNodes.first->setParent( scene->getSceneGraph()->getRootNode() );

						bunnyNode->setParent( rotatingNodes.second );
					}
					else
					{
						bunnyNode->setParent(scene->getSceneGraph()->getRootNode() );
					}

					testRoomNode->setParent(scene->getSceneGraph()->getRootNode() );
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 									REGULAR RENDERING
		 **************************************************************************************/
		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating perspective phong renderpass");
			CameraRenderPass* phongPerspectiveRenderPass = createPhongRenderPass( );

			DEBUGLOG->log("Adding objects to perspective phong render pass");
			for (unsigned int i = 0; i < renderables.size(); i++)
			{
				phongPerspectiveRenderPass->addRenderable( renderables[i] );
			}

			DEBUGLOG->log("Creating presentation render passes");
			DEBUGLOG->indent();
				DEBUGLOG->log("Creating texture presentation shader");
					Shader* showTexture = new Shader(SHADERS_PATH "/screenspace/screenFill.vert" ,SHADERS_PATH "/screenspace/simpleTexture.frag");

				DEBUGLOG->indent();
					DEBUGLOG->log("Creating phong presentation render pass");
					TriangleRenderPass* showRenderPassPerspective = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassPerspective->setViewport(0,0,512,512);
					showRenderPassPerspective->addClearBit(GL_COLOR_BUFFER_BIT);
					showRenderPassPerspective->addClearBit(GL_DEPTH_BUFFER_BIT);

					showRenderPassPerspective->addUniformTexture( new Texture( phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0) ), "uniformTexture" );

					DEBUGLOG->log("Adding phong framebuffer presentation render pass now");
					// add screen fill render pass now
					m_renderManager.addRenderPass(showRenderPassPerspective);

				DEBUGLOG->outdent();

			DEBUGLOG->outdent();

		DEBUGLOG->outdent();
		/**************************************************************************************
		 * 								COMPUTE SHADER CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Compute Shader");
		DEBUGLOG->indent();

			DEBUGLOG->log("Loading and Compiling simple compute shader program");

			ComputeShader* simpleComputeShader = new ComputeShader(SHADERS_PATH "/compute/simpleCompute.comp");

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/
		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

			DEBUGLOG->log( " -  TODO - TODO - TODO - TODO ");

		DEBUGLOG->outdent();

		/**************************************************************************************
		* 								INPUT CONFIGURATION
		**************************************************************************************/
		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();

			DEBUGLOG->log("Configuring camera movement");
			Camera* movableCam = phongPerspectiveRenderPass->getCamera();
			SimpleScene::configureSimpleCameraMovement(movableCam, this);

			DEBUGLOG->log("Configuring Turntable for root node");
			Turntable* turntable = SimpleScene::configureTurnTable( scene->getSceneGraph()->getRootNode(), this);

			DEBUGLOG->log("Configuring light movement");
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(0.0f,0.0f, 1.0f) ), GLFW_KEY_DOWN, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(0.0f,0.0f, -1.0f) ), GLFW_KEY_UP, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(-1.0f,0.0f, 0.0f) ), GLFW_KEY_LEFT, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(1.0f,0.0f, 1.0f) ), GLFW_KEY_RIGHT, GLFW_PRESS );

		DEBUGLOG->outdent();
	}
};

int main() {
	// configure a little bit
	Application::static_newWindowHeight = 512;
	Application::static_newWindowWidth = 512;

	ComputeShaderApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	return 0;
}
