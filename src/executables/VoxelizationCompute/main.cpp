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

static int VISIBLE_TEXTURE_LEVEL = 0;

static bool USE_ROTATING_BUNNY = false;

static bool VOXELIZE_REGULAR_ACTIVE = true;
static bool VOXELIZE_ACTIVE = true;

static int TEXATLAS_RESOLUTION  = 512;
static int VOXELGRID_RESOLUTION = 64;
static float VOXELGRID_WIDTH = 8.0f;
static float VOXELGRID_HEIGHT = 8.0f;

static glm::vec3 LIGHT_POSITION = glm::vec3(0.0f, 0.0f, 3.0f);
static float LIGHT_FLUX = 1.0f;
static bool USE_ORTHOLIGHTSOURCE = true;

static bool ENABLE_RSM_OVERLAY = true;	// view in render frame
static int RSM_WIDTH = 512;
static int RSM_HEIGHT = 512;
static int RSM_SAMPLES_AMOUNT = 200;
static float RSM_SAMPLES_MAX_OFFSET = 0.5f;

static bool ENABLE_BACKFACE_CULLING = true;
static bool USE_ORTHOCAM = true;
static float BACKGROUND_TRANSPARENCY = 0.25;

static int RENDER_FRAME_WIDTH = 512;
static int RENDER_FRAME_HEIGHT = 512;
static int GUI_FRAME_WIDTH = 256;
static int GUI_FRAME_HEIGHT = 512;

/**
 * Renderpass that overlays the slice map ontop of fbo
 */
class OverlayR32UITextureRenderPass : public TriangleRenderPass
{
private:
	Texture* p_texture;
	int* p_level;

public:
	OverlayR32UITextureRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* triangle, Texture* baseTexture, Texture* texture, int* level = 0)
	: TriangleRenderPass(shader, fbo, triangle)
	{
		addUniformTexture(baseTexture, "uniformBaseTexture");
		addEnable(GL_BLEND);
		p_texture = texture;

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
		p_texture->getTextureHandle(),  // texture
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
private:
	VoxelGridGPU* p_voxelGrid;
	Texture* p_bitMask;
public:
	ProjectSliceMapRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* triangle, Texture* baseTexture, VoxelGridGPU* voxelGrid, Texture* bitMask, Texture* positionMap, glm::mat4* viewMatrix)
	: TriangleRenderPass(shader, fbo, triangle)
	{
		addUniformTexture(baseTexture, "uniformBaseTexture");
		addUniformTexture(positionMap, "uniformPositionMap");

		addUniform( new Uniform< glm::mat4 >("uniformWorldToVoxel", &( voxelGrid->worldToVoxel ) ) );
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

	CameraRenderPass* createReflectiveShadowMapRenderPass( )
	{
		// render scene information into framebuffer attachments
		Shader* writeRSMShader = new Shader( SHADERS_PATH "/rsm/rsm.vert", SHADERS_PATH "/rsm/rsm_backfaceCulling.frag" );

		// allow values > 1.0 and < 0.0
		GLenum internalFormat = FramebufferObject::static_internalFormat;
		FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;

		FramebufferObject* rsmFramebufferObject = new FramebufferObject ( RSM_WIDTH, RSM_HEIGHT );
		// 3 attachments : world position, normals, flux
		rsmFramebufferObject->addColorAttachments(3);

		FramebufferObject::static_format = internalFormat;

		CameraRenderPass* writeRSMRenderPass = new CameraRenderPass( writeRSMShader, rsmFramebufferObject );
		writeRSMRenderPass->addEnable(GL_DEPTH_TEST);	// write depth map
		writeRSMRenderPass->setClearColor(0.0f,0.0f,0.0f,0.0f);
		writeRSMRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);
		writeRSMRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);

		// create light source node
		m_lightParentNode = new Node( m_sceneManager.getActiveScene()->getSceneGraph()->getRootNode() );
		m_lightSourceNode = new CameraNode( m_lightParentNode );
		m_lightSourceNode->translate( LIGHT_POSITION );
		m_lightSourceNode->setProjectionMatrix( glm::ortho( -6.0f, 6.0f, -6.0f, 6.0f, 0.0f, 15.0f) );

		// TODO implement setCenter for CameraNode
