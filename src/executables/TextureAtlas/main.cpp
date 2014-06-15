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

static bool rotatingBunny			= true;

static int textureAtlasResolution   = 256;
static int voxelGridResolution		= 128;

class TextureAtlasBuildingApp : public Application
{
private:

	TexAtlas::TextureAtlasRenderPass* m_textureAtlasRenderer;
	TexAtlas::TextureAtlasVertexGenerator* m_textureAtlasVertexGenerator;

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
			GLenum internalFormat = FramebufferObject::internalFormat;
			FramebufferObject::internalFormat = GL_RGBA32F_ARB;	// change this first

			m_textureAtlasRenderer = new TexAtlas::TextureAtlasRenderPass( renderableNode, atlasResolution, atlasResolution );

			FramebufferObject::internalFormat = internalFormat;	// restore default
		DEBUGLOG->outdent();

		return m_textureAtlasRenderer;
	}


	TexAtlas::TextureAtlasVertexGenerator* createTextureAtlasVertexGenerator( 	TexAtlas::TextureAtlas* textureAtlas )
	{
		DEBUGLOG->log("Creating TextureAtlasVertexGenerator for provided Texture Atlas");
		DEBUGLOG->indent();
			m_textureAtlasVertexGenerator = new TexAtlas::TextureAtlasVertexGenerator( textureAtlas );
		DEBUGLOG->outdent();

		return m_textureAtlasVertexGenerator;
	}

public:
	TextureAtlasBuildingApp()
	{
		m_textureAtlasRenderer = 0;
		m_textureAtlasVertexGenerator = 0;

		m_name = "Texture Atlas Voxelization App";
	}
	virtual ~TextureAtlasBuildingApp()
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

			RenderableNode* someObjectNode = SimpleScene::loadObject("/stanford/bunny/blender_bunny.dae" , this);

			DEBUGLOG->log("Scaling object node up by 25");
			someObjectNode->scale( glm::vec3( 25.0f, 25.0f, 25.0f ) );

			renderables.push_back(someObjectNode);

			DEBUGLOG->log("Attaching objects to scene graph");
			DEBUGLOG->indent();
				if ( rotatingBunny )
				{
					std::pair<Node*, Node*> rotatingNodes = SimpleScene::createRotatingNodes( this, 0.1f, 0.1f);
					rotatingNodes.first->setParent( scene->getSceneGraph()->getRootNode() );

					someObjectNode->setParent( rotatingNodes.second );
				}
				else
				{
					someObjectNode->setParent(scene->getSceneGraph()->getRootNode() );
				}

				testRoomNode->setParent(scene->getSceneGraph()->getRootNode() );
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								   TEXTURE ATLAS CONFIG
		 **************************************************************************************/

		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

			// create TextureAtlas render pass
			TexAtlas::TextureAtlasRenderPass* textureAtlasRenderPass = createTextureAtlasRenderPass( someObjectNode , textureAtlasResolution);

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

					Texture* renderPassPerspectiveTexture = new Texture();
					renderPassPerspectiveTexture->setTextureHandle(phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassPerspective->addUniformTexture(renderPassPerspectiveTexture, "uniformTexture");

					// dont add to render loop yet
					DEBUGLOG->log("NOT Adding phong presentation render pass yet.");

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

			glPointSize( 2.0f );

			DEBUGLOG->log("Creating Texture Atlas vertices world position render pass");
			DEBUGLOG->indent();

				DEBUGLOG->log("Using phong framebuffer as target");
				// a Renderpass which transforms the vertices to the world position proposed by the texture atlas
				Shader* transformTextureAtlasShader = new Shader( SHADERS_PATH "/textureAtlas/textureAtlasWorldPosition.vert" , SHADERS_PATH "/screenspace/simpleTexture.frag");
				CameraRenderPass* transformVerticesByTextureAtlasRenderPass = new CameraRenderPass(transformTextureAtlasShader, phongPerspectiveRenderPass->getFramebufferObject());

				transformVerticesByTextureAtlasRenderPass->setCamera( phongPerspectiveRenderPass->getCamera() );
				transformVerticesByTextureAtlasRenderPass->addRenderable( verticesNode );
				transformVerticesByTextureAtlasRenderPass->addEnable(GL_DEPTH_TEST);

				// add vertex rendering before image presentation
				m_renderManager.addRenderPass( transformVerticesByTextureAtlasRenderPass );

			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding phong framebuffer presentation render pass now");
			// add screen fill render pass now
			m_renderManager.addRenderPass(showRenderPassPerspective);

		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/
		DEBUGLOG->log("Configuring Voxelization");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating slice map render pass");
			// shader using texture atlas vertex shader ( to move vertex to world position ) and slice map fragment shader ( to write into bit mask )
			std::string vertexShader( SHADERS_PATH "/textureAtlas/textureAtlasWorldPosition.vert" );

			// create slice map renderpass
			SliceMap::SliceMapRenderPass* voxelizeWithTextureAtlas = SliceMap::getSliceMapRenderPass( 5.0f, 5.0f, 5.0f, voxelGridResolution, voxelGridResolution, 4, SliceMap::BITMASK_MULTIPLETARGETS, vertexShader);

			DEBUGLOG->log("Configuring slice map render pass");
			// configure slice map render pass
			voxelizeWithTextureAtlas->getCamera()->setPosition(0.0f,0.0f,2.5f);	// tiny offset to set infront of bunny

			DEBUGLOG->log("Adding Texture Atlas vertices to slice map render pass");
			// add texture atlas vertices
			voxelizeWithTextureAtlas->addRenderable( verticesNode );

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
				composevoxelizeWithTextureAtlas->addUniformTexture(( i == 0 ) ? new Texture() : composedImageTexture,  "uniformBaseTexture");

				m_renderManager.addRenderPass(composevoxelizeWithTextureAtlas);
			}

			DEBUGLOG->log("Creating composed image presentation render pass");
			TriangleRenderPass* showComposedImage= new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());

			showComposedImage->setViewport(512,0,512,512);
			showComposedImage->addUniformTexture(composedImageTexture, "uniformTexture");

			m_renderManager.addRenderPass(showComposedImage);
		DEBUGLOG->outdent();

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

		DEBUGLOG->outdent();
	}
};

int main(){
	// configure a little bit
	Application::static_newWindowHeight = 700;

	TextureAtlasBuildingApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	
	return 0;
}
