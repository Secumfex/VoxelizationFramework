#include <Application/Application.h>

#include <iostream>
#include <stdlib.h>
#include <time.h>

#include <windows.h>

#include <Rendering/Shader.h>
#include <Rendering/ShaderInfo.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>
#include <Rendering/RenderState.h>

#include <Scene/CameraNode.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>
#include <Voxelization/TextureAtlas.h>
#include <Voxelization/SliceMapRendering.h>

#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>
#include <Misc/SimpleSceneTools.h>
#include <Utility/Timer.h>

#include <Input/Frame.h>
//#include <Input/InputField.h>

#include "VoxelGridTools.h"
#include "VoxelizerTools.h"

#define PI 3.14159265f
#define DEG_TO_RAD 0.01745329f

static int   VISIBLE_TEXTURE_LEVEL = 0;

static bool  VOXELIZE_REGULAR_ACTIVE = true;
static bool  VOXELIZE_ACTIVE = true;

static int   TEXATLAS_RESOLUTION  = 512;

static bool  ENABLE_VOXELGRID_OVERLAY = true;

static float VOXELGRID_WIDTH = 5.5f;
static float VOXELGRID_HEIGHT = 5.5f;

//static float VOXELGRID_WIDTH = 1.5f;
//static float VOXELGRID_HEIGHT = 1.5f;

static glm::vec3 LIGHT_POSITION = glm::vec3(6.0f, 6.0f, 6.0f);
static bool      USE_ORTHOLIGHTSOURCE = true;
static glm::mat4 LIGHT_ORTHO_PROJECTION = glm::ortho( -6.0f, 6.0f, -6.0f, 6.0f, 0.0f, 20.0f);
static float     LIGHT_ANGLE_RAD = 60.0f * DEG_TO_RAD;
static glm::mat4 LIGHT_PERSPECTIVE_PROJECTION = glm::perspective( LIGHT_ANGLE_RAD / DEG_TO_RAD, 1.0f, 1.0f, 20.0f);

static bool  ENABLE_BACKFACE_CULLING = true;
static bool  USE_ORTHOCAM = false;
static glm::mat4 ORTHOCAM_PROJECTION;
static glm::mat4 PERSPECTIVECAM_PROJECTION = glm::perspective( 65.0f, 1.0f, 0.1f, 35.0f);

static float BACKGROUND_TRANSPARENCY = 0.3;

static int RENDER_FRAME_WIDTH = 512;
static int RENDER_FRAME_HEIGHT = 512;
static int GUI_FRAME_WIDTH = 512;
static int GUI_FRAME_HEIGHT = 512;

/**
 * Renderpass that overlays the slice map ontop of fbo
 */
class OverlayR32UITextureRenderPass : public TriangleRenderPass
{
public:
	VoxelGridGPU* p_voxelGrid;
	int* p_level;

public:
	OverlayR32UITextureRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* triangle, Texture* baseTexture, VoxelGridGPU* voxelGrid, int* level = 0)
	: TriangleRenderPass(shader, fbo, triangle)
	{
		addUniformTexture(baseTexture, "uniformBaseTexture");
		addEnable(GL_BLEND);
		p_voxelGrid = voxelGrid;

		if ( level == 0 )
		{
			p_level = new int( 0 );
		}
		else
		{
			p_level = level;
		}
	}


	virtual void uploadUniforms()
	{
		TriangleRenderPass::uploadUniforms();

		// upload texture
		glBindImageTexture(0,           // image unit binding
		p_voxelGrid->handle,  // texture
		VISIBLE_TEXTURE_LEVEL,			// texture level
		GL_FALSE,                       // layered
		0,                              // layer
		GL_READ_ONLY,                   // access
		GL_R32UI                        // format
		);
	}

	virtual void postRender()
	{
		TriangleRenderPass::postRender();
		//unbind texture
		glBindImageTexture(0, 0, 0,
		GL_FALSE, 0,
		GL_READ_ONLY,
		GL_R32UI);
	}
};

/**
 * Renderpass that overlays the slice map ontop of fbo using gbuffer info
 */
class ProjectSliceMapRenderPass : public TriangleRenderPass
{
public:
	VoxelGridGPU* p_voxelGrid;
	Texture* p_bitMask;
	Uniform< glm::mat4 >* m_uniformWorldToVoxel;
public:
	ProjectSliceMapRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* triangle, Texture* baseTexture, VoxelGridGPU* voxelGrid, Texture* bitMask, Texture* positionMap, glm::mat4* viewMatrix)
	: TriangleRenderPass(shader, fbo, triangle)
	{
		addUniformTexture(baseTexture, "uniformBaseTexture");
		addUniformTexture(positionMap, "uniformPositionMap");

		m_uniformWorldToVoxel = new Uniform<glm::mat4>("uniformWorldToVoxel", &voxelGrid->worldToVoxel);

		addUniform( m_uniformWorldToVoxel );
		addUniform( new Uniform< glm::mat4 >("uniformView", viewMatrix ) );

		p_voxelGrid = voxelGrid;
		p_bitMask = bitMask;
	}

	virtual void uploadUniforms()
	{
		TriangleRenderPass::uploadUniforms();

		// upload texture
		glBindImageTexture(0,
		p_voxelGrid->handle,
		0,
		GL_FALSE,
		0,
		GL_READ_ONLY,
		GL_R32UI
		);

		// upload bitmask
		glBindImageTexture(1,
		p_bitMask->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_READ_ONLY,
		GL_R32UI
		);

	}

	virtual void postRender()
	{
		TriangleRenderPass::postRender();

		// unbind textures
		glBindImageTexture(0, 0, 0,
		GL_FALSE, 0,
		GL_READ_ONLY,
		GL_R32UI);

		glBindImageTexture(1, 0, 0,
		GL_FALSE, 0,
		GL_READ_ONLY,
		GL_R32UI);
	}
};