//		m_lightSourceNode->setCenter( glm::vec3(0.0f, 0.0f, 0.0f) );

		// look at center
//		m_lightSourceNode->setDirection( glm::normalize( -1.0f * LIGHT_POSITION ) );

		writeRSMRenderPass->setCamera( m_lightSourceNode );

		DEBUGLOG->log("Adding render pass to application");
		m_renderManager.addRenderPass(writeRSMRenderPass);

		return writeRSMRenderPass;
	}

	std::vector< float > generateRandomSamplingPattern( int numSamples, float r_max = 1.0f)
		{
			srand( time( 0 ) );
			std::vector< float > result;

			for ( int i = 0; i < numSamples; i++)
			{
				// generate uniform random number
				float r1 = ( (float) std::rand() / (float) RAND_MAX - 0.5f ) * 2.0f; // -1..1
				float r2 = ( (float) std::rand() / (float) RAND_MAX - 0.5f ) * 2.0f; // -1..1

				// generate polar coordinates and weight
				float x = r_max * r1 * sin( 2.0f * PI * r2 );
				float y = r_max * r1 * cos( 2.0f * PI * r2 );
				float weight = r1 * r1;

				// make normalized by bringing the samples into 0..1

//				DEBUGLOG->log("Generated sample : ", glm::vec3(x,y,weight) );

				// save generated values
				result.push_back(x);
				result.push_back(y);
				result.push_back(weight);
			}

			return result;
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
		m_name = "Compute Voxelization";
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

			RenderableNode* testRoomNode = SimpleScene::loadTestRoomObject( this );
			renderables.push_back(testRoomNode);

			RenderableNode* bunnyNode= SimpleScene::loadObject("/stanford/bunny/blender_bunny.dae" , this);

			DEBUGLOG->log("Scaling bunny up by 25");
			bunnyNode->scale( glm::vec3( 25.0f, 25.0f, 25.0f ) );

			renderables.push_back(bunnyNode);

			DEBUGLOG->log("Attaching objects to scene graph");
			DEBUGLOG->indent();
					if ( USE_ROTATING_BUNNY )
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

		DEBUGLOG->outdent();

		/**************************************************************************************
		* 						REFLECTIVE SHADOW MAP CONFIGURATION
		**************************************************************************************/
		DEBUGLOG->log("Configuring reflective shadow map rendering");
		DEBUGLOG->indent();
			CameraRenderPass* rsmRenderPass = createReflectiveShadowMapRenderPass();

			rsmRenderPass->addUniform(new Uniform< bool >( std::string( "uniformEnableBackfaceCulling" ),    &ENABLE_BACKFACE_CULLING ) );
			rsmRenderPass->addUniform(new Uniform< bool >( std::string( "uniformOrtho" ),    &USE_ORTHOLIGHTSOURCE ) );
			rsmRenderPass->addUniform(new Uniform<float >( std::string( "uniformFlux" ) , &LIGHT_FLUX ) );

			// for future use
			Texture* rsmPositionMap = new Texture( rsmRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) );
			Texture* rsmNormalMap = new Texture( rsmRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT1 ) );
			Texture* rsmFluxMap = new Texture( rsmRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT2 ) );
			Texture* rsmDepthMap = new Texture ( rsmRenderPass->getFramebufferObject()->getDepthTextureHandle() );

			DEBUGLOG->log("Adding objects to reflective shadow map render pass");
			for (unsigned int i = 0; i < renderables.size(); i++)
			{
				rsmRenderPass->addRenderable( renderables[i] );
			}

		DEBUGLOG->outdent();

		/**************************************************************************************
		* 				REFLECTIVE SHADOW MAP LIGHT GATHERING CONFIGURATION
		**************************************************************************************/
		DEBUGLOG->log("Configuring reflective shadow map light gathering");
		DEBUGLOG->indent();

			// create sampling Pattern
			DEBUGLOG->log("Generating random samples : ", RSM_SAMPLES_AMOUNT);
			std::vector<float> rsmSamples = generateRandomSamplingPattern( RSM_SAMPLES_AMOUNT, RSM_SAMPLES_MAX_OFFSET );

			DEBUGLOG->log("Buffering samples to texture object");
			Texture* rsmSamplesTexture = new Texture1D();
			GLuint rsmSamplesTextureHandle;
			glGenTextures(1, &rsmSamplesTextureHandle);

			glBindTexture(GL_TEXTURE_1D, rsmSamplesTextureHandle);

			// allocate mem:  1D Texture,  1 level,   3 channel float format (32bit), same amount of texels as samples
			glTexStorage1D( GL_TEXTURE_1D, 1 , GL_RGB32F_ARB , RSM_SAMPLES_AMOUNT );

			// buffer data to GPU
			glTexSubImage1D( GL_TEXTURE_1D, 0, 0, RSM_SAMPLES_AMOUNT, GL_RGB, GL_FLOAT, &rsmSamples[0] );

			// set filter parameters so samplers can work
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			float* pixels = new float[RSM_SAMPLES_AMOUNT * 3];
			glGetTexImage(GL_TEXTURE_1D, 0, GL_RGB, GL_FLOAT, pixels);
			for ( int i = 0; i < RSM_SAMPLES_AMOUNT; i++)
			{
				DEBUGLOG->log("pixel            : ", i);
				DEBUGLOG->log("Value in buffer x: " ,	pixels[i*3 +0]);
				DEBUGLOG->log("Value in buffer y: " ,	pixels[i*3 +1]);
//				DEBUGLOG->log("Value in buffer w: " ,	pixels[i*3 +2]);
//				DEBUGLOG->log("Value in sample x: ", rsmSamples[ i*3 + 0 ]);
//				DEBUGLOG->log("Value in sample y: ", rsmSamples[ i*3 + 1 ]);
//				DEBUGLOG->log("Value in sample w: ", rsmSamples[ i*3 + 2 ]);
			}

			rsmSamplesTexture->setTextureHandle( rsmSamplesTextureHandle );
			glBindTexture(GL_TEXTURE_1D, 0);

			DEBUGLOG->log("Creating Light Gathering RenderPass");
			// compile rsm sampling shader
			Shader* rsmLightGatheringShader = new Shader( SHADERS_PATH "/screenspace/screenFill.vert", SHADERS_PATH "/rsm/rsmSampling.frag");

			// create framebuffer object as big as compositing output
			FramebufferObject* rsmLightGatheringFramebuffer = new FramebufferObject( gbufferRenderPass->getFramebufferObject()->getWidth(), gbufferRenderPass->getFramebufferObject()->getHeight() );
			// 2 attachments : direct light, indirect light
			rsmLightGatheringFramebuffer->addColorAttachments( 2 );

			// create render pass
			TriangleRenderPass* rsmLightGatheringRenderPass = new TriangleRenderPass( rsmLightGatheringShader, rsmLightGatheringFramebuffer, m_resourceManager.getScreenFillingTriangle() );
			rsmLightGatheringRenderPass->addClearBit( GL_COLOR_BUFFER_BIT );

			DEBUGLOG->log("Configuring uniforms");
			DEBUGLOG->indent();

				// upload reflective shadow map information
				rsmLightGatheringRenderPass->addUniformTexture(rsmPositionMap, "uniformRSMPositionMap");
				rsmLightGatheringRenderPass->addUniformTexture(rsmNormalMap,   "uniformRSMNormalMap");
				rsmLightGatheringRenderPass->addUniformTexture(rsmFluxMap, "uniformRSMFluxMap");
				rsmLightGatheringRenderPass->addUniformTexture(rsmDepthMap,    "uniformRSMDepthMap");

				rsmLightGatheringRenderPass->addUniform( new Uniform<glm::mat4>( "uniformRSMView"      , m_lightSourceNode->getViewMatrixPointer() ) );
				rsmLightGatheringRenderPass->addUniform( new Uniform<glm::mat4>( "uniformRSMProjection", m_lightSourceNode->getProjectionMatrixPointer() ) );

				// upload gbuffer information
				rsmLightGatheringRenderPass->addUniformTexture(gbufferPositionMap, "uniformGBufferPositionMap");
				rsmLightGatheringRenderPass->addUniformTexture(gbufferNormalMap,   "uniformGBufferNormalMap");

				rsmLightGatheringRenderPass->addUniform( new Uniform<glm::mat4>( "uniformGBufferView", mainCamera->getViewMatrixPointer() ) );

				// upload sampling pattern information
				rsmLightGatheringRenderPass->addUniformTexture( rsmSamplesTexture, "uniformSamplingPattern" );

				rsmLightGatheringRenderPass->addUniform( new Uniform< int > ( "uniformNumSamples" , &RSM_SAMPLES_AMOUNT ) );

			DEBUGLOG->outdent();

			// for future use
			Texture* rsmDirectLightMap = new Texture( rsmLightGatheringFramebuffer->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0) );
			Texture* rsmIndirectLightMap = new Texture( rsmLightGatheringFramebuffer->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT1) );

			DEBUGLOG->log("Adding RSM light gathering render pass to application");
			m_renderManager.addRenderPass( rsmLightGatheringRenderPass );

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								FINAL COMPOSITING RENDERING
		 **************************************************************************************/

		DEBUGLOG->log("Creating compositing render passes");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating gbuffer compositing shader");
				Shader* gbufferCompositingShader = new Shader(SHADERS_PATH "/screenspace/screenFill.vert", SHADERS_PATH "/screenspace/gbuffer_compositing_phong.frag");

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

				gbufferCompositing->addUniformTexture( gbufferPositionMap, "uniformPositionMap" );
				gbufferCompositing->addUniformTexture( gbufferNormalMap, "uniformNormalMap" );
				gbufferCompositing->addUniformTexture( gbufferColorMap, "uniformColorMap" );

				gbufferCompositing->addUniform(new Uniform< glm::vec3 >( std::string( "uniformLightPosition" ), &LIGHT_POSITION  ) );
				gbufferCompositing->addUniform(new Uniform< glm::mat4 >( std::string( "uniformViewMatrix" ),    mainCamera->getViewMatrixPointer() ) );

				gbufferCompositing->addUniform(new Uniform< bool >( std::string( "uniformEnableRSMOverlay" ),   &ENABLE_RSM_OVERLAY) );

				gbufferCompositing->addUniformTexture( rsmDirectLightMap, "uniformRSMDirectLightMap" );
				gbufferCompositing->addUniformTexture( rsmIndirectLightMap, "uniformRSMIndirectLightMap" );


				DEBUGLOG->log("Adding compositing render pass now");
				m_renderManager.addRenderPass(gbufferCompositing);
			DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELGRID CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring voxel grid");
		DEBUGLOG->indent();
			VoxelGridGPU* voxelGrid = new VoxelGridGPU();

			voxelGrid->setUniformCellSizeFromResolutionAndMapping(VOXELGRID_WIDTH,VOXELGRID_HEIGHT, VOXELGRID_RESOLUTION, VOXELGRID_RESOLUTION, 32);

			DEBUGLOG->log("Creating voxel grid texture");
			DEBUGLOG->indent();

			// generate Texture
			GLuint voxelGridHandle;
			glGenTextures(1, &voxelGridHandle);
			glBindTexture(GL_TEXTURE_2D, voxelGridHandle);

			// compute number of mipmaps
			double integralPart;
			double fractionalPart = modf( log2( max( VOXELGRID_RESOLUTION, VOXELGRID_RESOLUTION ) ), &integralPart );
			voxelGrid->numMipmaps = (int) integralPart;
			if ( fractionalPart != 0.0 )
			{
				DEBUGLOG->log("WARNING : texture requires a irregular number of mipMaps: ");
				DEBUGLOG->indent();
					DEBUGLOG->log("Integral part   : ", integralPart);
					DEBUGLOG->log("Fractional part : ", fractionalPart);
				DEBUGLOG->outdent();
				voxelGrid->numMipmaps += 1;
			}
			DEBUGLOG->log("Number of mipmap levels : ", voxelGrid->numMipmaps );

			// allocate memory
			glTexStorage2D(
					GL_TEXTURE_2D,			// 2D Texture
					voxelGrid->numMipmaps+1,// levels : Base level + mipMaplevels
					GL_R32UI,				// 1 channel 32 bit unsigned int
					VOXELGRID_RESOLUTION,	// res X
					VOXELGRID_RESOLUTION);	// rex Y

			// clear texture
			std::vector < GLuint > emptyData( VOXELGRID_RESOLUTION * VOXELGRID_RESOLUTION , 0);
			glTexSubImage2D(
					GL_TEXTURE_2D,	// target
					0,				// level
					0,				// xOffset
					0,				// yOffset
					VOXELGRID_RESOLUTION, // width
					VOXELGRID_RESOLUTION, // height
					GL_RED,			// format
					GL_UNSIGNED_INT,// type
					&emptyData[0] );// data

			// set filter parameters for samplers
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
			// save handle
			voxelGrid->handle = voxelGridHandle;
			voxelGrid->texture = new Texture(voxelGridHandle);

			// unbind
			glBindTexture(GL_TEXTURE_2D, 0);

			DEBUGLOG->outdent();

			// for other use (i.e. tex atlas)
