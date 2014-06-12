#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Rendering/CustomRenderPasses.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>
#include <Voxelization/VoxelGrid.h>

#include <Misc/MiscListeners.h>
#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>

class GridRenderPass : public CameraRenderPass
{
protected:
public:

	GridRenderPass(Shader* shader, FramebufferObject* fbo)
	: CameraRenderPass(shader, fbo)
	{

	}

	void enableStates()
	{
		RenderPass::enableStates();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void restoreStates()
	{
		RenderPass::restoreStates();
		glBlendFunc(GL_ONE, GL_ONE);
	}
};

class UniformVoxelGridApp : public Application
{
	public:
	virtual ~UniformVoxelGridApp()
	{

	}
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading test room dae file");
			DEBUGLOG->indent();
				std::vector< Object* > testRoom=  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/testRoom.dae" );
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading overlapping geometry file");
			DEBUGLOG->indent();
				std::vector< Object* > overlappingGeometry =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/overlappingGeometry.dae" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( testRoom );
			scene->addObjects( overlappingGeometry );

			DEBUGLOG->log("Creating scene graph nodes");
			DEBUGLOG->indent();

				DEBUGLOG->log("Creating node tree for rotating node");
				Node* positionNode = new Node(scene->getSceneGraph()->getRootNode());
				positionNode->translate( glm::vec3(0.0f, 0.0f, 0.0f) );

				RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
				yAxisRotationNode->setAngle(0.005f);
				yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );

				RotatingNode* rotatingNode = new RotatingNode(yAxisRotationNode);
				rotatingNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
				rotatingNode->setAngle(0.01f);

				DEBUGLOG->log("Adding updatable rotation nodes to scene");
//				scene->addUpdatable(yAxisRotationNode);
//				scene->addUpdatable(rotatingNode);

				DEBUGLOG->log("Creating renderable node for overlapping geometry attached to rotating node");
				RenderableNode* overlappingGeometryNode = new RenderableNode(rotatingNode);
				overlappingGeometryNode->setObject(overlappingGeometry[0]);

				DEBUGLOG->log("Creating renderable node for test room");
				RenderableNode* testRoomNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				testRoomNode->scale( glm::vec3(0.75f, 0.75f, 0.75f) );
				testRoomNode->setObject(testRoom[0]);

