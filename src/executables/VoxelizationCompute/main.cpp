#include <Application/Application.h>

#include <iostream>

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>
#include <Rendering/RenderState.h>

#include <Scene/CameraNode.h>
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

// compute shader related information
static int MAX_COMPUTE_WORK_GROUP_COUNT[3];
static int MAX_COMPUTE_WORK_GROUP_SIZE[3];
static int MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
static int MAX_COMPUTE_SHARED_MEMORY_SIZE;

static glm::vec3 lightPosition = glm::vec3(2.5f, 2.5f, 2.5f);

class DispatchVoxelizeComputeShader : public DispatchComputeShaderListener
{
protected:

	std::vector<std::pair<Object*, RenderableNode* > > m_objects;

	glm::mat4 m_voxelizeView;
	glm::mat4 m_voxelizeProjection;
	Texture* p_voxelGridTexture;
	Texture* p_bitMask;
public:
	DispatchVoxelizeComputeShader(ComputeShader* computeShader, std::vector< std::pair<Object*, RenderableNode*> > objects, glm::mat4 voxelizeView, glm::mat4 voxelizeProjection, Texture* voxelGridTexture, Texture* bitMask, int x= 0, int y= 0, int z = 0 )
	: DispatchComputeShaderListener(computeShader, x,y,z)
{
		m_objects = objects;
		m_voxelizeView = voxelizeView;
		m_voxelizeProjection = voxelizeProjection;
		p_voxelGridTexture = voxelGridTexture;
		p_bitMask = bitMask;
}
	void call()
	{
		// use compute program
		p_computeShader->useProgram();

		// unbind output texture
		p_voxelGridTexture->unbindFromActiveUnit();

		// upload output texture
		glBindImageTexture(1,
		p_voxelGridTexture->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_READ_WRITE,							// allow both
		GL_RGBA32F);							// TODO find most suitable format

		// upload bit mask
		glBindImageTexture(2,
		p_bitMask->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_READ_ONLY,
		GL_R32UI
		);


		// dispatch this shader once per object
		for ( unsigned int i = 0; i < m_objects.size(); i++)
		{
			Object* object = m_objects[i].first;
			RenderableNode* objectNode = m_objects[i].second;

			int numVertices = 0;

			if ( object->getModel() )
			{
				RenderState::getInstance()->bindVertexArrayObjectIfDifferent(object->getModel()->getVAOHandle());
				numVertices = object->getModel()->getNumVertices();
			}
			else
			{
				continue;
			}

			glm::mat4 modelMatrix = glm::mat4(1.0f);
			if (objectNode)
			{
				modelMatrix = objectNode->getAccumulatedModelMatrix();
			}

			// upload uniform model matrix
			p_computeShader->uploadUniform( modelMatrix, "uniformModel");

			// upload uniform voxel grid matrices
			p_computeShader->uploadUniform( m_voxelizeView, "uniformVoxelizeView");
			p_computeShader->uploadUniform( m_voxelizeProjection, "uniformVoxelizeProjection");

			// upload uniform vertices amount
			p_computeShader->uploadUniform( numVertices, "uniformNumVertices");

			// TODO do samplers still work !?
			// upload uniform  bit mask
			//p_computeShader->uploadUniform( 8, "uniformBitMask");

			// dispatch as usual
			DispatchComputeShaderListener::call();
		}

		// since models can be voxelized concurrently, put memory barrier after voxelization of all objects instead of inbetween
		glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		glMemoryBarrier( GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT );	// put this aswell, since
	}
};

class ComputeShaderApp: public Application {
private:
	Node* m_cameraParentNode;
	Node* m_objectsNode;
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