class ComputeShaderApp: public Application {
private:
	Node* m_lightParentNode;
	CameraNode* m_lightSourceNode;
	Node* m_cameraParentNode;
	Node* m_objectsNode;

	VoxelizationManager m_voxelizationManager;

	/**
	 * generates a voxel grid texture from the provided voxel grid settings
	 * @param voxelGrid whose properties shall be used
	 * @return
	 */
	GLuint generateAndSetVoxelGridTexture(VoxelGridGPU* voxelGrid){
		DEBUGLOG->log("Creating voxel grid texture");
		DEBUGLOG->indent();

		// generate Texture
		GLuint voxelGridHandle;
		glGenTextures(1, &voxelGridHandle);
		glBindTexture(GL_TEXTURE_2D, voxelGridHandle);

		// compute number of mipmaps
		double integralPart = 0.0;
//		double fractionalPart = modf( log2( max( voxelGrid->resX, voxelGrid->resY ) ), &integralPart );
		voxelGrid->numMipmaps = (int) integralPart;
//		if ( fractionalPart != 0.0 )
//		{
//			DEBUGLOG->log("WARNING : texture requires a irregular number of mipMaps: ");
//			DEBUGLOG->indent();
//			DEBUGLOG->log("Integral part   : ", integralPart);
//			DEBUGLOG->log("Fractional part : ", fractionalPart);
//			DEBUGLOG->outdent();
//			voxelGrid->numMipmaps += 1;
//		}

//		DEBUGLOG->log("Number of mipmap levels : ", voxelGrid->numMipmaps );

		// allocate memory
		glTexStorage2D(
				GL_TEXTURE_2D,// 2D Texture
				voxelGrid->numMipmaps+1,// levels : Base level + mipMaplevels
				GL_R32UI,// 1 channel 32 bit unsigned int
				voxelGrid->resX,// res X
				voxelGrid->resY);// rex Y

		// clear texture
		std::vector < GLuint > emptyData( voxelGrid->resX * voxelGrid->resY, 0);
		glTexSubImage2D(
				GL_TEXTURE_2D,// target
				0,// level
				0,// xOffset
				0,// yOffset
				voxelGrid->resX,// width
				voxelGrid->resY,// height
				GL_RED,// format
				GL_UNSIGNED_INT,// type
				&emptyData[0] );// data

		// set filter parameters for samplers
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		voxelGrid->texture = new Texture(voxelGridHandle);
		voxelGrid->handle = voxelGridHandle;

		glBindTexture(GL_TEXTURE_2D, 0);

		DEBUGLOG->outdent();

		return voxelGridHandle;
	}

	CameraRenderPass* createGBufferRenderPass( )
	{
		DEBUGLOG->indent();

		// Render Scene into GBUFFER )
		Shader* writeGbufferShader = new Shader(SHADERS_PATH "/gbuffer/gbuffer.vert", SHADERS_PATH "/gbuffer/gbuffer_backfaceCulling.frag");

		GLenum internalFormat = FramebufferObject::static_internalFormat;
		FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;

		FramebufferObject* gbufferFramebufferObject = new FramebufferObject ( RENDER_FRAME_WIDTH, RENDER_FRAME_HEIGHT );
		// 3 attachments : position, normals, color
		gbufferFramebufferObject->addColorAttachments(3);

		FramebufferObject::static_internalFormat = internalFormat;

		CameraRenderPass* writeGbufferRenderPass = new CameraRenderPass(writeGbufferShader, gbufferFramebufferObject);
		writeGbufferRenderPass->addEnable(GL_DEPTH_TEST);
		writeGbufferRenderPass->setClearColor(0.0f,0.0f,0.0f,0.0f);
		writeGbufferRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);
		writeGbufferRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);

		m_cameraParentNode = new Node( m_sceneManager.getActiveScene()->getSceneGraph()->getRootNode() );
		CameraNode* camera = new CameraNode( m_cameraParentNode );
		camera->setProjectionMatrix( glm::perspective(65.0f, 1.0f, 0.1f, 30.0f) );
		writeGbufferRenderPass->setCamera( camera );

		DEBUGLOG->log("Adding render pass to application");
		m_renderManager.addRenderPass(writeGbufferRenderPass);

		DEBUGLOG->outdent();

		return writeGbufferRenderPass;
	}

	/**
	 * print some information from the GPU about compute shader related stuff
	 */
	void printComputeShaderInformation()
	{
		ShaderInfo::printGlobalInfo();
	}

