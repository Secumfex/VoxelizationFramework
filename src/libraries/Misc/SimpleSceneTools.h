#ifndef SIMPLESCENETOOLS_H
#define SIMPLESCENETOOLS_H

#include <Application/Application.h>
#include <Scene/Camera.h>
#include <Scene/RenderableNode.h>
#include <Misc/Turntable.h>

namespace SimpleScene
{
	Scene* createNewScene( Application* app);

	// loads an object, adds it to the scene and returns a node which can be added to the scenegraph
	RenderableNode* loadObject( std::string object, Application* app);

	RenderableNode* loadTestRoomObject( Application* app );

	RenderableNode* loadOverlappingGeometry( Application* app );

	std::pair<Node*, Node*> createRotatingNodes( Application* app , float angularVelocityXY = 0.2f, float angularVelocityY = 0.05f);

	void configureSimpleCameraMovement(Camera* movableCam, Application* app, float speed = 2.0f);

	Turntable* configureTurnTable( Node* node, Application* app, float sensitivity = 0.1f, int key = GLFW_MOUSE_BUTTON_LEFT, Camera* cam = 0);

	// Save the current state of the scene graph and restore it when called
	class SceneGraphState : public Listener
	{
	protected:
		SceneGraph* p_sceneGraph;
		std::map<Node*, glm::mat4 > m_modelMatrices;	// maps a Node to a model matrix

		void traverseAndAdd( Node* parent );	// adds all child node model matrices
		void traverseAndRestore( Node* parent); // restores all child node model matrices and removes
	public:
		SceneGraphState( SceneGraph* sceneGraph);	// save the provided scenegraph in its current state
		void restoreSceneGraph(SceneGraph* sceneGraph); // restores the scenegraph

		// restores the saved state upon call
		void call();
	};




}

#endif
