#include <Application/Application.h>

#include <iostream>

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

#include <Misc/MiscListeners.h>
#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>
#include <Misc/SimpleSceneTools.h>
#include <Utility/Timer.h>

#include <Input/InputField.h>

#include "VoxelGridTools.h"
#include "VoxelizerTools.h"

static bool rotatingBunny = false;

static bool voxelizeRegularActive = true;
static bool voxelizeActive = true;

static int texAtlasResolution  = 512;
static int voxelGridResolution = 64;
static float voxelGridWidth = 8.0f;
static float voxelGridHeight = 8.0f;

static glm::vec3 lightPosition = glm::vec3(3.0f, 3.0f, 3.0f);
static bool enableBackfaceCulling = true;
static bool orthoCam = true;
static float backgroundTransparency = 0.25;

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
		glBindImageTexture(0,
		p_texture->getTextureHandle(),
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


/**
 * Clear voxel grid texture with 0
 */
class DispatchClearVoxelGridComputeShader : public DispatchComputeShaderListener
{
protected:
	VoxelGridGPU* p_voxelGrid;
public:
	DispatchClearVoxelGridComputeShader(ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x = 0, int y = 0, int z = 0 )
	: DispatchComputeShaderListener(computeShader, x, y, z)
{
		p_voxelGrid = voxelGrid;
}
	void call()
	{
		// only thing that works
//		glBindTexture(GL_TEXTURE_2D, p_voxelGrid->handle);
//
//		std::vector < GLuint > emptyData( voxelGridResolution * voxelGridResolution , 0);
//		glTexSubImage2D(
//				GL_TEXTURE_2D,	// target
//				0,				// level
//				0,				// xOffset
//				0,				// yOffset
//				voxelGridResolution, // width
//				voxelGridResolution, // height
//				GL_RED,			// format
//				GL_UNSIGNED_INT,// type
//				&emptyData[0] );// data
//
//		glBindTexture(GL_TEXTURE_2D, 0);

		p_computeShader->useProgram();

		// upload clear texture index binding
		glBindImageTexture(0,
		p_voxelGrid->handle,
		0,
		GL_FALSE,
		0,
		GL_WRITE_ONLY,						// only write
		GL_R32UI);							// 1 channel 32 bit unsigned int

		// set suitable amount of work groups
		m_num_groups_x = voxelGridResolution / p_computeShader->getLocalGroupSizeX() + ( ( voxelGridResolution % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
		m_num_groups_y = voxelGridResolution / p_computeShader->getLocalGroupSizeY() + ( ( voxelGridResolution % p_computeShader->getLocalGroupSizeY() == 0 ) ? 0 : 1 );
		m_num_groups_z = 1;

		// dispatch as usual
		DispatchComputeShaderListener::call();

		// barrier bla whatever
		glMemoryBarrier( GL_ALL_BARRIER_BITS );

		// unbind clear texture
		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI );
	}
};

/**
 * Voxelize scene via tex atlas
 */
class DispatchVoxelizeWithTexAtlasComputeShader : public DispatchComputeShaderListener
{
protected:

	std::vector<std::pair<Object*, TexAtlas::TextureAtlas* > > m_objects;

	VoxelGridGPU* p_voxelGrid;
	Texture* p_bitMask;
public:
	DispatchVoxelizeWithTexAtlasComputeShader(ComputeShader* computeShader, std::vector< std::pair<Object*, TexAtlas::TextureAtlas*> > objects, VoxelGridGPU* voxelGrid, Texture* bitMask, int x= 0, int y= 0, int z = 0 )
	: DispatchComputeShaderListener(computeShader, x,y,z)
	{
		m_objects = objects;
		p_voxelGrid = voxelGrid;
		p_bitMask = bitMask;
	}

	void call()
	{
		// use compute program
		p_computeShader->useProgram();

		// upload output texture
		glBindImageTexture(0,
		p_voxelGrid->handle,
		0,
		GL_FALSE,
		0,
		GL_READ_WRITE,						// allow both for atomic operations
		GL_R32UI);							// 1 channel 32 bit unsigned int to make sure OR-ing works

		// upload bit mask
		glBindImageTexture(1,
		p_bitMask->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_READ_ONLY,
		GL_R32UI
		);

		double totalExecutionTime = 0.0;

		// dispatch this shader once per object
		for ( unsigned int i = 0; i < m_objects.size(); i++)
		{
			Object* pixelsObject = m_objects[i].first;
			TexAtlas::TextureAtlas* textureAtlas = m_objects[i].second;

			int numVertices = 0;

			if ( pixelsObject->getModel() )
			{
				// bind positions VBO to shader storage buffer
				glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, pixelsObject->getModel()->getPositionBufferHandle() );

				numVertices = pixelsObject->getModel()->getNumVertices();
			}
			else
			{
				continue;
			}

			if (textureAtlas)
			{
				textureAtlas->unbindFromActiveUnit();

				// bind textureAtlas
				textureAtlas->bindToTextureUnit(3);

				// upload uniform textureAtlas position
				if ( !p_computeShader->uploadUniform(3 , "uniformTextureAtlas" ) )
				{
					DEBUGLOG->log("ERROR : Failed to upload uniform texture atlas");
				}
			}

			// upload uniform voxel grid matrix
			p_computeShader->uploadUniform( p_voxelGrid->worldToVoxel, "uniformWorldToVoxel" );

			// upload uniform vertices amount
			p_computeShader->uploadUniform( numVertices, "uniformNumVertices");

			// set local group amount suitable for object size:
			m_num_groups_x = numVertices / p_computeShader->getLocalGroupSizeX() + ( ( numVertices % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
			m_num_groups_y = 1;
			m_num_groups_z = 1;

			// dispatch as usual
			DispatchComputeShaderListener::call();

			glMemoryBarrier( GL_ALL_BARRIER_BITS );

			if ( m_queryTime )
			{
				totalExecutionTime += m_executionTime;
			}
		}

		if ( m_queryTime )
		{
			m_executionTime = totalExecutionTime;
		}

		// unbind image textures
		glBindImageTexture(0, 0, 0,
		GL_FALSE, 0,
		GL_READ_WRITE,
		GL_R32UI);

		glBindImageTexture(1, 0, 0,
		GL_FALSE, 0,
		GL_READ_ONLY,
		GL_R32UI);
	}
};

/**
 * Voxelize scene
 */
class DispatchVoxelizeComputeShader : public DispatchComputeShaderListener
{
protected:

	std::vector<std::pair<Object*, RenderableNode* > > m_objects;

	VoxelGridGPU* p_voxelGrid;
	Texture* p_bitMask;
public:
	DispatchVoxelizeComputeShader(ComputeShader* computeShader, std::vector< std::pair<Object*, RenderableNode*> > objects,
			VoxelGridGPU* voxelGrid, Texture* bitMask,
			int x= 0, int y= 0, int z = 0 )
	: DispatchComputeShaderListener(computeShader, x,y,z)
{
		m_objects = objects;
		p_voxelGrid = voxelGrid;
		p_bitMask = bitMask;
}
	void call()
	{
		// use compute program
		p_computeShader->useProgram();

		// unbind output texture
		p_voxelGrid->texture->unbindFromActiveUnit();

		// upload output texture
		glBindImageTexture(0,
		p_voxelGrid->handle,
		0,
		GL_FALSE,
		0,
		GL_READ_WRITE,						// allow both
		GL_R32UI);							// 1 channel 32 bit unsigned int to make sure OR-ing works

		// upload bit mask
		glBindImageTexture(1,
		p_bitMask->getTextureHandle(),
		0,
		GL_FALSE,
		0,
		GL_READ_ONLY,
		GL_R32UI
		);

		double totalExecutionTime = 0.0;

		// dispatch this shader once per object
		for ( unsigned int i = 0; i < m_objects.size(); i++)
		{
			Object* object = m_objects[i].first;
			RenderableNode* objectNode = m_objects[i].second;

			int numVertices = 0;
			int numIndices = 0;
			int numFaces = 0;

			if ( object->getModel() )
			{
				// bind positions VBO to shader storage buffer
				glBindBufferBase(
						GL_SHADER_STORAGE_BUFFER,   // target
						0,							// target binding index
						object->getModel()->getPositionBufferHandle() );	// buffer

				numVertices = object->getModel()->getNumVertices();

				// bind index buffer
				glBindBufferBase(
						GL_SHADER_STORAGE_BUFFER,	// target
						1,							// target binding index
						object->getModel()->getIndexBufferHandle() ); // buffer

				numIndices = object->getModel()->getNumIndices();
				numFaces = object->getModel()->getNumFaces();
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

			// upload voxel grid info
			p_computeShader->uploadUniform( p_voxelGrid->worldToVoxel, "uniformWorldToVoxel");

			// upload geometric info
			p_computeShader->uploadUniform( numVertices, "uniformNumVertices");
			p_computeShader->uploadUniform( numIndices, "uniformNumIndices");
			p_computeShader->uploadUniform( numFaces, "uniformNumFaces");

			// set local group amount suitable for object size:
			m_num_groups_x = numFaces / p_computeShader->getLocalGroupSizeX() + ( ( numVertices % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
			m_num_groups_y = 1;
			m_num_groups_z = 1;

			// dispatch as usual
			DispatchComputeShaderListener::call();

			glMemoryBarrier( GL_ALL_BARRIER_BITS );

			if ( m_queryTime )
			{
				totalExecutionTime += m_executionTime;
			}
		}

		if ( m_queryTime )
		{
			m_executionTime = totalExecutionTime;
		}

		// unbind image textures
		glBindImageTexture(0, 0, 0,
		GL_FALSE, 0,
		GL_READ_WRITE,
		GL_R32UI);
		glBindImageTexture(1, 0, 0,
		GL_FALSE, 0,
		GL_READ_ONLY,
		GL_R32UI);
	}
};

class ComputeShaderApp: public Application {
private:
	Node* m_cameraParentNode;
	Node* m_objectsNode;

	CameraRenderPass* createGBufferRenderPass( )
	{
		DEBUGLOG->indent();

		// Render Scene into GBUFFER )
		Shader* writeGbufferShader = new Shader(SHADERS_PATH "/gbuffer/gbuffer.vert", SHADERS_PATH "/gbuffer/gbuffer_backfaceCulling.frag");

		GLenum internalFormat = FramebufferObject::static_internalFormat;
		FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;

		FramebufferObject* gbufferFramebufferObject = new FramebufferObject (512,512);
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

	CameraRenderPass* createPhongRenderPass( )
	{
		DEBUGLOG->indent();
		Shader* phongPersp= new Shader(SHADERS_PATH "/myShader/phong_uniformLight.vert", SHADERS_PATH "/myShader/phong_backfaceCulling_ortho.frag");
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
//		camera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
//		camera->setProjectionMatrix(voxelizePerspective);
//		camera->setPosition( voxelizePosition );
		phongPerspectiveRenderPass->setCamera(camera);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding render pass to application");
		m_renderManager.addRenderPass(phongPerspectiveRenderPass);

		return phongPerspectiveRenderPass;
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
			DEBUGLOG->log("Creating gbuffer renderpass");
			CameraRenderPass* gbufferRenderPass = createGBufferRenderPass();

			gbufferRenderPass->addUniform(new Uniform< bool >( std::string( "uniformEnableBackfaceCulling" ),    &enableBackfaceCulling ) );
			gbufferRenderPass->addUniform(new Uniform< bool >( std::string( "uniformOrtho" ),    &orthoCam ) );

			Texture* gbufferPositionMap = new Texture( gbufferRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0 ) );
			Texture* gbufferNormalMap = new Texture( gbufferRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT1 ) );
			Texture* gbufferColorMap = new Texture( gbufferRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT2 ) );

			DEBUGLOG->log("Adding objects to perspective phong render pass");
			for (unsigned int i = 0; i < renderables.size(); i++)
			{
				gbufferRenderPass->addRenderable( renderables[i] );
			}

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

					gbufferCompositing->addUniform(new Uniform< glm::vec3 >( std::string( "uniformLightPosition" ), &lightPosition  ) );
					gbufferCompositing->addUniform(new Uniform< glm::mat4 >( std::string( "uniformViewMatrix" ),    gbufferRenderPass->getCamera()->getViewMatrixPointer() ) );

					DEBUGLOG->log("Adding compositing render pass now");
					m_renderManager.addRenderPass(gbufferCompositing);
				DEBUGLOG->outdent();

			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELGRID CREATION
		 **************************************************************************************/

		DEBUGLOG->log("Configuring voxel grid");
		DEBUGLOG->indent();
			VoxelGridGPU* voxelGrid = new VoxelGridGPU();

			voxelGrid->setUniformCellSizeFromResolutionAndMapping(voxelGridWidth,voxelGridHeight, voxelGridResolution, voxelGridResolution, 32);

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
			
			// buffer empty data, i.e. clear memory
			glBufferData( GL_TEXTURE_2D, // target
					voxelGridResolution * voxelGridResolution, // amount
					NULL,				// no initial data
					GL_DYNAMIC_COPY );	// usage of buffer

			// clear texture
			std::vector < GLuint > emptyData( voxelGridResolution * voxelGridResolution , 0);
			glTexSubImage2D(
					GL_TEXTURE_2D,	// target
					0,				// level
					0,				// xOffset
					0,				// yOffset
					voxelGridResolution, // width
					voxelGridResolution, // height
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
				TexAtlas::TextureAtlasRenderPass* textureAtlasRenderPass = new TexAtlas::TextureAtlasRenderPass(bunnyNode, texAtlasResolution, texAtlasResolution, gbufferRenderPass->getCamera() );

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

			DEBUGLOG->log("Enabling execution time queries");
			dispatchClearVoxelGridComputeShader->setQueryTime( true );
			dispatchVoxelizeComputeShader->setQueryTime( true );
			dispatchVoxelizeWithTexAtlasComputeShader->setQueryTime( true );

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
					&voxelizeActive,
					false),
				"CLEAR");

			attach(
					new ConditionalProxyListener(
					new ConditionalProxyListener(
						dispatchVoxelizeComputeShader,
						&voxelizeRegularActive,
						false ),
					&voxelizeActive,
					false),
				"VOXELIZE"
			);

			attach(
					new ConditionalProxyListener(
					new ConditionalProxyListener(
							dispatchVoxelizeWithTexAtlasComputeShader,
							&voxelizeRegularActive,
							true),
					&voxelizeActive,
					false),
				"VOXELIZE");

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION DISPLAY RENDERING
		 **************************************************************************************/
		DEBUGLOG->log("Configuring display of voxelized scene");
		DEBUGLOG->indent();

			// Align Camera with voxelization view
			gbufferRenderPass->getCamera()->setProjectionMatrix( glm::ortho( voxelGrid->width * -0.5f, voxelGrid->width * 0.5f, voxelGrid->height * -0.5f, voxelGrid->height * 0.5f, -10.0f, 15.0f) );
			gbufferRenderPass->getCamera()->setPosition( glm::vec3 ( glm::inverse ( voxelGrid->view ) * glm::vec4 ( 0.0, 0.0f, voxelGrid->depth / 2.0f, 1.0f ) ) );

			Shader* 			overlaySliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH "/sliceMap/sliceMapOverLayGLSL4_3.frag");
			TriangleRenderPass* overlaySliceMap = new OverlayR32UITextureRenderPass(
					overlaySliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					compositingOutput,
					voxelGrid->texture );
			overlaySliceMap->addUniform( new Uniform<float>("uniformBackgroundTransparency", &backgroundTransparency) );

			Shader* projectSliceMapShader = new Shader( SHADERS_PATH "/screenspace/screenFillGLSL4_3.vert", SHADERS_PATH"/sliceMap/sliceMapProjectionGLSL4_3.frag");
			TriangleRenderPass* projectSliceMap = new ProjectSliceMapRenderPass(
					projectSliceMapShader,
					0,
					m_resourceManager.getScreenFillingTriangle(),
					compositingOutput,
					voxelGrid,
					SliceMap::get32BitUintMask(),
					gbufferPositionMap,
					gbufferRenderPass->getCamera()->getViewMatrixPointer()
					);
			projectSliceMap->addClearBit( GL_COLOR_BUFFER_BIT );
			projectSliceMap->addUniform( new Uniform<float>( "uniformBackgroundTransparency", &backgroundTransparency ) );

			RenderPass* showSliceMap = projectSliceMap;

			// create listener to switch through display methods
			int showSliceMapIndex = m_renderManager.addRenderPass( showSliceMap );

			std::vector <RenderPass* > showSliceMapCandidates;
			showSliceMapCandidates.push_back( projectSliceMap );
			showSliceMapCandidates.push_back( overlaySliceMap );

			SwitchThroughValuesListener< RenderPass* >* switchShowSliceMaps = new SwitchThroughValuesListener<RenderPass*>(
							&( *m_renderManager.getRenderPassesPtr())[showSliceMapIndex],
							showSliceMapCandidates
							);

		DEBUGLOG->outdent();
		/**************************************************************************************
		* 								INPUT CONFIGURATION
		**************************************************************************************/

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->log("---------------------------------------------------------");
		DEBUGLOG->indent();
			DEBUGLOG->log("Disable/Enable real-time voxelization : Y");
			m_inputManager.attachListenerOnKeyPress( new InvertBooleanListener( &voxelizeActive ), GLFW_KEY_Y, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new DebugPrintBooleanListener(&voxelizeActive,          "Voxelize real-time enabled : "), GLFW_KEY_Y, GLFW_PRESS);

			DEBUGLOG->log("Switch active Voxelization Method     : X");
			m_inputManager.attachListenerOnKeyPress( new InvertBooleanListener( &voxelizeRegularActive ), GLFW_KEY_X, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( new DebugPrintListener( "Active voxelize mode     : regular"), &voxelizeRegularActive), GLFW_KEY_X, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( new DebugPrintListener( "Active voxelize mode     : texAtlas"), &voxelizeRegularActive, true), GLFW_KEY_X, GLFW_PRESS);

			DEBUGLOG->log("Clear voxel grid on key press         : C");
			m_inputManager.attachListenerOnKeyPress(  dispatchClearVoxelGridComputeShader, GLFW_KEY_C, GLFW_PRESS);

			DEBUGLOG->log("Voxelize scene with active voxelizer  : V");
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( dispatchVoxelizeComputeShader, &voxelizeRegularActive ), GLFW_KEY_V, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( new ConditionalProxyListener( dispatchVoxelizeWithTexAtlasComputeShader, &voxelizeRegularActive, true ), GLFW_KEY_V, GLFW_PRESS);

			DEBUGLOG->log("Switch voxel grid display             : B");
			m_inputManager.attachListenerOnKeyPress( switchShowSliceMaps, GLFW_KEY_B, GLFW_PRESS );

			DEBUGLOG->log("Reset scenegraph                      : R");
			m_inputManager.attachListenerOnKeyPress( new SimpleScene::SceneGraphState(scene->getSceneGraph()),GLFW_KEY_R, GLFW_PRESS);

			DEBUGLOG->log("Print compute shader execution times  : T");
			m_inputManager.attachListenerOnKeyPress( dispatchClearVoxelGridComputeShader->getPrintExecutionTimeListener(		"Clear Voxel Grid      "), GLFW_KEY_T, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeComputeShader->getPrintExecutionTimeListener(				"Voxelize Models       "), GLFW_KEY_T, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress( dispatchVoxelizeWithTexAtlasComputeShader->getPrintExecutionTimeListener(	"Voxelize Texture Atlas"), GLFW_KEY_T, GLFW_PRESS);

			DEBUGLOG->log("Turn camera                           : MOUSE - RIGHT");
			Camera* movableCam = gbufferRenderPass->getCamera();
			SimpleScene::configureSimpleCameraMovement(movableCam, this, 2.5f);

			DEBUGLOG->log("Turn objects                          : MOUSE - LEFT");
			Turntable* turntable = SimpleScene::configureTurnTable( m_objectsNode, this, 0.05f, GLFW_MOUSE_BUTTON_LEFT, gbufferRenderPass->getCamera() );
			Turntable* turntableCam = SimpleScene::configureTurnTable( m_cameraParentNode, this, 0.05f , GLFW_MOUSE_BUTTON_RIGHT, gbufferRenderPass->getCamera());

			DEBUGLOG->log("Configuring light movement            : Arrow keys");
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(0.0f,0.0f, 1.0f) ), GLFW_KEY_DOWN, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(0.0f,0.0f, -1.0f) ), GLFW_KEY_UP, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(-1.0f,0.0f, 0.0f) ), GLFW_KEY_LEFT, GLFW_PRESS );
			m_inputManager.attachListenerOnKeyPress( new IncrementValueListener<glm::vec3>( &lightPosition, glm::vec3(1.0f,0.0f, 1.0f) ), GLFW_KEY_RIGHT, GLFW_PRESS );

			DEBUGLOG->log("In- /Decrease background transparency : upper / lower left corner");
			InputField* inputFieldIncTransparency = new InputField(0,  0, 100, 256, &m_inputManager, GLFW_MOUSE_BUTTON_LEFT);
			InputField* inputFieldDecTransparency = new InputField(0,256, 100, 256, &m_inputManager, GLFW_MOUSE_BUTTON_LEFT);
			inputFieldIncTransparency->attachListenerOnPress( new IncrementValueListener<float>( &backgroundTransparency, 0.1f ) );
			inputFieldIncTransparency->attachListenerOnPress( new DebugPrintValueListener<float>( &backgroundTransparency, "background transparency : "));
			inputFieldDecTransparency->attachListenerOnPress( new DecrementValueListener<float>( &backgroundTransparency, 0.1f ) );
			inputFieldDecTransparency->attachListenerOnPress( new DebugPrintValueListener<float>( &backgroundTransparency, "background transparency : "));

		DEBUGLOG->outdent();

		DEBUGLOG->log("---------------------------------------------------------");
	}



	void programCycle()
	{
		call("CLEAR");					// call listeners attached to clear interface

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