public:
	ComputeShaderApp()
	{
		m_name = "Performance Test Application";
		m_objectsNode = 0;
		m_cameraParentNode = 0;
		m_lightParentNode = 0;
		m_lightSourceNode = 0;
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

			RenderableNode* blackBox= SimpleScene::loadObject("/blackBox.dae" , this);
			blackBox->setParent(scene->getSceneGraph()->getRootNode());
			renderables.push_back(blackBox);

			RenderableNode* testRoomNode = SimpleScene::loadTestRoomObject( this );
			testRoomNode->setParent( m_objectsNode );
//			renderables.push_back(testRoomNode);

			RenderableNode* mainObjectNode= SimpleScene::loadObject("/stanford/bunny/blender_bunny.dae" , this);
//			RenderableNode* mainObjectNode= SimpleScene::loadObject("/stanford/buddha/blender_buddha.dae" , this);
//			RenderableNode* mainObjectNode= SimpleScene::loadObject("/cube2.dae" , this);
//			RenderableNode* mainObjectNode= SimpleScene::loadObject("/bigQuad11858.dae" , this);
//			RenderableNode* mainObjectNode= SimpleScene::loadObject("/bigQuad2.dae" , this);

			Node* scaleNode = new Node(m_objectsNode );
			mainObjectNode->setParent( scaleNode );
//			DEBUGLOG->log("Scaling main object up by 25");
			scaleNode->scale( glm::vec3( 25.0f, 25.0f, 25.0f ) );
			renderables.push_back(mainObjectNode);
//			scaleNode->scale( glm::vec3( 0.5f, 0.5f, 0.5f ) );

			// add to voxelization candidate list
			CandidateObject* candidateObject = new CandidateObject();
			candidateObject->m_node = mainObjectNode;
			candidateObject->m_object = mainObjectNode->getObject();

			m_voxelizationManager.m_candidateObjects.push_back( candidateObject );

			m_voxelizationManager.m_activeCandidateObject = candidateObject;

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 									GBUFFER RENDERING
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating gbuffer renderpass");
			CameraRenderPass* gbufferRenderPass = createGBufferRenderPass();
			Camera* mainCamera = gbufferRenderPass->getCamera();

			gbufferRenderPass->addUniform(new Uniform< bool >( std::string( "uniformEnableBackfaceCulling" ),    &ENABLE_BACKFACE_CULLING ) );
			gbufferRenderPass->addUniform(new Uniform< bool >( std::string( "uniformOrtho" ),    &USE_ORTHOCAM ) );

			Texture* gbufferPositionMap = new Texture( gbufferRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0 ) );
			Texture* gbufferNormalMap = new Texture( gbufferRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT1 ) );
			Texture* gbufferColorMap = new Texture( gbufferRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT2 ) );

			DEBUGLOG->log("Adding objects to gbuffer render pass");
			for (unsigned int i = 0; i < renderables.size(); i++)
			{
				gbufferRenderPass->addRenderable( renderables[i] );
			}

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELGRID CANDIDATES CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring voxel grid candidates");
		DEBUGLOG->indent();
			VoxelGridGPU* voxelGrid32 = new VoxelGridGPU();
			VoxelGridGPU* voxelGrid64 = new VoxelGridGPU();
			VoxelGridGPU* voxelGrid128 = new VoxelGridGPU();
			VoxelGridGPU* voxelGrid256 = new VoxelGridGPU();
			VoxelGridGPU* voxelGrid512 = new VoxelGridGPU();

			int resZ = 32;

			voxelGrid32->setUniformCellSizeFromResolutionAndMapping(VOXELGRID_WIDTH,VOXELGRID_HEIGHT, 32, 32, resZ);
			voxelGrid64->setUniformCellSizeFromResolutionAndMapping(VOXELGRID_WIDTH,VOXELGRID_HEIGHT, 64, 64, resZ);
			voxelGrid128->setUniformCellSizeFromResolutionAndMapping(VOXELGRID_WIDTH,VOXELGRID_HEIGHT, 128, 128, resZ);
			voxelGrid256->setUniformCellSizeFromResolutionAndMapping(VOXELGRID_WIDTH,VOXELGRID_HEIGHT, 256, 256, resZ);
			voxelGrid512->setUniformCellSizeFromResolutionAndMapping(VOXELGRID_WIDTH,VOXELGRID_HEIGHT, 512, 512, resZ);

			generateAndSetVoxelGridTexture(voxelGrid32);
			generateAndSetVoxelGridTexture(voxelGrid64);
			generateAndSetVoxelGridTexture(voxelGrid128);
			generateAndSetVoxelGridTexture(voxelGrid256);
			generateAndSetVoxelGridTexture(voxelGrid512);

			voxelGrid32->fbo  = new FramebufferObject(voxelGrid32->handle);
			voxelGrid64->fbo  = new FramebufferObject(voxelGrid64->handle);
			voxelGrid128->fbo = new FramebufferObject(voxelGrid128->handle);
			voxelGrid256->fbo = new FramebufferObject(voxelGrid256->handle);
			voxelGrid512->fbo = new FramebufferObject(voxelGrid512->handle);

			m_voxelizationManager.m_voxelGrids.push_back( voxelGrid32 );
			m_voxelizationManager.m_voxelGrids.push_back( voxelGrid64 );
			m_voxelizationManager.m_voxelGrids.push_back( voxelGrid128 );
			m_voxelizationManager.m_voxelGrids.push_back( voxelGrid256 );
			m_voxelizationManager.m_voxelGrids.push_back( voxelGrid512 );

			m_voxelizationManager.m_activeVoxelGrid = voxelGrid32;

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								TEXTURE ATLAS CONFIGURATIONS
		 **************************************************************************************/

		DEBUGLOG->log("Configuring texture atlas objects");
		DEBUGLOG->indent();

		for ( unsigned int i = 0; i < m_voxelizationManager.m_candidateObjects.size(); i++ )
		{
			CandidateObject* currentCandidate = m_voxelizationManager.m_candidateObjects[i];

			RenderableNode* currentObject = currentCandidate->m_node;

			DEBUGLOG->log("Creating TextureAtlasRenderPass for provided renderable node");
			DEBUGLOG->indent();
				GLenum internalFormat = FramebufferObject::static_internalFormat;
				FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;// change this first

				// create renderpass that generates a textureAtlas for models
				TexAtlas::TextureAtlasRenderPass* textureAtlasRenderPass = new TexAtlas::TextureAtlasRenderPass(currentObject, TEXATLAS_RESOLUTION, TEXATLAS_RESOLUTION );

				FramebufferObject::static_internalFormat = internalFormat;	// restore default
			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating TextureAtlasVertexGenerator for provided Texture Atlas");
			DEBUGLOG->indent();

				//create texture atlas vertex generator to generate vertices
				TexAtlas::TextureAtlasVertexGenerator* textureAtlasVertexGenerator = new TexAtlas::TextureAtlasVertexGenerator( textureAtlasRenderPass->getTextureAtlas() );
			DEBUGLOG->outdent();

			DEBUGLOG->log("Initializing Texture Atlas functionality");
			DEBUGLOG->indent();

				DEBUGLOG->log("Generating Texture Atlas valid coordinates");
				// render texture atlas once so it can be validated
				textureAtlasRenderPass->render();

				DEBUGLOG->log("Generating Texture Atlas vertex array object");
				// generate vertices from texture atlas
				textureAtlasVertexGenerator->call();

			DEBUGLOG->outdent();

			// set missing variables
			currentCandidate->m_texAtlasRenderPass = textureAtlasRenderPass;
			currentCandidate->m_atlas = textureAtlasRenderPass->getTextureAtlas();
			currentCandidate->m_atlasObject = textureAtlasVertexGenerator->getPixelsObject();
		}
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
			ComputeShader* voxelizeComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeComputeGPUPro_simplifications.comp");

			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading and compiling voxel grid filling texture atlas enabled compute shader program");
			DEBUGLOG->indent();

			// shader that voxelizes an object by its texture atlas
			ComputeShader* voxelizeWithTexAtlasComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeWithTexAtlasComputeIndexBuffer.comp");

			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading and compiling voxel counting atomic counter enabled compute shader program");
			DEBUGLOG->indent();

			// shader that voxelizes an object by its texture atlas
			ComputeShader* countVoxelsComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeCountFullVoxelsCompute.comp");

			DEBUGLOG->outdent();

