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
#include <Scene/CameraNode.h>

class SceneGraphState : public Listener
{
protected:
	SceneGraph* p_sceneGraph;
	std::map<Node*, glm::mat4 > m_modelMatrices;	// maps a Node to a model matrix
	void traverseAndAdd( Node* parent )
	{
		std::vector<Node* > children = parent->getChildren();
		for ( std::vector<Node* >::iterator it = children.begin(); it != children.end(); ++it)
		{
			m_modelMatrices[ (*it) ] = (*it)->getModelMatrix();
			traverseAndAdd(*it);
		}
	}
	void traverseAndRestore( Node* parent)
	{
		std::vector<Node* > children = parent->getChildren();

		if ( children.size() != 0 )
		{
			DEBUGLOG->log("num children : ", children.size());
		}

		for ( std::vector<Node* >::iterator it = children.begin(); it != children.end(); ++it)
		{
			if ( m_modelMatrices.find( (*it) ) != m_modelMatrices.end())
			{
				(*it)->setModelMatrix( m_modelMatrices[ (*it) ] );
			}
			traverseAndRestore(*it);
		}
	}
public:
	SceneGraphState( SceneGraph* sceneGraph)
	{
		p_sceneGraph = sceneGraph;
		Node* root = sceneGraph->getRootNode();
		m_modelMatrices[root] = root->getModelMatrix();
		traverseAndAdd(root);
	}
	void restoreSceneGraph(SceneGraph* sceneGraph)
	{
		DEBUGLOG->log("Restoring scene graph");
		DEBUGLOG->indent();

		Node* root = sceneGraph->getRootNode();
		if ( m_modelMatrices.find( root ) != m_modelMatrices.end())
		{
			root->setModelMatrix( m_modelMatrices[ root ] );
		}
		traverseAndRestore(root);

		DEBUGLOG->outdent();
	}
	void call()
	{
		if(p_sceneGraph)
		{
			restoreSceneGraph(p_sceneGraph);
		}
	}
};

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

/**
 * class that resets and voxelizes a set of objects upon call
 */
class Voxelizer : public Listener
{
protected:
	ResourceManager* p_resourceManager;
	Scene*			 p_scene;
	Node* 		     p_parentNode;
	Grid::AxisAlignedVoxelGrid* p_axisAlignedVoxelGrid;
	std::vector <Object*> m_objects; // objects to be voxelized
	std::vector < std::pair < Grid::GridCell*, glm::vec3 > > m_filledGridCells;	// vector to keep track of filled cells
	std::vector < RenderableNode* > m_renderableNodes;	// vector consisiting of all generated renderable nodes
	std::vector < RenderPass* > p_gridCellRenderPasses; 	// renderpasses to be updated with the renderablenodes
public:
	Voxelizer(Grid::AxisAlignedVoxelGrid* axisAlignedVoxelGrid, Scene* scene, ResourceManager* resourceManager, const std::vector<Object* >& objects, Node* parentNode, std::vector <RenderPass* > gridCellRenderPasses = std::vector<RenderPass* >())
	{
		p_axisAlignedVoxelGrid = axisAlignedVoxelGrid;
		p_resourceManager = resourceManager;
		p_scene = scene;
		p_gridCellRenderPasses = gridCellRenderPasses;
		p_parentNode = parentNode;

		m_objects = objects;
	}

	void clear()
	{
		DEBUGLOG->log("Clearing voxel grid and renderable nodes");

		// delete all references to filled grid cells from previous calls
		for ( std::vector<std::pair<Grid::GridCell*, glm::vec3> >::iterator it = m_filledGridCells.begin(); it != m_filledGridCells.end(); ++it)
		{
			(*it).first->setOccupied( false );
		}
		m_filledGridCells.clear();

		// delete all grid cell nodes from previous calls
		for ( int i = m_renderableNodes.size()-1 ; i >= 0; i--)
		{
			for ( unsigned int j = 0; j < p_gridCellRenderPasses.size(); j++)
			{
				p_gridCellRenderPasses[j]->removeRenderable( m_renderableNodes[i] );
			}

			delete ( m_renderableNodes[i] );
		}
		m_renderableNodes.clear();
	}

	void voxelize()
	{
		DEBUGLOG->log("Voxelizing scene");
		DEBUGLOG->indent();

		int filledCells = 0;

		for (unsigned int i = 0; i < m_objects.size(); i++)
		{
			filledCells += voxelizeObject(m_objects[i]);
		}

		DEBUGLOG->log("Filled voxel grid cells: ", filledCells);
		DEBUGLOG->outdent();

	}

