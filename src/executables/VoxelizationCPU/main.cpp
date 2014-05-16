#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "CustomRenderPasses.h"
#include "Listeners.h"
#include "VoxelGrid.h"

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>

#include <Misc/Turntable.h>
#include <Misc/RotatingNode.h>

class ObjectLoadingApp : public Application
{
	public:
	virtual ~ObjectLoadingApp()
	{

	}
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading cube dae file");
			DEBUGLOG->indent();
				std::vector< Object* > daeCube = m_resourceManager.loadObjectsFromFile(RESOURCES_PATH "/cube.dae");
				DEBUGLOG->log("Loading custom Texture for dae Cube");
				daeCube[0]->getMaterial()->setTexture("diffuseTexture", m_resourceManager.loadTexture("cvlogo.png", RESOURCES_PATH "/"));
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading cube obj file");
			DEBUGLOG->indent();
				std::vector< Object* > objCube =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/cube.obj" );
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading background dae file");
			DEBUGLOG->indent();
				std::vector< Object* > daeBackground=  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/background.dae" );
			DEBUGLOG->outdent();

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
			scene->addObjects( daeCube );
			scene->addObjects( objCube );
			scene->addObjects( daeBackground );
			scene->addObjects( testRoom );
			scene->addObjects( overlappingGeometry );

			DEBUGLOG->log("Creating scene graph nodes");
			DEBUGLOG->indent();

				DEBUGLOG->log("Creating renderable node for .dae cube");
				RenderableNode* cubeNode1 = new RenderableNode( scene->getSceneGraph()->getRootNode( ) );
				cubeNode1->rotate( glm::rotate (glm::mat4(1.0f), 60.0f, glm::vec3(0.0f,1.0f,0.0f) ) );
				cubeNode1->translate( glm::translate( glm::mat4(1.0f), glm::vec3(1.5f, 0.5f,-1.0f) ) );
				cubeNode1->setObject(daeCube[0]);

				DEBUGLOG->log("Creating node tree for rotating node");
				Node* positionNode = new Node(scene->getSceneGraph()->getRootNode());
				positionNode->translate( glm::translate(glm::mat4(1.0f),  glm::vec3(0.0f, 0.0f, 0.0f) ) );

				RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
				yAxisRotationNode->setAngle(0.005f);
				yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );

				RotatingNode* rotatingNode = new RotatingNode(yAxisRotationNode);
				rotatingNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
				rotatingNode->setAngle(0.01f);

				DEBUGLOG->log("Adding updatable rotation nodes to scene");
				scene->addUpdatable(yAxisRotationNode);
				scene->addUpdatable(rotatingNode);

				DEBUGLOG->log("Creating renderable node for overlapping geometry attached to rotating node");
				RenderableNode* overlappingGeometryNode = new RenderableNode(rotatingNode);
				overlappingGeometryNode->setObject(overlappingGeometry[0]);

				DEBUGLOG->log("Creating renderable node for .obj cube attached to rotating node");
				RenderableNode* cubeNode_2 = new RenderableNode(rotatingNode);
				cubeNode_2->setObject(objCube[0]);

				DEBUGLOG->log("Creating renderable node for background");
				RenderableNode* backgroundNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				backgroundNode->translate( glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, -1.0f ,0.0f) ) );
				backgroundNode->setObject(daeBackground[0]);

				DEBUGLOG->log("Creating renderable node for test room");
				RenderableNode* testRoomNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				testRoomNode->scale(glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)));
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


			DEBUGLOG->log("Adding objects to perspective phong render pass");
			phongPerspectiveRenderPass->addRenderable(overlappingGeometryNode);
			phongPerspectiveRenderPass->addRenderable(testRoomNode);

			DEBUGLOG->log("Adding objects to ortho phong render pass");