//			DEBUGLOG->log("Loading and compiling voxel grid mipmapping compute shader program");
//			DEBUGLOG->indent();
//
//			// shader that voxelizes an object by its texture atlas
//			ComputeShader* voxelizeMipmapComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeMipmapCompute.comp");
//
//			DEBUGLOG->outdent();


		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 							COMPUTE SHADER VOXELIZATION CONFIGURATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring compute shader dispatcher");
		DEBUGLOG->indent();

			// objects and their corresponding scene graph node
			std::vector<std::pair < Object*, RenderableNode*> > voxelizeObjects;
			voxelizeObjects.push_back( std::pair< Object*, RenderableNode* >( mainObjectNode->getObject(), mainObjectNode ) );

			// texture atlas pixel objects and their corresponding texture atlas
			std::vector<std::pair < Object*, TexAtlas::TextureAtlas* > > texAtlasObjects;
			texAtlasObjects.push_back( std::pair< Object*, TexAtlas::TextureAtlas* >(
)
					);


			DEBUGLOG->log("Creating voxel grid clearing compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchClearVoxelGridComputeShader* dispatchClearVoxelGridComputeShader = new DispatchClearVoxelGridComputeShader(
					clearVoxelGridComputeShader,
					m_voxelizationManager.m_activeVoxelGrid
			);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid filling compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchVoxelizeComputeShader* dispatchVoxelizeComputeShader = new DispatchVoxelizeComputeShader(
					voxelizeComputeShader,
					voxelizeObjects,
					m_voxelizationManager.m_activeVoxelGrid,
					SliceMap::get32BitUintMask()
					);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid filling tex atlas enabled compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchVoxelizeWithTexAtlasComputeShader* dispatchVoxelizeWithTexAtlasComputeShader = new DispatchVoxelizeWithTexAtlasComputeShader(
					voxelizeWithTexAtlasComputeShader,
					texAtlasObjects,
					m_voxelizationManager.m_activeVoxelGrid,
					SliceMap::get32BitUintMask()
					);

			DEBUGLOG->outdent();

//			DEBUGLOG->log("Creating voxel grid mipmapping compute shader dispatcher");
//			DEBUGLOG->indent();
//
//			DispatchMipmapVoxelGridComputeShader* dispatchMipmapVoxelGridComputeShader = new DispatchMipmapVoxelGridComputeShader(
//					voxelizeMipmapComputeShader,
//					m_voxelizationManager.m_activeVoxelGrid
//					);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Enabling execution time queries");
			dispatchClearVoxelGridComputeShader->setQueryTime( true );
			dispatchVoxelizeComputeShader->setQueryTime( false );
			dispatchVoxelizeWithTexAtlasComputeShader->setQueryTime( false );
//			dispatchMipmapVoxelGridComputeShader->setQueryTime(true);

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								SLICE MAP VOXELIZATION CONFIGURATION
		 **************************************************************************************/
		DEBUGLOG->log("Configuring Slice map Voxelization");
		DEBUGLOG->indent();

		SliceMap::SliceMapRenderPass* sliceMapRenderPass = SliceMap::getSliceMapRenderPassR32UI(
				m_voxelizationManager.m_activeVoxelGrid->fbo,
				m_voxelizationManager.m_activeVoxelGrid->perspective,
				SliceMap::BITMASK_SINGLETARGET);

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								TEXTURE ATLAS VOXELIZATION CONFIGURATION
		 **************************************************************************************/
		DEBUGLOG->log("Configuring Slice map Voxelization");
		DEBUGLOG->indent();

		SliceMap::SliceMapRenderPass* texAtlasSliceMapRenderPass = SliceMap::getSliceMapRenderPassR32UI(
				m_voxelizationManager.m_activeVoxelGrid->fbo,
				m_voxelizationManager.m_activeVoxelGrid->perspective,
				SliceMap::BITMASK_SINGLETARGET,
				SHADERS_PATH "/textureAtlas/textureAtlasWorldPositionR32UI.vert" );

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

			DEBUGLOG->log("Setting all the Voxelization methods to voxelization manager");
			m_voxelizationManager.m_dispatchVoxelizeCompute = dispatchVoxelizeComputeShader;
			m_voxelizationManager.m_dispatchVoxelizeTexAtlasCompute = dispatchVoxelizeWithTexAtlasComputeShader;
			m_voxelizationManager.m_sliceMapRenderPass = sliceMapRenderPass;

			m_voxelizationManager.m_texAtlasSliceMapRenderPass = texAtlasSliceMapRenderPass;
			m_voxelizationManager.m_activeVoxelizationMethod = VoxelizationManager::COMPUTE;

			DEBUGLOG->log( "Attaching voxelize dispatchers to program cycle via VOXELIZE interface");

			attach(dispatchClearVoxelGridComputeShader, "CLEAR");

			attach(&m_voxelizationManager,	"VOXELIZE");

