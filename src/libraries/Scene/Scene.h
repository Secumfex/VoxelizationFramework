#ifndef SCENE_H
#define SCENE_H

#include "Scene/SceneGraph.h"
#include "Resources/Object.h"
#include "Utility/Updatable.h"

#include <vector>

class Scene : public Updatable
{
private:
	SceneGraph m_sceneGraph;
	std::vector<Object* > m_objects;
	std::vector<Updatable* > m_updatables;
public:
	Scene();
	~Scene();

	SceneGraph* getSceneGraph();

	void addObject(Object* object);
	void addObjects(std::vector< Object* > objects);
	std::vector<Object* > getObjects();

	void addUpdatable( Updatable* updatable );
	void update(float d_t = 0.1f);
};

#endif