//			Texture* voxelGridTexture = voxelGrid->texture;
		DEBUGLOG->outdent();


		/**************************************************************************************
		 * 								TEXTURE ATLAS VAO CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring texture atlas objects");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating TextureAtlasRenderPass for provided renderable node");
			DEBUGLOG->indent();
				GLenum internalFormat = FramebufferObject::static_internalFormat;
				FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;// change this first

				// create renderpass that generates a textureAtlas for models
				TexAtlas::TextureAtlasRenderPass* textureAtlasRenderPass = new TexAtlas::TextureAtlasRenderPass(bunnyNode, TEXATLAS_RESOLUTION, TEXATLAS_RESOLUTION, mainCamera );

				FramebufferObject::static_internalFormat = internalFormat;	// restore default
			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding texture atlas render pass");
			m_renderManager.addRenderPass( textureAtlasRenderPass );

			DEBUGLOG->log("Creating TextureAtlasVertexGenerator for provided Texture Atlas");
			DEBUGLOG->indent();

				//create texture atlas vertex generator to generate vertices
				TexAtlas::TextureAtlasVertexGenerator* textureAtlasVertexGenerator = new TexAtlas::TextureAtlasVertexGenerator( textureAtlasRenderPass->getTextureAtlas() );
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								TEXTURE ATLAS INITIALIZATION
		 **************************************************************************************/

		DEBUGLOG->log("Initializing Texture Atlas functionality");
		DEBUGLOG->indent();

			DEBUGLOG->log("Generating Texture Atlas valid coordinates");
			// render texture atlas once so it can be validated
			textureAtlasRenderPass->render();

			DEBUGLOG->log("Generating Texture Atlas vertex array object");
			// generate vertices from texture atlas
			textureAtlasVertexGenerator->call();

			// attach to a Node for rendering
			RenderableNode* verticesNode = new RenderableNode( m_objectsNode );
			verticesNode->setObject( textureAtlasVertexGenerator->getPixelsObject() );
			verticesNode->scale( glm::vec3( 10.0f, 10.0f, 10.0f ) );