	int voxelizeObject( Object* object )
	{
		int filledCells = 0;
		Model* model = object->getModel();

		Node* objectNode = p_scene->getSceneGraph()->findObjectNode( object );

		glm::mat4 modelMatrix;
		if (objectNode)
		{
			modelMatrix = objectNode->getAccumulatedModelMatrix();
		}

		const std::vector < glm::vec4 >& assimpMesh = p_resourceManager->getAssimpMeshForModel(model);
		const std::vector < std::vector <unsigned int> >& assimpMeshFaces = p_resourceManager->getAssimpMeshFacesForModel(model);

		// fill voxel grid by checking faces against grid volume
		for (unsigned int j = 0; j < assimpMeshFaces.size(); j++)
		{
			std::vector< glm::vec3 > worldSpaceFaceVertices;

			// retrieve current face vertices indices
			std::vector< unsigned int > currentFace = assimpMeshFaces[j];

			// retrieve world space face vertices
			for (unsigned int k = 0; k < currentFace.size(); k++)
			{
				// retrieve vertex by face index
				glm::vec4 worldSpaceVertex = modelMatrix * assimpMesh[ currentFace[k] ];
				if ( worldSpaceVertex.w != 1.0f )
				{
					DEBUGLOG->log("WARNING : Weird homogeneous coordinate detected: ", worldSpaceVertex.w);
				}
				worldSpaceFaceVertices.push_back( glm::vec3(worldSpaceVertex) );
			}

			// retrieve intersected grid cells of voxel grid
			std::vector< std::pair< Grid::GridCell*, glm::vec3 > > gridCells = p_axisAlignedVoxelGrid->getGridCellsForTriangle( worldSpaceFaceVertices );

			// for every intersected grid cell : check occupation and set if empty
			for (unsigned int k = 0; k < gridCells.size(); k++)
			{
				if (gridCells[k].first && (! (gridCells[k].first->isOccupied( ) ) ) )
				{
					gridCells[k].first->setOccupied(true);
					m_filledGridCells.push_back( gridCells[k] );
					filledCells ++;
				}
			}
		}
		return filledCells;
	}

	void generateRenderableNodes()
	{
		DEBUGLOG->log("Generating renderable nodes from filled cells");
		for ( unsigned int i = 0; i < m_filledGridCells.size(); i++)
		{
			RenderableNode* filledCell = new RenderableNode( p_parentNode );

			filledCell->scale( glm::vec3(p_axisAlignedVoxelGrid->getCellSize() ) );
			filledCell->translate( p_axisAlignedVoxelGrid->getGridCellCenter( m_filledGridCells[i].second) );
			filledCell->setObject( p_resourceManager->getCube( ) );

			m_renderableNodes.push_back(filledCell);
		}
	}

	void updateRenderPasses()
	{
		for ( unsigned int i = 0; i < p_gridCellRenderPasses.size(); i++)
		{
			DEBUGLOG->log("Updating render pass renderables with renderable nodes");

			for ( std::vector<RenderableNode* >::iterator it = m_renderableNodes.begin(); it != m_renderableNodes.end(); ++it)
			{
				p_gridCellRenderPasses[i]->addRenderable( (*it) );
			}
		}
	}

	virtual void call()
	{
		clear();

		voxelize();

		generateRenderableNodes();

		updateRenderPasses();
	}