			DEBUGLOG->outdent();
		DEBUGLOG->outdent();
		
		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);	

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();

			DEBUGLOG->log("Creating phong renderpass");
			DEBUGLOG->indent();
				Shader* phongOrtho = new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong_backfaceCulling_ortho.frag");
				FramebufferObject* fbo = new FramebufferObject(512,512);
				fbo->addColorAttachments(1);

				CameraRenderPass* phongOrthoRenderPass = new CameraRenderPass(phongOrtho, fbo);
				phongOrthoRenderPass->setViewport(0,0,512,512);
				phongOrthoRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
				phongOrthoRenderPass->addEnable(GL_DEPTH_TEST);
				phongOrthoRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
				phongOrthoRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

				Camera* orthographicCamera = new Camera();
				orthographicCamera->setProjectionMatrix(glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 20.0f));
				orthographicCamera->setPosition(0.0f,0.0f,5.0f);
				phongOrthoRenderPass->setCamera(orthographicCamera);
			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating perspective phong renderpass");
			DEBUGLOG->indent();
				Shader* phongPersp= new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong_backfaceCulling_persp.frag");
				FramebufferObject* fbo2 = new FramebufferObject(512,512);
				fbo2->addColorAttachments(1);

				CameraRenderPass* phongPerspectiveRenderPass = new CameraRenderPass(phongPersp, fbo2);
				phongPerspectiveRenderPass->setViewport(0,0,512,512);
				phongPerspectiveRenderPass->setClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
				phongPerspectiveRenderPass->addEnable(GL_DEPTH_TEST);
				phongPerspectiveRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);
				phongPerspectiveRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

				Camera* perspectiveCamera = new Camera();
				perspectiveCamera->setProjectionMatrix(glm::perspective(60.0f, 1.0f, 0.1f, 100.0f));
				perspectiveCamera->setPosition(0.0f,0.0f,5.0f);
				phongPerspectiveRenderPass->setCamera(perspectiveCamera);
			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating grid overlay renderpasses");
			DEBUGLOG->indent();
				Shader* gridPersp= new Shader(SHADERS_PATH "/grid/simpleVertex.vert", SHADERS_PATH "/grid/simpleColor.frag");

				GridRenderPass* gridPerspectiveRenderPass = new GridRenderPass(gridPersp, fbo2);	// just render on top of that other render pass
				gridPerspectiveRenderPass->setViewport(0,0,512,512);
				gridPerspectiveRenderPass->addEnable(GL_DEPTH_TEST);
				gridPerspectiveRenderPass->addEnable(GL_BLEND);

				gridPerspectiveRenderPass->setCamera(perspectiveCamera);

				GridRenderPass* gridOrthoRenderPass = new GridRenderPass(gridPersp, fbo);	// just render on top of that other render pass
				gridOrthoRenderPass->setViewport(0,0,512,512);
				gridOrthoRenderPass->addEnable(GL_DEPTH_TEST);
				gridOrthoRenderPass->addEnable(GL_BLEND);

				gridOrthoRenderPass->setCamera(orthographicCamera);

			DEBUGLOG->outdent();

			DEBUGLOG->log("Adding objects to perspective phong render pass");
			phongPerspectiveRenderPass->addRenderable(overlappingGeometryNode);
			phongPerspectiveRenderPass->addRenderable(testRoomNode);
		//	phongPerspectiveRenderPass->addRenderable( m_resourceManager.getCube() );

			DEBUGLOG->log("Adding objects to ortho phong render pass");
			phongOrthoRenderPass->addRenderable(overlappingGeometryNode);
			phongOrthoRenderPass->addRenderable(testRoomNode);

			DEBUGLOG->log("Adding renderpasses to application");
			m_renderManager.addRenderPass(phongPerspectiveRenderPass);
			m_renderManager.addRenderPass(phongOrthoRenderPass);
			m_renderManager.addRenderPass(gridPerspectiveRenderPass);
			m_renderManager.addRenderPass(gridOrthoRenderPass);

			DEBUGLOG->log("Creating screen filling triangle render passes");
			DEBUGLOG->indent();
				Shader* showTexture = new Shader(SHADERS_PATH "/screenspace/screenFill.vert" ,SHADERS_PATH "/screenspace/simpleTexture.frag");
				DEBUGLOG->indent();
					DEBUGLOG->log("Creating screen filling triangle rendering on screen for ortho phong render pass");
					TriangleRenderPass* showRenderPass = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPass->setViewport(0,0,512,512);
					showRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);

					Texture* renderPassTexture = new Texture();
					renderPassTexture->setTextureHandle(phongOrthoRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPass->addUniformTexture(renderPassTexture, "uniformTexture");

					m_renderManager.addRenderPass(showRenderPass);
				DEBUGLOG->outdent();

				DEBUGLOG->indent();
				DEBUGLOG->log("Creating screen filling triangle rendering for perspective phong render pass");
					TriangleRenderPass* showRenderPassPerspective = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassPerspective->setViewport(512,0,512,512);

					Texture* renderPassPerspectiveTexture = new Texture();
					renderPassPerspectiveTexture->setTextureHandle(phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassPerspective->addUniformTexture(renderPassPerspectiveTexture, "uniformTexture");

					m_renderManager.addRenderPass(showRenderPassPerspective);
				DEBUGLOG->outdent();
			DEBUGLOG->outdent();
		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring voxel grid");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating voxel grid object");
			DEBUGLOG->indent();
				Grid::AxisAlignedVoxelGrid* axisAlignedVoxelGrid = new Grid::AxisAlignedVoxelGrid(-5.0f, -5.0f, -5.0f, 32, 32, 32, 0.3125f);
				DEBUGLOG->log("Grid width    : ", axisAlignedVoxelGrid->getWidth());
				DEBUGLOG->log("Grid height   : ", axisAlignedVoxelGrid->getHeight());
				DEBUGLOG->log("Grid depth    : ", axisAlignedVoxelGrid->getDepth());
				DEBUGLOG->log("Grid cell size: ", axisAlignedVoxelGrid->getCellSize());
			DEBUGLOG->outdent();

			DEBUGLOG->log("Voxelizing scene");
			DEBUGLOG->indent();

				// objects to be voxelized
				std::vector< Object* > objects = scene->getObjects();
				int filledCells = 0;

				for (unsigned int i = 0; i < objects.size(); i++)
				{
					Model* model = objects[i]->getModel();

//					DEBUGLOG->log("trying to find object: ", (int) objects[i]);
					Node* objectNode = scene->getSceneGraph()->findObjectNode(objects[i]);

					glm::mat4 modelMatrix;
					if (objectNode)
					{
//						DEBUGLOG->log("found object Node:", (int) objectNode);
						modelMatrix = objectNode->getAccumulatedModelMatrix();
					}
					std::vector < glm::vec4 > assimpMesh = m_resourceManager.getAssimpMeshForModel(model);
//					DEBUGLOG->log("found assimp mesh with vertices :", (int) assimpMesh.size());

					// fill voxel grid by checking vertices against grid volume
						for (unsigned int j = 0; j < assimpMesh.size(); j++)
						{
							glm::vec4 worldSpaceVertex = modelMatrix * assimpMesh[j];

							Grid::GridCell* gridCell = axisAlignedVoxelGrid->getGridCell(glm::vec3(worldSpaceVertex.x,worldSpaceVertex.y,worldSpaceVertex.z) );

							if (gridCell && (! (gridCell->isOccupied( ) ) )  )
							{
								gridCell->setOccupied(true);
								filledCells ++;

//								DEBUGLOG->log("Creating Cube Node for rendering purposes");

									RenderableNode* filledCell = new RenderableNode(scene->getSceneGraph()->getRootNode() );
									filledCell->scale( glm::vec3(axisAlignedVoxelGrid->getCellSize() ) );
									filledCell->translate(  axisAlignedVoxelGrid->getGridCellCenter( glm::vec3( worldSpaceVertex.x ,worldSpaceVertex.y ,worldSpaceVertex.z  ) ) );
									filledCell->setObject( m_resourceManager.getCube( ) );

									gridOrthoRenderPass->addRenderable( filledCell );
									gridPerspectiveRenderPass->addRenderable( filledCell );

//								DEBUGLOG->log("Set Grid Cell to occupied :", (int) gridCell);
							}
						}
				}

				// TODO : find a way to clone objects and attach objects with the same model but different material to the nodes
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformRed", 0.5f);
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformGreen", 0.1f);
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformBlue", 0.1f);
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformAlpha", 0.8f);


				DEBUGLOG->log("Filled voxel grid cells: ", filledCells);
				DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid model");
			Model* gridModel = m_resourceManager.generateVoxelGridModel(axisAlignedVoxelGrid->getWidth(), axisAlignedVoxelGrid->getHeight(), axisAlignedVoxelGrid->getDepth(), axisAlignedVoxelGrid->getCellSize());
			axisAlignedVoxelGrid->setModel(gridModel);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformRed",   0.5f);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformGreen", 0.5f);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformBlue",  0.5f);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformAlpha", 0.2f);

			DEBUGLOG->log("Creating renderable node voxel grid object");
			RenderableNode* axisAlignedVoxelGridNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
			axisAlignedVoxelGridNode->setObject(axisAlignedVoxelGrid);
			axisAlignedVoxelGridNode->translate( glm::vec3(axisAlignedVoxelGrid->getX(), axisAlignedVoxelGrid->getY(), axisAlignedVoxelGrid->getZ() ) );

			DEBUGLOG->log("Adding voxel grid object to render passes");
			gridPerspectiveRenderPass->addRenderable(axisAlignedVoxelGridNode);
			gridOrthoRenderPass->addRenderable(axisAlignedVoxelGridNode);

		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();
			DEBUGLOG->log("Configuring camera movement");
			Camera* movableCam = phongOrthoRenderPass->getCamera();
			Camera* movableCamClone = phongPerspectiveRenderPass->getCamera();
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 1.0f),GLFW_KEY_W, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_W, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 1.0f),GLFW_KEY_D, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_D, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, -1.0f),GLFW_KEY_S, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_S, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, -1.0f),GLFW_KEY_A, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_A, GLFW_RELEASE);

			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, 1.0f),GLFW_KEY_W, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_W, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, 1.0f),GLFW_KEY_D, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_D, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, -1.0f),GLFW_KEY_S, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_S, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, -1.0f),GLFW_KEY_A, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_A, GLFW_RELEASE);

			DEBUGLOG->log("Adding updatable camera to scene");
			scene->addUpdatable(movableCam);
			scene->addUpdatable(movableCamClone);

			DEBUGLOG->log("Configuring Turntable for root node");
			Turntable* turntable = new Turntable(scene->getSceneGraph()->getRootNode(), &m_inputManager);
			turntable->setSensitivity(0.05f);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);

			scene->addUpdatable(turntable);
		DEBUGLOG->outdent();

	}
};

int main(){

	UniformVoxelGridApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	
	return 0;
}