//			attach(
//					new ConditionalProxyListener(
//							dispatchMipmapVoxelGridComputeShader,
//					&VOXELIZE_ACTIVE,
//					false),
//				"MIPMAP");

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								ATOMIC COUNTER CONFIGURATION
		 **************************************************************************************/
		DEBUGLOG->log("Configuring atomic counter to quantify set voxels");
		DEBUGLOG->indent();

			// generate buffer
			GLuint atomicsBuffer;
			glGenBuffers(1, &atomicsBuffer);

			//bind buffer
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 3, NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

			// configure Dispatch Compute Shader
			DispatchCountFullVoxelsComputeShader* dispatchCountFullVoxels = new DispatchCountFullVoxelsComputeShader(
					countVoxelsComputeShader,
					m_voxelizationManager.m_activeVoxelGrid,
					atomicsBuffer,
					0,0,0);

//			attach(dispatchCountFullVoxels, "COUNTVOXELS");
			m_voxelizationManager.m_dispatchCountFullVoxels = dispatchCountFullVoxels;

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								FINAL COMPOSITING RENDERING
		 **************************************************************************************/
		DEBUGLOG->log("Creating light source node");
		DEBUGLOG->indent();

		// create light source node
			m_lightParentNode = new Node( m_sceneManager.getActiveScene()->getSceneGraph()->getRootNode() );
			m_lightSourceNode = new CameraNode( m_lightParentNode );
			m_lightSourceNode->translate( LIGHT_POSITION );
			if ( USE_ORTHOLIGHTSOURCE )
			{
				m_lightSourceNode->setProjectionMatrix( LIGHT_ORTHO_PROJECTION );
			}
			else
			{
				m_lightSourceNode->setProjectionMatrix( LIGHT_PERSPECTIVE_PROJECTION );
			}
		DEBUGLOG->outdent();

		DEBUGLOG->log("Creating compositing render passes");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating gbuffer compositing shader");
			Shader* gbufferCompositingShader = new Shader(SHADERS_PATH "/screenspace/screenFill.vert", SHADERS_PATH "/screenspace/gbuffer_compositing_simple.frag");

			DEBUGLOG->log("Creating framebuffer for compositing render pass");
			FramebufferObject* compositingFramebuffer = new FramebufferObject(512,512);
			compositingFramebuffer->addColorAttachments(1);
			Texture* compositingOutput = new Texture( compositingFramebuffer->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0) );

			DEBUGLOG->indent();
			DEBUGLOG->log("Creating compositing render pass");
			TriangleRenderPass* gbufferCompositing = new TriangleRenderPass(gbufferCompositingShader, compositingFramebuffer, m_resourceManager.getScreenFillingTriangle());
			gbufferCompositing->setClearColor( 0.1f ,0.1f, 0.1f, 1.0 );
			gbufferCompositing->addClearBit(GL_COLOR_BUFFER_BIT);
			gbufferCompositing->addClearBit(GL_DEPTH_BUFFER_BIT);

			// upload gbuffer information
			gbufferCompositing->addUniformTexture( gbufferPositionMap, "uniformPositionMap" );
			gbufferCompositing->addUniformTexture( gbufferNormalMap, "uniformNormalMap" );
			gbufferCompositing->addUniformTexture( gbufferColorMap, "uniformColorMap" );

			gbufferCompositing->addUniform(new Uniform< glm::mat4 >( std::string( "uniformLightViewMatrix" ), m_lightSourceNode->getModelMatrixPtr() ) );
			gbufferCompositing->addUniform(new Uniform< glm::mat4 >( std::string( "uniformViewMatrix" ), mainCamera->getViewMatrixPointer() ) );


			DEBUGLOG->log("Adding compositing render pass now");
			m_renderManager.addRenderPass(gbufferCompositing);
			DEBUGLOG->outdent();


		/**************************************************************************************
		 * 								VOXELIZATION DISPLAY RENDERING
		 **************************************************************************************/
		DEBUGLOG->log("Configuring display of voxelized scene");
		DEBUGLOG->indent();

			// Align Camera with voxelization view
			ORTHOCAM_PROJECTION = glm::ortho( m_voxelizationManager.m_activeVoxelGrid->width * -0.5f, m_voxelizationManager.m_activeVoxelGrid->width * 0.5f, m_voxelizationManager.m_activeVoxelGrid->height * -0.5f, m_voxelizationManager.m_activeVoxelGrid->height * 0.5f, -10.0f, 15.0f);
			mainCamera->setPosition( glm::vec3 ( glm::inverse ( m_voxelizationManager.m_activeVoxelGrid->view ) * glm::vec4 ( 0.0, 0.0f, m_voxelizationManager.m_activeVoxelGrid->depth , 1.0f ) ) );

			Shader* 			overlaySliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH "/sliceMap/sliceMapOverLayGLSL4_3.frag");
			OverlayR32UITextureRenderPass* overlaySliceMap = new OverlayR32UITextureRenderPass(
					overlaySliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					compositingOutput,
					m_voxelizationManager.m_activeVoxelGrid,
					&VISIBLE_TEXTURE_LEVEL );
			overlaySliceMap->addUniform( new Uniform<float>("uniformBackgroundTransparency", &BACKGROUND_TRANSPARENCY) );
			overlaySliceMap->addUniform( new Uniform<bool>("enabled", &ENABLE_VOXELGRID_OVERLAY) );
			overlaySliceMap->setViewport(0,0,RENDER_FRAME_WIDTH,RENDER_FRAME_HEIGHT);

			Shader* projectSliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH"/sliceMap/sliceMapProjectionGLSL4_3.frag");
			ProjectSliceMapRenderPass* projectSliceMap = new ProjectSliceMapRenderPass(
					projectSliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					compositingOutput,
					m_voxelizationManager.m_activeVoxelGrid,
					SliceMap::get32BitUintMask(),
					gbufferPositionMap,
					mainCamera->getViewMatrixPointer()
					);
			projectSliceMap->addClearBit( GL_COLOR_BUFFER_BIT );
			projectSliceMap->addUniform( new Uniform<float>( "uniformBackgroundTransparency", &BACKGROUND_TRANSPARENCY ) );
			projectSliceMap->addUniform( new Uniform<bool>("enabled", &ENABLE_VOXELGRID_OVERLAY) );
			projectSliceMap->setViewport(0,0,RENDER_FRAME_WIDTH,RENDER_FRAME_HEIGHT);

			RenderPass* showSliceMap = projectSliceMap;

			m_renderManager.addRenderPass( showSliceMap );

			// for later use in switch through values listener
			std::vector <RenderPass* > showSliceMapCandidates;
			showSliceMapCandidates.push_back( projectSliceMap );
			showSliceMapCandidates.push_back( overlaySliceMap );

		DEBUGLOG->outdent();

		/**************************************************************************************
		* 						DEBUG GEOMETRY DISPLAY CONFIGURATION
		**************************************************************************************/

		DEBUGLOG->log("Configuring debug geometry display");
		DEBUGLOG->indent();

			DEBUGLOG->log("Configuring debug geometry renderpass");
			// create simple render pass / shader for arbitrary geometry
			Shader* debugShader = new Shader(SHADERS_PATH "/myShader/simpleVertex.vert" , SHADERS_PATH "/myShader/simpleColoring.frag");
			CameraRenderPass* debugGeometry = new CameraRenderPass( debugShader, 0 );
			debugGeometry->setCamera( mainCamera );
			debugGeometry->setViewport( 0, 0, RENDER_FRAME_WIDTH, RENDER_FRAME_HEIGHT);
			debugGeometry->addEnable( GL_BLEND );
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			DEBUGLOG->log("Configuring debug geometry : voxel grid");
			// debug geometry for voxel grid
			RenderableNode* voxelGridDebugGeometry = new RenderableNode( scene->getSceneGraph()->getRootNode() );
			Object* boundingBox = new Object ( *m_resourceManager.getCube() );
			Material* vMaterial = new Material( *boundingBox->getMaterial() );
			vMaterial->setAttribute( "uniformHasColor", 1.0f );
			vMaterial->setAttribute( "uniformRed", 0.2f );
			vMaterial->setAttribute( "uniformGreen", 1.0f );
			vMaterial->setAttribute( "uniformBlue", 1.0f );
			vMaterial->setAttribute( "uniformAlpha", 0.5f );
			boundingBox->setMaterial( vMaterial );

			voxelGridDebugGeometry->scale( glm::vec3 ( m_voxelizationManager.m_activeVoxelGrid->width, m_voxelizationManager.m_activeVoxelGrid->height, m_voxelizationManager.m_activeVoxelGrid->depth ) );

			voxelGridDebugGeometry->setObject( boundingBox );
			debugGeometry->addRenderable( voxelGridDebugGeometry );

			DEBUGLOG->log("Configuring debug geometry : light source");
			// debug geometry for light Source