	virtual ~Voxelizer()
	{

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

//				DEBUGLOG->log("Creating node tree for rotating node");
//				Node* positionNode = new Node(scene->getSceneGraph()->getRootNode());
//				positionNode->translate( glm::vec3(0.0f, 0.0f, 0.0f) );

//				RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
//				yAxisRotationNode->setAngle(0.005f);
//				yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );
//
//				RotatingNode* rotatingNode = new RotatingNode(yAxisRotationNode);
//				rotatingNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
//				rotatingNode->setAngle(0.01f);

//				DEBUGLOG->log("Adding updatable rotation nodes to scene");
//				scene->addUpdatable(yAxisRotationNode);
//				scene->addUpdatable(rotatingNode);

//				DEBUGLOG->log("Creating renderable node for overlapping geometry attached to rotating node");
//				RenderableNode* overlappingGeometryNode = new RenderableNode(rotatingNode);

				Node* sceneNode = new Node(scene->getSceneGraph()->getRootNode() );

				RenderableNode* overlappingGeometryNode = new RenderableNode( sceneNode );
				overlappingGeometryNode->setObject(overlappingGeometry[0]);

				DEBUGLOG->log("Creating renderable node for test room");
				RenderableNode* testRoomNode = new RenderableNode( sceneNode );
				testRoomNode->scale( glm::vec3(0.6f, 0.6f, 0.6f) );
				testRoomNode->setObject(testRoom[0]);

				DEBUGLOG->log("Creating camera parent node");
				Node* 		cameraParentNode = new Node( scene->getSceneGraph()->getRootNode() );

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

				CameraNode* orthographicCamera = new CameraNode(cameraParentNode);
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

				CameraNode* perspectiveCamera = new CameraNode(cameraParentNode);
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

		/**************************************************************************************
		 * 								VOXELGRID SETUP
		 **************************************************************************************/

		DEBUGLOG->log("Configuring voxel grid");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating voxel grid object");
			DEBUGLOG->indent();
				Grid::AxisAlignedVoxelGrid* axisAlignedVoxelGrid = new Grid::AxisAlignedVoxelGrid(-5.0f, -5.0f, -5.0f, 20, 20, 20, 0.5f);
				DEBUGLOG->log("Grid width    : ", axisAlignedVoxelGrid->getWidth());
				DEBUGLOG->log("Grid height   : ", axisAlignedVoxelGrid->getHeight());
				DEBUGLOG->log("Grid depth    : ", axisAlignedVoxelGrid->getDepth());
				DEBUGLOG->log("Grid cell size: ", axisAlignedVoxelGrid->getCellSize());
			DEBUGLOG->outdent();

			DEBUGLOG->log("Creating parent node for voxel grid related geometry");
			Node* voxelGridNode = new Node( scene->getSceneGraph()->getRootNode() );
		DEBUGLOG->outdent();

		/**************************************************************************************
		 * 								VOXELIZATION
		 **************************************************************************************/
		DEBUGLOG->log("Voxelizing scene");
		DEBUGLOG->indent();
			DEBUGLOG->log("Creating voxelizer object");
			DEBUGLOG->indent();
				// objects to be voxelized
				std::vector<Object* > objects = scene->getObjects();
				std::vector<RenderPass* > gridCellRenderPasses;
				gridCellRenderPasses.push_back( gridOrthoRenderPass );
				gridCellRenderPasses.push_back( gridPerspectiveRenderPass );
				// create voxelizer
				Voxelizer* voxelizer = new Voxelizer(axisAlignedVoxelGrid, scene, &m_resourceManager, objects, voxelGridNode, gridCellRenderPasses);

				// configure display of cells
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformRed", 0.5f);
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformGreen", 0.1f);
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformBlue", 0.1f);
				m_resourceManager.getCube()->getMaterial()->setAttribute("uniformAlpha", 0.6f);

				// call voxelizer once
				DEBUGLOG->log("Executing voxelization");
				DEBUGLOG->indent();
					voxelizer->call();
				DEBUGLOG->outdent();
			DEBUGLOG->outdent();
		DEBUGLOG->outdent();

			DEBUGLOG->log("Creating voxel grid model");
			Model* gridModel = m_resourceManager.generateVoxelGridModel(axisAlignedVoxelGrid->getWidth(), axisAlignedVoxelGrid->getHeight(), axisAlignedVoxelGrid->getDepth(), axisAlignedVoxelGrid->getCellSize());
			axisAlignedVoxelGrid->setModel(gridModel);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformRed",   0.7f);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformGreen", 0.7f);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformBlue",  0.7f);
			axisAlignedVoxelGrid->getMaterial()->setAttribute("uniformAlpha", 0.1f);

			DEBUGLOG->log("Creating renderable node voxel grid object");
			RenderableNode* axisAlignedVoxelGridNode = new RenderableNode( voxelGridNode );
			axisAlignedVoxelGridNode->setObject(axisAlignedVoxelGrid);
			axisAlignedVoxelGridNode->translate( glm::vec3(axisAlignedVoxelGrid->getX(), axisAlignedVoxelGrid->getY(), axisAlignedVoxelGrid->getZ() ) );

//			DEBUGLOG->log("Adding voxel grid object to render passes");
//			gridPerspectiveRenderPass->addRenderable(axisAlignedVoxelGridNode);
//			gridOrthoRenderPass->addRenderable(axisAlignedVoxelGridNode);

		DEBUGLOG->outdent();

		DEBUGLOG->log("Configuring Input");
		DEBUGLOG->indent();
			DEBUGLOG->log("Configuring camera movement");
			Camera* movableCam = phongOrthoRenderPass->getCamera();
			Camera* movableCamClone = phongPerspectiveRenderPass->getCamera();
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 2.5f),GLFW_KEY_W, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_W, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 2.5f),GLFW_KEY_D, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_D, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, -2.5f),GLFW_KEY_S, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_S, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, -2.5f),GLFW_KEY_A, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_A, GLFW_RELEASE);

			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, 2.5f),GLFW_KEY_W, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_W, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, 2.5f),GLFW_KEY_D, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_D, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, -2.5f),GLFW_KEY_S, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_S, GLFW_RELEASE);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, -2.5f),GLFW_KEY_A, GLFW_PRESS);
			m_inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCamClone, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_A, GLFW_RELEASE);

			// voxelize on key press : V
			m_inputManager.attachListenerOnKeyPress( voxelizer, GLFW_KEY_V, GLFW_PRESS);

			// restore on key press  : R
			m_inputManager.attachListenerOnKeyPress( new SceneGraphState(scene->getSceneGraph()), GLFW_KEY_R, GLFW_PRESS);

			DEBUGLOG->log("Adding updatable camera to scene");
			scene->addUpdatable(movableCam);
			scene->addUpdatable(movableCamClone);

			DEBUGLOG->log("Configuring Turntable for scene node");
			Turntable* turntable = new Turntable( sceneNode, &m_inputManager);
			turntable->setSensitivity(0.05f);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntable), GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
			scene->addUpdatable(turntable);

			Turntable* turntableCamera = new Turntable( cameraParentNode, &m_inputManager);
			turntableCamera->setSensitivity(0.05f);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntableCamera), GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
			m_inputManager.attachListenerOnMouseButtonPress(new Turntable::ToggleTurntableDragListener(turntableCamera), GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
			scene->addUpdatable( turntableCamera );

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