		m_cameraParentNode = new Node( m_sceneManager.getActiveScene()->getSceneGraph()->getRootNode() );
		CameraNode* camera = new CameraNode( m_cameraParentNode );
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
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,0 ,  &( MAX_COMPUTE_WORK_GROUP_COUNT[0]) );
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,1 ,  &( MAX_COMPUTE_WORK_GROUP_COUNT[1]) );
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,2 ,  &( MAX_COMPUTE_WORK_GROUP_COUNT[2]) );
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &( MAX_COMPUTE_WORK_GROUP_SIZE[0] ));
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &( MAX_COMPUTE_WORK_GROUP_SIZE[1] ));
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &( MAX_COMPUTE_WORK_GROUP_SIZE[2] ));
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &MAX_COMPUTE_SHARED_MEMORY_SIZE);

		DEBUGLOG->log("max compute work group invocations : ", MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
		DEBUGLOG->log("max compute work group size x      : ", MAX_COMPUTE_WORK_GROUP_SIZE[0]);
		DEBUGLOG->log("max compute work group size y      : ", MAX_COMPUTE_WORK_GROUP_SIZE[1]);
		DEBUGLOG->log("max compute work group size z      : ", MAX_COMPUTE_WORK_GROUP_SIZE[2]);
		DEBUGLOG->log("max compute work group count x     : ", MAX_COMPUTE_WORK_GROUP_COUNT[0]);
		DEBUGLOG->log("max compute work group count y     : ", MAX_COMPUTE_WORK_GROUP_COUNT[1]);
		DEBUGLOG->log("max compute work group count z     : ", MAX_COMPUTE_WORK_GROUP_COUNT[2]);
		DEBUGLOG->log("max compute shared memory size     : ", MAX_COMPUTE_SHARED_MEMORY_SIZE);
	}

public:
	ComputeShaderApp()
	{
		m_name = "Compute Shader App";
		m_objectsNode = 0;
		m_cameraParentNode = 0;
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
			m_objectsNode = new Node( scene->getSceneGraph()->getRootNode() );

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
						rotatingNodes.first->setParent( m_objectsNode );

						bunnyNode->setParent( rotatingNodes.second );
					}
					else
					{
						bunnyNode->setParent( m_objectsNode );
					}

					testRoomNode->setParent( m_objectsNode );
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
		 * 								TEXTURE ATLAS VAO CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring texture atlas objects");
		DEBUGLOG->indent();
			DEBUGLOG->log("TODO TODO TODO TODO TODO "); // TODO
		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								COMPUTE SHADER CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring compute shader");
		DEBUGLOG->indent();

			DEBUGLOG->log("Loading and compiling voxelize compute shader program");
			DEBUGLOG->indent();

			ComputeShader* voxelizeComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeCompute.comp");
			
			GLint numBlocks;
			glGetProgramiv(voxelizeComputeShader->getProgramHandle(), GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

			DEBUGLOG->log( "Active Uniform blocks : ", numBlocks);

			std::vector<std::string> nameList;
			nameList.reserve(numBlocks);

			for ( int blockIx = 0; blockIx < numBlocks; ++blockIx)
			{
				GLint nameLen;
				glGetActiveUniformBlockiv(voxelizeComputeShader->getProgramHandle(),blockIx,GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);

				std::vector<GLchar> name;
				name.resize(nameLen);
				glGetActiveUniformBlockName(voxelizeComputeShader->getProgramHandle(), blockIx, nameLen, 0, &name[0]);

				nameList.push_back(std::string());
				nameList.back().assign(name.begin(),name.end() -1);
			}

			for( unsigned int i = 0; i < nameList.size(); i++)
			{
				DEBUGLOG->log( "Uniform : " + nameList[i] );
			}

			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 							COMPUTE SHADER VOXELIZATION CONFIGURATION
		 **************************************************************************************/


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
			SimpleScene::configureSimpleCameraMovement(movableCam, this, 2.5f);

			DEBUGLOG->log("Configuring Turntable for root node and camera");
			Turntable* turntable = SimpleScene::configureTurnTable( m_objectsNode, this, 0.05f );
			Turntable* turntableCam = SimpleScene::configureTurnTable( m_cameraParentNode, this, 0.05f , GLFW_MOUSE_BUTTON_RIGHT);

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
	FramebufferObject::static_useTexStorage2D = true;

	ComputeShaderApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	return 0;
}