//			Node* lightPositionNode  = new Node( scene->getSceneGraph()->getRootNode() );
//			Node* lightPositionNode = m_lightParentNode;

			RenderableNode* lightDebugGeometry = new RenderableNode( m_lightSourceNode );
			Object* lightBox = new Object ( *m_resourceManager.getCube() );
			Material* lMaterial = new Material( *lightBox->getMaterial() );
			lMaterial->setAttribute( "uniformHasColor", 1.0f );
			lMaterial->setAttribute( "uniformRed", 1.0f );
			lMaterial->setAttribute( "uniformGreen", 1.0f );
			lMaterial->setAttribute( "uniformBlue", 0.8f );
			lMaterial->setAttribute( "uniformAlpha", 1.0f );
			lightBox->setMaterial( lMaterial );
			lightDebugGeometry->scale( glm::vec3 ( 0.1, 0.1, 0.1 ) );
			lightDebugGeometry->setObject( lightBox );
			debugGeometry->addRenderable( lightDebugGeometry );

			m_renderManager.addRenderPass( debugGeometry );

		DEBUGLOG->outdent();

		/**************************************************************************************
		* 								GUI DISPLAY CONFIGURATION
		**************************************************************************************/
		DEBUGLOG->log("Configuring GUI");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating GUI frame object");
			Frame* guiFrame = new Frame( RENDER_FRAME_WIDTH, 0, GUI_FRAME_WIDTH, GUI_FRAME_HEIGHT );

			DEBUGLOG->log("Creating GUI render pass");
			Shader* guiShader = new Shader(SHADERS_PATH "/myShader/simpleVertex.vert", SHADERS_PATH "/myShader/simpleColoring.frag");
			RenderPass* guiRenderPass = new RenderPass( guiShader, 0);

			guiRenderPass->setViewport(RENDER_FRAME_WIDTH, 0, GUI_FRAME_WIDTH, GUI_FRAME_HEIGHT );
			guiRenderPass->addUniform( new Uniform<glm::mat4>("uniformView", new glm::mat4(1.0f) ) );
			guiRenderPass->addUniform( new Uniform<glm::mat4>("uniformProjection", new glm::mat4( glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -0.5f, 0.5f ) ) ) );
			guiRenderPass->addUniform( new Uniform<float >("uniformTextureTransparency", new float( -1.0f ) ) );

			int buttonWidth = 512;
			int buttonHeight = 512;

			DEBUGLOG->log("Creating some debug views");
			std::pair<InputField*, std::pair< Node*, RenderableNode*> > button;

			// other interesting debug textures
			std::vector<Texture* > debugTextures;
			debugTextures.push_back( m_voxelizationManager.m_activeCandidateObject->m_atlas );
			debugTextures.push_back( gbufferNormalMap );
			debugTextures.push_back( gbufferColorMap );
			debugTextures.push_back( new Texture( gbufferCompositing->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0) ) );

			// create button
			button = guiFrame->createButton(
				0, 0,
				buttonWidth, buttonHeight,
				&m_inputManager,
				GLFW_MOUSE_BUTTON_LEFT,
				&m_resourceManager,
				m_voxelizationManager.m_activeCandidateObject->m_atlas);

			// make interactive
			SwitchThroughValuesListener<Texture* >* debugTexturesSwitcher = new SwitchThroughValuesListener<Texture* >(
							&( *button.second.second->getObject()->getMaterial()->getTexturesPtr() )["uniformTexture"]
					        , debugTextures
					);
			button.first->attachListenerOnRelease( debugTexturesSwitcher );

			// add to gui render pass
			guiRenderPass->addRenderable( button.second.second );

			m_renderManager.addRenderPass( guiRenderPass );

		DEBUGLOG->outdent();
		/**************************************************************************************
		* 								INPUT CONFIGURATION
		**************************************************************************************/

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->log("---------------------------------------------------------");
		DEBUGLOG->indent();
			DEBUGLOG->log("Disable/Enable real-time voxelization  : Y");
			m_inputManager.attachListenerOnKeyPress( new InvertBooleanListener( &VOXELIZE_ACTIVE ), GLFW_KEY_Y, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new DebugPrintBooleanListener(&VOXELIZE_ACTIVE,          "Voxelize real-time enabled : "), GLFW_KEY_Y, GLFW_PRESS);

			DEBUGLOG->log("Switch active Voxel Grid               : G");
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getSwitchThroughVoxelGridsListener(),GLFW_KEY_G, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getPrintCurrentVoxelGridInfoListener(),GLFW_KEY_G, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getUpdateDebugGeometrySizeListener(voxelGridDebugGeometry),GLFW_KEY_G, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getUpdateDispatchVoxelGridComputeShaderListener( dispatchClearVoxelGridComputeShader ),GLFW_KEY_G, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getUpdateVoxelgridReferenceListener(&overlaySliceMap->p_voxelGrid),GLFW_KEY_G, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getUpdateVoxelgridReferenceListener(&projectSliceMap->p_voxelGrid),GLFW_KEY_G, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getUpdateVoxelgridWorldToVoxelReferenceListener( (glm::mat4**) &projectSliceMap->m_uniformWorldToVoxel->p_value),GLFW_KEY_G, GLFW_PRESS);

			//TODO update Uniform Variables of display render passes aswell !

			DEBUGLOG->log("Switch active voxelization object      : F");
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getSwitchThroughCandidatesListener(), GLFW_KEY_F, GLFW_PRESS);

			DEBUGLOG->log("Switch active voxelization method      : H");
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getSwitchThroughVoxelizaionMethodsListener(), GLFW_KEY_H, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getPrintCurrentVoxelizationMethodListener(), GLFW_KEY_H, GLFW_PRESS);


			DEBUGLOG->log("Clear voxel grid on key press          : C");
			m_inputManager.attachListenerOnKeyPress(  dispatchClearVoxelGridComputeShader, GLFW_KEY_C, GLFW_PRESS);

			DEBUGLOG->log("Voxelize scene with active voxelizer   : V");
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( dispatchVoxelizeComputeShader, &VOXELIZE_REGULAR_ACTIVE ), GLFW_KEY_V, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( dispatchVoxelizeWithTexAtlasComputeShader, &VOXELIZE_REGULAR_ACTIVE, true ), GLFW_KEY_V, GLFW_PRESS);

			DEBUGLOG->log("Switch voxel grid display              : B");
			// create listener to switch through display methods
			int showSliceMapIndex = m_renderManager.getRenderPassIndex( showSliceMap );
			SwitchThroughValuesListener< RenderPass* >* switchShowSliceMaps = new SwitchThroughValuesListener<RenderPass*>(
					&( *m_renderManager.getRenderPassesPtr() )[showSliceMapIndex],
					showSliceMapCandidates
			);
			m_inputManager.attachListenerOnKeyPress( switchShowSliceMaps, GLFW_KEY_B, GLFW_PRESS );

			std::vector<glm::mat4> projectionMatrices;
			projectionMatrices.push_back( PERSPECTIVECAM_PROJECTION );
			projectionMatrices.push_back( ORTHOCAM_PROJECTION );
			SwitchThroughValuesListener< glm::mat4 >* switchProjectionMatrix = new SwitchThroughValuesListener< glm::mat4 >(
					mainCamera->getProjectionMatrixPointer(),
					projectionMatrices
			);
			m_inputManager.attachListenerOnKeyPress( switchProjectionMatrix, GLFW_KEY_B, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new InvertBooleanListener( &USE_ORTHOCAM ), GLFW_KEY_B, GLFW_PRESS);

			DEBUGLOG->log("Dis-/Enable voxel grid display         : N");
			m_inputManager.attachListenerOnKeyPress( new InvertBooleanListener( &ENABLE_VOXELGRID_OVERLAY), GLFW_KEY_N, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new DebugPrintBooleanListener(&ENABLE_VOXELGRID_OVERLAY, "Overlay voxel grid enabled : " ), GLFW_KEY_N, GLFW_PRESS );

			DEBUGLOG->log("Reset scenegraph                       : R");
			m_inputManager.attachListenerOnKeyPress( new SimpleScene::SceneGraphState( scene->getSceneGraph() ),GLFW_KEY_R, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new SetValueListener<glm::vec3>(&LIGHT_POSITION, LIGHT_POSITION ),GLFW_KEY_R, GLFW_PRESS);

			DEBUGLOG->log("Print voxelization times               : T");
			m_inputManager.attachListenerOnKeyPress( dispatchClearVoxelGridComputeShader->getPrintExecutionTimeListener(		"Clear Voxel Grid      "), GLFW_KEY_T, GLFW_PRESS);
