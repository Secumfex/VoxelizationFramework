#ifndef SCENE_H
#define SCENE_H

#include "Scene/SceneGraph.h"
#include "Resources/Object.h"

#include <vector>

class Scene 
{
private:
	SceneGraph m_sceneGraph;
	std::vector<Object* > m_Objects;
public:
	Scene();
	~Scene();

	std::vector<Object* > getObjects();
};

#endif