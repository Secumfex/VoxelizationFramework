#include <Misc/SimpleSceneTools.h>
#include <Misc/MiscListeners.h>

#include <Utility/DebugLog.h>

#include <Misc/RotatingNode.h>

#include <glm/gtc/matrix_transform.hpp>

RenderableNode* SimpleScene::loadObject( std::string object, Application* app )
{
	DEBUGLOG->log("Loading file " + object);
	DEBUGLOG->indent();
		std::vector< Object* > loadedObject= app->getResourceManager().loadObjectsFromFile( RESOURCES_PATH + object );
	DEBUGLOG->outdent();

	app->getSceneManager().getActiveScene()->addObjects( loadedObject );

	DEBUGLOG->log("Creating renderable node for " + object);
	RenderableNode* objectNode = new RenderableNode( );

	objectNode->setObject(loadedObject[0]);

	return objectNode;

}

RenderableNode* SimpleScene::loadTestRoomObject( Application* app)
{
	DEBUGLOG->log("Loading test room dae file");
	DEBUGLOG->indent();
		std::vector< Object* > testRoom=  app->getResourceManager().loadObjectsFromFile( RESOURCES_PATH "/testRoom.dae" );
	DEBUGLOG->outdent();

	app->getSceneManager().getActiveScene()->addObjects( testRoom );

	DEBUGLOG->log("Creating renderable node for test room");
	RenderableNode* testRoomNode = new RenderableNode( );
	testRoomNode->scale( glm::vec3(0.75f, 0.75f, 0.75f) );
	testRoomNode->setObject(testRoom[0]);

	return testRoomNode;
}

RenderableNode* SimpleScene::loadOverlappingGeometry( Application* app )
{
	DEBUGLOG->log("Loading overlapping geometry file");
	DEBUGLOG->indent();
		std::vector< Object* > overlappingGeometry =  app->getResourceManager().loadObjectsFromFile( RESOURCES_PATH "/overlappingGeometry.dae" );
	DEBUGLOG->outdent();

	DEBUGLOG->log("Adding overlapping geometry object to scene");
	DEBUGLOG->indent();
		app->getSceneManager().getActiveScene()->addObjects( overlappingGeometry );
	DEBUGLOG->outdent();

	DEBUGLOG->log("Creating renderable node for overlapping geometry");
	DEBUGLOG->indent();
		RenderableNode* overlappingGeometryNode = new RenderableNode( );
		overlappingGeometryNode->setObject(overlappingGeometry[0]);
	DEBUGLOG->outdent();

	return overlappingGeometryNode;
}

// returns a pair < root, rotatingnode >
std::pair<Node*, Node* > SimpleScene::createRotatingNodes(Application* app)
{
	std::pair<Node*, Node*> result;

	DEBUGLOG->log("Creating node tree for rotating node");
	Node* positionNode = new Node( );
	positionNode->translate( glm::vec3(0.0f, 0.0f, 0.0f) );
	result.first = positionNode;

	RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
	yAxisRotationNode->setAngle(0.05f);
	yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );

	RotatingNode* rotatingNode = new RotatingNode(yAxisRotationNode);
	rotatingNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
	rotatingNode->setAngle(0.2f);
	result.second = rotatingNode;

	DEBUGLOG->log("Adding updatable rotation nodes to scene");
	Scene* scene = app->getSceneManager().getActiveScene();
	scene->addUpdatable(yAxisRotationNode);
	scene->addUpdatable(rotatingNode);

	return result;
}

Scene* SimpleScene::createNewScene(Application* app) {

	DEBUGLOG->log("Creating a scene instance");
	DEBUGLOG->indent();
	Scene* scene = new Scene();
	DEBUGLOG->outdent();

	DEBUGLOG->log("Setting scene instance as active scene ");
	app->getSceneManager().setActiveScene(scene);

	return scene;
}

void SimpleScene::configureSimpleCameraMovement(Camera* movableCam,
		Application* app) {
	{
		Scene* scene = app->getSceneManager().getActiveScene();

		InputManager& inputManager = app->getInputManager();
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 2.0f),GLFW_KEY_W, GLFW_PRESS);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_W, GLFW_RELEASE);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 2.0f),GLFW_KEY_D, GLFW_PRESS);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_D, GLFW_RELEASE);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, -2.0f),GLFW_KEY_S, GLFW_PRESS);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::FORWARD, 0.0f),GLFW_KEY_S, GLFW_RELEASE);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, -2.0f),GLFW_KEY_A, GLFW_PRESS);
		inputManager.attachListenerOnKeyPress(new SetCameraSpeedListener(movableCam, SetCameraSpeedListener::RIGHT, 0.0f),GLFW_KEY_A, GLFW_RELEASE);

		DEBUGLOG->log("Adding updatable camera to scene");
		scene->addUpdatable(movableCam);
	}

}
