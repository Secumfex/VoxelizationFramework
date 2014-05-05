#include <Scene/Scene.h>

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::addObject(Object* object)
{
	m_objects.push_back(object);
}

void Scene::addObjects(std::vector < Object* > objects)
{
	for (unsigned int i = 0; i < objects.size(); i++)
	{
		addObject( objects[i] );
	}
}

std::vector<Object* > Scene::getObjects()
{
	return m_objects;
}

SceneGraph* Scene::getSceneGraph()
{
	return &m_sceneGraph;
}

void Scene::update(float d_t)
{
	for (unsigned int i = 0; i < m_updatables.size(); i++)
	{
		m_updatables[i]->update(d_t);
	}
}

void Scene::addUpdatable( Updatable* updatable)
{
	m_updatables.push_back(updatable);
}
