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

static bool rotatingObjects = false;

static int textureAtlasResolution = 512;
static int voxelGridResolution = 1024;

static glm::vec3 lightPosition = glm::vec3(2.5f, 2.5f, 2.5f);

class TransmittanceShadowMappingApp: public Application {
private:

	TexAtlas::TextureAtlasRenderPass* m_textureAtlasRenderer;
	TexAtlas::TextureAtlasVertexGenerator* m_textureAtlasVertexGenerator;

	class SliceMapShadowMappingRenderPass: public TriangleRenderPass {
	private:
		Camera* m_projectionCamera;
		Camera* m_gbufferCamera; // camera with which gbuffer was filled
		Texture* m_bitmask;
	public:
		SliceMapShadowMappingRenderPass(Shader* shader, FramebufferObject* fbo,
				Renderable* screenFillingTriangle, Camera* projectionCamera = 0,
				Camera* gbufferCamera = 0, Texture* bitmask = 0) :
				TriangleRenderPass(shader, fbo, screenFillingTriangle) {
			m_projectionCamera = projectionCamera;
			m_gbufferCamera = gbufferCamera;
			m_bitmask = bitmask;
		}

		virtual void preRender()
		{
			if (m_bitmask)
			{
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_1D, m_bitmask->getTextureHandle());
				glActiveTexture(GL_TEXTURE0);
			}
		}

