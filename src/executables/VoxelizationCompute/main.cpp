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

/**
 * Renderpass that overlays the slice map ontop of fbo
 */
class OverlayR32UITextureRenderPass : public TriangleRenderPass
{
private:
	Texture* p_texture;
public:
	OverlayR32UITextureRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* triangle, Texture* baseTexture, Texture* texture)
	: TriangleRenderPass(shader, fbo, triangle)
	{
		addUniformTexture(baseTexture, "uniformBaseTexture");
		addEnable(GL_BLEND);
		p_texture = texture;
	}

	virtual void uploadUniforms()
	{
		TriangleRenderPass::uploadUniforms();

		// upload texture
		glBindImageTexture(1,
		p_texture->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_READ_ONLY,
		GL_R32UI
		);
	}
};

/**
 * Clear voxel grid texture with 0
 */
class DispatchClearVoxelGridComputeShader : public DispatchComputeShaderListener
{
protected:
	Texture* p_voxelGridTexture;
public:
	DispatchClearVoxelGridComputeShader(ComputeShader* computeShader, Texture* voxelGridTexture, int x = 0, int y = 0, int z = 0 )
	: DispatchComputeShaderListener(computeShader, x, y, z)
{
		p_voxelGridTexture = voxelGridTexture;
}
	void call()
	{
		p_computeShader->useProgram();

		// unbind output texture
		p_voxelGridTexture->unbindFromActiveUnit();

		// upload clear texture index binding
		glBindImageTexture(0,
		p_voxelGridTexture->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_WRITE_ONLY,						// only write
		GL_R32UI);							// 1 channel 32 bit unsigned int

		// set suitable amount of work groups
		m_num_groups_x = voxelGridResolution / 32 + 1;
		m_num_groups_y = voxelGridResolution / 32 + 1;
		m_num_groups_z = 1;

		// dispatch as usual
		DispatchComputeShaderListener::call();

		// but memory barriers for future shader program
		glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
		glMemoryBarrier( GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT );
	}
};

/**
 * Voxelize scene
 */
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
		GL_R32UI);							// 1 channel 32 bit unsigned int to make sure OR-ing works

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
				// bind positions VBO to shader storage buffer
				glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, object->getModel()->getPositionBufferHandle() );
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

			// set local group amount suitable for object size:
			m_num_groups_x = numVertices / 1024 + 1;
			m_num_groups_y = 1;
			m_num_groups_z = 1;

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
		 * 								VOXELGRID CREATION
		 **************************************************************************************/
		DEBUGLOG->log("Configuring voxel grid");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating voxel grid view matrix");
			// create view matrix
			glm::mat4 voxelizeView = glm::lookAt(
					glm::vec3( 0.0f, 0.0f, 5.0f),	// eye
					glm::vec3( 0.0f , 0.0f , 0.0f ),// center
					glm::vec3( 0.0f, 1.0f, 0.0f) ); // up

			DEBUGLOG->log("Creating voxel grid projection matrix");
			// create projection matrix
			glm::mat4 voxelizeProj = glm::ortho(
					-5.0f, 5.0f,	// left,   right
					-5.0f, 5.0f,	// bottom, top
					0.0f, 10.0f);	// front,  back

			DEBUGLOG->log("Creating voxel grid texture");
			// generate Texture
			GLuint voxelGridHandle;
			glGenTextures(1, &voxelGridHandle);
			glBindTexture(GL_TEXTURE_2D, voxelGridHandle);

			// allocate memory
			glTexStorage2D(
					GL_TEXTURE_2D,			// 2D Texture
					1,						// 1 level
					GL_R32UI,				// 1 channel 32 bit unsigned int
					voxelGridResolution,	// res X
					voxelGridResolution);	// rex Y
			
			// set filter parameters for samplers
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
			// unbind
			glBindTexture(GL_TEXTURE_2D, 0);

			Texture* voxelGridTexture = new Texture(voxelGridHandle);
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

			DEBUGLOG->log("Loading and compiling voxel grid clearing compute shader program");
			DEBUGLOG->indent();

			// shader that clears a r32ui texture with 0
			ComputeShader* clearVoxelGridComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeClearCompute.comp");

			DEBUGLOG->outdent();


			DEBUGLOG->log("Loading and compiling voxel grid filling compute shader program");
			DEBUGLOG->indent();

			// shader that voxelizes an object
			ComputeShader* voxelizeComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeCompute.comp");
			
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 							COMPUTE SHADER VOXELIZATION CONFIGURATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring compute shader dispatcher");
		DEBUGLOG->indent();

			DEBUGLOG->log("Configuring objects to voxelize");
			std::vector<std::pair < Object*, RenderableNode*> > objects;
			objects.push_back( std::pair< Object*, RenderableNode* >( bunnyNode->getObject(), bunnyNode ) );
			objects.push_back( std::pair< Object*, RenderableNode* >( testRoomNode->getObject(), testRoomNode ) );

			DEBUGLOG->log("Creating voxel grid clearing compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchClearVoxelGridComputeShader* dispatchClearVoxelGridComputeShader = new DispatchClearVoxelGridComputeShader(
					clearVoxelGridComputeShader,
					voxelGridTexture
			);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid filling compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchVoxelizeComputeShader* dispatchVoxelizeComputeShader = new DispatchVoxelizeComputeShader(
					voxelizeComputeShader,
					objects,
					voxelizeView,
					voxelizeProj,
					voxelGridTexture,
					SliceMap::get32BitUintMask()
					);

			DEBUGLOG->outdent();

		DEBUGLOG->outdent();
		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

			DEBUGLOG->log( "Attaching voxelize dispatchers to program cycle via VOXELIZE interface");
			// voxelize in every frame
			attach(dispatchClearVoxelGridComputeShader, "VOXELIZE");
			attach(dispatchVoxelizeComputeShader, "VOXELIZE");

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION DISPLAY RENDERING
		 **************************************************************************************/
		DEBUGLOG->log("Configuring display of voxelized scene");
		DEBUGLOG->indent();

			Shader* 			showSliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH "/sliceMap/sliceMapOverLayGLSL4_3.frag");
			TriangleRenderPass* showSliceMap = new OverlayR32UITextureRenderPass(
					showSliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					phongPerspectiveRenderPassOutput,
					voxelGridTexture );

			m_renderManager.addRenderPass( showSliceMap );

		DEBUGLOG->outdent();
		/**************************************************************************************
		* 								INPUT CONFIGURATION
		**************************************************************************************/

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();

			DEBUGLOG->log("Clear and Voxelize Scene on key press : V");
			m_inputManager.attachListenerOnKeyPress( dispatchClearVoxelGridComputeShader, GLFW_KEY_V, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeComputeShader, GLFW_KEY_V, GLFW_PRESS);

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

	void programCycle()
	{
		call("VOXELIZE");				// call listeners attached to voxelize interface

		Application::programCycle(); 	// regular rendering and image presentation
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

	SliceMap::get32BitUintMask();

	myApp.run();

	return 0;
}