//			phongOrthoRenderPass->addRenderable(cubeNode1);
//			phongOrthoRenderPass->addRenderable(backgroundNode);
//			phongOrthoRenderPass->addRenderable(cubeNode_2);
			phongOrthoRenderPass->addRenderable(overlappingGeometryNode);
			phongOrthoRenderPass->addRenderable(testRoomNode);

			DEBUGLOG->log("Adding renderpasses to application");
			m_renderManager.addRenderPass(phongPerspectiveRenderPass);
			m_renderManager.addRenderPass(phongOrthoRenderPass);

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

				DEBUGLOG->log("Creating screen filling triangle rendering for perspective phong render pass");
				DEBUGLOG->indent();
					TriangleRenderPass* showRenderPassPerspective = new TriangleRenderPass(showTexture, 0, m_resourceManager.getScreenFillingTriangle());
					showRenderPassPerspective->setViewport(512,0,512,512);

					Texture* renderPassPerspectiveTexture = new Texture();
					renderPassPerspectiveTexture->setTextureHandle(phongPerspectiveRenderPass->getFramebufferObject()->getColorAttachmentTextureHandle(GL_COLOR_ATTACHMENT0));
					showRenderPassPerspective->addUniformTexture(renderPassPerspectiveTexture, "uniformTexture");

					m_renderManager.addRenderPass(showRenderPassPerspective);
				DEBUGLOG->outdent();
			DEBUGLOG->outdent();

			DEBUGLOG->indent();
				DEBUGLOG->log("Setting the slicemap's camera in ortho phong renderpass");
			DEBUGLOG->outdent();

		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring voxel grid");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating voxel grid object");
			DEBUGLOG->indent();
				Grid::AxisAlignedVoxelGrid* axisAlignedVoxelGrid = new Grid::AxisAlignedVoxelGrid(-5.0f,-5.0f,5.0f,100,100,100,0.1f);
				DEBUGLOG->log("Grid width    : ", axisAlignedVoxelGrid->getWidth());
				DEBUGLOG->log("Grid height   : ", axisAlignedVoxelGrid->getHeight());
				DEBUGLOG->log("Grid depth    : ", axisAlignedVoxelGrid->getDepth());
				DEBUGLOG->log("Grid cell size: ", axisAlignedVoxelGrid->getCellSize());
			DEBUGLOG->outdent();

			DEBUGLOG->log("Filling voxel grid");
			DEBUGLOG->indent();
				std::vector< Object* > objects = scene->getObjects();
				for (unsigned int i = 0; i < objects.size(); i++)
				{
					Model* model = objects[i]->getModel();
					Node* objectNode = scene->getSceneGraph()->findObjectNode(objects[i]);
					glm::mat4 modelMatrix;
					if (objectNode)
					{
						modelMatrix = objectNode->getAccumulatedModelMatrix();
						DEBUGLOG->log("found object Node:", (int) objectNode);
					}
					std::vector < glm::vec4 > assimpMesh = m_resourceManager.getAssimpMeshForModel(model);
						DEBUGLOG->log("found assimp mesh with vertices :", (int) assimpMesh.size());
						// TODO fill voxel grid by checking vertices against grid volume
						for (unsigned int j = 0; j < assimpMesh.size(); j++)
						{
							glm::vec4 transformedVertex = modelMatrix * assimpMesh[j];
							DEBUGLOG->log("transformed vertex: ", transformedVertex);
							Grid::GridCell* gridCell = axisAlignedVoxelGrid->getGridCell(glm::vec3(transformedVertex.x,transformedVertex.y,transformedVertex.z) );
							if (gridCell != 0 && !gridCell->isOccupied())
							{
								gridCell->setOccupied(true);
								DEBUGLOG->log("Set Grid Cell to occupied :", (int) gridCell);
							}
						}
				}
			DEBUGLOG->outdent();

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
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);

			scene->addUpdatable(turntable);
		DEBUGLOG->outdent();

	}
};

int main(){

	ObjectLoadingApp myApp;

	myApp.configure();

	myApp.initialize();
	myApp.run();

	
	return 0;
}
