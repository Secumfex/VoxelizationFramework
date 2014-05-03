#include <Scene/Scene.h>

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::addObject(Object* object)
{
	m_Objects.push_back(object);
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
	return m_Objects;
}