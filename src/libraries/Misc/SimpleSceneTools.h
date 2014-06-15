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

	Turntable* configureTurnTable( Node* node, Application* app, float sensitivity = 0.1f);
}

#endif