//			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeComputeShader->getPrintExecutionTimeListener(				"Voxelize Models       "), GLFW_KEY_T, GLFW_PRESS);
//			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeWithTexAtlasComputeShader->getPrintExecutionTimeListener(	"Voxelize Texture Atlas"), GLFW_KEY_T, GLFW_PRESS);
//			m_inputManager.attachListenerOnKeyPress( dispatchMipmapVoxelGridComputeShader->getPrintExecutionTimeListener(       "Mipmap Voxel Grid     "), GLFW_KEY_T, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getPrintVoxelizationTimesListener(), GLFW_KEY_T, GLFW_PRESS);

			DEBUGLOG->log("Print voxelization fill amounts        : Z");
			m_inputManager.attachListenerOnKeyPress( m_voxelizationManager.getPrintVoxelizationFillAmountListener(), GLFW_KEY_Z, GLFW_PRESS);

			DEBUGLOG->log("Turn objects                           : MOUSE - LEFT");
			Turntable* turntable = SimpleScene::configureTurnTable( m_objectsNode, this, 0.05f, GLFW_MOUSE_BUTTON_LEFT, mainCamera );
			Turntable* turntableCam = SimpleScene::configureTurnTable( m_cameraParentNode, this, 0.05f , GLFW_MOUSE_BUTTON_RIGHT, mainCamera );

			DEBUGLOG->log("Turn camera                            : MOUSE - RIGHT");
			SimpleScene::configureSimpleCameraMovement(mainCamera, this, 2.5f);

			DEBUGLOG->log("Move light source                      : Arrow keys");
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &LIGHT_POSITION, glm::vec3(0.0f,0.0f, 1.0f) ), GLFW_KEY_DOWN, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &LIGHT_POSITION, glm::vec3(0.0f,0.0f, -1.0f) ), GLFW_KEY_UP, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &LIGHT_POSITION, glm::vec3(-1.0f,0.0f, 0.0f) ), GLFW_KEY_LEFT, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &LIGHT_POSITION, glm::vec3(1.0f,0.0f, 0.0f) ), GLFW_KEY_RIGHT, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new MultiplyValueListener<glm::mat4>( m_lightSourceNode->getModelMatrixPtr(), glm::translate( glm::mat4(1.0), glm::vec3(0.0f,0.0f, 1.0f ) ) ), GLFW_KEY_DOWN, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new MultiplyValueListener<glm::mat4>( m_lightSourceNode->getModelMatrixPtr(), glm::translate( glm::mat4(1.0), glm::vec3(0.0f,0.0f, -1.0f) ) ), GLFW_KEY_UP, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new MultiplyValueListener<glm::mat4>( m_lightSourceNode->getModelMatrixPtr(), glm::translate( glm::mat4(1.0), glm::vec3(-1.0f,0.0f, 0.0f) ) ), GLFW_KEY_LEFT, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new MultiplyValueListener<glm::mat4>( m_lightSourceNode->getModelMatrixPtr(), glm::translate( glm::mat4(1.0), glm::vec3(1.0f,0.0f, 0.0f ) ) ), GLFW_KEY_RIGHT, GLFW_PRESS );

			DEBUGLOG->log("De-/Increase background transparency   : lower / upper left corner");
			InputField* inputFieldIncTransparency = new InputField(0,  0, 100, 256, &m_inputManager, GLFW_MOUSE_BUTTON_LEFT);
			InputField* inputFieldDecTransparency = new InputField(0,256, 100, 256, &m_inputManager, GLFW_MOUSE_BUTTON_LEFT);
			inputFieldIncTransparency->attachListenerOnPress( new IncrementValueListener<float>( &BACKGROUND_TRANSPARENCY, 0.1f ) );
			inputFieldIncTransparency->attachListenerOnPress( new DebugPrintValueListener<float>( &BACKGROUND_TRANSPARENCY, "background transparency : "));
			inputFieldDecTransparency->attachListenerOnPress( new DecrementValueListener<float>( &BACKGROUND_TRANSPARENCY, 0.1f ) );
			inputFieldDecTransparency->attachListenerOnPress( new DebugPrintValueListener<float>( &BACKGROUND_TRANSPARENCY, "background transparency : "));