		virtual void uploadUniforms() {
			TriangleRenderPass::uploadUniforms();

			if (m_projectionCamera != 0) {
				//upload projector view perspective matrix
				m_shader->uploadUniform(m_projectionCamera->getViewMatrix(),
						"uniformProjectorView");
				m_shader->uploadUniform(
						m_projectionCamera->getProjectionMatrix(),
						"uniformProjectorPerspective");
			}
			if (m_gbufferCamera != 0) {
				m_shader->uploadUniform(m_gbufferCamera->getViewMatrix(),
						"uniformView");
			}
			if( m_bitmask != 0)
			{
				m_shader->uploadUniform(5, "uniformBitMask");
			}
		}
	};

	CameraRenderPass* createUVRenderPass() {
		DEBUGLOG->indent();
		Shader* uvShader= new Shader(SHADERS_PATH "/textureAtlas/simpleUV.vert", SHADERS_PATH "/textureAtlas/worldPosition.frag");
		FramebufferObject* fbo = new FramebufferObject(512,512);
		fbo->addColorAttachments(1);

		CameraRenderPass* uvRenderPass = new CameraRenderPass(uvShader, fbo);
		uvRenderPass->setViewport(0,0,512,512);
		uvRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
		uvRenderPass->addEnable(GL_DEPTH_TEST);

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
	 * 	creates a Texture Atlas Renderpass for the provided renderable node in the provided resolution
	 * @param renderableNode node with object to be used
	 * @param atlasResolution desired resolution of atlas to be created
	 * @return
	 */
	TexAtlas::TextureAtlasRenderPass* createTextureAtlasRenderPass( RenderableNode* renderableNode , int atlasResolution = 512)
	{
		DEBUGLOG->log("Creating TextureAtlasRenderPass for provided renderable node");
		DEBUGLOG->indent();
		GLenum internalFormat = FramebufferObject::static_internalFormat;
		FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;	// change this first

		m_textureAtlasRenderer = new TexAtlas::TextureAtlasRenderPass( renderableNode, atlasResolution, atlasResolution );

		FramebufferObject::static_internalFormat = internalFormat;// restore default
		DEBUGLOG->outdent();

		return m_textureAtlasRenderer;
	}

	TexAtlas::TextureAtlasVertexGenerator* createTextureAtlasVertexGenerator( TexAtlas::TextureAtlas* textureAtlas )
	{
		DEBUGLOG->log("Creating TextureAtlasVertexGenerator for provided Texture Atlas");
		DEBUGLOG->indent();
		m_textureAtlasVertexGenerator = new TexAtlas::TextureAtlasVertexGenerator( textureAtlas );
		DEBUGLOG->outdent();

		return m_textureAtlasVertexGenerator;
	}

public:
	TransmittanceShadowMappingApp()
	{
		m_textureAtlasRenderer = 0;
		m_textureAtlasVertexGenerator = 0;

		m_name = "Transmittance Shadow Mapping App";
	}
	virtual ~TransmittanceShadowMappingApp()
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
		std::vector<RenderableNode* > renderables;

//		RenderableNode* testRoomNode = SimpleScene::loadTestRoomObject( this );
//		renderables.push_back(testRoomNode);

		RenderableNode* groundNode = SimpleScene::loadObject("/ground.dae",this);
		groundNode->translate( glm::vec3( 0.0f, -2.5f, 0.0f ) );
		renderables.push_back(groundNode);

		RenderableNode* treeTrunkObjectNode = SimpleScene::loadObject("/tree_trunk.dae" , this);
		treeTrunkObjectNode->translate( glm::vec3( 0.0f, -2.5f, 0.0f ) );

		RenderableNode* treeCrownObjectNode = SimpleScene::loadObject("/tree_crown.dae" , this);
		treeCrownObjectNode->translate( glm::vec3( 0.0f, -2.5f, 0.0f ) );

		renderables.push_back(treeTrunkObjectNode);
		renderables.push_back(treeCrownObjectNode);

		DEBUGLOG->log("Attaching objects to scene graph");
		DEBUGLOG->indent();
		if ( rotatingObjects )
		{
			std::pair<Node*, Node*> rotatingNodes = SimpleScene::createRotatingNodes( this, 0.1f, 0.1f);
			rotatingNodes.first->setParent( scene->getSceneGraph()->getRootNode() );

			for ( unsigned int i = 0; i < renderables.size(); i++ )
			{
				renderables[i]->setParent( rotatingNodes.second );
			}
		}
		else
		{
			for ( unsigned int i = 0; i < renderables.size(); i++ )
			{
				renderables[i]->setParent( scene->getSceneGraph()->getRootNode() );
			}
		}

//		testRoomNode->setParent(scene->getSceneGraph()->getRootNode() );
		DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								   TEXTURE ATLAS CONFIG
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

		// create TextureAtlas render pass
		TexAtlas::TextureAtlasRenderPass* textureAtlasRenderPass = createTextureAtlasRenderPass( treeCrownObjectNode , textureAtlasResolution);

		m_renderManager.addRenderPass( textureAtlasRenderPass );

		// create Texture Atlas vertex generator
		TexAtlas::TextureAtlasVertexGenerator* textureAtlasVertexGenerator = createTextureAtlasVertexGenerator( textureAtlasRenderPass->getTextureAtlas() );
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

		Texture* renderPassPerspectiveTexture = new Texture();
		renderPassPerspectiveTexture->setTextureHandle(phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
		showRenderPassPerspective->addUniformTexture(renderPassPerspectiveTexture, "uniformTexture");

		DEBUGLOG->log("Adding phong framebuffer presentation render pass now");
		// add screen fill render pass now
		m_renderManager.addRenderPass(showRenderPassPerspective);

		DEBUGLOG->outdent();

		DEBUGLOG->indent();
		DEBUGLOG->log("Creating Texture Atlas presentation render pass");
		TriangleRenderPass* showTextureAtlasRenderPass = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
		showTextureAtlasRenderPass->setViewport(0,513,188,188);

		showTextureAtlasRenderPass->addUniformTexture(textureAtlasRenderPass->getTextureAtlas(), "uniformTexture");

					m_renderManager.addRenderPass(showTextureAtlasRenderPass);
		DEBUGLOG->outdent();
		DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								TEXTURE ATLAS INITIALIZATION
		 **************************************************************************************/
		DEBUGLOG->log("Initializing Texture Atlas functionality");
		DEBUGLOG->indent();

		DEBUGLOG->log("Generating Texture Atlas valid coordinates");
		// render texture atlas once so it can be validated
		m_textureAtlasRenderer->render();

		DEBUGLOG->log("Generating Texture Atlas vertex array object");
		// generate vertices from texture atlas
		m_textureAtlasVertexGenerator->call();

		// attach to a Node for rendering
		RenderableNode* verticesNode = new RenderableNode( scene->getSceneGraph()->getRootNode());
		verticesNode->setObject( m_textureAtlasVertexGenerator->getPixelsObject() );

		glPointSize( 1.0f );

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/
//		DEBUGLOG->log("Configuring Voxelization");
//		DEBUGLOG->indent();
//
//		DEBUGLOG->log("Creating slice map render pass");
//		// shader using texture atlas vertex shader ( to move vertex to world position ) and slice map fragment shader ( to write into bit mask )
//		std::string vertexShader( SHADERS_PATH "/textureAtlas/textureAtlasWorldPosition.vert" );
//
//		// create slice map renderpass
//		SliceMap::SliceMapRenderPass* voxelizeWithTextureAtlas = SliceMap::getSliceMapRenderPass( 6.0f, 6.0f, 3.2f, voxelGridResolution, voxelGridResolution, 1, SliceMap::BITMASK_SINGLETARGET, vertexShader);
//
//		DEBUGLOG->log("Configuring slice map render pass");
//		// configure slice map render pass
//		voxelizeWithTextureAtlas->getCamera()->setPosition(1.5f,1.5f,1.5f);
//		voxelizeWithTextureAtlas->getCamera()->setCenter(glm::vec3(0.0f,0.0f,0.0f));
//
//		DEBUGLOG->log("Adding Texture Atlas vertices to slice map render pass");
//		// add texture atlas vertices
//		voxelizeWithTextureAtlas->addRenderable( verticesNode );
//
//		// add voxelization renderpass
//		m_renderManager.addRenderPass( voxelizeWithTextureAtlas );
//
//		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

		DEBUGLOG->log("Creating slice map render pass");
		// shader using texture atlas vertex shader ( to move vertex to world position ) and slice map fragment shader ( to write into bit mask )
		std::string vertexShader( SHADERS_PATH "/slicemap/simpleVertex.vert" );

		// create slice map renderpass
		SliceMap::SliceMapRenderPass* voxelizeWithTextureAtlas = SliceMap::getSliceMapRenderPass( 6.0f, 6.0f, 4.2f, voxelGridResolution, voxelGridResolution, 1, SliceMap::BITMASK_SINGLETARGET, vertexShader);

		DEBUGLOG->log("Configuring slice map render pass");
		// configure slice map render pass
		voxelizeWithTextureAtlas->getCamera()->setPosition(1.0f,1.0f,1.0f);
		voxelizeWithTextureAtlas->getCamera()->setCenter(glm::vec3(-1.0f,-1.0f,-1.0f));

		DEBUGLOG->log("Adding Texture Atlas vertices to slice map render pass");
		// add texture atlas vertices
//		voxelizeWithTextureAtlas->addRenderable( verticesNode );
		voxelizeWithTextureAtlas->addRenderable( treeCrownObjectNode );

		// add voxelization renderpass
		m_renderManager.addRenderPass( voxelizeWithTextureAtlas );

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 							VOXELIZATION PRESENTATION
		 **************************************************************************************/
		DEBUGLOG->log("Creating slice map presentation render passes");
		DEBUGLOG->indent();

		DEBUGLOG->log("Creating compositing framebuffer object");
		FramebufferObject* compositingFramebufferObject = new FramebufferObject(512,512);
		compositingFramebufferObject->addColorAttachments(1);

		Texture* composedImageTexture = new Texture();
		composedImageTexture->setTextureHandle(compositingFramebufferObject->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));

		int numColorAttachments =voxelizeWithTextureAtlas->getFramebufferObject()->getNumColorAttachments();
		Shader* overlaySliceMap = new Shader(SHADERS_PATH "/screenspace/screenFill.vert" ,SHADERS_PATH "/slicemap/sliceMapOverlay.frag");

		DEBUGLOG->log("Creating compositing render passes");
		for (int i = 0; i < numColorAttachments; i++)
		{
			Texture* voxelizeWithTextureAtlasTexture = new Texture();
			voxelizeWithTextureAtlasTexture->setTextureHandle(voxelizeWithTextureAtlas->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0 + i));

			TriangleRenderPass* composevoxelizeWithTextureAtlas = new TriangleRenderPass(overlaySliceMap, compositingFramebufferObject, m_resourceManager.getScreenFillingTriangle());
			composevoxelizeWithTextureAtlas->setViewport(0,0,512,512);
			composevoxelizeWithTextureAtlas->addUniformTexture(voxelizeWithTextureAtlasTexture, "uniformSliceMapTexture");
			composevoxelizeWithTextureAtlas->addUniformTexture(( i == 0 ) ? new Texture() : composedImageTexture, "uniformBaseTexture");

			m_renderManager.addRenderPass(composevoxelizeWithTextureAtlas);
		}

		DEBUGLOG->log("Creating composed image presentation render pass");
		TriangleRenderPass* showComposedImage= new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());

		showComposedImage->setViewport(188,512,188,188);
		showComposedImage->addUniformTexture(composedImageTexture, "uniformTexture");

		m_renderManager.addRenderPass(showComposedImage);
		DEBUGLOG->outdent();

		DEBUGLOG->log("Creating Texture Atlas vertices world position render pass");
		DEBUGLOG->indent();

			// a Renderpass which transforms the vertices to the world position proposed by the texture atlas
			Shader* transformTextureAtlasShader = new Shader( SHADERS_PATH "/textureAtlas/textureAtlasWorldPosition.vert" , SHADERS_PATH "/screenspace/simpleTexture.frag");
			CameraRenderPass* transformVerticesByTextureAtlasRenderPass = new CameraRenderPass(transformTextureAtlasShader, 0);

			transformVerticesByTextureAtlasRenderPass->setViewport(3 * 188, 512, 188, 188);

			transformVerticesByTextureAtlasRenderPass->setCamera( phongPerspectiveRenderPass->getCamera() );
			transformVerticesByTextureAtlasRenderPass->addRenderable( verticesNode );
			transformVerticesByTextureAtlasRenderPass->addEnable(GL_DEPTH_TEST);

			// add vertex rendering before image presentation
			m_renderManager.addRenderPass( transformVerticesByTextureAtlasRenderPass );

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 									SHADOW MAPPING
		 **************************************************************************************/
		// Render Tree Trunk once
		Shader* writeDepthBufferShader = new Shader( SHADERS_PATH "/myShader/simpleVertex.vert", SHADERS_PATH "/myShader/simpleDepth.frag");
		FramebufferObject* depthFramebufferObject = new FramebufferObject( voxelGridResolution, voxelGridResolution);

		CameraRenderPass* writeDepthBufferRenderPass = new CameraRenderPass(writeDepthBufferShader, depthFramebufferObject);
		writeDepthBufferRenderPass->addEnable(GL_DEPTH_TEST);
		writeDepthBufferRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
		writeDepthBufferRenderPass->setCamera( voxelizeWithTextureAtlas->getCamera() );

		writeDepthBufferRenderPass->addRenderable(treeTrunkObjectNode);
		writeDepthBufferRenderPass->addRenderable(groundNode);

		m_renderManager.addRenderPass( writeDepthBufferRenderPass );


		// Render Scene into GBUFFER )
		Shader* writeGbufferShader = new Shader(SHADERS_PATH "/gbuffer/gbuffer.vert", SHADERS_PATH "/gbuffer/gbuffer_backfaceCulling_persp.frag");

		GLenum internalFormat = FramebufferObject::static_internalFormat;
		FramebufferObject::static_internalFormat = GL_RGBA32F_ARB;

		FramebufferObject* gbufferFramebufferObject = new FramebufferObject (512,512);
		// 3 attachments : position, normals, color
		gbufferFramebufferObject->addColorAttachments(3);

		FramebufferObject::static_internalFormat = internalFormat;

		CameraRenderPass* writeGbufferRenderPass = new CameraRenderPass(writeGbufferShader, gbufferFramebufferObject);
		writeGbufferRenderPass->addEnable(GL_DEPTH_TEST);
		writeGbufferRenderPass->setClearColor(0.0f,0.0f,0.0f,1.0f);
		writeGbufferRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);
		writeGbufferRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);

		writeGbufferRenderPass->setCamera( phongPerspectiveRenderPass->getCamera() );

		// render everything
		for( unsigned int i = 0; i < renderables.size(); i++)
		{
			writeGbufferRenderPass->addRenderable( renderables[i] );
		}

		m_renderManager.addRenderPass(writeGbufferRenderPass);

		// assertion : shadow map ( slice map ) has been rendered before

		Shader* shadowMappingShader = new Shader(SHADERS_PATH "/screenspace/screenFill.vert", SHADERS_PATH "/shadowMapping/shadowmappingSliceMap.frag");

		FramebufferObject* shadowMappingFramebufferObject = new FramebufferObject(512,512);
		shadowMappingFramebufferObject->addColorAttachments(1);// write one texture

		// take gbuffer and slice map and create a shadow mapped image in screen space
		SliceMapShadowMappingRenderPass* shadowMappingRenderPass = new SliceMapShadowMappingRenderPass(
				shadowMappingShader,
				shadowMappingFramebufferObject,
				m_resourceManager.getScreenFillingTriangle(),
				voxelizeWithTextureAtlas->getCamera(),
				writeGbufferRenderPass->getCamera(),
				SliceMap::get8BitRGBAMask());
		shadowMappingRenderPass->addUniformTexture( new Texture( gbufferFramebufferObject->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) ), "uniformPositionMap" );// position map
		shadowMappingRenderPass->addUniformTexture( new Texture( gbufferFramebufferObject->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT1 ) ), "uniformNormalMap" );// normal   map
		shadowMappingRenderPass->addUniformTexture( new Texture( gbufferFramebufferObject->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT2 ) ), "uniformColorMap" );// color    map

		shadowMappingRenderPass->addUniformTexture( new Texture( voxelizeWithTextureAtlas->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) ), "uniformSliceMap" );// slice  map
		shadowMappingRenderPass->addUniformTexture( new Texture( depthFramebufferObject->getDepthTextureHandle()), "uniformShadowMap" ); // shadow map

		m_renderManager.addRenderPass( shadowMappingRenderPass );

		DEBUGLOG->log("Creating shadow mapped gbuffer presentation render pass");
		TriangleRenderPass* showShadowMappedGbuffer= new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());

		showShadowMappedGbuffer->setViewport(2 * 188,512, 188, 188);
//		showShadowMappedGbuffer->addUniformTexture( new Texture( shadowMappingRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) ), "uniformTexture");
		showShadowMappedGbuffer->addUniformTexture( new Texture( depthFramebufferObject->getDepthTextureHandle() ), "uniformTexture");

		m_renderManager.addRenderPass(showShadowMappedGbuffer);

		DEBUGLOG->log("Creating composed image presentation render pass");
		Shader* multiplyTexture = new Shader( SHADERS_PATH "/screenspace/screenFill.vert", SHADERS_PATH "/screenspace/multiplyTexture.frag");
		TriangleRenderPass* showShadowMappedImage= new TriangleRenderPass(multiplyTexture, 0, m_resourceManager.getScreenFillingTriangle());

		showShadowMappedImage->setViewport(512,0,512,512);
		showShadowMappedImage->addUniformTexture( new Texture( shadowMappingRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) ), "uniformMultiply");
		showShadowMappedImage->addUniformTexture( new Texture( phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0 ) ), "uniformTexture");

		m_renderManager.addRenderPass(showShadowMappedImage);

		/**************************************************************************************
		 * 									INPUT
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
	Application::static_newWindowHeight = 700;

	TransmittanceShadowMappingApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	return 0;
}
