#ifndef SIMPLESCENETOOLS_H
#define SIMPLESCENETOOLS_H

#include <Application/Application.h>
#include <Scene/Camera.h>
#include <Scene/RenderableNode.h>

namespace SimpleScene
{
	RenderableNode* loadTestRoomObject( Application* app );

	RenderableNode* loadOverlappingGeometry( Application* app );

	std::pair<Node*, Node*> createRotatingNodes( Application* app );

	void configureSimpleCameraMovement(Camera* movableCam, Application* app);
}

#endif