//			DEBUGLOG->log("De-/Increase visible voxel grid level  : J / K    ( overlay view only )");
//			m_inputManager.attachListenerOnKeyPress( new DecrementValueListener< int >( &VISIBLE_TEXTURE_LEVEL, 1 ) , GLFW_KEY_K);
//			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener< int >( &VISIBLE_TEXTURE_LEVEL, 1 ) , GLFW_KEY_J);
//			m_inputManager.attachListenerOnKeyPress( new DebugPrintValueListener< int >( &VISIBLE_TEXTURE_LEVEL, "visible voxel grid level : "), GLFW_KEY_K);
//			m_inputManager.attachListenerOnKeyPress( new DebugPrintValueListener< int >( &VISIBLE_TEXTURE_LEVEL, "visible voxel grid level : "), GLFW_KEY_J);

		DEBUGLOG->outdent();

		DEBUGLOG->log("---------------------------------------------------------");

	}



	void programCycle()
	{
		call("CLEAR");					// call listeners attached to clear interface

		call("VOXELIZE");				// call listeners attached to voxelize interface

//		call ("MIPMAP");				// call listeners attached to mipmap interface

		call("COUNTVOXELS");

		Application::programCycle(); 	// regular rendering and image presentation
	}

};


int main() {
	// configure a little bit
	Application::static_newWindowHeight = RENDER_FRAME_HEIGHT;
	Application::static_newWindowWidth = RENDER_FRAME_WIDTH + GUI_FRAME_WIDTH;

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
