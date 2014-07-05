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

// static int voxelGridResolution = 256;

// compute shader related information
static int MAX_COMPUTE_WORK_GROUP_COUNT;
static int MAX_COMPUTE_WORK_GROUP_SIZE;
static int MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
static int MAX_COMPUTE_SHARED_MEMORY_SIZE;

static glm::vec3 lightPosition = glm::vec3(2.5f, 2.5f, 2.5f);

class DispatchImageComputeShader : public DispatchComputeShaderListener
{
protected:
	Texture* p_inputTexture;
	Texture* p_outputTexture;
public:
	DispatchImageComputeShader(ComputeShader* computeShader, Texture* inputTexture, Texture* outputTexture, int x= 0, int y= 0, int z = 0 )
	: DispatchComputeShaderListener(computeShader, x,y,z)
{
		p_inputTexture = inputTexture;
		p_outputTexture = outputTexture;
}
	void call()
	{
		p_computeShader->useProgram();

		p_inputTexture->unbindFromActiveUnit();

		// upload input texture
		glBindImageTexture(
				0,
				p_inputTexture->getTextureHandle(),
				0,
				GL_FALSE,
				0,
				GL_READ_ONLY,
				GL_RGBA32F);

		// upload output texture
		glBindImageTexture(1,
				p_outputTexture->getTextureHandle(),
				0,
				GL_FALSE,
				0,
				GL_WRITE_ONLY,
				GL_RGBA32F);



		// dispatch as usual
		DispatchComputeShaderListener::call();

		glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
	}
};

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

	/**
	 * print some information from the GPU about compute shader related stuff
	 */
	void printComputeShaderInformation()
	{
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_COUNT, &MAX_COMPUTE_WORK_GROUP_COUNT);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_SIZE, &MAX_COMPUTE_WORK_GROUP_SIZE);
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &MAX_COMPUTE_SHARED_MEMORY_SIZE);

		DEBUGLOG->log("max compute work group invocations : ", MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
		DEBUGLOG->log("max compute work group size        : ", MAX_COMPUTE_WORK_GROUP_SIZE);
		DEBUGLOG->log("max compute work group count       : ", MAX_COMPUTE_WORK_GROUP_COUNT);
		DEBUGLOG->log("max compute shared memory size     : ", MAX_COMPUTE_SHARED_MEMORY_SIZE);
	}

public:
	ComputeShaderApp()
	{
		m_name = "Compute Shader App";
	}

	virtual ~ComputeShaderApp()
	{

	}

	void postConfigure()
	{
		DEBUGLOG->log("Printing some compute shader information");
		DEBUGLOG->indent();
			printComputeShaderInformation();
		DEBUGLOG->outdent();
	}

	void programCycle()
	{
		Application::programCycle(); 	// regular rendering and image presentation
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

					Texture* phongPerspectiveRenderPassOutput = new Texture( phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0) );
					showRenderPassPerspective->addUniformTexture( phongPerspectiveRenderPassOutput, "uniformTexture" );

					DEBUGLOG->log("Adding phong framebuffer presentation render pass now");
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

			DEBUGLOG->log("Creating a texture to write to ");
			GLuint output_texture;

			glGenTextures(1, &output_texture);
			glBindTexture(GL_TEXTURE_2D, output_texture);
			glTexStorage2D( GL_TEXTURE_2D,
					1,
					GL_RGBA32F,
					512,
					512);
			Texture* outputTexture = new Texture( output_texture );
			Texture* inputTexture  = phongPerspectiveRenderPassOutput;

			DEBUGLOG->log("Creating a dispatch listener for simple compute shader");

			// dispatch 16 shader groups in x and y direction since window is 512x512 in resolution and group size is 32x32
			DispatchImageComputeShader* dispatchListener = new DispatchImageComputeShader(
				simpleComputeShader,
				inputTexture,
				outputTexture,
				16,16,1 );

			DEBUGLOG->log("Setting compute shader output texture as presentation texture");
			DEBUGLOG->indent();

				// remove old uniform, then add new uniform
				showRenderPassPerspective->removeUniformTexture("uniformTexture");
				showRenderPassPerspective->addUniformTexture(outputTexture, "uniformTexture");

			DEBUGLOG->outdent();

			DEBUGLOG->log("Attaching dispatch listener to perspective phong renderpass");

			phongPerspectiveRenderPass->attach(dispatchListener, "POSTRENDERPASS");

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/

//		DEBUGLOG->log("Configuring Voxelization");
//		DEBUGLOG->indent();
//
//			DEBUGLOG->log( " -  TODO - TODO - TODO - TODO ");
//
//		DEBUGLOG->outdent();

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

	// formats to be used whenever a framebuffer object is instantiated
	FramebufferObject::static_internalFormat = GL_RGBA32F;
	FramebufferObject::static_format = GL_RGBA;

	ComputeShaderApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	return 0;
}