//			phongPerspectiveRenderPass->addRenderable( verticesNode );

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
			ComputeShader* voxelizeWithTexAtlasComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeWithTexAtlasCompute.comp");

			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading and compiling voxel grid mipmapping compute shader program");
			DEBUGLOG->indent();

			// shader that voxelizes an object by its texture atlas
			ComputeShader* voxelizeMipmapComputeShader = new ComputeShader(SHADERS_PATH "/compute/voxelizeMipmapCompute.comp");

			DEBUGLOG->outdent();


		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 							COMPUTE SHADER VOXELIZATION CONFIGURATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring compute shader dispatcher");
		DEBUGLOG->indent();

			DEBUGLOG->log("Configuring list of objects to voxelize");

			// objects and their corresponding scene graph node
			std::vector<std::pair < Object*, RenderableNode*> > objects;
			objects.push_back( std::pair< Object*, RenderableNode* >( bunnyNode->getObject(), bunnyNode ) );
//			objects.push_back( std::pair< Object*, RenderableNode* >( testRoomNode->getObject(), testRoomNode ) );

			// texture atlas pixel objects and their corresponding texture atlas
			std::vector<std::pair < Object*, TexAtlas::TextureAtlas* > > texAtlasObjects;
			texAtlasObjects.push_back( std::pair< Object*, TexAtlas::TextureAtlas* >(
					textureAtlasVertexGenerator->getPixelsObject(),
					textureAtlasVertexGenerator->getTextureAtlas())
					);


			DEBUGLOG->log("Creating voxel grid clearing compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchClearVoxelGridComputeShader* dispatchClearVoxelGridComputeShader = new DispatchClearVoxelGridComputeShader(
					clearVoxelGridComputeShader,
					voxelGrid
			);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid filling compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchVoxelizeComputeShader* dispatchVoxelizeComputeShader = new DispatchVoxelizeComputeShader(
					voxelizeComputeShader,
					objects,
					voxelGrid,
					SliceMap::get32BitUintMask()
					);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid filling tex atlas enabled compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchVoxelizeWithTexAtlasComputeShader* dispatchVoxelizeWithTexAtlasComputeShader = new DispatchVoxelizeWithTexAtlasComputeShader(
					voxelizeWithTexAtlasComputeShader,
					texAtlasObjects,
					voxelGrid,
					SliceMap::get32BitUintMask()
					);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid mipmapping compute shader dispatcher");
			DEBUGLOG->indent();

			DispatchMipmapVoxelGridComputeShader* dispatchMipmapVoxelGridComputeShader = new DispatchMipmapVoxelGridComputeShader(
					voxelizeMipmapComputeShader,
					voxelGrid
					);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Enabling execution time queries");
			dispatchClearVoxelGridComputeShader->setQueryTime( true );
			dispatchVoxelizeComputeShader->setQueryTime( true );
			dispatchVoxelizeWithTexAtlasComputeShader->setQueryTime( true );
			dispatchMipmapVoxelGridComputeShader->setQueryTime(true);

		DEBUGLOG->outdent();


		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

			DEBUGLOG->log( "Attaching voxelize dispatchers to program cycle via VOXELIZE interface");

			attach(
					new ConditionalProxyListener(
					dispatchClearVoxelGridComputeShader,
					&VOXELIZE_ACTIVE,
					false),
				"CLEAR");

			attach(
					new ConditionalProxyListener(
					new ConditionalProxyListener(
						dispatchVoxelizeComputeShader,
						&VOXELIZE_REGULAR_ACTIVE,
						false ),
					&VOXELIZE_ACTIVE,
					false),
				"VOXELIZE"
			);

			attach(
					new ConditionalProxyListener(
					new ConditionalProxyListener(
							dispatchVoxelizeWithTexAtlasComputeShader,
							&VOXELIZE_REGULAR_ACTIVE,
							true),
					&VOXELIZE_ACTIVE,
					false),
				"VOXELIZE");

			attach(
					new ConditionalProxyListener(
							dispatchMipmapVoxelGridComputeShader,
					&VOXELIZE_ACTIVE,
					false),
				"MIPMAP");

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION DISPLAY RENDERING
		 **************************************************************************************/
		DEBUGLOG->log("Configuring display of voxelized scene");
		DEBUGLOG->indent();

			// Align Camera with voxelization view
			mainCamera->setProjectionMatrix( glm::ortho( voxelGrid->width * -0.5f, voxelGrid->width * 0.5f, voxelGrid->height * -0.5f, voxelGrid->height * 0.5f, -10.0f, 15.0f) );
			mainCamera->setPosition( glm::vec3 ( glm::inverse ( voxelGrid->view ) * glm::vec4 ( 0.0, 0.0f, voxelGrid->depth / 2.0f, 1.0f ) ) );

			Shader* 			overlaySliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH "/sliceMap/sliceMapOverLayGLSL4_3.frag");
			TriangleRenderPass* overlaySliceMap = new OverlayR32UITextureRenderPass(
					overlaySliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					compositingOutput,
					voxelGrid->texture,
					&VISIBLE_TEXTURE_LEVEL );
			overlaySliceMap->addUniform( new Uniform<float>("uniformBackgroundTransparency", &BACKGROUND_TRANSPARENCY) );
			overlaySliceMap->setViewport(0,0,RENDER_FRAME_WIDTH,RENDER_FRAME_HEIGHT);

			Shader* projectSliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH"/sliceMap/sliceMapProjectionGLSL4_3.frag");
			TriangleRenderPass* projectSliceMap = new ProjectSliceMapRenderPass(
					projectSliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					compositingOutput,
					voxelGrid,
					SliceMap::get32BitUintMask(),
					gbufferPositionMap,
					mainCamera->getViewMatrixPointer()
					);
			projectSliceMap->addClearBit( GL_COLOR_BUFFER_BIT );
			projectSliceMap->addUniform( new Uniform<float>( "uniformBackgroundTransparency", &BACKGROUND_TRANSPARENCY ) );
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
			voxelGridDebugGeometry->scale( glm::vec3 ( voxelGrid->width, voxelGrid->height, voxelGrid->depth ) );
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
			guiRenderPass->addUniform( new Uniform<float >("uniformAlpha", new float( 1.0f ) ) );
			guiRenderPass->addUniform( new Uniform<float >("uniformTextureTransparency", new float( -1.0f ) ) );

			int buttonWidth = 128;
			int buttonHeight = 128;

			DEBUGLOG->log("Creating some debug views");

			// RSM Debug view
			std::vector<Texture* > debugRSMTextures;
			debugRSMTextures.push_back( rsmDepthMap );
			debugRSMTextures.push_back( rsmPositionMap );
			debugRSMTextures.push_back( rsmFluxMap );
			debugRSMTextures.push_back( rsmNormalMap );

			// create input field
			std::pair<InputField*, std::pair< Node*, RenderableNode*> > button = guiFrame->createButton(
				0, 0,
				buttonWidth, buttonHeight,
				&m_inputManager,
				GLFW_MOUSE_BUTTON_LEFT,
				&m_resourceManager,
				rsmDepthMap);

			// make interactive
			SwitchThroughValuesListener<Texture* >* debugRSMSwitcher = new SwitchThroughValuesListener<Texture* >(
							&( *( button.second.second->getObject()->getMaterial()->getTexturesPtr() ) )["uniformTexture"]
					        , debugRSMTextures
					);
			button.first->attachListenerOnRelease( debugRSMSwitcher );

			// add to gui render pass
			guiRenderPass->addRenderable( button.second.second );

			// other interesting debug textures
			std::vector<Texture* > debugTextures;
			debugTextures.push_back( textureAtlasRenderPass->getTextureAtlas() );
			debugTextures.push_back( gbufferNormalMap );
			debugTextures.push_back( gbufferColorMap );
			debugTextures.push_back( new Texture( gbufferCompositing->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0) ) );

			// create button
			button = guiFrame->createButton(
				buttonWidth, 0,
				buttonWidth, buttonHeight,
				&m_inputManager,
				GLFW_MOUSE_BUTTON_LEFT,
				&m_resourceManager,
				textureAtlasVertexGenerator->getTextureAtlas());

			// make interactive
			SwitchThroughValuesListener<Texture* >* debugTexturesSwitcher = new SwitchThroughValuesListener<Texture* >(
							&( *button.second.second->getObject()->getMaterial()->getTexturesPtr() )["uniformTexture"]
					        , debugTextures
					);
			button.first->attachListenerOnRelease( debugTexturesSwitcher );

			// add to gui render pass
			guiRenderPass->addRenderable( button.second.second );

			// RSM light gathering debug view
			std::vector<Texture* > debugRSMLightMapTextures;
			debugRSMLightMapTextures.push_back( rsmDirectLightMap );
			debugRSMLightMapTextures.push_back( rsmIndirectLightMap );

			// create button
			button = guiFrame->createButton(
				0, buttonHeight,
				buttonWidth, buttonHeight,
				&m_inputManager,
				GLFW_MOUSE_BUTTON_LEFT,
				&m_resourceManager,
				rsmDirectLightMap );


			// make interactive
			SwitchThroughValuesListener<Texture* >* debugRSMLightMapSwitcher = new SwitchThroughValuesListener<Texture* >(
							&( *button.second.second->getObject()->getMaterial()->getTexturesPtr() )["uniformTexture"]
					        , debugRSMLightMapTextures
					);
			button.first->attachListenerOnRelease( debugRSMLightMapSwitcher );

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

			DEBUGLOG->log("Switch active Voxelization Method      : X");
			m_inputManager.attachListenerOnKeyPress( new InvertBooleanListener( &VOXELIZE_REGULAR_ACTIVE ), GLFW_KEY_X, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( new DebugPrintListener( "Active voxelize mode     : regular"), &VOXELIZE_REGULAR_ACTIVE), GLFW_KEY_X, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( new DebugPrintListener( "Active voxelize mode     : texAtlas"), &VOXELIZE_REGULAR_ACTIVE, true), GLFW_KEY_X, GLFW_PRESS);

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

			DEBUGLOG->log("Reset scenegraph                       : R");
			m_inputManager.attachListenerOnKeyPress( new SimpleScene::SceneGraphState( scene->getSceneGraph() ),GLFW_KEY_R, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new SetValueListener<glm::vec3>(&LIGHT_POSITION, glm::vec3( 0.0f, 0.0f, 0.0f) ),GLFW_KEY_R, GLFW_PRESS);

			DEBUGLOG->log("Print compute shader execution times   : T");
			m_inputManager.attachListenerOnKeyPress( dispatchClearVoxelGridComputeShader->getPrintExecutionTimeListener(		"Clear Voxel Grid      "), GLFW_KEY_T, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeComputeShader->getPrintExecutionTimeListener(				"Voxelize Models       "), GLFW_KEY_T, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeWithTexAtlasComputeShader->getPrintExecutionTimeListener(	"Voxelize Texture Atlas"), GLFW_KEY_T, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( dispatchMipmapVoxelGridComputeShader->getPrintExecutionTimeListener(       "Mipmap Voxel Grid     "), GLFW_KEY_T, GLFW_PRESS);

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
			// TODO create some actual GUI elements for these
			InputField* inputFieldIncTransparency = new InputField(0,  0, 100, 256, &m_inputManager, GLFW_MOUSE_BUTTON_LEFT);
			InputField* inputFieldDecTransparency = new InputField(0,256, 100, 256, &m_inputManager, GLFW_MOUSE_BUTTON_LEFT);
			inputFieldIncTransparency->attachListenerOnPress( new IncrementValueListener<float>( &BACKGROUND_TRANSPARENCY, 0.1f ) );
			inputFieldIncTransparency->attachListenerOnPress( new DebugPrintValueListener<float>( &BACKGROUND_TRANSPARENCY, "background transparency : "));
			inputFieldDecTransparency->attachListenerOnPress( new DecrementValueListener<float>( &BACKGROUND_TRANSPARENCY, 0.1f ) );
			inputFieldDecTransparency->attachListenerOnPress( new DebugPrintValueListener<float>( &BACKGROUND_TRANSPARENCY, "background transparency : "));

			DEBUGLOG->log("De-/Increase visible voxel grid level  : K / L    ( overlay view only )");
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener< int >( &VISIBLE_TEXTURE_LEVEL, 1 ) , GLFW_KEY_L);
			m_inputManager.attachListenerOnKeyPress( new DecrementValueListener< int >( &VISIBLE_TEXTURE_LEVEL, 1 ) , GLFW_KEY_K);
			m_inputManager.attachListenerOnKeyPress( new DebugPrintValueListener< int >( &VISIBLE_TEXTURE_LEVEL, "visible voxel grid level : "), GLFW_KEY_L);
			m_inputManager.attachListenerOnKeyPress( new DebugPrintValueListener< int >( &VISIBLE_TEXTURE_LEVEL, "visible voxel grid level : "), GLFW_KEY_K);


		DEBUGLOG->outdent();

		DEBUGLOG->log("---------------------------------------------------------");
	}



	void programCycle()
	{
		call("CLEAR");					// call listeners attached to clear interface

		call("VOXELIZE");				// call listeners attached to voxelize interface

		call ("MIPMAP");				// call listeners attached to mipmap interface

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
